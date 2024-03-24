//
// Created by hungpm on 24/03/2024.
//

#ifndef ENCLAVE_OPERATORS_H
#define ENCLAVE_OPERATORS_H

#include "data_types.h"

void print(const char *fmt, ...);
int nestedLoopJoin
(
    const MyEvent* eventArr1,
    const MyEvent* eventArr2,
    int n1, int n2,
    JoinResult* &results,
    bool (*predicate)(MyEvent, MyEvent)
);

MyEvent* filter(MyEvent* event, bool (*predicate)(const MyEvent&));
MyEvent* map(MyEvent* event, void (*mapRule)(MyEvent*));
int reduce(const MyEvent &event, int accumulator, int (*reduceFunc)(MyEvent, int));

#endif //ENCLAVE_OPERATORS_H
