//
// Created by hungpm on 26/03/2024.
//

#ifndef HOT_CALL_PERORMANCE_H
#define HOT_CALL_PERORMANCE_H

#include <chrono>
#include <vector>

struct HotOCallPerformanceParams
{
    std::vector<std::chrono::_V2::system_clock::time_point> startTimes;
    std::vector<std::chrono::_V2::system_clock::time_point> endTimes;
};

// class HotOCallPerformance {
//
// };

#endif //HOT_CALL_PERORMANCE_H
