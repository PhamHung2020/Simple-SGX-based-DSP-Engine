//
// Created by hungpm on 14/02/2025.
//

#ifndef STREAM_BOX_SCHEMAS_H
#define STREAM_BOX_SCHEMAS_H

#include <inttypes.h>

struct SensorData {
    char date[16];
    char time[16];
    uint64_t epoch;
    uint64_t moteId;
    double temperature;
    double humidity;
    double light;
    double voltage;
};

struct SensorAggregationData {
    double temperature;
    double humidity;
    double light;
    double voltage;
};

struct SyntheticData {
    int64_t key;
    int64_t value;
};

struct SyntheticJoinData {
    SyntheticData data1;
    SyntheticData data2;
};

struct TripData {
    char medallion[64];
    char hackLicense[64];
    char pickupDateTime[64];
    char dropOffDateTime[64];
    uint64_t tripTimeInSecs;
    double tripDistance;
    double pickupLongitude;
    double pickupLatitude;
    double dropOffLongitude;
    double dropOffLatitude;
};

struct CountTripData {
    char medallion[64];
    uint64_t count;
};

#endif //STREAM_BOX_SCHEMAS_H
