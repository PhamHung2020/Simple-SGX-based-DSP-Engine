#include "DataStructure/circular_buffer.h"

#include <string.h>

int circular_buffer_push(circular_buffer* cir_buf, void* data)
{
    int next = cir_buf->head + 1;
    if (next >= cir_buf->maxlen)
    {
        next = 0;
    }

    if (next == cir_buf->tail)
    {
        return -1;
    }

    memcpy((char*) cir_buf->buffer + cir_buf->data_size * cir_buf->head, (char*) data, cir_buf->data_size);
    cir_buf->head = next;

    return 0;
}

int circular_buffer_pop(circular_buffer* cir_buf, void** data)
{
    if (cir_buf->head == cir_buf->tail)
    {
        return -1;
    }

    int next = cir_buf->tail + 1;
    if (next >= cir_buf->maxlen)
    {
        next = 0;
    }

    // *data = &cir_buf->buffer[cir_buf->tail];
    // memcpy(*(char**) data, (char*) cir_buf->buffer + cir_buf->data_size * cir_buf->tail, cir_buf->data_size);
    *(char**)data = (char*) cir_buf->buffer + cir_buf->data_size * cir_buf->tail;

    cir_buf->tail = next;
    return 0;
}