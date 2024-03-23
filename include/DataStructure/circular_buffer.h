#ifndef __CIRCULAR_BUFFER_
#define __CIRCULAR_BUFFER_

#include "data_types.h"

struct circular_buffer
{
    MyEvent* const buffer;
    volatile int head;
    volatile int tail;
    const int maxlen;
};

int circular_buffer_push(struct circular_buffer* cir_buf, MyEvent data);
int circular_buffer_pop(struct circular_buffer* cir_buf, MyEvent** data);


#endif