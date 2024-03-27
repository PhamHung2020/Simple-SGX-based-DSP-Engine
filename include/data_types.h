#ifndef DATA_TYPES_
#define DATA_TYPES_

#include <inttypes.h>

typedef struct
{
    double timestamp;
    int sourceId;
    int key;
    int data;
    char message[32];
} MyEvent;

typedef struct {
    MyEvent event1;
    MyEvent event2;
} JoinResult;

struct HotOCallParams
{
    // JoinResult joinResult;
    int reduceResult;
    MyEvent eventResult;
};

struct FlightData {
    char uniqueCarrier[10];
    int arrDelay;
};

struct ReducedFlightData {
    char uniqueCarrier[10];
    uint32_t count;
    int total;
};

#endif