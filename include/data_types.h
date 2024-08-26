#ifndef DATA_TYPES_
#define DATA_TYPES_

#include <inttypes.h>

struct FlightData {
    char uniqueCarrier[8];
    int arrDelay;
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