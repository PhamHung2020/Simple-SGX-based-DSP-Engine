//
// Created by hungpm on 14/02/2025.
//

#include <iostream>
#include "StreamBox/sinks.h"

std::ofstream streamBoxSinkFileStream;

std::ofstream* getStreamBoxSinkFileStream() {
    return &streamBoxSinkFileStream;
}

void sinkSensorAggregationData(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto data = static_cast<SensorAggregationData*>(rawData);
    streamBoxSinkFileStream << data->temperature << "," << data->humidity << "," << data->light << "," << data->voltage << std::endl;
}

void sinkSyntheticData(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto data = static_cast<SyntheticData*>(rawData);
    streamBoxSinkFileStream << data->key << " " << data->value << std::endl;

}

void sinkSyntheticJoinData(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto data = static_cast<SyntheticJoinData*>(rawData);
    streamBoxSinkFileStream << data->data1.key << "," << data->data1.value << "," << data->data2.key << "," << data->data2.value << std::endl;
}