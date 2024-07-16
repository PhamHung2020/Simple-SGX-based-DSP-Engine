//
// Created by hungpm on 16/07/2024.
//

#include "sgx_lib.h"
#include "sgx_urts.h"
#include "Engine/EngineWithBufferObserverCrypto.h"
#include "Source/FastCallEncryptedEmitter.h"
#include "Enclave_u.h"

bool EngineWithBufferObserverCrypto::shouldEncryptInput_ = true;
bool EngineWithBufferObserverCrypto::shouldEncryptOutput_ = true;

void EngineWithBufferObserverCrypto::encryptInput(bool shouldEncryptInput) {
    EngineWithBufferObserverCrypto::shouldEncryptInput_ = shouldEncryptInput;
}

void EngineWithBufferObserverCrypto::encryptOutput(bool shouldEncryptOutput) {
    EngineWithBufferObserverCrypto::shouldEncryptOutput_ = shouldEncryptOutput;
}

void *EngineWithBufferObserverCrypto::enclaveResponderThread_(void *fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastECall = fastCallPair->fastECall;
    FastCallStruct *fastOCall = fastCallPair->fastOCall;
    const uint8_t no = fastCallPair->no;

    startEnclaveTimes_[no] = std::chrono::high_resolution_clock::now();

    sgx_status_t status = SGX_SUCCESS;
    if (EngineWithBufferObserverCrypto::shouldEncryptInput_) {
        status = EcallStartResponderWithDecryption(fastCallPair->enclaveId, fastECall, fastOCall, fastCallPair->callId);
    } else {
        status = EcallStartResponder(fastCallPair->enclaveId, fastECall, fastOCall, fastCallPair->callId);
    }
    endEnclaveTimes_[no] = std::chrono::high_resolution_clock::now();

    if (status == SGX_SUCCESS)
    {
        printf("Polling success\n");
    }
    else
    {
        printf("Polling failed\n");
        print_error_message(status);
    }

    return nullptr;
}

void *EngineWithBufferObserverCrypto::appResponderThread_(void *fastOCallAsVoidP) {
    const auto fastOCallStruct = static_cast<FastOCallStruct *>(fastOCallAsVoidP);

    void (*callbacks[1])(void*);
    callbacks[0] = fastOCallStruct->sinkFunc;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    if (EngineWithBufferObserverCrypto::shouldEncryptOutput_) {
        FastCall_wait_decrypt(fastOCallStruct->fastOCallData, &callTable, 0);
    } else {
        FastCall_wait(fastOCallStruct->fastOCallData, &callTable, 0);
    }
    endPipelineTime_ = std::chrono::high_resolution_clock::now();

    return nullptr;
}

void EngineWithBufferObserverCrypto::setEmitter(FastCallEmitter &emitter) {}

int EngineWithBufferObserverCrypto::start() {
    delete this->emitter_;
    if (EngineWithBufferObserverCrypto::shouldEncryptInput_) {
        this->emitter_ = new FastCallEncryptedEmitter;
    } else {
        this->emitter_ = new FastCallEmitter;
    }

    return EngineWithBufferObserver::start();
}
