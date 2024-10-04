//
// Created by hungpm on 03/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_FASTCALLENCRYPTEDEMITTER_H
#define SIMPLE_DSP_ENGINE_FASTCALLENCRYPTEDEMITTER_H

#include "../fast_call.h"
#include "FastCallEmitter.h"

class FastCallEncryptedEmitter : public FastCallEmitter
{
protected:
    size_t dataLength_ = 0;
public:
    void setDataLength(size_t dataLength);
    void emit(void* data) override;
};

#endif //SIMPLE_DSP_ENGINE_FASTCALLENCRYPTEDEMITTER_H
