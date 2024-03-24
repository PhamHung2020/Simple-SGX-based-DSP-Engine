#ifndef __FAST_CALL_
#define __FAST_CALL_

#include <stdbool.h>

#include "DataStructure/circular_buffer.h"
#include "data_types.h"
#include "inttypes.h"

#pragma GCC diagnostic ignored "-Wunused-function"

typedef unsigned long int pthread_t;

typedef struct {
    pthread_t               responderThread;
    struct circular_buffer* data_buffer;
    bool                    keepPolling;
} FastCallStruct;

typedef struct 
{
    uint16_t numEntries;
    void (**callbacks)(void*);
} FastCallTable;

static inline void _mm_pause(void) __attribute__((always_inline));
static inline void _mm_pause(void)
{
    __asm __volatile(
        "pause"
    );
}

static inline int FastCall_request(FastCallStruct* fastCallData, MyEvent *data)
{
    int i = 0;
    const uint32_t MAX_RETRIES = 100;
    uint32_t numRetries = 0;
    // Request call
    while(true) {

        if (circular_buffer_push(fastCallData->data_buffer, data) == 0)
        {
            break;
        }

        numRetries++;
        if(numRetries > MAX_RETRIES)
            return -1;

        for (i = 0; i<3; ++i)
            _mm_pause();
    }

    return numRetries;
}

static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId) 
{
    static int i;
    while(true)
    {
        if (fastCallData->keepPolling != true) {
            break;
        }

        // MyEvent* data;
        char* data;
        // int result = circular_buffer_pop(fastCallData->data_buffer, data);
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            if (callId < callTable->numEntries)
            {
                callTable->callbacks[callId](data);
            }
            continue;
        }

        for( i = 0; i<3; ++i)
            _mm_pause();
    }
}

static inline void StopResponder(FastCallStruct *fastCallData);
static inline void StopResponder(FastCallStruct *fastCallData)
{
    fastCallData->keepPolling = false;
}

#endif