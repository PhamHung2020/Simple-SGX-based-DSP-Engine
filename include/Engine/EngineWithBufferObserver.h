//
// Created by hungpm on 27/03/2024.
//

#ifndef ENGINEWITHBUFFEROBSERVER_H
#define ENGINEWITHBUFFEROBSERVER_H

#include "SimpleEngine.h"
#include "DataStructure/circular_buffer.h"
#include <chrono>
#include <vector>

#define MAX_ITEM 10000

class EngineWithBufferObserver : public SimpleEngine {
public:

    typedef struct {
        pthread_t observedThread;
        circular_buffer* buffer;
        bool isHead;
        uint16_t previousValue;
        uint16_t noItem[MAX_ITEM];
        std::chrono::_V2::system_clock::time_point timePoints[MAX_ITEM];
        uint16_t count;
        bool keepPolling;
        std::chrono::_V2::system_clock::time_point startTime;
    } ObservedData;

    ObservedData& getHeadObservedData(int index);
    ObservedData& getTailObservedData(int index);

    int start() override;

protected:
    std::vector<ObservedData> headOservedDatas_;
    std::vector<ObservedData> tailOservedDatas_;

    static void* observationThread_(void* observedDataAsVoidP);
    int initializeDataStructures() override;
};

#endif //ENGINEWITHBUFFEROBSERVER_H
