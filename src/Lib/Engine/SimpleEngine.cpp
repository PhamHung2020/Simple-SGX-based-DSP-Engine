//
// Created by hungpm on 25/03/2024.
//

#include "Engine/SimpleEngine.h"
#include "sgx_lib.h"
#include "sgx_urts.h"
#include "Enclave_u.h"

#include <cstdio>
#include <stdexcept>
#include <unistd.h>

#include "hot_call_perormance.h"

std::chrono::_V2::system_clock::time_point SimpleEngine::startSourceTime_;
std::chrono::_V2::system_clock::time_point SimpleEngine::endSourceTime_;
std::chrono::_V2::system_clock::time_point SimpleEngine::endPipelineTime_;
std::vector<std::chrono::_V2::system_clock::time_point> SimpleEngine::startEnclaveTimes_;
std::vector<std::chrono::_V2::system_clock::time_point> SimpleEngine::endEnclaveTimes_;

SimpleEngine::SimpleEngine() {
    this->sourceThread_ = 0;
    this->emitter_ = nullptr;
}

void* SimpleEngine::startSource_(void* sourceEmitterPairAsVoid) {
    const auto sourceEmitterPair = static_cast<SourceEmitterPair *>(sourceEmitterPairAsVoid);
    const auto source = sourceEmitterPair->source;
    const auto emitter = sourceEmitterPair->emitter;
    startSourceTime_ = std::chrono::high_resolution_clock::now();
    source->start(*emitter);
    endSourceTime_ = std::chrono::high_resolution_clock::now();
    return nullptr;
}

void *SimpleEngine::enclaveResponderThread_(void *fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall = fastCallPair->fastECall;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    const uint8_t no = fastCallPair->no;

    startEnclaveTimes_[no] = std::chrono::high_resolution_clock::now();
    const sgx_status_t status = EcallStartResponder(fastCallPair->enclaveId, fastEcall, fastOcall, fastCallPair->callId);
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

void* SimpleEngine::appResponderThread_(void* fastOCallAsVoidP)
{
    const auto fastOCallStruct = static_cast<FastOCallStruct *>(fastOCallAsVoidP);

    void (*callbacks[1])(void*);
    callbacks[0] = fastOCallStruct->sinkFunc;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastOCallStruct->fastOCallData, &callTable, 0);
    endPipelineTime_ = std::chrono::high_resolution_clock::now();

    return nullptr;
}

