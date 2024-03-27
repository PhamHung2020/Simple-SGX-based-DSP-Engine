#ifndef FAST_CALL_
#define FAST_CALL_

#include <stdbool.h>
#include <sgx_spinlock.h>
#include "DataStructure/circular_buffer.h"
#include "sgx_eid.h"     /* sgx_enclave_id_t */
#include "hot_calls.h"

#pragma GCC diagnostic ignored "-Wunused-function"

typedef unsigned long int pthread_t;

typedef struct {
    sgx_spinlock_t          spinlock;
    pthread_t               responderThread;
    struct circular_buffer* data_buffer;
    bool                    keepPolling;
} FastCallStruct;

typedef struct
{
    sgx_enclave_id_t enclaveId;
    FastCallStruct* fastECall;
    FastCallStruct* fastOCall;
    uint16_t callId;
    HotCall* hotCall;
} FastCallPair;

typedef struct 
{
    uint16_t numEntries;
    void (**callbacks)(void*);
} FastCallTable;

static inline void _fastcall_mm_pause(void) __attribute__((always_inline));
static inline void _fastcall_mm_pause(void)
{
    __asm __volatile(
        "pause"
    );
}

static inline int FastCall_request(FastCallStruct* fastCallData, void *data)
{
    int i = 0;
    const uint32_t MAX_RETRIES = 100;
    uint32_t numRetries = 0;
    // Request call
    while(true)
    {
        // sgx_spin_lock(&fastCallData->spinlock);

        if (circular_buffer_push(fastCallData->data_buffer, data) == 0)
        {
            // sgx_spin_unlock(&fastCallData->spinlock);
            break;
        }
        // sgx_spin_unlock(&fastCallData->spinlock);

        numRetries++;
        if(numRetries > MAX_RETRIES)
            return -1;

        // for (i = 0; i<3; ++i)
        //     _mm_pause();
    }

    return numRetries;
}

static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    while(true)
    {
        if (fastCallData->keepPolling != true) {
            break;
        }

        char* data;
        // sgx_spin_lock((&fastCallData->spinlock));
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            // sgx_spin_unlock((&fastCallData->spinlock));
            if (callId < callTable->numEntries)
            {
                callTable->callbacks[callId](data);
            }
            continue;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }
}

static inline void FastCall_wait_hotcall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId, HotCall* hotCall)  __attribute__((always_inline));
static inline void FastCall_wait_hotcall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId, HotCall* hotCall)
{
    static int i = 0;
    while(true)
    {
        if (fastCallData->keepPolling != true) {
            break;
        }

        char* data;
        // sgx_spin_lock((&fastCallData->spinlock));
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            // sgx_spin_unlock((&fastCallData->spinlock));
            if (callId < callTable->numEntries)
            {
                HotCall_requestCall(hotCall, 0, hotCall->data);
                callTable->callbacks[callId](data);
                HotCall_requestCall(hotCall, 1, hotCall->data);
            }
            continue;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }
}

static inline void StopFastCallResponder(FastCallStruct *fastCallData);
static inline void StopFastCallResponder(FastCallStruct *fastCallData)
{
    fastCallData->keepPolling = false;
}



#endif