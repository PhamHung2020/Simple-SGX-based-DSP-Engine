//
// Created by hungpm on 27/03/2024.
//

#include "Engine/EngineWithBufferObserver.h"

#include <stdexcept>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"

uint64_t EngineWithBufferObserver::processedPerSecond[1];
int EngineWithBufferObserver::processedCountIndex = 0;
bool EngineWithBufferObserver::shouldContinue_ = true;

EngineWithBufferObserver::ObservedData & EngineWithBufferObserver::getHeadObservedData(const int index) {
    if (index < 0 || index > this->headObservedDataList_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->headObservedDataList_[index];
}

EngineWithBufferObserver::ObservedData & EngineWithBufferObserver::getTailObservedData(const int index) {
    if (index < 0 || index > this->tailObservedDataList_.size()) {
        throw std::out_of_range("Index out of range");
    }

    return this->tailObservedDataList_[index];
}

size_t EngineWithBufferObserver::getBufferCount() const {
    return this->buffers_.size();
}

void * EngineWithBufferObserver::observationThread_(void *observedDataAsVoidP) {
    const auto observedData = static_cast<ObservedData*>(observedDataAsVoidP);

    const auto buffer = observedData->buffer;
//    observedData->startTime = std::chrono::high_resolution_clock::now();
    uint32_t count = 1000000;
    observedData->startTimestamp = rdtscp();
    if (observedData->isHead) {
        while (count > 0) {
            const int value = buffer->head;
            if (value != observedData->previousValue) {
                count--;
//                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                uint64_t timestamp = rdtscp();
                observedData->timestampCounterPoints[observedData->count] = timestamp;
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
        printf("Observer head ended\n");

    } else {
        while (count > 0) {
            const int value = buffer->tail;
            if (value != observedData->previousValue) {
                count--;
//                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                uint64_t timestamp = rdtscp();
                observedData->timestampCounterPoints[observedData->count] = timestamp;
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
        printf("Observer tail ended\n");
    }

    return nullptr;
}

int EngineWithBufferObserver::initializeDataStructures() {
    const int baseInitialization = SimpleEngine::initializeDataStructures();
    if (baseInitialization == 0) {
        return baseInitialization;
    }

    const int nBuffer = static_cast<int>(this->buffers_.size());
    this->headObservedDataList_.reserve(nBuffer);
    this->tailObservedDataList_.reserve(nBuffer);

    for (size_t i = 0; i < nBuffer; ++i) {
        ObservedData headObservedData;
        headObservedData.observedThread = 0;
        headObservedData.buffer = &this->buffers_[i];
        headObservedData.isHead = true;
        headObservedData.count = 0;
        headObservedData.keepPolling = true;
        headObservedData.previousValue = 0;
        headObservedData.noItem = new uint64_t[MAX_ITEM];
//        headObservedData.timePoints = new std::chrono::_V2::system_clock::time_point[MAX_ITEM];
        headObservedData.timestampCounterPoints = new uint64_t[MAX_ITEM];

        // initialize timestamp
        for (size_t j = 0; j < MAX_ITEM; ++j) {
            headObservedData.timestampCounterPoints[j] = -1;
        }

        this->headObservedDataList_.push_back(headObservedData);

        ObservedData tailObservedData;
        tailObservedData.observedThread = 0;
        tailObservedData.buffer = &this->buffers_[i];
        tailObservedData.isHead = false;
        tailObservedData.count = 0;
        tailObservedData.keepPolling = true;
        tailObservedData.previousValue = 0;
        tailObservedData.noItem = new uint64_t[MAX_ITEM];
//        tailObservedData.timePoints = new std::chrono::_V2::system_clock::time_point[MAX_ITEM];
        tailObservedData.timestampCounterPoints = new uint64_t[MAX_ITEM];

        // initialize timestamp
        for (size_t j = 0; j < MAX_ITEM; ++j) {
            tailObservedData.timestampCounterPoints[j] = -1;
        }
        this->tailObservedDataList_.push_back(tailObservedData);
    }

    return nBuffer;
}

int EngineWithBufferObserver::start() {
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

            pthread_create(&this->fastCallDataList_[i].responderThread, &enclaveAttr, enclaveResponderThread_, &fastCallPairs_[i]);

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
        pthread_create(&this->fastCallDataList_.back().responderThread, &sinkAttr, appResponderThread_, &fastOCallStruct);
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

void *EngineWithBufferObserver::processedCountThread_(void *circularBufferAsVoidP) {
    const auto buffer = (circular_buffer*) circularBufferAsVoidP;
    uint64_t value;
    while (shouldContinue_) {
        sgx_spin_lock(&buffer->lock_count);
        value = buffer->popped_count;
        buffer->popped_count = 0;
        sgx_spin_unlock(&buffer->lock_count);

        processedPerSecond[processedCountIndex] = value;
        processedCountIndex++;
        sleep(1);
    }

    return nullptr;
}

void EngineWithBufferObserver::clean() {
    SimpleEngine::clean();

    for (auto & headObservedData : this->headObservedDataList_) {
        delete[] headObservedData.noItem;
//        delete[] headObservedData.timePoints;
        delete[] headObservedData.timestampCounterPoints;
    }
    this->headObservedDataList_.clear();

    for (auto & tailObservedData : this->tailObservedDataList_) {
        delete[] tailObservedData.noItem;
//        delete[] tailObservedData.timePoints;
        delete[] tailObservedData.timestampCounterPoints;
    }
    this->tailObservedDataList_.clear();
}