void SimpleEngine::clearTime() {
    startSourceTime_ = std::chrono::_V2::system_clock::time_point::min();
    endSourceTime_ = std::chrono::_V2::system_clock::time_point::min();
    endPipelineTime_ = std::chrono::_V2::system_clock::time_point::min();
    startEnclaveTimes_.clear();
    endEnclaveTimes_.clear();
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

int SimpleEngine::destroyEnclaves() const {
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

int SimpleEngine::initializeDataStructures() {
    // initialize necessary data structure: circular buffer, fast call struct
    const int nEnclave = static_cast<int>(this->callIdVector_.size());

    startEnclaveTimes_.reserve(nEnclave);
    endEnclaveTimes_.reserve(nEnclave);

    this->buffers_.reserve(nEnclave + 1);
    this->enclaveIds_.reserve(nEnclave);
    this->enclaveThreads_.reserve(nEnclave);
    this->fastCallDatas_.reserve(nEnclave + 1);
    this->fastCallPairs_.reserve(nEnclave);

    for (size_t i = 0; i < this->callIdVector_.size(); ++i)
    {
        startEnclaveTimes_.push_back(std::chrono::_V2::system_clock::time_point::min());
        endEnclaveTimes_.push_back(std::chrono::_V2::system_clock::time_point::min());

        this->enclaveIds_.push_back(0);
        this->enclaveThreads_.push_back(0);
        this->buffers_.push_back({
            new char[MAX_BUFFER_SIZE * this->dataSizeVector_[i]],
            0,
            0,
            MAX_BUFFER_SIZE,
            this->dataSizeVector_[i],
            SGX_SPINLOCK_INITIALIZER,
            0
        });
        this->fastCallDatas_.push_back({
            .spinlock = SGX_SPINLOCK_INITIALIZER,
            .responderThread = 0,
            .data_buffer = &this->buffers_[i],
            .keepPolling = true
        });
    }

    this->buffers_.push_back({
            new char[MAX_BUFFER_SIZE * this->dataSizeVector_.back()],
            0,
            0,
            MAX_BUFFER_SIZE,
            this->dataSizeVector_.back()
        });
    this->fastCallDatas_.push_back({
        .spinlock = SGX_SPINLOCK_INITIALIZER,
        .responderThread = 0,
        .data_buffer = &this->buffers_.back(),
        .keepPolling = true
    });

    return nEnclave;
}

void SimpleEngine::setSource(Source &source)
{
    this->source_ = &source;
}

void SimpleEngine::setEmitter(FastCallEmitter &emitter)
{
    this->emitter_ = &emitter;
}

void SimpleEngine::addTask(const uint16_t callId, const uint16_t inputDataSize)
{
    this->callIdVector_.push_back(callId);
    this->dataSizeVector_.push_back(inputDataSize);
}

void SimpleEngine::setSink(void (*sink)(void *), const uint16_t outputDataSize)
{
    this->sink_ = sink;
    this->dataSizeVector_.push_back(outputDataSize);
}

int SimpleEngine::getNumberOfTask() {
    return static_cast<int>(this->callIdVector_.size());
}

int SimpleEngine::start()
{
    // validate before starting
    if (this->source_ == nullptr || this->callIdVector_.empty() || this->sink_ == nullptr || this->emitter_ == nullptr)
        return -1;

    // clearTime();

    printf("Start\n");

    this->initializeDataStructures();
    printf("Initialized %lu data structures\n", this->enclaveIds_.size());

    const int initializationResult = initializeEnclaves();
    printf("Initialized %d enclaves\n", initializationResult);

    if (initializationResult == this->enclaveIds_.size())
    {
        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            fastCallPairs_.push_back({
                static_cast<uint8_t>(i),
                this->enclaveIds_[i],
                &this->fastCallDatas_[i],
                &this->fastCallDatas_[i+1],
                this->callIdVector_[i],
                nullptr
            });
            pthread_create(&this->fastCallDatas_[i].responderThread, nullptr, enclaveResponderThread_, &fastCallPairs_[i]);
        }

        FastOCallStruct fastOCallStruct = {
            .fastOCallData = &this->fastCallDatas_.back(),
            .sinkFunc = this->sink_
        };
        pthread_create(&this->fastCallDatas_.back().responderThread, nullptr, appResponderThread_, &fastOCallStruct);
        printf("Start sink...\n");

        emitter_->setFastCallData(&this->fastCallDatas_[0]);
        SourceEmitterPair sourceEmitterPair = { this->source_, this->emitter_ };
        pthread_create(&this->sourceThread_, nullptr, startSource_, &sourceEmitterPair);
        printf("Start source...\n");

        if (this->sourceThread_ != 0)
            pthread_join(this->sourceThread_, nullptr);
        printf("Ended source\n");

        for (size_t i = 0; i < this->enclaveIds_.size(); ++i)
        {
            printf("Wait for enclave %lu end\n", i);
            StopFastCallResponder(&this->fastCallDatas_[i]);
            pthread_join(this->fastCallDatas_[i].responderThread, nullptr);
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

std::chrono::_V2::system_clock::time_point SimpleEngine::getStartSourceTime() {
    return startSourceTime_;
}

std::chrono::_V2::system_clock::time_point SimpleEngine::getEndSourceTime() {
    return endSourceTime_;
}

std::chrono::_V2::system_clock::time_point SimpleEngine::getEndPipelineTime() {
    return endPipelineTime_;
}

std::chrono::_V2::system_clock::time_point SimpleEngine::getStartEnclaveTime(const int index) {
    if (index < 0 || index >= startEnclaveTimes_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return startEnclaveTimes_[index];
}

std::chrono::_V2::system_clock::time_point SimpleEngine::getEndEnclaveTime(const int index) {
    if (index < 0 || index >= endEnclaveTimes_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return endEnclaveTimes_[index];
}
