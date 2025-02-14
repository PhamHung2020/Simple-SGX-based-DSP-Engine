//
// Created by hungpm on 16/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_ENGINE_WITH_BUFFER_OBSERVER_CRYPTO_H
#define SIMPLE_DSP_ENGINE_ENGINE_WITH_BUFFER_OBSERVER_CRYPTO_H

#include "EngineWithBufferObserver.h"

class EngineWithBufferObserverCrypto : public EngineWithBufferObserver {
public:
    uint32_t sourceCPU = 3;
    uint32_t enclaveCPU = 4;
    uint32_t observerCPU = 5;
    uint32_t headObserverCPU = 6;
    uint32_t sinkCPU = 7;

    static void encryptInput(bool);
    static void encryptOutput(bool);

    int start() override;
    void setEmitter(FastCallEmitter &emitter) override;

protected:
    static bool shouldEncryptInput_;
    static bool shouldEncryptOutput_;
    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void* appResponderThread_(void* fastOCallAsVoidP);
};

#endif //SIMPLE_DSP_ENGINE_ENGINE_WITH_BUFFER_OBSERVER_CRYPTO_H
