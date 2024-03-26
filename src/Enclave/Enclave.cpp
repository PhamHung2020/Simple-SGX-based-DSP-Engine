#include <unistd.h>

#include "Enclave_t.h"  /* print_string */

// #include "fast_call.h"
#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"

void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

	void (*callbacks[4])(void*);
    callbacks[0] = TaskExecutor;
    callbacks[1] = TaskExecutor2;
    callbacks[2] = TaskExecutor3;
    callbacks[3] = MapCsvRowToEvent;

    FastCallTable callTable;
    callTable.numEntries = 4;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastECallData, &callTable, callId);
}
