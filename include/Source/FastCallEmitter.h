//
// Created by hungpm on 25/03/2024.
//

#ifndef FASTCALLEMITTER_H
#define FASTCALLEMITTER_H

#include "Emitter.h"
#include "../fast_call.h"

class FastCallEmitter : public Emitter
{
protected:
    FastCallStruct* fastCallDataP_ = nullptr;
public:
    void setFastCallData(FastCallStruct* fastCallData);
    void emit(void* data) override;
};

#endif //FASTCALLEMITTER_H
