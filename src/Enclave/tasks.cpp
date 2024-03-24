//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "data_types.h"
#include "Enclave/enclave_operators.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;

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