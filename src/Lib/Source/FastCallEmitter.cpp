//
// Created by hungpm on 25/03/2024.
//
#include "Source/FastCallEmitter.h"
#include <cstdio>

void FastCallEmitter::setFastCallData(FastCallStruct* fastCallData)
{
    this->fastCallDataP_ = fastCallData;
}

void FastCallEmitter::emit(void *data)
{
    if (this->fastCallDataP_->responderThread != 0 && this->fastCallDataP_->data_buffer != nullptr) {
        const auto row = static_cast<char *>(data);
        printf("%s\n", row);
        FastCall_request(this->fastCallDataP_, data);
    }
}
