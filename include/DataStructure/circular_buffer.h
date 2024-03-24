#ifndef __CIRCULAR_BUFFER_
#define __CIRCULAR_BUFFER_

#include "data_types.h"

struct circular_buffer
{
    void* const buffer;
    volatile int head;
    volatile int tail;
    const int maxlen;
    const int data_size;
};

int circular_buffer_push(struct circular_buffer* cir_buf, void* data);
int circular_buffer_pop(struct circular_buffer* cir_buf, void** data);


#endif