#include "fstack_daemon.h"
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

void *worker(void *_) {
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
    fstack_init(argc, argv);
    pthread_t run;
    pthread_create(&run, NULL, worker, NULL);
    sleep(1);
    fstack_run(NULL);
    return 0;
}