//
// Created by hungpm on 11/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_OPERATORS_H
#define SIMPLE_DSP_ENGINE_OPERATORS_H

#include <vector>

class Operator {
public:
    virtual void* operate(void* data) = 0;
};

class MapOperator : public Operator{
private:
    void* (*mapRule_)(void*);
public:
    explicit MapOperator(void* (*mapRule)(void*));
    void *operate(void *data) override;
};

class FilterOperator : public Operator {
private:
    bool (*predicate_)(void* data);
public:
    explicit FilterOperator(bool (*predicate)(void*));
    void *operate(void *data) override;
};

class JoinOperator : public Operator {
private:
    std::vector<void*> stream1_;
    std::vector<void*> stream2_;
    uint64_t timeRange_;
    uint64_t maxTimeStream1_;
    uint64_t maxTimeStream2_;
    bool (*predicate_)(void* data1, void* data2);
public:
    JoinOperator(bool (*predicate)(void*, void*), uint64_t timeRange);
    void *operate(void *data) override;
};

#endif //SIMPLE_DSP_ENGINE_OPERATORS_H
