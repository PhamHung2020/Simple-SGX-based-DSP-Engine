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
#include <vector>

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


void MapCsvRowToFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto row = static_cast<char *>(data);
    std::string rowStr = row;
    FlightData flightData{};

    try {
        std::vector<std::string> words;
        std::size_t previousPos = 0;
        std::size_t pos = rowStr.find(',');
        while (pos != std::string::npos)
        {
            std::string word = rowStr.substr(previousPos, pos - previousPos);
            words.push_back(word);

            previousPos = pos + 1;
            pos = rowStr.find(',', previousPos);
        }

        const std::string word = rowStr.substr(pos + 1);
        words.push_back(word);

        flightData.arrDelay = static_cast<int>(words.size());
        if (!words[0].empty()) {
            strncpy(flightData.uniqueCarrier, words[8].c_str(), 10);
        } else {
            strncpy(flightData.uniqueCarrier, "UNKNOW", 10);
        }
        if (!words[14].empty() && words[14] != "NA") {
            flightData.arrDelay = std::stoi(words[14]);
        } else {
            flightData.arrDelay = 0;
        }
    }
    catch(const std::invalid_argument&)
    {
        return;
    }

    FastCall_request(globalFastOCall, &flightData);
}

void FilterFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    if (flightData->arrDelay > 0) {
        FastCall_request(globalFastOCall, flightData);
    }
}

uint16_t reduceWindow = 100;
uint16_t reduceCount = 0;
std::vector<ReducedFlightData> reducedDatas;
void ReduceFlight(void* data) {
    if (data == NULL) {
        for (auto &reduceFlightData: reducedDatas) {
            FastCall_request(globalFastOCall, &reduceFlightData);
        }
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    char uniqueCarrier[10];
    strncpy(uniqueCarrier, flightData->uniqueCarrier, 10);

    const int arrDelay = flightData->arrDelay;

    bool found = false;
    for (auto &reduceFlightData: reducedDatas) {
        if (strcmp(uniqueCarrier, reduceFlightData.uniqueCarrier) == 0) {
            reduceFlightData.count += 1;
            reduceFlightData.total += arrDelay;

            found = true;
            break;
        }
    }

    if (!found) {
        ReducedFlightData reducedFlightData{};
        strncpy(reducedFlightData.uniqueCarrier, uniqueCarrier, 10);
        reducedFlightData.count = 1;
        reducedFlightData.total = arrDelay;
        reducedDatas.push_back(reducedFlightData);
    }

    reduceCount++;
    if (reduceCount == reduceWindow) {
        for (auto &reduceFlightData: reducedDatas) {
            FastCall_request(globalFastOCall, &reduceFlightData);
        }
        reduceCount = 0;
    }
}

constexpr int joinWindow = 100;
FlightData buffer1[joinWindow];
FlightData buffer2[joinWindow];
JoinedFlightData joinedData[joinWindow * joinWindow + 5];
int n1 = 0, n2 = 0, nJoin = 0;
void JoinFlight(void *data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);
    // FastCall_request(globalFastOCall, flightData);
    if (n1 < joinWindow) {
        strncpy(buffer1[n1].uniqueCarrier, flightData->uniqueCarrier, 10);
        buffer1[n1].arrDelay = flightData->arrDelay;
        n1++;
    } else if (n2 < joinWindow) {
        strncpy(buffer2[n2].uniqueCarrier, flightData->uniqueCarrier, 10);
        buffer2[n2].arrDelay = flightData->arrDelay;
        n2++;
    }

    if (n1 == joinWindow && n2 == joinWindow) {
        for (auto & flight1 : buffer1) {
            for (auto & flight2 : buffer2) {
                // if (true) {
//                if (strcmp(flight1.uniqueCarrier, flight2.uniqueCarrier) == 0) {
                 if (flight1.arrDelay == flight2.arrDelay && flight1.arrDelay != 0) {
                    strncpy(joinedData[nJoin].uniqueCarrier1, flight1.uniqueCarrier, 10);
                    strncpy(joinedData[nJoin].uniqueCarrier2, flight2.uniqueCarrier, 10);
                    joinedData[nJoin].arrDelay = flight1.arrDelay;
                    nJoin++;
                }
            }
        }

        if (nJoin > 0) {
            for (int i = 0; i < nJoin; ++i) {
                FastCall_request(globalFastOCall, &joinedData[i]);
            }
            nJoin = 0;
        }

        for (int i = 0; i < joinWindow; ++i) {
            strncpy(buffer2[i].uniqueCarrier, buffer1[i].uniqueCarrier, 10);
            buffer2[i].arrDelay = buffer1[i].arrDelay;
        }
        n1 = 0;
        n2 = joinWindow;
    }
}
