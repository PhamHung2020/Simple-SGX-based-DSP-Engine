#ifndef FAST_CALL_
#define FAST_CALL_

#include <stdbool.h>
#include <sgx_spinlock.h>
#include "Enclave/cryptography.h"
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
    uint8_t no;
    sgx_enclave_id_t enclaveId;
    FastCallStruct* fastECall;
    FastCallStruct* fastOCall;
    uint16_t callId;
    HotCall* hotCall;
} FastCallPair;

struct FastCallPair2ECall {
    uint8_t no;
    sgx_enclave_id_t enclaveId;
    FastCallStruct* fastECall1;
    FastCallStruct* fastECall2;
    FastCallStruct* fastOCall;
    uint16_t callId;
};

typedef struct 
{
    uint16_t numEntries;
    void (**callbacks)(void*);
} FastCallTable;

typedef struct {
    void* data1;
    void* data2;
} FastCallDataGroup;

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
    const uint32_t MAX_RETRIES = 10000;
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

        for (i = 0; i<3; ++i)
            _mm_pause();
    }

    return numRetries;
}

static inline int FastCall_request_encrypt(FastCallStruct* fastCallData, void *data)
{
    int i = 0;
    const uint32_t MAX_RETRIES = 10000;
    uint32_t numRetries = 0;

    const int encryptedDataLength = fastCallData->data_buffer->data_size - 4;
    const int originalDataLength = encryptedDataLength - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE;
    char* encryptedData = (char*) malloc((encryptedDataLength + 1) * sizeof(char));
//    char encryptedData[encryptedDataLength+1];
    // Request call
    while(true)
    {
        // sgx_spin_lock(&fastCallData->spinlock);

//        aesGcmEncrypt((char*)data, originalDataLength, encryptedData, encryptedDataLength);
        encryptedData[encryptedDataLength] = '\0';
        circular_buffer_push(fastCallData->data_buffer, encryptedData);
        break;
//        if (circular_buffer_push(fastCallData->data_buffer, encryptedData) == 0)
//        {
//            // sgx_spin_unlock(&fastCallData->spinlock);
//            break;
//        }
//        // sgx_spin_unlock(&fastCallData->spinlock);
//
//        numRetries++;m
//        if(numRetries > MAX_RETRIES)
//            return -1;
//
//        for (i = 0; i<3; ++i)
//            _mm_pause();
    }

//    free(encryptedData);
    return numRetries;
}

static inline int FastCall_request_encrypt2(FastCallStruct* fastCallData, void *data, char* encryptedData)
{
    int i = 0;
    const uint32_t MAX_RETRIES = 10000;
    uint32_t numRetries = 0;

    const int encryptedDataLength = fastCallData->data_buffer->data_size - 4;
    const int originalDataLength = encryptedDataLength - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE;
    // Request call
    while(true)
    {
        // sgx_spin_lock(&fastCallData->spinlock);
        aesGcmEncrypt((char*)data, originalDataLength, encryptedData, encryptedDataLength);
        encryptedData[encryptedDataLength] = '\0';
        if (circular_buffer_push(fastCallData->data_buffer, encryptedData) == 0)
        {
            // sgx_spin_unlock(&fastCallData->spinlock);
            break;
        }
        // sgx_spin_unlock(&fastCallData->spinlock);

        numRetries++;
        if(numRetries > MAX_RETRIES)
            return -1;

        for (i = 0; i<3; ++i)
            _mm_pause();
    }

//    free(encryptedData);
    return numRetries;
}

static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    // char* data = new char[fastCallData->data_buffer->data_size];
    while(true)
    {
        // if (!fastCallData->keepPolling) {
        //     break;
        // }
        char* data;
        // sgx_spin_lock((&fastCallData->spinlock));
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            // sgx_spin_unlock((&fastCallData->spinlock));
            if (callId < callTable->numEntries)
            {
                callTable->callbacks[callId](data);
            }

            int next = fastCallData->data_buffer->tail + 1;
            if (next >= fastCallData->data_buffer->maxlen)
            {
                next = 0;
            }
            fastCallData->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData->data_buffer->lock_count);
            fastCallData->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData->data_buffer->lock_count);

            continue;
        }

        if (!fastCallData->keepPolling) {
            break;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }

    if (callId < callTable->numEntries)
    {
        callTable->callbacks[callId](NULL);
    }

    // delete[] data;
}

static inline void FastCall_wait_decrypt(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait_decrypt(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    char* data;
    const int dataSize = fastCallData->data_buffer->data_size - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE - 4;
    char* decryptedData =  (char *) malloc((dataSize+1) * sizeof(char ));
    while(true)
    {
        // if (!fastCallData->keepPolling) {
        //     break;
        // }

        // sgx_spin_lock((&fastCallData->spinlock));
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            // sgx_spin_unlock((&fastCallData->spinlock));
            if (callId < callTable->numEntries)
            {
                aesGcmDecrypt(
                        data,
                        dataSize,
                        decryptedData,
                        dataSize);
                decryptedData[dataSize] = '\0';
                callTable->callbacks[callId](decryptedData);
            }

            int next = fastCallData->data_buffer->tail + 1;
            if (next >= fastCallData->data_buffer->maxlen)
            {
                next = 0;
            }
            fastCallData->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData->data_buffer->lock_count);
            fastCallData->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData->data_buffer->lock_count);

            continue;
        }

        if (!fastCallData->keepPolling) {
            break;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }

    if (callId < callTable->numEntries)
    {
        callTable->callbacks[callId](NULL);
    }

    // delete[] data;
}


