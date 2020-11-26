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

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"

#include "buffer.h"
#include "mutex_stack.h"

#include "fstack_daemon.h"

#define PKT_SIZE 64
#define MAX_EVENTS 512

char buffer[PKT_SIZE];

struct epoll_event events[MAX_EVENTS];
int epfd;

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
    bzero(&ev, sizeof(ev));
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

struct mutex_stack_t controls;

void control() {
    if (mutex_stack_size(&controls)) {
        struct control_t *control = mutex_stack_pop(&controls);
        printf("Get control %d\n", control->command);
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
}

int daemon_loop(void (*user_poll)()) {
    control();
    if (user_poll != NULL)
        user_poll();

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

static int initialized = 0;

void fstack_init(int argc, char *argv[]) {
    if (initialized == 0) {
        printf("args: ");
        for (int i = 0; i < argc; ++i) {
            printf("%s ", argv[i]);
        }
        printf("\n");

        ff_init(argc, argv);
        assert((epfd = ff_epoll_create(0)) > 0);
        mutex_stack_init(&controls, 1000, sizeof(struct control_t));
        initialized = 1;
    }
}

void fstack_run(void (*user_poll)()) {
    fstack_init(0, NULL);
    ff_run(daemon_loop, user_poll);
}