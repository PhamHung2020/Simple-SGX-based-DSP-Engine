#include <unistd.h>

#include "Enclave_t.h"

#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"

void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId, HotCall* hotCall)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    // FastCall_wait(fastECallData, &callTable, callId);
    FastCall_wait_hotcall(fastECallData, &callTable, callId, hotCall);
}
