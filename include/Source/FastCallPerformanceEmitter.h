//
// Created by hungpm on 26/03/2024.
//

#ifndef FASTCALLPERFORMANCEEMITTER_H
#define FASTCALLPERFORMANCEEMITTER_H

#include "FastCallEmitter.h"
#include <chrono>

class FastCallPerformanceEmitter final : public FastCallEmitter
{
private:
    std::chrono::_V2::system_clock::time_point startTimesList_[1000];
    int count_ = 0;
public:
    std::chrono::_V2::system_clock::time_point getStartTime(int index) const;
    int getCount() const;
    void emit(void* data) override;
};

#endif //FASTCALLPERFORMANCEEMITTER_H
