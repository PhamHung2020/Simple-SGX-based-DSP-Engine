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

    sgx_status_t status;
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

    // validate before starting
    if (this->source_ == nullptr || this->callIdVector_.empty() || this->sink_ == nullptr || this->emitter_ == nullptr)
        return -1;

    printf("Engine started\n");

    this->initializeDataStructures();
    printf("Initialized %lu data structures\n", this->enclaveIds_.size());

    const int initializationResult = initializeEnclaves();
    printf("Initialized %d enclaves\n", initializationResult);

    // start processed count thread
//    if (!this->enclaveId_.empty()) {
//        pthread_create(&this->processedCountThreadId_, nullptr, processedCountThread_, &this->buffers_[0]);
//    }

    pthread_attr_t enclaveAttr;
    cpu_set_t enclaveCpu;
    pthread_attr_t sourceAttr;
    cpu_set_t sourceCpu;
    pthread_attr_t sinkAttr;
    cpu_set_t sinkCpu;
    pthread_attr_t observerAttr;
    cpu_set_t observerCpu;
    pthread_attr_t headObserverAttr;
    cpu_set_t headObserverCpu;

    if (initializationResult == this->enclaveIds_.size())
    {
        pthread_attr_init(&enclaveAttr);
        CPU_ZERO(&enclaveCpu);
        CPU_SET(7, &enclaveCpu);
        pthread_attr_setaffinity_np(&enclaveAttr, sizeof(cpu_set_t), &enclaveCpu);

        pthread_attr_init(&observerAttr);
        CPU_ZERO(&observerCpu);
        CPU_SET(6, &observerCpu);
        pthread_attr_setaffinity_np(&observerAttr, sizeof(cpu_set_t), &observerCpu);

        pthread_attr_init(&headObserverAttr);
        CPU_ZERO(&headObserverCpu);
        CPU_SET(5, &headObserverCpu);
        pthread_attr_setaffinity_np(&headObserverAttr, sizeof(cpu_set_t), &headObserverCpu);

        // start enclave responder thread
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            fastCallPairs_.push_back({
                                             static_cast<uint8_t>(i),
                                             this->enclaveIds_[i],
                                             &this->fastCallDataList_[i],
                                             &this->fastCallDataList_[i + 1],
                                             this->callIdVector_[i],
                                             nullptr
                                     });

            pthread_create(&this->fastCallDataList_[i].responderThread, &enclaveAttr, EngineWithBufferObserverCrypto::enclaveResponderThread_, &fastCallPairs_[i]);

            // start tail observer thread and head observer thread for this enclave
            if (this->shouldObserved_[i]) {
                pthread_create(&this->tailObservedDataList_[i].observedThread, &observerAttr, observationThread_, &this->tailObservedDataList_[i]);
                pthread_create(&this->headObservedDataList_[i + 1].observedThread, &headObserverAttr, observationThread_, &this->headObservedDataList_[i + 1]);
            }
        }

        // create source thread
        emitter_->setFastCallData(&this->fastCallDataList_[0]);
        SourceEmitterPair sourceEmitterPair = { this->source_, this->emitter_ };

        pthread_attr_init(&sourceAttr);
        CPU_ZERO(&sourceCpu);
        CPU_SET(3, &sourceCpu);
        pthread_attr_setaffinity_np(&sourceAttr, sizeof(cpu_set_t), &sourceCpu);
        pthread_create(&this->sourceThread_, &sourceAttr, startSource_, &sourceEmitterPair);
        printf("Start source...\n");

        // create sink thread
        FastOCallStruct fastOCallStruct = {
                .fastOCallData = &this->fastCallDataList_.back(),
                .sinkFunc = this->sink_
        };
        pthread_attr_init(&sinkAttr);
        CPU_ZERO(&sinkCpu);
        CPU_SET(4, &sinkCpu);
        pthread_attr_setaffinity_np(&sinkAttr, sizeof(cpu_set_t), &sinkCpu);
        pthread_create(&this->fastCallDataList_.back().responderThread, &sinkAttr, EngineWithBufferObserverCrypto::appResponderThread_, &fastOCallStruct);
        printf("Start sink...\n");

        // wait for source end
        if (this->sourceThread_ != 0)
            pthread_join(this->sourceThread_, nullptr);
        printf("Ended source\n");

        // wait for all enclaves end
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            printf("Wait for enclave %lu end\n", i);
            StopFastCallResponder(&this->fastCallDataList_[i]);
            pthread_join(this->fastCallDataList_[i].responderThread, nullptr);
        }
        StopFastCallResponder(&this->fastCallDataList_.back());
        pthread_join(this->fastCallDataList_.back().responderThread, nullptr);

//        this->shouldContinue_ = false;
//        pthread_join(this->processedCountThreadId_, nullptr);

        // wait for all observation threads end
        for (size_t i = 0; i < this->buffers_.size(); ++i) {
            printf("Wait for head observer %lu end\n", i);
            this->headObservedDataList_[i].keepPolling = false;

            printf("Wait for tail observer %lu end\n", i);
            this->tailObservedDataList_[i].keepPolling = false;

            pthread_join(this->headObservedDataList_[i].observedThread, nullptr);
            pthread_join(this->tailObservedDataList_[i].observedThread, nullptr);
        }
    }

    // destroy enclaves
    const int destroyEnclaveResult = destroyEnclaves();
    printf("Destroyed %d enclaves\n", destroyEnclaveResult);
    printf("Engine stopped\n");
    return 0;
}
