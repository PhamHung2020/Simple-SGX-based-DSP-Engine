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

    static void* enclaveResponderThread_(void* fastCallPairAsVoidP);
    static void  addStartTime_(void* hotCallPerformanceAsVoidP);
    static void  addEndTime_(void* hotCallPerformanceAsVoidP);
    static void* appPerformanceThread_(void* hotCallAsVoidP);
    int initializeDataStructures() override;
public:
    virtual std::vector<HotOCallPerformanceParams>& getHotCallPerformanceParams();
    int start() override;
};

#endif //ENGINEWITHHOTCALLPERFORMANCE_H
