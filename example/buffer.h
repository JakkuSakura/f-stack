#ifndef F_STACK_BUFFER_H
#define F_STACK_BUFFER_H

#include <stdio.h>

struct buffer_t {
    int size;
    int read;
    int write;
    char *buf;
};

void create_buffer(struct buffer_t *buf, int size) {
    buf->size = size;
    buf->read = buf->write = 0;
    buf->buf = malloc(size);
}

/// Returns the readable length of the buffer. Can be negative when being recycled.
inline int buffer_readable_len(struct buffer_t *buf) {
    return buf->write - buf->read;
}

inline int buffer_writable_len(struct buffer_t *buf) {
    return buf->size - buf->write;
}

void buffer_destroy(struct buffer_t *buf) {
    free(buf->buf);
    buf->buf = NULL;
}

void buffer_allocate_writable(struct buffer_t *buf, char **ptr, int *size) {
    if (buffer_readable_len(buf) <= 0) {
        *ptr = NULL;
        *size = 0;
        return;
    }
    if (buffer_writable_len(buf) == 0) {
        buf->write = 0;
    }
    *ptr = buf->buf + buf->write;
    *size = buffer_writable_len(buf);
}

void buffer_allocate_readable(struct buffer_t *buf, char **ptr, int *size) {
    int len = buffer_readable_len(buf);
    if (len == 0) {
        *ptr = NULL;
        *size = 0;
        return;
    } else if (len < 0) {
        buf->read = 0;
    }
    int write = buf->write;
    *ptr = buf->buf + buf->read;
    *size = write - buf->read;
    printf("start: %d size: %d\n", buf->read, *size);
}

#endif //F_STACK_BUFFER_H
