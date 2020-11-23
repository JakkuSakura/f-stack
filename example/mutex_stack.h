//
// Created by jack on 11/21/20.
//

#ifndef F_STACK_SIMPLE_STRUCTURE_H
#define F_STACK_SIMPLE_STRUCTURE_H

#include <semaphore.h>
#include <stdlib.h>
#include <memory.h>

struct mutex_stack_t {
    int size;
    int head;
    sem_t mutex;
    void **data;
};

void mutex_stack_init(struct mutex_stack_t *st, int size, int size_per_element);

void mutex_stack_push(struct mutex_stack_t *st, void *element);

void *mutex_stack_pop(struct mutex_stack_t *st);

int mutex_stack_size(struct mutex_stack_t *st);


#endif //F_STACK_SIMPLE_STRUCTURE_H
