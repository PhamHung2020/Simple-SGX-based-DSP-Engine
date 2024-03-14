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
std::queue<MyEvent> queue1;
std::queue<MyEvent> queue2;

/* 
 * print: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void print(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

int nestedLoopJoin(MyEvent* eventArr1, MyEvent* eventArr2, int n1, int n2, JoinResult* &results)
{
    int nItems = 0;
    for (int i = 0; i < n1; ++i)
    {
        for (int j = 0; j < n2; ++j)
        {
            if (eventArr1[i].data == eventArr2[j].data)
            {
                results[nItems].event1 = eventArr1[i];
                results[nItems].event2 = eventArr2[j];
                nItems++;
            }
        }
    }

    return nItems;
}

void MyCustomEcall( void* data )
{
    MyEvent* event = (MyEvent*) data;
    queue1.push(*event);
}

void Filter()
{
    while (true)
    {
        if (queue1.empty()) 
        {
            __asm __volatile(
                "pause"
            );
            continue;
        }
        MyEvent event = queue1.front();
        if (event.sourceId == 0)
        {
            queue2.push(event);
            break;
        }

        if (event.sourceId == 1 && event.data > 5)
        {
            queue2.push(event);
            // test(&event);
        }
        else if (event.sourceId == 2 && event.data > 0)
        {
            queue2.push(event);
            // test(&event);
        }

        queue1.pop();
    }
}

void NestedJoin()
{
    const int MAX_ITEM = 10;
    int n1 = 0, n2 = 0;
    MyEvent eventArr1[MAX_ITEM], eventArr2[MAX_ITEM];
    JoinResult* results = new JoinResult[MAX_ITEM * MAX_ITEM];

    while (true)
    {
        if (queue2.empty())
        {
            __asm __volatile(
                "pause"
            );
            continue;
        }

        MyEvent event = queue2.front();

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

        queue2.pop();

        if ((n1 == MAX_ITEM && n2 > 0) || (n2 == MAX_ITEM && n1 > 0))
        {
            // Call nestedLoopJoin
            int joinedItems = nestedLoopJoin(eventArr1, eventArr2, n1, n2, results);
            for (int i = 0; i < joinedItems; ++i)
            {
                test(&results[i]);
            }

            n1 = 0;
            n2 = 0;
        }
    }

    if (n1 > 0 && n2 > 0)
    {
        // Call nestedLoopJoin
        int joinedItems = nestedLoopJoin(eventArr1, eventArr2, n1, n2, results);
        for (int i = 0; i < joinedItems; ++i)
        {
            test(&results[i]);
        }

        delete[] results;
    }
}

void EcallStartResponder( HotCall* hotEcall )
{
	void (*callbacks[1])(void*);
    callbacks[0] = MyCustomEcall;
	// callbacks[1] = EFilter;

    HotCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    HotCall_waitForCall( hotEcall, &callTable );
}
