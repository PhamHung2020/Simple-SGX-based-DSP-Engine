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

private:
    Source* source_ = nullptr;
    std::vector<uint16_t> callIdVector_;
    void (*sink_) (void*) = nullptr;

    pthread_t sourceThread_;
    std::vector<sgx_enclave_id_t> enclaveIds_;
    std::vector<pthread_t> enclaveThreads_;
    std::vector<circular_buffer> buffers_;
    // circular_buffer buffers_[10];
    std::vector<FastCallStruct> fastCallDatas_;
    std::vector<FastCallPair> fastCallPairs_;
    FastCallEmitter emitter_;

    static void* startSource_(void* sourceEmitterPairAsVoid);
    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void* appResponserThread_(void* fastOCallAsVoidP);
    int initializeEnclaves();
    int destroyEnclaves();
public:
    SimpleEngine();
    void setSource(Source &source);
    void addOperator(uint16_t callId);
    void setSink(void (*sink) (void*));
    int start();

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
