#include "buffer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int main() {
    struct buffer_t read_buf;
    create_buffer(&read_buf, 8);
    printf("R:%d W:%d S:%d\n", read_buf.read, read_buf.write, read_buf.size);
    char *begin;
    int size;
    buffer_allocate_writable(&read_buf, &begin, &size);
    printf("[%d, %d]\n", (int)(begin - read_buf.buf), size);
    read_buf.write += size;
    buffer_allocate_readable(&read_buf, &begin, &size);
    printf("[%d, %d]\n", (int)(begin - read_buf.buf), size);
    read_buf.read += size;

    printf("R:%d W:%d S:%d\n", read_buf.read, read_buf.write, read_buf.size);
    buffer_allocate_writable(&read_buf, &begin, &size);
    printf("[%d, %d]\n", (int)(begin - read_buf.buf), size);
}