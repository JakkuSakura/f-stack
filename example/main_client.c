
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

// struct hostent and gethostbyname()
#include <netdb.h>

// sleep function
#include <unistd.h>
#include <pthread.h>

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"

#define PKT_SIZE 64
#define MAX_EVENTS 512
#define TEST_TOGGLE 0   // 0: latency, 1: throughput

#define MAXIMUM_RUN 10000
#define TIME_LIMIT 10000000

static long int limit_bytes = PKT_SIZE * MAXIMUM_RUN;
static long int curr_bytes;
static double latency;
struct timeval t1, t2;

struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
char hello[PKT_SIZE];
char buffer[PKT_SIZE];
int status = 0;
int succ = 0;

struct dispatch_t {
    int sockfd;
    int error_code;
    struct buffer_t read_buf;
    struct buffer_t write_buf;
};
int my_connect_addr(struct sockaddr_in serv_addr, struct dispatch_t **dispatch) {
    int sockfd = ff_socket(AF_INET, SOCK_STREAM, 0);

    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    *dispatch = malloc(sizeof(struct dispatch_t));
    create_dispatch(*dispatch, sockfd, 8192);

    struct epoll_event ev;
    ev.data.ptr = *dispatch;
    ev.events = EPOLLOUT;
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

int loop(void *arg) {

    int nevents = ff_epoll_wait(epfd, events, MAX_EVENTS, 0);
    struct epoll_event event;
    for (int i = 0; i < nevents; ++i) {
        if (events[i].events & EPOLLOUT) {
            if (status++ == 0)
                printf("connection established, fd %d\n", events[i].data.fd);

            int n = strlen(hello);
            int nsend = ff_write(events[i].data.fd, hello, PKT_SIZE);
            if (nsend < 0 && errno != EAGAIN) {
                perror("send error");
                close(events[i].data.fd);
                exit(1);
            }
            // printf("message delivered!\n");
            ev.data.fd = sockfd;
            ev.events = EPOLLIN;
            assert(ff_epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev) == 0);
            // printf("events modified!\n");
        }
        if (events[i].events & EPOLLIN) {
            printf("receiving data... fd %d\n", events[i].data.fd);
            printf("read success %d times\n", succ);

            struct epoll_event ev;
            ev.data.fd = sockfd;
            ev.events = EPOLLOUT;

            ff_epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);

            int nrecv = ff_read(events[i].data.fd, buffer, PKT_SIZE);
            if (nrecv == -1 && errno != EAGAIN)
                perror("read error");
            if ((nrecv == -1 && errno == EAGAIN) || nrecv == 0)
                break;
            if (nrecv > 0) succ++;

            curr_bytes += strlen(buffer);
            printf("string length: %ld\n", strlen(buffer));
        }
    }
}


int main(int argc, char *argv[]) {

    ff_init(argc, argv);
    memset(hello, '*', PKT_SIZE * sizeof(char));

    my_connect_addr(get_address("192.168.60.132", 3000));
    ff_run(loop, NULL);
    return 0;
}