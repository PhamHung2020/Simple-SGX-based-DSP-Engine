#include <unistd.h>
#include "Enclave_t.h"

#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"

char decryptedData[1000];
char decryptedData1[1000];
char decryptedData2[1000];

const size_t size = 64;
char from[size];
char to[64];

void testMemCpy() {
    memcpy(to, from, 64);
}

void testMemCpy2(char* outMem) {
//    for (int i = 0; i < size; ++i) {
//        from[i] = (char )i;
//    }
    memcpy(outMem, from, size);
}

int ecallSum(int a, int b) {
    return a + b;
}

void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastECallData, &callTable, callId);
}

void EcallStartResponderWithDecryption(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_decrypt(fastECallData, &callTable, callId, decryptedData);
}

void EcallStartResponder2(FastCallStruct* fastECallData1, FastCallStruct* fastECallData2, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

//    FastCall_wait(fastECallData, &callTable, callId);
    FastCall_wait_2(fastECallData1, fastECallData2, &callTable, callId);
}

void EcallStartResponder2WithDecryption(FastCallStruct* fastECallData1, FastCallStruct* fastECallData2, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_2_decrypt(fastECallData1, fastECallData2, &callTable, callId, decryptedData1, decryptedData2);
}

void EcallStartResponderWithHotCall(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId, HotCall* hotCall)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_hotcall(fastECallData, &callTable, callId, hotCall);
}
