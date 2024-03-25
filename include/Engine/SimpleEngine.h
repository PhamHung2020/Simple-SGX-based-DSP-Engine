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
    std::vector<pthread_t> enclaveThreads_;
    std::vector<circular_buffer> buffers_;
    std::vector<FastCallStruct> fastCallDatas_;
    FastCallEmitter emitter_;

    void* startSource_(void* sourceAsVoid);
public:
    SimpleEngine();
    void setSource(Source &source);
    void addOperator(uint16_t callId);
    void setSink(void (*sink) (void*));
    int start();
};



#endif //SIMPLEENGINE_H
