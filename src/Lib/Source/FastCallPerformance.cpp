//
// Created by hungpm on 26/03/2024.
//

#include <stdexcept>

#include "Source/FastCallPerformanceEmitter.h"

std::chrono::_V2::system_clock::time_point FastCallPerformanceEmitter::getStartTime(const int index) const
{
    if (index < 0 || index > this->count_)
    {
        throw std::out_of_range("Index out bound");
    }
    return this->startTimesList_[index];
}

int FastCallPerformanceEmitter::getCount() const
{
    return this->count_;
}

void FastCallPerformanceEmitter::emit(void *data)
{
    FastCallEmitter::emit(data);
    this->startTimesList_[this->count_] = std::chrono::high_resolution_clock::now();
    this->count_++;
}
