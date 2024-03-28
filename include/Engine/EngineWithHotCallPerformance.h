//
// Created by hungpm on 27/03/2024.
//

#ifndef ENGINEWITHHOTCALLPERFORMANCE_H
#define ENGINEWITHHOTCALLPERFORMANCE_H

#include "Engine/SimpleEngine.h"
#include "hot_call_perormance.h"

class EngineWithHotCallPerformance : public SimpleEngine {
protected:
    std::vector<HotOCallPerformanceParams> hotCallPerformances_;
    std::vector<HotCall> hotCalls_;
    std::vector<bool> withHotCall_;

    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void* enclaveResponderThreadWitHotCall_(void* fastCallPairAsVoidP);
    static void  addStartTime_(void* hotCallPerformanceAsVoidP);
    static void  addEndTime_(void* hotCallPerformanceAsVoidP);
    static void* appPerformanceThread_(void* hotCallAsVoidP);
    int initializeDataStructures() override;
public:
    void addTask(uint16_t callId, uint16_t inputDataSize) override;
    void addTask(uint16_t callId, uint16_t inputDataSize, bool withHotCall);

    virtual std::vector<HotOCallPerformanceParams>& getHotCallPerformanceParams();
    bool isWithHotCall(int index);
    int start() override;
};

#endif //ENGINEWITHHOTCALLPERFORMANCE_H
