//
// Created by hungpm on 11/06/2024.
//
#include <ctime>
#include "Enclave/operators.h"
#include "fast_call_enclave.h"

MapOperator::MapOperator(void *(*mapRule)(void *)) {
    this->mapRule_ = mapRule;
}

void *MapOperator::operate(void *data) {
    if (data == NULL) {
        return NULL;
    }

    return this->mapRule_(data);
}

FilterOperator::FilterOperator(bool (*predicate)(void *)) {
    this->predicate_ = predicate;
}

void *FilterOperator::operate(void *data) {
    if (this->predicate_(data)) {
        return data;
    }
    return nullptr;
}

JoinOperator::JoinOperator(bool (*predicate)(void *, void *), uint64_t timeRange) {
    this->predicate_ = predicate;
    this->timeRange_ = timeRange;
    this->maxTimeStream1_ = 0;
    this->maxTimeStream2_ = 0;
}

void *JoinOperator::operate(void *data) {
    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto data1 = dataGroup->data1;
    const auto data2 = dataGroup->data2;

    return nullptr;
}

