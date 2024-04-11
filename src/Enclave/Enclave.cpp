#include <unistd.h>

#include "Enclave_t.h"

#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"


void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastECallData, &callTable, callId);
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

void EcallStartResponderWithHotCall(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId, HotCall* hotCall)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_hotcall(fastECallData, &callTable, callId, hotCall);
}
