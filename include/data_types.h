#ifndef DATA_TYPES_
#define DATA_TYPES_

#include <inttypes.h>

struct FlightData {
    char uniqueCarrier[10];
    int arrDelay;
};

struct ReducedFlightData {
    char uniqueCarrier[10];
    uint32_t count;
    int total;
};

struct JoinedFlightData {
    char uniqueCarrier1[10];
    char uniqueCarrier2[10];
    int arrDelay;
};

#endif