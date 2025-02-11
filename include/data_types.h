#ifndef DATA_TYPES_
#define DATA_TYPES_

#include <inttypes.h>

struct FlightData {
    char uniqueCarrier[8];
    int arrDelay;
};

struct FlightFullData {
    uint64_t year;
    uint64_t month;
    uint64_t dayOfMonth;
    uint64_t dayOfWeek;
    uint64_t depTime;
    uint64_t arrTime;
    char uniqueCarrier[16];
    uint64_t flightNum;
    int64_t arrDelay;
    int64_t depDelay;
    uint64_t distance;
};

struct ReducedFlightData {
    char uniqueCarrier[8];
    uint32_t count;
    int total;
};

struct JoinedFlightData {
    char uniqueCarrier1[8];
    char uniqueCarrier2[8];
    int arrDelay;
};

#endif