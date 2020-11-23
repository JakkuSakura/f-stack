
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"

#include "buffer.h"
#include "simple_structure.h"

#define PKT_SIZE 64
#define MAX_EVENTS 512

char buffer[PKT_SIZE];

struct epoll_event events[MAX_EVENTS];
int epfd;

struct dispatch_t {
    int sockfd;
    int error_code;
    struct buffer_t read_buf;
    struct buffer_t write_buf;
};

void create_dispatch(struct dispatch_t *dispatch, int socket_fd, int buffer_size) {
    dispatch->sockfd = socket_fd;
    dispatch->error_code = 0;
    create_buffer(&dispatch->read_buf, buffer_size);
    create_buffer(&dispatch->write_buf, buffer_size);
}

int my_connect_addr(struct sockaddr_in serv_addr, struct dispatch_t **dispatch) {
    int sockfd = ff_socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    *dispatch = malloc(sizeof(struct dispatch_t));
    create_dispatch(*dispatch, sockfd, 8192);

    struct epoll_event ev;
    ev.data.ptr = *dispatch;
    ev.events = EPOLLOUT | EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    int ret = ff_connect(sockfd, (struct linux_sockaddr *) &serv_addr, sizeof(serv_addr));

    if (ret == 0 || errno == 115) {
        errno = 0;
        return sockfd;
    } else
        return ret;
}

struct sockaddr_in get_address(char *addr, int port) {
    printf("Creating address from %s:%d\n", addr, port);
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(addr);
    serv_addr.sin_port = htons(port);
    return serv_addr;
}


enum commands {
    CONNECT,
    CLOSE,
    RECONNECT
};

struct control_t {
    struct sockaddr_in addr;
    int fd;
    enum commands command;
    int errno_number;
    int done;
    struct dispatch_t *dispatch;
};

struct mutex_stack_t controls;

int loop(void *arg) {
    if (mutex_stack_size(&controls)) {
        struct control_t *control = mutex_stack_pop(&controls);
        switch (control->command) {
            case CONNECT: {
                control->fd = my_connect_addr(control->addr, &control->dispatch);
                control->errno_number = errno;
                break;
            }
            case CLOSE:
                ff_close(control->fd);
                // FIXME memory leakage. dispatch_t in event.data.ptr needs freeing
//                ff_epoll_ctl(epfd, EPOLL_CTL_DEL, control->fd, NULL);
                control->errno_number = errno;
                control->fd = 0;
                break;
            case RECONNECT: {
                if (control->fd > 0)
                    ff_close(control->fd);
                control->fd = my_connect_addr(control->addr, &control->dispatch);
                control->errno_number = errno;
                break;
            }
        }
        control->done = 1;
    }

    int nevents = ff_epoll_wait(epfd, events, MAX_EVENTS, 0);
    struct epoll_event event;
    for (int i = 0; i < nevents; ++i) {
        struct dispatch_t *dispatch = (struct dispatch_t *) events[i].data.ptr;

        if (events[i].events & EPOLLOUT) {
            char *buf;
            int len;
            buffer_allocate_readable(&dispatch->write_buf, &buf, &len);
            if (len > 0) {
                int nsend = ff_write(dispatch->sockfd, buf, len);
                if (nsend > 0) {
                    dispatch->write_buf.read += nsend;
                } else if (nsend < 0) {
                    dispatch->error_code = errno;
                }
            }
        }

        if (events[i].events & EPOLLIN) {
            char *buf;
            int len;
            buffer_allocate_writable(&dispatch->read_buf, &buf, &len);

            int nrecv = ff_read(dispatch->sockfd, buf, len);
            if (nrecv == -1 && errno != EAGAIN) {
                perror("read error");
                dispatch->error_code = errno;
                continue;
            }
            if ((nrecv == -1 && errno == EAGAIN) || nrecv == 0)
                continue;
            dispatch->read_buf.write += nrecv;
        }

    }

}


void *worker(void *_) {
    sleep(1);
    printf("Creating connection\n");
    struct control_t *control = malloc(sizeof(struct control_t));
    bzero(control, sizeof(struct control_t));
    control->addr = get_address("192.168.60.132", 3000);

    mutex_stack_push(&controls, control);
    while (!control->done) {
        printf("Waiting\n");
        sleep(1);
    }
    printf("Connection started. fd: %d, errno %d\n", control->fd, errno);

    while (1) {
        char *begin;
        int len;
        buffer_allocate_readable(&control->dispatch->read_buf, &begin, &len);
        write(STDOUT_FILENO, begin, len);
        control->dispatch->read_buf.read += len;
        char *begin2;
        int len2;
        buffer_allocate_writable(&control->dispatch->write_buf, &begin2, &len2);
        memcpy(begin2, begin, len);
        control->dispatch->write_buf.write += len;

    }


    control->done = 0;
    control->command = CLOSE;
    mutex_stack_push(&controls, control);
    while (!control->done) {
        printf("Waiting\n");
        sleep(1);
    }

    printf("Close done. fd: %d, errno %d\n", control->fd, errno);

    exit(0);
}


int main(int argc, char *argv[]) {
    ff_init(argc, argv);
    assert((epfd = ff_epoll_create(0)) > 0);

    mutex_stack_init(&controls, 1000, sizeof(struct control_t));
    pthread_t run;
    pthread_create(&run, NULL, worker, NULL);
    ff_run(loop, NULL);

    return 0;
}