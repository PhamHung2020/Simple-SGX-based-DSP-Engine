//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "data_types.h"
#include "Enclave/enclave_operators.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"

#include <cstring>
#include <string>
#include <chrono>
#include <ctime>

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;

void MapCsvRowToEvent(void* data)
{
    const auto row = static_cast<char *>(data);
    std::string rowStr = row;

    MyEvent event;
    try {
        int count = 0;
        std::size_t previousPos = 0;
        std::size_t pos = rowStr.find(',');
        while (pos != std::string::npos)
        {
            std::string word = rowStr.substr(previousPos, pos - previousPos);

            if (count == 0)
            {
                event.timestamp = std::stod(word);
            }
            else if (count == 1)
            {
                event.key = std::stoi(word);
            }
            else if (count == 2)
            {
                event.data = std::stoi(word);
            }
            count++;

            if (count > 2)
                break;
            previousPos = pos + 1;
            pos = rowStr.find(',', previousPos);
        }

        const std::string word = rowStr.substr(pos + 1);
        strncpy(event.message, word.c_str(), 32);
    }
    catch(const std::invalid_argument&)
    {
        return;
    }

    event.sourceId = 1;
    FastCall_request(globalFastOCall, &event);
}

void TaskExecutor(void* data)
{
    auto* event = static_cast<MyEvent *>(data);
    event = filter(event, [](const MyEvent &e) { return e.data > 0; });
    if (event != nullptr)
    {
        MyEvent newEvent = {
            .timestamp = event->timestamp,
            .sourceId = event->sourceId,
            .key = event->key,
            .data = event->data,
            .message = "Abc"
        };
        FastCall_request(globalFastOCall, &newEvent);
    }
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

int task3Result = 0;
int task3WindowLength = 4;
int task3Count = 0;
void TaskExecutor3(void *data) {
    const auto* event = static_cast<MyEvent *>(data);
    task3Result = reduce(*event, task3Result, [](MyEvent e, int acc) { return e.data + acc; });
    task3Count++;
    if (task3Count == task3WindowLength) {
        task3Count = 0;
        FastCall_request(globalFastOCall, &task3Result);
    }
}
