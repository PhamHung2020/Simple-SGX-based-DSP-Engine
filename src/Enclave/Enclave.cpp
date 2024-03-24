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

	void (*callbacks[2])(void*);
    callbacks[0] = TaskExecutor;
    callbacks[1] = TaskExecutor2;

    FastCallTable callTable;
    callTable.numEntries = 2;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastECallData, &callTable, callId);
}
