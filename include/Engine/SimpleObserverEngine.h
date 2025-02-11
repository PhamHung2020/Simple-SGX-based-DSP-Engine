//
// Created by hungpm on 10/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_SIMPLEOBSERVERENGINE_H
#define SIMPLE_DSP_ENGINE_SIMPLEOBSERVERENGINE_H

#include "SimpleEngine.h"
#include "DataStructure/circular_buffer.h"
#include <chrono>
#include <vector>

class SimpleObserverEngine : public SimpleEngine {
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
private:

};

#endif //SIMPLE_DSP_ENGINE_SIMPLEOBSERVERENGINE_H
