//
// Created by hungpm on 27/03/2024.
//

#ifndef ENGINE_WITH_BUFFER_OBSERVER_H
#define ENGINE_WITH_BUFFER_OBSERVER_H

#include "SimpleEngine.h"
#include "DataStructure/circular_buffer.h"
#include <chrono>
#include <vector>

#define MAX_ITEM 60000000

class EngineWithBufferObserver : public SimpleEngine {
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

    ObservedData& getHeadObservedData(int index);
    ObservedData& getTailObservedData(int index);
    size_t getBufferCount() const;

    int start() override;

    void addTask(uint16_t callId, uint16_t inputDataSize, bool shouldObserved);
    void addTask(uint16_t callId, uint16_t inputDataSize) override;
    bool isObserved(int index);

    static uint64_t processedPerSecond[1];
    static int processedCountIndex;
    void clean() override;

protected:
    std::vector<ObservedData> headObservedDataList_;
    std::vector<ObservedData> tailObservedDataList_;
    std::vector<bool> shouldObserved_;
    pthread_t processedCountThreadId_;
    static bool shouldContinue_;

    static void* observationThread_(void* observedDataAsVoidP);
    static void* processedCountThread_(void* circularBufferAsVoidP);

    int initializeDataStructures() override;
};

#endif //ENGINE_WITH_BUFFER_OBSERVER_H
