//
// Created by hungpm on 25/03/2024.
//

#ifndef FASTCALLEMITTER_H
#define FASTCALLEMITTER_H

#include "Emitter.h"
#include "data_types.h"
#include "../fast_call.h"

class FastCallEmitter final : public Emitter
{
private:
    // FastCallStruct fastCallData_ = { 0, nullptr, false };
    FastCallStruct* fastCallDataP_ = nullptr;
    MyEvent event_;
public:
    void setFastCallData(FastCallStruct* fastCallData);
    void emit(void* data) override;
};

#endif //FASTCALLEMITTER_H
