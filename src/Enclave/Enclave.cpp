#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <stdbool.h>
#include <unistd.h>
#include <queue>

#include "Enclave_t.h"  /* print_string */

// #include "hot_calls.h"
#include "fast_call.h"
#include "data_types.h"
#include "DataStructure/circular_buffer.h"

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;

void print(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

int nestedLoopJoin
(
    MyEvent* eventArr1, 
    MyEvent* eventArr2, 
    int n1, int n2, 
    JoinResult* &results, 
    bool (*predicate)(MyEvent, MyEvent)
)
{
    if (eventArr1 == NULL || eventArr2 == NULL || predicate == NULL) return NULL;
    int nItems = 0;
    for (int i = 0; i < n1; ++i)
    {
        for (int j = 0; j < n2; ++j)
        {
            if (predicate(eventArr1[i], eventArr2[j]))
            {
                results[nItems].event1 = eventArr1[i];
                results[nItems].event2 = eventArr2[j];
                nItems++;
            }
        }
    }

    return nItems;
}

MyEvent* filter(MyEvent* event, bool (*predicate)(MyEvent))
{
    if (event == NULL || predicate == NULL) return NULL;
    if (predicate(*event))
    {
        return event;
    }

    return NULL;
}

MyEvent* map(MyEvent* event, void (*mapRule)(MyEvent*))
{
    if (event == NULL || mapRule == NULL) return NULL;
    mapRule(event);
    return event;
}

int reduce(MyEvent event, int accumulator, int (*reduceFunc)(MyEvent, int))
{
    if (reduceFunc == NULL) return accumulator;
    return reduceFunc(event, accumulator);
}

void TaskExecutor(void* data)
{
    MyEvent* event = (MyEvent*) data;
    event = filter(event, [](MyEvent e) { return e.data > 5; });
    if (event != NULL)
        FastCall_request(globalFastOCall, event);
    // printEvent(*event);
}

void TaskExecutor2(void* data)
{
    MyEvent* event = (MyEvent*) data;
    event = map(event, [](MyEvent* event) { event->data *= 2; });
    if (event != NULL)
        FastCall_request(globalFastOCall, event);
    // printEvent(*event);
}


void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, uint16_t callId)
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
