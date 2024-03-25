//
// Created by hungpm on 25/03/2024.
//
#include "Source/FastCallEmitter.h"
#include <cstdio>

void FastCallEmitter::setFastCallData(const FastCallStruct &fastCallData)
{
    this->fastCallData_ = fastCallData;
}

void FastCallEmitter::emit(void *data)
{
    // if (this->fastCallData_.responderThread != 0 && this->fastCallData_.data_buffer != nullptr)
    //     FastCall_request(&this->fastCallData_, static_cast<MyEvent *>(data));
    //
    auto* event = static_cast<MyEvent *>(data);
    printf(
        "Sink Result: (%lf %d %d %d %s)\n",
        event->timestamp, event->sourceId, event->key, event->data, event->message
    );
}
