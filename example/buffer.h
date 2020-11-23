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

void create_buffer(struct buffer_t *buf, int size) {
    assert((size & (size - 1)) == 0);
    buf->size = size;
    buf->read = buf->write = 0;
    buf->mask = size - 1;
    buf->buf = malloc(size);
}

/// Returns the readable length of the buffer. Can be negative when being recycled.
int buffer_readable_len(struct buffer_t *buf) {
    return buf->write - buf->read;
}

int buffer_writable_len(struct buffer_t *buf) {
    return buf->size - buffer_readable_len(buf);
}

void buffer_destroy(struct buffer_t *buf) {
    free(buf->buf);
    buf->buf = NULL;
}

void buffer_allocate_writable(struct buffer_t *buf, char **ptr, int *size) {
    if (buffer_writable_len(buf) == 0) {
        *ptr = buf->buf + (buf->write & buf->mask);
        *size = 0;
        return;
    }
    *ptr = buf->buf + (buf->write & buf->mask);
    *size = buffer_writable_len(buf);
}

void buffer_allocate_readable(struct buffer_t *buf, char **ptr, int *size) {
    int len = buffer_readable_len(buf);
    if (len == 0) {
        *ptr = buf->buf + (buf->read & buf->mask);
        *size = 0;
        return;
    }
    int write = buf->write;
    *ptr = buf->buf + (buf->read & buf->mask);
    *size = write - buf->read;
}

#endif //F_STACK_BUFFER_H
