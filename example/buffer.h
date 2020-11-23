#ifndef F_STACK_BUFFER_H
#define F_STACK_BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct buffer_t {
    int size;
    int read;
    int write;
    int mask;
    char *buf;
};

void create_buffer(struct buffer_t *buf, int size);

int buffer_readable_len(struct buffer_t *buf);

int buffer_writable_len(struct buffer_t *buf);

void buffer_destroy(struct buffer_t *buf);

void buffer_allocate_writable(struct buffer_t *buf, char **ptr, int *size);

void buffer_allocate_readable(struct buffer_t *buf, char **ptr, int *size);

#endif //F_STACK_BUFFER_H
