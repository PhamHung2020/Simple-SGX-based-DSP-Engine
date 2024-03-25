//
// Created by hungpm on 25/03/2024.
//

#include "Engine/SimpleEngine.h"
#include "sgx_lib.h"
#include "sgx_urts.h"
#include "Enclave_u.h"

#include <cstdio>
#include <unistd.h>

SimpleEngine::SimpleEngine() {
    this->sourceThread_ = 0;
}

void* SimpleEngine::startSource_(void* sourceEmitterPairAsVoid) {
    const auto sourceEmitterPair = static_cast<SourceEmitterPair *>(sourceEmitterPairAsVoid);
    const auto source = sourceEmitterPair->source;
    const auto emitter = sourceEmitterPair->emitter;
    source->start(*emitter);
    return nullptr;
}

void *SimpleEngine::enclaveResponderThread_(void *fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall = fastCallPair->fastECall;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    const sgx_status_t status = EcallStartResponder(fastCallPair->enclaveId, fastEcall, fastOcall, fastCallPair->callId);
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

void* SimpleEngine::appResponserThread_(void* fastOCallAsVoidP)
{
    const auto fastOCallStruct = static_cast<FastOCallStruct *>(fastOCallAsVoidP);

    void (*callbacks[1])(void*);
    callbacks[0] = fastOCallStruct->sinkFunc;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastOCallStruct->fastOCallData, &callTable, 0);
    return nullptr;
}

int SimpleEngine::initializeEnclaves()
{
    int initializationSuccessCount = 0;
    for (size_t i = 0; i < this->callIdVector_.size(); ++i) {
        if (initialize_enclave(&this->enclaveIds_[i]) == SGX_SUCCESS) {
            initializationSuccessCount++;
        } else {
            break;
        }
    }

    return initializationSuccessCount;
}

int SimpleEngine::destroyEnclaves()
{
    int destructionEnclaveSuccess = 0;
    for (size_t i = 0; i < this->callIdVector_.size(); ++i) {
        if (sgx_destroy_enclave(this->enclaveIds_[i]) == SGX_SUCCESS) {
            destructionEnclaveSuccess++;
        } else {
            break;
        }
    }

    return destructionEnclaveSuccess;
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

int SimpleEngine::start()
{
    if (this->source_ == nullptr || this->callIdVector_.empty() || this->sink_ == nullptr)
        return -1;

    printf("Start\n");

    this->buffers_.reserve(this->callIdVector_.size() + 1);
    for (size_t i = 0; i < this->callIdVector_.size(); ++i)
    {
        this->enclaveIds_.push_back(0);
        this->enclaveThreads_.push_back(0);
        this->buffers_.push_back({
            new MyEvent[MAX_BUFFER_SIZE],
            0,
            0,
            MAX_BUFFER_SIZE,
            sizeof(MyEvent)
        });
        this->fastCallDatas_.push_back({
            .responderThread = 0,
            .data_buffer = &this->buffers_[i],
            .keepPolling = true
        });
    }

    this->buffers_.push_back({
        new MyEvent[MAX_BUFFER_SIZE],
        0,
        0,
        MAX_BUFFER_SIZE,
        sizeof(MyEvent)
    });

    this->fastCallDatas_.push_back({
        .responderThread = 0,
        .data_buffer = &this->buffers_[this->callIdVector_.size()],
        .keepPolling = true
    });

    printf("Initialized %lu datas\n", this->enclaveIds_.size());

    const int initializationResult = initializeEnclaves();

    printf("Initialized %d enclaves\n", initializationResult);

    if (initializationResult == this->enclaveIds_.size())
    {
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            fastCallPairs_.push_back({
                this->enclaveIds_[i],
                &this->fastCallDatas_[i],
                &this->fastCallDatas_[i+1],
                this->callIdVector_[i],
            });
            pthread_create(&this->fastCallDatas_[i].responderThread, nullptr, enclaveResponderThread_, &fastCallPairs_[i]);
        }

        emitter_.setFastCallData(&this->fastCallDatas_[0]);
        SourceEmitterPair sourceEmitterPair = { this->source_, &this->emitter_ };

        pthread_create(&this->sourceThread_, nullptr, startSource_, &sourceEmitterPair);
        printf("Start source...\n");

        FastOCallStruct fastOCallStruct = {
            .fastOCallData = &this->fastCallDatas_.back(),
            .sinkFunc = this->sink_
        };
        pthread_create(&this->fastCallDatas_.back().responderThread, nullptr, appResponserThread_, &fastOCallStruct);

        printf("Start sink...\n");

        if (this->sourceThread_ != 0)
            pthread_join(this->sourceThread_, nullptr);

        printf("Ended source\n");

        sleep(5);
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            printf("Wait for enclave %lu end\n", i);
            StopResponder(&this->fastCallDatas_[i]);
            pthread_join(this->fastCallDatas_[i].responderThread, nullptr);
        }

    }

    const int destroyEnclaveResult = destroyEnclaves();
    printf("Destroyed %d enclaves\n", destroyEnclaveResult);

    for (const auto & buffer : this->buffers_)
    {
        delete[] static_cast<MyEvent *>(buffer.buffer);
    }

    printf("DONE");
    return 0;
}
