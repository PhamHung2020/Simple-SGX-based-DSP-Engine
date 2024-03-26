#include <unistd.h>

#include "Enclave_t.h"  /* print_string */

// #include "fast_call.h"
#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"

// FastCallStruct* globalFastOCall;
// circular_buffer* fastOCallBuffer;

void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

	void (*callbacks[3])(void*);
    callbacks[0] = TaskExecutor;
    callbacks[1] = TaskExecutor2;
    callbacks[2] = TaskExecutor3;

    FastCallTable callTable;
    callTable.numEntries = 3;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastECallData, &callTable, callId);
}
