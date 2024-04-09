//
// Created by hungpm on 27/03/2024.
//

#include "Engine/EngineWithBufferObserver.h"

#include <stdexcept>


EngineWithBufferObserver::ObservedData & EngineWithBufferObserver::getHeadObservedData(const int index) {
    if (index < 0 || index > this->headOservedDatas_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->headOservedDatas_[index];
}

EngineWithBufferObserver::ObservedData & EngineWithBufferObserver::getTailObservedData(const int index) {
    if (index < 0 || index > this->tailOservedDatas_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->tailOservedDatas_[index];
}

size_t EngineWithBufferObserver::getBufferCount() const {
    return this->buffers_.size();
}

void * EngineWithBufferObserver::observationThread_(void *observedDataAsVoidP) {
    const auto observedData = static_cast<ObservedData*>(observedDataAsVoidP);

    const auto buffer = observedData->buffer;
    observedData->startTime = std::chrono::high_resolution_clock::now();
    if (observedData->isHead) {
        while (true) {
            const int value = buffer->head;
            if (value != observedData->previousValue) {
                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
    } else {
        while (true) {
            const int value = buffer->tail;
            if (value != observedData->previousValue) {
                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
    }

    return nullptr;
}

int EngineWithBufferObserver::initializeDataStructures() {
    const int baseInitialization = SimpleEngine::initializeDataStructures();
    if (baseInitialization == 0) {
        return baseInitialization;
    }

    const int nBuffer = static_cast<int>(this->buffers_.size());
    this->headOservedDatas_.reserve(nBuffer);
    this->tailOservedDatas_.reserve(nBuffer);

    for (size_t i = 0; i < nBuffer; ++i) {
        ObservedData headObservedData;
        headObservedData.observedThread = 0;
        headObservedData.buffer = &this->buffers_[i];
        headObservedData.isHead = true;
        headObservedData.count = 0;
        headObservedData.keepPolling = true;
        headObservedData.previousValue = 0;
        this->headOservedDatas_.push_back(headObservedData);

        ObservedData tailObservedData;
        tailObservedData.observedThread = 0;
        tailObservedData.buffer = &this->buffers_[i];
        tailObservedData.isHead = false;
        tailObservedData.count = 0;
        tailObservedData.keepPolling = true;
        tailObservedData.previousValue = 0;
        this->tailOservedDatas_.push_back(tailObservedData);
    }

    return nBuffer;
}

int EngineWithBufferObserver::start() {
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
            fastCallPairs_.push_back({
                static_cast<uint8_t>(i),
                this->enclaveIds_[i],
                &this->fastCallDatas_[i],
                &this->fastCallDatas_[i+1],
                this->callIdVector_[i],
                nullptr
            });
            pthread_create(&this->fastCallDatas_[i].responderThread, nullptr, enclaveResponderThread_, &fastCallPairs_[i]);

            if (this->shouldObserved_[i]) {
                pthread_create(&this->tailOservedDatas_[i].observedThread, nullptr, observationThread_, &this->tailOservedDatas_[i]);
//                pthread_create(&this->headOservedDatas_[i+1].observedThread, nullptr, observationThread_, &this->headOservedDatas_[i+1]);
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
        }
        StopFastCallResponder(&this->fastCallDatas_.back());
        pthread_join(this->fastCallDatas_.back().responderThread, nullptr);

        for (size_t i = 0; i < this->buffers_.size(); ++i) {
            printf("Wait for head observer %lu end\n", i);
            this->headOservedDatas_[i].keepPolling = false;

            printf("Wait for tail observer %lu end\n", i);
            this->tailOservedDatas_[i].keepPolling = false;

            pthread_join(this->headOservedDatas_[i].observedThread, nullptr);
            pthread_join(this->tailOservedDatas_[i].observedThread, nullptr);
        }
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

void EngineWithBufferObserver::addTask(const uint16_t callId, const uint16_t inputDataSize, const bool shouldObserved) {
    this->shouldObserved_.push_back(shouldObserved);
    SimpleEngine::addTask(callId, inputDataSize);
}

void EngineWithBufferObserver::addTask(const uint16_t callId, const uint16_t inputDataSize) {
    this->shouldObserved_.push_back(true);
    SimpleEngine::addTask(callId, inputDataSize);
}

bool EngineWithBufferObserver::isObserved(const int index) {
    if (index < 0 || index > this->shouldObserved_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->shouldObserved_[index];
}
