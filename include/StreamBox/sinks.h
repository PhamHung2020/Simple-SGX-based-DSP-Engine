//
// Created by hungpm on 14/02/2025.
//

#ifndef STREAM_BOX_SINKS_H
#define STREAM_BOX_SINKS_H

#include <fstream>
#include "schemas.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern std::ofstream streamBoxSinkFileStream;

#if defined(__cplusplus)
}
#endif

std::ofstream* getStreamBoxSinkFileStream();
void sinkSensorAggregationData(void* rawData);
void sinkSyntheticData(void* rawData);
void sinkSyntheticJoinData(void* rawData);
void sinkTripData(void* rawData);
void sinkCountTripData(void* rawData);

#endif //STREAM_BOX_SINKS_H
