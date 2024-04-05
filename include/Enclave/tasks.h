//
// Created by hungpm on 24/03/2024.
//

#ifndef TASKS_H
#define TASKS_H

#include <vector>
#include "data_types.h"

#define TASK_COUNT 8

void MapCsvRowToEvent(void* data);
void TaskExecutor(void* data);
void TaskExecutor2(void* data);
void TaskExecutor3(void* data);

void MapCsvRowToFlight(void* data);
void FilterFlight(void* data);
void ReduceFlight(void* data);
void JoinFlight(void* data);

static void (*callbacks[TASK_COUNT])(void*) = {
    MapCsvRowToEvent,
    TaskExecutor,
    TaskExecutor2,
    TaskExecutor3,
    MapCsvRowToFlight,
    FilterFlight,
    ReduceFlight,
    JoinFlight,
};

#if defined(__cplusplus)
extern "C" {
#endif
    extern int task3Result;
    extern int task3WindowLength;
    extern int task3Count;

    extern std::vector<ReducedFlightData> reducedDatas;
#if defined(__cplusplus)
}
#endif


#endif //TASKS_H
