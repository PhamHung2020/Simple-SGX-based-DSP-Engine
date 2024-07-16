//
// Created by hungpm on 10/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_SIMPLE2SOURCEOBSERVERENGINE_H
#define SIMPLE_DSP_ENGINE_SIMPLE2SOURCEOBSERVERENGINE_H

#include "SimpleObserverEngine.h"
#include "DataStructure/circular_buffer.h"
#include <chrono>
#include <vector>

#define MAX_BUFFER_SIZE 2000000

class Simple2SourceObserverEngine {
public:
    typedef struct {
        pthread_t observedThread;
        circular_buffer* buffer;
        bool isHead;
        uint64_t previousValue;
        uint64_t* noItem;
        std::chrono::_V2::system_clock::time_point* timePoints;
        uint64_t* timestampCounterPoints;
        uint64_t count;
        bool keepPolling;
        std::chrono::_V2::system_clock::time_point startTime;
        uint64_t startTimestamp;
    } ObservedData;

    typedef struct {
        Source* source;
        Emitter* emitter;
    } SourceEmitterPair;

    typedef struct
    {
        FastCallStruct* fastOCallData;
        void (*sinkFunc) (void* data);
    } FastOCallStruct;

    virtual ~Simple2SourceObserverEngine() = default;

    Simple2SourceObserverEngine();
    virtual void setSource1(Source &source);
    virtual void setSource2(Source &source);
    virtual void setEmitter(FastCallEmitter &emitter1, FastCallEmitter &emitter2);
    virtual void setTask(uint16_t callId, uint16_t inputDataSize1, uint16_t inputDataSize2);
    virtual void setSink(void (*sink) (void*), uint16_t outputDataSize);

    virtual int start();
    virtual void clean();

    ObservedData tailObserveredData1_;
    ObservedData tailObserveredData2_;
    ObservedData headObserveredData_;
protected:
    Source* source1_ = nullptr;
    Source* source2_ = nullptr;
    uint16_t taskId_ = 0;
    uint16_t inputDataSize1_ = 0;
    uint16_t inputDataSize2_ = 0;
    uint16_t outputDataSize_ = 0;
    void (*sink_) (void*) = nullptr;
    FastCallEmitter* emitter1_ = nullptr;
    FastCallEmitter* emitter2_ = nullptr;

    pthread_t sourceThread1_ = 0;
    pthread_t sourceThread2_ = 0;
    sgx_enclave_id_t enclaveId_ = 0;
    circular_buffer* inBuffer1_ = nullptr;
    circular_buffer* inBuffer2_ = nullptr;
    circular_buffer* outBuffer_ = nullptr;

    FastCallStruct inFastCallData1_{};
    FastCallStruct inFastCallData2_{};
    FastCallStruct outFastCallData_{};
    FastOCallStruct fastOCallStruct_{};

    FastCallPair2ECall fastCallPair_{};

    static bool shouldContinue_;

    static void* startSource_(void* sourceEmitterPairAsVoid);
    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void* appResponderThread_(void* fastOCallAsVoidP);
    static void* observationThread_(void* observedDataAsVoidP);

    virtual int initializeEnclaves();
    virtual int destroyEnclaves() const;
    virtual int initializeDataStructures();
};

#endif //SIMPLE_DSP_ENGINE_SIMPLE2SOURCEOBSERVERENGINE_H
