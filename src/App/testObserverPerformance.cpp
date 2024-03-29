//
// Created by hungpm on 27/03/2024.
//

#include "App/test.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include <chrono>

#include "Source/CsvSource.h"
#include "data_types.h"
#include "Engine/EngineWithBufferObserver.h"
#include "App/utils.h"

void testObserverPerformance_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }
    // const auto flight = static_cast<FlightData*> (rawData);
    // printf("Sink result: (%s %d)\n", flight->uniqueCarrier, flight->arrDelay);

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    printf(
        "Sink result:\n\t- Unique carrier: %s\n\t- Count: %d\n\t- Total: %d\n\n",
        reducedFlight->uniqueCarrier,
        reducedFlight->count,
        reducedFlight->total
        );
}

void writeBufferObserverMeasurementToFile(const std::string& pathToFile, const EngineWithBufferObserver::ObservedData& observedData) {
    std::ofstream measurementFile;
    measurementFile.open(pathToFile, std::ios::app);

    for (size_t i = 0; i < observedData.count; ++i) {
        if (i == 0) {
            measurementFile << observedData.noItem[i] << " " << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.startTime).count() << std::endl;
        } else {
            measurementFile << observedData.noItem[i] << " " << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.timePoints[i-1]).count() << std::endl;
        }
    }

    measurementFile.close();
}

void testObserverPerformance() {
    FastCallEmitter emitter;
    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 1000);

    EngineWithBufferObserver engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    engine.addTask(4, 200, false);
    engine.addTask(5, sizeof(FlightData), false);
    engine.addTask(6, sizeof(ReducedFlightData), true);

    engine.setSink(testObserverPerformance_sinkResult, sizeof(FlightData));
    engine.start();

    const std::string measurementDirName = createMeasurementsDirectory("../../measurements/testBufferObserver");
    const size_t nBuffer = engine.getBufferCount();

    for (size_t i = 0; i < nBuffer; ++i) {
        if (!engine.isObserved(static_cast<int>(i))) {
            continue;
        }

        std::string tailFilename = "Tail_Buffer_" + std::to_string(i);
        std::string tailFileFullPath = measurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << i << std::endl;
        writeBufferObserverMeasurementToFile(tailFileFullPath, engine.getTailObservedData(static_cast<int>(i)));

        std::string headFilename = "Head_Buffer_" + std::to_string(i+1);
        std::string headFileFullPath = measurementDirName;
        headFileFullPath.append("/").append(headFilename);

        std::cout << "Writing measurements for head buffer " << i+1 << std::endl;
        writeBufferObserverMeasurementToFile(headFileFullPath, engine.getHeadObservedData(static_cast<int>(i+1)));
    }

    printf("Info: Engine successfully returned.\n");
}