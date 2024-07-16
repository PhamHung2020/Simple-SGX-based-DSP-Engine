//
// Created by hungpm on 10/06/2024.
//
#include "Engine/Simple2SourceObserverEngine.h"
#include "Enclave_u.h"
#include "sgx_lib.h"
#include "sgx_urts.h"
#include "utils.h"
#include <iostream>

Simple2SourceObserverEngine::Simple2SourceObserverEngine() = default;

void *Simple2SourceObserverEngine::startSource_(void *sourceEmitterPairAsVoid) {
    const auto sourceEmitterPair = static_cast<SourceEmitterPair *>(sourceEmitterPairAsVoid);
    const auto source = sourceEmitterPair->source;
    const auto emitter = sourceEmitterPair->emitter;
    source->start(*emitter);
    return nullptr;
}

void *Simple2SourceObserverEngine::enclaveResponderThread_(void *fastCallPairAsVoidP) {
    const auto* fastCallPair = static_cast<FastCallPair2ECall *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall1 = fastCallPair->fastECall1;
    FastCallStruct *fastEcall2 = fastCallPair->fastECall2;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    const uint8_t no = fastCallPair->no;

    const sgx_status_t status = EcallStartResponder2WithDecryption(fastCallPair->enclaveId, fastEcall1, fastEcall2, fastOcall, fastCallPair->callId);
    if (status == SGX_SUCCESS) {
        printf("Polling success\n");
    } else {
        printf("Polling failed\n");
        print_error_message(status);
    }

    return nullptr;
}

void *Simple2SourceObserverEngine::appResponderThread_(void *fastOCallAsVoidP) {
    const auto fastOCallStruct = static_cast<FastOCallStruct *>(fastOCallAsVoidP);

    void (*callbacks[1])(void*);
    callbacks[0] = fastOCallStruct->sinkFunc;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    FastCall_wait_decrypt(fastOCallStruct->fastOCallData, &callTable, 0);

    return nullptr;
}


