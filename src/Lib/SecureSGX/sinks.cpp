//
// Created by hungpm on 23/07/2024.
//

#include "SecureSGX/sinks.h"
#include "data_types.h"

std::ofstream secureSgxSinkFileStream;

std::ofstream* getSecureSgxSinkFileStream() {
    return &secureSgxSinkFileStream;
}

void sinkMap(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*>(rawData);
    secureSgxSinkFileStream << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;
}

void sinkFlightFullData(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightFullData*>(rawData);
    secureSgxSinkFileStream
        << flightData->year << ","
        << flightData->month << ","
        << flightData->dayOfMonth << ","
        << flightData->dayOfWeek << ","
        << flightData->depTime << ","
        << flightData->arrTime << ","
        << flightData->uniqueCarrier << ","
        << flightData->flightNum << ","
        << flightData->arrDelay << ","
        << flightData->depDelay << ","
        << flightData->distance << std::endl;
}

void sinkReduce(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    secureSgxSinkFileStream << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;
}