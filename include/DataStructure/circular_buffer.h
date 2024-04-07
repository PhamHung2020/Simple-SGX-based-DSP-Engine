#ifndef CIRCULAR_BUFFER_
#define CIRCULAR_BUFFER_

struct circular_buffer
{
    void* const buffer;
    volatile int head;
    volatile int tail;
    const int maxlen;
    const int data_size;
    void (*pushCallback)(void*);
    void (*popCallback)(void*);
};

int circular_buffer_push(struct circular_buffer* cir_buf, void* data);
int circular_buffer_pop(struct circular_buffer* cir_buf, void** data);


#endif