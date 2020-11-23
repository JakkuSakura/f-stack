//
// Created by jack on 11/23/20.
//

#ifndef F_STACK_FSTACK_DAEMON_H
#define F_STACK_FSTACK_DAEMON_H
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
#include "buffer.h"
#include "mutex_stack.h"

struct dispatch_t {
    int sockfd;
    int error_code;
    struct buffer_t read_buf;
    struct buffer_t write_buf;
};


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

struct sockaddr_in get_address(char *addr, int port);
void fstack_init(int argc, char *argv[]);
void fstack_run(void (*user_poll)());
extern struct mutex_stack_t controls;

#endif //F_STACK_FSTACK_DAEMON_H
