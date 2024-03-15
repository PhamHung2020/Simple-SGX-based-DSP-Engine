#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <stdbool.h>
#include <unistd.h>
#include <queue>

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

#include "../include/common.h"
#include "../include/data_types.h"

// int secret = 0;
std::queue<MyEvent> sourceQueue;
std::queue<MyEvent> joinQueue;
std::queue<MyEvent> reduceQueue;

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

MyEvent* map(MyEvent* event, MyEvent* (*mapRule)(MyEvent*))
{
    if (event == NULL || mapRule == NULL) return NULL;
    return mapRule(event);
}

int reduce(MyEvent event, int accumulator, int (*reduceFunc)(MyEvent, int))
{
    if (reduceFunc == NULL) return accumulator;
    return reduceFunc(event, accumulator);
}

void MyCustomEcall( void* data )
{
    MyEvent* event = (MyEvent*) data;
    sourceQueue.push(*event);
}

void TaskExecutor1()
{
    while (true)
    {
        if (sourceQueue.empty()) 
        {
            __asm __volatile(
                "pause"
            );
            continue;
        }
        MyEvent event = sourceQueue.front();

        // sourceId = 0 => event sent by engine to terminate this executor
        if (event.sourceId == 0)
        {
            joinQueue.push(event);
            reduceQueue.push(event);
            break;
        }

        // transformation pipeline 1: filter --> map --> join
        if (event.sourceId == 1)
        {
            MyEvent* newEvent = map
            (
                filter
                (
                    &event, 
                    [](MyEvent e) { return e.data > 5; }
                ),

                [](MyEvent* e) { e->data *= 2; return e; }
            );

            if (newEvent)
            {
                joinQueue.push(*newEvent);
            }
        }
        // transformation pipeline 2: filter --> join
        //                              |------> map --> reduce
        else if (event.sourceId == 2)
        {
            if (
                filter(&event, [](MyEvent e) { return e.data > 0; })
            )
            {
                joinQueue.push(event);
                map(&event, [](MyEvent* e) { e->data += 1; return e; });
                reduceQueue.push(event);
            }

        }

        sourceQueue.pop();
    }
}

void TaskExecutor2(HotCall* hotOcall)
{
    const int MAX_ITEM = 5;
    int n1 = 0, n2 = 0;
    MyEvent eventArr1[MAX_ITEM + 5], eventArr2[MAX_ITEM + 5];
    JoinResult* results = new JoinResult[MAX_ITEM * MAX_ITEM + 5];
    HotOCallParams* hotOCallParams = (HotOCallParams*) hotOcall->data;

    auto joinPredicate = [](MyEvent e1, MyEvent e2) { return e1.data == e2.data; };

    while (true)
    {
        if (joinQueue.empty())
        {
            __asm __volatile(
                "pause"
            );
            continue;
        }

        MyEvent event = joinQueue.front();

        if (event.sourceId == 1)
        {
            eventArr1[n1] = event;
            ++n1;
        }
        else if (event.sourceId == 2)
        {
            eventArr2[n2] = event;
            ++n2;
        }
        else if (event.sourceId == 0)
        {
            break;
        }

        joinQueue.pop();

        if ((n1 == MAX_ITEM && n2 > 0) || (n2 == MAX_ITEM && n1 > 0))
        {
            // Call nestedLoopJoin
            int joinedItems = nestedLoopJoin(eventArr1, eventArr2, n1, n2, results, joinPredicate);
            for (int i = 0; i < joinedItems; ++i)
            {
                hotOCallParams->joinResult.event1 = results[i].event1;
                hotOCallParams->joinResult.event2 = results[i].event2;
                HotCall_requestCall(hotOcall, 0, hotOCallParams);
                // test(&results[i]);
            }

            n1 = 0;
            n2 = 0;
        }
    }

    if (n1 > 0 && n2 > 0)
    {
        // Call nestedLoopJoin
        int joinedItems = nestedLoopJoin(eventArr1, eventArr2, n1, n2, results, joinPredicate);
        for (int i = 0; i < joinedItems; ++i)
        {
            hotOCallParams->joinResult.event1 = results[i].event1;
            hotOCallParams->joinResult.event2 = results[i].event2;
            HotCall_requestCall(hotOcall, 0, hotOCallParams);
            // test(&results[i]);
        }
    }

    delete[] results;
}

void TaskExecutor3(HotCall* hotOcall)
{
    const int MAX_ITEM = 3;
    int nItem = 0;
    int accumulator = 0;
    HotOCallParams* hotOCallParams = (HotOCallParams*) hotOcall->data;

    while (true)
    {
        if (reduceQueue.empty())
        {
            __asm __volatile(
                "pause"
            );
            continue;
        }

        MyEvent event = reduceQueue.front();
        if (event.sourceId == 0)
        {
            break;
        }

        accumulator = reduce(event, accumulator, [](MyEvent e, int acc) { return e.data + acc; });
        nItem++;
        if (nItem >= MAX_ITEM)
        {
            // call outside
            hotOCallParams->reduceResult = accumulator;
            HotCall_requestCall(hotOcall, 1, hotOCallParams);
            nItem = 0;
            accumulator = 0;
        }

        reduceQueue.pop();
    }

    if (nItem > 0)
    {
        // call outside
        hotOCallParams->reduceResult = accumulator;
        HotCall_requestCall(hotOcall, 1, hotOCallParams);
    }
}

void EcallStartResponder(HotCall* hotEcall)
{
	void (*callbacks[1])(void*);
    callbacks[0] = MyCustomEcall;
	// callbacks[1] = EFilter;

    HotCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    HotCall_waitForCall( hotEcall, &callTable );
}
