//
// Created by hungpm on 16/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_ENGINE_WITH_BUFFER_OBSERVER_CRYPTO_H
#define SIMPLE_DSP_ENGINE_ENGINE_WITH_BUFFER_OBSERVER_CRYPTO_H

#include "EngineWithBufferObserver.h"

class EngineWithBufferObserverCrypto : EngineWithBufferObserver {
public:
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
