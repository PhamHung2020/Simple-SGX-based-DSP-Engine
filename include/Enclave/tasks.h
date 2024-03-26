//
// Created by hungpm on 24/03/2024.
//

#ifndef TASKS_H
#define TASKS_H

#if defined(__cplusplus)
extern "C" {
#endif
    extern int task3Result;
    extern int task3WindowLength;
    extern int task3Count;
#if defined(__cplusplus)
}
#endif


void TaskExecutor(void* data);
void TaskExecutor2(void* data);
void TaskExecutor3(void* data);
#endif //TASKS_H
