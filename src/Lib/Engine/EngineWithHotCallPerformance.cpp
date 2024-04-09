//
// Created by hungpm on 27/03/2024.
//

#include "Engine/EngineWithHotCallPerformance.h"

#include <stdexcept>

#include "sgx_lib.h"
#include "sgx_urts.h"
#include "Enclave_u.h"

void *EngineWithHotCallPerformance::enclaveResponderThread_(void *fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall = fastCallPair->fastECall;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    // HotCall* hotCall = fastCallPair->hotCall;
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

void * EngineWithHotCallPerformance::enclaveResponderThreadWitHotCall_(void *fastCallPairAsVoidP) {
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall = fastCallPair->fastECall;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    HotCall* hotCall = fastCallPair->hotCall;
    const sgx_status_t status = EcallStartResponderWithHotCall(fastCallPair->enclaveId, fastEcall, fastOcall, fastCallPair->callId, hotCall);
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

void EngineWithHotCallPerformance::addStartTime_(void *hotCallPerformanceAsVoidP) {
    const auto hotCallPerformance = static_cast<HotOCallPerformanceParams*> (hotCallPerformanceAsVoidP);
    hotCallPerformance->startTimes.push_back(std::chrono::high_resolution_clock::now());
}

void EngineWithHotCallPerformance::addEndTime_(void *hotCallPerformanceAsVoidP) {
    const auto hotCallPerformance = static_cast<HotOCallPerformanceParams*> (hotCallPerformanceAsVoidP);
    hotCallPerformance->endTimes.push_back(std::chrono::high_resolution_clock::now());
}

void * EngineWithHotCallPerformance::appPerformanceThread_(void *hotCallAsVoidP) {
    void (*callbacks[2])(void*);

    callbacks[0] = addStartTime_;
    callbacks[1] = addEndTime_;

    HotCallTable callTable;
    callTable.numEntries = 2;
    callTable.callbacks = callbacks;

    HotCall_waitForCall(static_cast<HotCall*> (hotCallAsVoidP), &callTable);
    return nullptr;
}

int EngineWithHotCallPerformance::initializeDataStructures() {
    SimpleEngine::initializeDataStructures();

    const int nEnclave = static_cast<int>(this->callIdVector_.size());

    this->hotCallPerformances_.reserve(nEnclave);
    this->hotCalls_.reserve(nEnclave);

    for (size_t i = 0; i < this->callIdVector_.size(); ++i) {
        this->hotCallPerformances_.push_back({
            .startTimes = std::vector<std::chrono::_V2::system_clock::time_point>(),
            .endTimes = std::vector<std::chrono::_V2::system_clock::time_point>()
        });

        this->hotCalls_.push_back(HOTCALL_INITIALIZER);
    }

    return nEnclave;
}

void EngineWithHotCallPerformance::addTask(const uint16_t callId, const uint16_t inputDataSize) {
    this->withHotCall_.push_back(true);
    SimpleEngine::addTask(callId, inputDataSize);
}

void EngineWithHotCallPerformance::addTask(const uint16_t callId, const uint16_t inputDataSize, const bool withHotCall) {
    this->withHotCall_.push_back(withHotCall);
    SimpleEngine::addTask(callId, inputDataSize);
}

std::vector<HotOCallPerformanceParams> & EngineWithHotCallPerformance::getHotCallPerformanceParams() {
    return this->hotCallPerformances_;
}

bool EngineWithHotCallPerformance::isWithHotCall(const int index) {
    if (index < 0 || index > this->withHotCall_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->withHotCall_[index];
}

int EngineWithHotCallPerformance::start()
{
    // validate before starting
    if (this->source_ == nullptr || this->callIdVector_.empty() || this->sink_ == nullptr || this->emitter_ == nullptr)
        return -1;

    printf("Start\n");

    this->initializeDataStructures();
    printf("Initialized %lu data structures\n", this->enclaveIds_.size());

    const int initializationResult = initializeEnclaves();
    printf("Initialized %d enclaves\n", initializationResult);

    if (initializationResult == this->enclaveIds_.size())
    {
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            this->hotCalls_[i].data = &this->hotCallPerformances_[i];

            fastCallPairs_.push_back({
                static_cast<uint8_t>(i),
                this->enclaveIds_[i],
                &this->fastCallDatas_[i],
                &this->fastCallDatas_[i+1],
                this->callIdVector_[i],
                &this->hotCalls_[i]
            });

            if (this->withHotCall_[i]) {
                pthread_create(&this->fastCallDatas_[i].responderThread, nullptr, enclaveResponderThreadWitHotCall_, &fastCallPairs_[i]);
                pthread_create(&this->hotCalls_[i].responderThread, nullptr, appPerformanceThread_, &this->hotCalls_[i]);
            } else {
                pthread_create(&this->fastCallDatas_[i].responderThread, nullptr, enclaveResponderThread_, &fastCallPairs_[i]);
            }
        }

        emitter_->setFastCallData(&this->fastCallDatas_[0]);
        SourceEmitterPair sourceEmitterPair = { this->source_, this->emitter_ };

        pthread_create(&this->sourceThread_, nullptr, startSource_, &sourceEmitterPair);
        printf("Start source...\n");

        FastOCallStruct fastOCallStruct = {
            .fastOCallData = &this->fastCallDatas_.back(),
            .sinkFunc = this->sink_
        };
        pthread_create(&this->fastCallDatas_.back().responderThread, nullptr, appResponderThread_, &fastOCallStruct);
        printf("Start sink...\n");

        if (this->sourceThread_ != 0)
            pthread_join(this->sourceThread_, nullptr);
        printf("Ended source\n");

        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            printf("Wait for enclave %lu end\n", i);
            StopFastCallResponder(&this->fastCallDatas_[i]);
            pthread_join(this->fastCallDatas_[i].responderThread, nullptr);

            StopResponder(&this->hotCalls_[i]);
            pthread_join(this->hotCalls_[i].responderThread, nullptr);
        }
        StopFastCallResponder(&this->fastCallDatas_.back());
        pthread_join(this->fastCallDatas_.back().responderThread, nullptr);
    }

    const int destroyEnclaveResult = destroyEnclaves();
    printf("Destroyed %d enclaves\n", destroyEnclaveResult);

    for (const auto & buffer : this->buffers_)
    {
        delete[] static_cast<MyEvent *>(buffer.buffer);
    }

    printf("DONE\n");
    return 0;
}
