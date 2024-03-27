//
// Created by hungpm on 25/03/2024.
//

#ifndef SIMPLEENGINE_H
#define SIMPLEENGINE_H

#include <vector>

#include "fast_call.h"
#include "Source/Source.h"
#include "Source/FastCallEmitter.h"
#include "DataStructure/circular_buffer.h"

#define MAX_BUFFER_SIZE 128

class SimpleEngine {

protected:
    Source* source_ = nullptr;
    std::vector<uint16_t> callIdVector_;
    std::vector<uint16_t> dataSizeVector_;
    void (*sink_) (void*) = nullptr;
    FastCallEmitter* emitter_;

    pthread_t sourceThread_;
    std::vector<sgx_enclave_id_t> enclaveIds_;
    std::vector<pthread_t> enclaveThreads_;
    std::vector<circular_buffer> buffers_;
    std::vector<FastCallStruct> fastCallDatas_;
    std::vector<FastCallPair> fastCallPairs_;

    static void* startSource_(void* sourceEmitterPairAsVoid);
    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void* appResponserThread_(void* fastOCallAsVoidP);

    virtual int initializeEnclaves();
    virtual int destroyEnclaves() const;
    virtual int initializeDataStructures();
public:
    virtual ~SimpleEngine() = default;

    SimpleEngine();
    virtual void setSource(Source &source);
    virtual void setEmitter(FastCallEmitter &emitter);
    virtual void addTask(uint16_t callId, uint16_t inputDataSize);
    virtual void setSink(void (*sink) (void*), uint16_t outputDataSize);

    virtual int start();

    typedef struct {
        Source* source;
        Emitter* emitter;
    } SourceEmitterPair;

    typedef struct
    {
        FastCallStruct* fastOCallData;
        void (*sinkFunc) (void* data);
    } FastOCallStruct;
};

#endif //SIMPLEENGINE_H
