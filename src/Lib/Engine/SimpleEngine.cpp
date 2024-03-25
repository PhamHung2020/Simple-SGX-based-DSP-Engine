//
// Created by hungpm on 25/03/2024.
//

#include "Engine/SimpleEngine.h"

#include <cstdio>

SimpleEngine::SimpleEngine() {
    this->sourceThread_ = 0;
}

void* SimpleEngine::startSource_(void* sourceAsVoid) {
    const auto source = static_cast<Source *>(sourceAsVoid);
    source->start(this->emitter_);
    return nullptr;
}

void SimpleEngine::setSource(Source &source)
{
    this->source_ = &source;
}

void SimpleEngine::addOperator(const uint16_t callId)
{
    this->callIdVector_.push_back(callId);
}

void SimpleEngine::setSink(void (*sink)(void *))
{
    this->sink_ = sink;
}

int SimpleEngine::start() {
    if (this->source_ != nullptr)
        this->startSource_(this->source_);
    return 0;
}