static inline void FastCall_wait_2(FastCallStruct *fastCallData1, FastCallStruct *fastCallData2, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait_2(FastCallStruct *fastCallData1, FastCallStruct *fastCallData2, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    char* data1 = NULL;
    char* data2 = NULL;
    bool keepPolling = false;
    int popResult1, popResult2;
    // char* data = new char[fastCallData->data_buffer->data_size];
    while(true)
    {
        popResult1 = circular_buffer_pop(fastCallData1->data_buffer, (void**)&data1);
        popResult2 = circular_buffer_pop(fastCallData2->data_buffer, (void**)&data2);

        if (fastCallData1->keepPolling) {
            keepPolling = true;
        }

        if (fastCallData2->keepPolling) {
            keepPolling = true;
        }

        if (data1 != NULL || data2 != NULL) {
            FastCallDataGroup dataGroup = {
                    data1,
                    data2
            };
            callTable->callbacks[callId](&dataGroup);
        }

        if (popResult1 == 0) {
            int next = fastCallData1->data_buffer->tail + 1;
            if (next >= fastCallData1->data_buffer->maxlen) {
                next = 0;
            }
            fastCallData1->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData1->data_buffer->lock_count);
            fastCallData1->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData1->data_buffer->lock_count);
        }

        if (popResult2 == 0) {
            int next = fastCallData2->data_buffer->tail + 1;
            if (next >= fastCallData2->data_buffer->maxlen) {
                next = 0;
            }
            fastCallData2->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData2->data_buffer->lock_count);
            fastCallData2->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData2->data_buffer->lock_count);
        }

        if (!keepPolling && data1 == NULL && data2 == NULL) {
            break;
        }
        keepPolling = false;
        data1 = NULL;
        data2 = NULL;
    }

    if (callId < callTable->numEntries) {
        callTable->callbacks[callId](NULL);
    }

    // delete[] data;
}

static inline void FastCall_wait_2_decrypt(FastCallStruct *fastCallData1, FastCallStruct *fastCallData2, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait_2_decrypt(FastCallStruct *fastCallData1, FastCallStruct *fastCallData2, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    char* data1 = NULL;
    const int dataSize1 = fastCallData1->data_buffer->data_size - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE - 4;
    char* decryptedData1 =  (char *) malloc((dataSize1+1) * sizeof(char ));

    char* data2 = NULL;
    const int dataSize2 = fastCallData2->data_buffer->data_size - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE - 4;
    char* decryptedData2 =  (char *) malloc((dataSize2+1) * sizeof(char ));

    bool keepPolling = false;
    int popResult1, popResult2;
    // char* data = new char[fastCallData->data_buffer->data_size];
    while(true)
    {
        popResult1 = circular_buffer_pop(fastCallData1->data_buffer, (void**)&data1);
        if (popResult1 == 0) {
            aesGcmDecrypt(
                    data1,
                    dataSize1,
                    decryptedData1,
                    dataSize1);
            decryptedData1[dataSize1] = '\0';
        }

        popResult2 = circular_buffer_pop(fastCallData2->data_buffer, (void**)&data2);
        if (popResult2 == 0) {
            aesGcmDecrypt(
                    data2,
                    dataSize2,
                    decryptedData2,
                    dataSize2);
            decryptedData2[dataSize2] = '\0';
        }

        if (fastCallData1->keepPolling) {
            keepPolling = true;
        }

        if (fastCallData2->keepPolling) {
            keepPolling = true;
        }

        if (data1 != NULL || data2 != NULL) {
            FastCallDataGroup dataGroup = {
                    data1 != NULL ? decryptedData1 : data1,
                    data2 != NULL ? decryptedData2 : data2
            };
            callTable->callbacks[callId](&dataGroup);
        }

        if (popResult1 == 0) {
            int next = fastCallData1->data_buffer->tail + 1;
            if (next >= fastCallData1->data_buffer->maxlen) {
                next = 0;
            }
            fastCallData1->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData1->data_buffer->lock_count);
            fastCallData1->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData1->data_buffer->lock_count);
        }

        if (popResult2 == 0) {
            int next = fastCallData2->data_buffer->tail + 1;
            if (next >= fastCallData2->data_buffer->maxlen) {
                next = 0;
            }
            fastCallData2->data_buffer->tail = next;
            sgx_spin_lock(&fastCallData2->data_buffer->lock_count);
            fastCallData2->data_buffer->popped_count += 1;
            sgx_spin_unlock(&fastCallData2->data_buffer->lock_count);
        }

        if (!keepPolling && data1 == NULL && data2 == NULL) {
            break;
        }
        keepPolling = false;
        data1 = NULL;
        data2 = NULL;
    }

    if (callId < callTable->numEntries) {
        callTable->callbacks[callId](NULL);
    }

    // delete[] data;
}

static inline void FastCall_wait_hotcall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId, HotCall* hotCall)  __attribute__((always_inline));
static inline void FastCall_wait_hotcall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId, HotCall* hotCall)
{
    static int i = 0;
    while(true)
    {
        // if (!fastCallData->keepPolling) {
        //     break;
        // }

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

        if (!fastCallData->keepPolling) {
            break;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }

    if (callId < callTable->numEntries)
    {
        callTable->callbacks[callId](NULL);
    }
}

static inline void StopFastCallResponder(FastCallStruct *fastCallData);
static inline void StopFastCallResponder(FastCallStruct *fastCallData)
{
    fastCallData->keepPolling = false;
}



#endif