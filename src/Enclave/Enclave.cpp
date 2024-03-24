#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <stdbool.h>
#include <unistd.h>

#include "Enclave_t.h"  /* print_string */

#include "fast_call.h"
#include "data_types.h"
#include "DataStructure/circular_buffer.h"

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;

void print(const char *fmt, ...)
{
    char buf[BUFSIZ] = {};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

int nestedLoopJoin
(
    const MyEvent* eventArr1,
    const MyEvent* eventArr2,
    const int n1, const int n2,
    JoinResult* &results, 
    bool (*predicate)(MyEvent, MyEvent)
)
{
    if (eventArr1 == nullptr || eventArr2 == nullptr || predicate == nullptr) return NULL;
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

MyEvent* filter(MyEvent* event, bool (*predicate)(const MyEvent&))
{
    if (event == nullptr || predicate == nullptr) return nullptr;
    if (predicate(*event))
    {
        return event;
    }

    return nullptr;
}

MyEvent* map(MyEvent* event, void (*mapRule)(MyEvent*))
{
    if (event == nullptr || mapRule == nullptr) return nullptr;
    mapRule(event);
    return event;
}

int reduce(const MyEvent &event, const int accumulator, int (*reduceFunc)(MyEvent, int))
{
    if (reduceFunc == nullptr) return accumulator;
    return reduceFunc(event, accumulator);
}

void TaskExecutor(void* data)
{
    auto* event = static_cast<MyEvent *>(data);
    event = filter(event, [](const MyEvent &e) { return e.data > 5; });
    if (event != nullptr)
        FastCall_request(globalFastOCall, event);
    // printEvent(*event);
}

void TaskExecutor2(void* data)
{
    auto* event = static_cast<MyEvent *>(data);
    event = map(event, [](MyEvent* e) { e->data *= 2; });
    if (event != nullptr)
        FastCall_request(globalFastOCall, event);
    // printEvent(*event);
}


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