void *Simple2SourceObserverEngine::observationThread_(void *observedDataAsVoidP) {
    const auto observedData = static_cast<ObservedData*>(observedDataAsVoidP);

    const auto buffer = observedData->buffer;
    observedData->startTimestamp = rdtscp();
    if (observedData->isHead) {
        while (true) {
            const int value = buffer->head;
            if (value != observedData->previousValue) {
                uint64_t timestamp = rdtscp();
                observedData->timestampCounterPoints[observedData->count] = timestamp;
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
                uint64_t timestamp = rdtscp();
                observedData->timestampCounterPoints[observedData->count] = timestamp;
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

int Simple2SourceObserverEngine::initializeEnclaves() {
    return initialize_enclave(&this->enclaveId_) == SGX_SUCCESS;
}

int Simple2SourceObserverEngine::destroyEnclaves() const {
    return sgx_destroy_enclave(this->enclaveId_) == SGX_SUCCESS;
}

int Simple2SourceObserverEngine::initializeDataStructures() {
    this->inBuffer1_ = new circular_buffer {
        new char[MAX_BUFFER_SIZE * this->inputDataSize1_],
        0,
        0,
        MAX_BUFFER_SIZE,
        this->inputDataSize1_,
        SGX_SPINLOCK_INITIALIZER,
        0
    };

    this->inBuffer2_ = new circular_buffer {
            new char[MAX_BUFFER_SIZE * this->inputDataSize2_],
            0,
            0,
            MAX_BUFFER_SIZE,
            this->inputDataSize2_,
            SGX_SPINLOCK_INITIALIZER,
            0
    };

    this->outBuffer_ = new circular_buffer {
            new char[MAX_BUFFER_SIZE * this->outputDataSize_],
            0,
            0,
            MAX_BUFFER_SIZE,
            this->outputDataSize_,
            SGX_SPINLOCK_INITIALIZER,
            0
    };

    this->inFastCallData1_.spinlock = SGX_SPINLOCK_INITIALIZER;
    this->inFastCallData1_.responderThread = 0;
    this->inFastCallData1_.data_buffer = inBuffer1_;
    this->inFastCallData1_.keepPolling = true;

    this->inFastCallData2_.spinlock = SGX_SPINLOCK_INITIALIZER;
    this->inFastCallData2_.responderThread = 0;
    this->inFastCallData2_.data_buffer = inBuffer2_;
    this->inFastCallData2_.keepPolling = true;

    this->outFastCallData_.spinlock = SGX_SPINLOCK_INITIALIZER;
    this->outFastCallData_.responderThread = 0;
    this->outFastCallData_.data_buffer = outBuffer_;
    this->outFastCallData_.keepPolling = true;

    fastCallPair_.no = 1;
    fastCallPair_.enclaveId = this->enclaveId_;
    fastCallPair_.fastECall1 = &inFastCallData1_;
    fastCallPair_.fastECall2 = &inFastCallData2_;
    fastCallPair_.fastOCall = &outFastCallData_;
    fastCallPair_.callId = this->taskId_;

    this->tailObserveredData1_.observedThread = 0;
    this->tailObserveredData1_.buffer = this->inBuffer1_;
    this->tailObserveredData1_.isHead = false;
    this->tailObserveredData1_.count = 0;
    this->tailObserveredData1_.keepPolling = true;
    this->tailObserveredData1_.previousValue = 0;
    this->tailObserveredData1_.noItem = new uint64_t[60000000];
    this->tailObserveredData1_.timestampCounterPoints = new uint64_t[60000000];

    this->tailObserveredData2_.observedThread = 0;
    this->tailObserveredData2_.buffer = this->inBuffer2_;
    this->tailObserveredData2_.isHead = false;
    this->tailObserveredData2_.count = 0;
    this->tailObserveredData2_.keepPolling = true;
    this->tailObserveredData2_.previousValue = 0;
    this->tailObserveredData2_.noItem = new uint64_t[60000000];
    this->tailObserveredData2_.timestampCounterPoints = new uint64_t[60000000];

    this->headObserveredData_.observedThread = 0;
    this->headObserveredData_.buffer = this->outBuffer_;
    this->headObserveredData_.isHead = true;
    this->headObserveredData_.count = 0;
    this->headObserveredData_.keepPolling = true;
    this->headObserveredData_.previousValue = 0;
    this->headObserveredData_.noItem = new uint64_t[60000000];
    this->headObserveredData_.timestampCounterPoints = new uint64_t[60000000];

    return 0;
}

void Simple2SourceObserverEngine::setSource1(Source &source) {
    this->source1_ = &source;
}

void Simple2SourceObserverEngine::setSource2(Source &source) {
    this->source2_ = &source;
}

void Simple2SourceObserverEngine::setEmitter(FastCallEmitter &emitter1, FastCallEmitter &emitter2) {
    this->emitter1_ = &emitter1;
    this->emitter2_ = &emitter2;
}

void Simple2SourceObserverEngine::setTask(uint16_t callId, uint16_t inputDataSize1, uint16_t inputDataSize2) {
    this->taskId_ = callId;
    this->inputDataSize1_ = inputDataSize1;
    this->inputDataSize2_ = inputDataSize2;
}

void Simple2SourceObserverEngine::setSink(void (*sink)(void *), uint16_t outputDataSize) {
    this->sink_ = sink;
    this->outputDataSize_ = outputDataSize;
}

int Simple2SourceObserverEngine::start() {
    // validate before starting
    if (this->source1_ == nullptr || this->source2_ == nullptr || this->taskId_ == 0 || this->sink_ == nullptr || this->emitter1_ == nullptr || this->emitter2_ == nullptr)
        return -1;

    printf("Start\n");
    const int initializationResult = initializeEnclaves();
    std::cout << "Initialized " <<  initializationResult <<  " enclaves\n";
    this->initializeDataStructures();

    pthread_attr_t enclaveAttr;
    cpu_set_t enclaveCpu;
    pthread_attr_t sourceAttr1;
    cpu_set_t sourceCpu1;
    pthread_attr_t sourceAttr2;
    cpu_set_t sourceCpu2;
    pthread_attr_t sinkAttr;
    cpu_set_t sinkCpu;
    pthread_attr_t tailObserverAttr1;
    cpu_set_t tailObserverCpu1;
    pthread_attr_t tailObserverAttr2;
    cpu_set_t tailObserverCpu2;
    pthread_attr_t headObserverAttr;
    cpu_set_t headObserverCpu;
//    pthread_attr_t headObserverAttr2;
//    cpu_set_t headObserverCpu2;

    pthread_attr_init(&tailObserverAttr1);
    CPU_ZERO(&tailObserverCpu1);
    CPU_SET(6, &tailObserverCpu1);
    pthread_attr_setaffinity_np(&tailObserverAttr1, sizeof(cpu_set_t), &tailObserverCpu1);
    pthread_create(&this->tailObserveredData1_.observedThread, &tailObserverAttr1, Simple2SourceObserverEngine::observationThread_, &this->tailObserveredData1_);

    pthread_attr_init(&tailObserverAttr2);
    CPU_ZERO(&tailObserverCpu2);
    CPU_SET(5, &tailObserverCpu2);
    pthread_attr_setaffinity_np(&tailObserverAttr2, sizeof(cpu_set_t), &tailObserverCpu2);
    pthread_create(&this->tailObserveredData2_.observedThread, &tailObserverAttr2, Simple2SourceObserverEngine::observationThread_, &this->tailObserveredData2_);

    pthread_attr_init(&headObserverAttr);
    CPU_ZERO(&headObserverCpu);
    CPU_SET(4, &headObserverCpu);
    pthread_attr_setaffinity_np(&headObserverAttr, sizeof(cpu_set_t), &headObserverCpu);
    pthread_create(&this->headObserveredData_.observedThread, &headObserverAttr, Simple2SourceObserverEngine::observationThread_, &this->headObserveredData_);
    std::cout << "Started observer thread\n";

    pthread_attr_init(&enclaveAttr);
    CPU_ZERO(&enclaveCpu);
    CPU_SET(7, &enclaveCpu);
    pthread_attr_setaffinity_np(&enclaveAttr, sizeof(cpu_set_t), &enclaveCpu);
    pthread_create(&this->inFastCallData1_.responderThread, &enclaveAttr, Simple2SourceObserverEngine::enclaveResponderThread_, &this->fastCallPair_);
    this->inFastCallData2_.responderThread = this->inFastCallData2_.responderThread;
    std::cout << "Started enclave\n";

    this->fastOCallStruct_.fastOCallData = &this->outFastCallData_;
    this->fastOCallStruct_.sinkFunc = this->sink_;
    pthread_attr_init(&sinkAttr);
    CPU_ZERO(&sinkCpu);
    CPU_SET(3, &sinkCpu);
    pthread_attr_setaffinity_np(&sinkAttr, sizeof(cpu_set_t), &sinkCpu);
    pthread_create(&this->outFastCallData_.responderThread, &sinkAttr, Simple2SourceObserverEngine::appResponderThread_, &this->fastOCallStruct_);

    emitter1_->setFastCallData(&this->inFastCallData1_);
    SourceEmitterPair sourceEmitterPair1 = { this->source1_, this->emitter1_ };
    pthread_attr_init(&sourceAttr1);
    CPU_ZERO(&sourceCpu1);
    CPU_SET(2, &sourceCpu1);
    pthread_attr_setaffinity_np(&sourceAttr1, sizeof(cpu_set_t), &sourceCpu1);
    pthread_create(&this->sourceThread1_, &sourceAttr1, Simple2SourceObserverEngine::startSource_, &sourceEmitterPair1);
    std::cout << "Started sources 1\n";

    emitter2_->setFastCallData(&this->inFastCallData2_);
    SourceEmitterPair sourceEmitterPair2 = { this->source2_, this->emitter2_ };
    pthread_attr_init(&sourceAttr2);
    CPU_ZERO(&sourceCpu2);
    CPU_SET(1, &sourceCpu2);
    pthread_attr_setaffinity_np(&sourceAttr2, sizeof(cpu_set_t), &sourceCpu2);
    pthread_create(&this->sourceThread2_, &sourceAttr2, Simple2SourceObserverEngine::startSource_, &sourceEmitterPair2);
    std::cout << "Started sources 2\n";

    std::cout << "Waiting for source 1...\n";
    pthread_join(this->sourceThread1_, nullptr);

    std::cout << "Waiting for source 2...\n";
    pthread_join(this->sourceThread2_, nullptr);

//    std::cout << "Sleeping...\n";
//    sleep(60);
    std::cout << "Waiting for enclave...\n";
    StopFastCallResponder(&this->inFastCallData1_);
    StopFastCallResponder(&this->inFastCallData2_);
    pthread_join(this->inFastCallData1_.responderThread, nullptr);

    std::cout << "Waiting for sink...\n";
    StopFastCallResponder(&this->outFastCallData_);
    pthread_join(this->outFastCallData_.responderThread, nullptr);

    std::cout << "Waiting for observer thread...\n";
    this->tailObserveredData1_.keepPolling = false;
    pthread_join(this->tailObserveredData1_.observedThread, nullptr);
    this->tailObserveredData2_.keepPolling = false;
    pthread_join(this->tailObserveredData2_.observedThread, nullptr);
    this->headObserveredData_.keepPolling = false;
    pthread_join(this->headObserveredData_.observedThread, nullptr);

    const int destroyEnclaveResult = destroyEnclaves();
    std::cout << "Destroyed " <<  destroyEnclaveResult <<  " enclaves\n";

    std::cout << "DONE\n";
    return 0;
}

void Simple2SourceObserverEngine::clean() {
    delete[] static_cast<char *>(this->inBuffer1_->buffer);
    delete[] static_cast<char *>(this->inBuffer2_->buffer);
    delete[] static_cast<char *>(this->outBuffer_->buffer);
}
