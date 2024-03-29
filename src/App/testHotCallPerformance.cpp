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
#include "Engine/EngineWithHotCallPerformance.h"
#include "App/utils.h"

void testHotCallPerformance_sinkResult(void* rawData)
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

void testHotCallPerformance() {

    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 200);
    FastCallEmitter emitter;

    EngineWithHotCallPerformance engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    engine.addTask(4, 200, false);
    engine.addTask(5, sizeof(FlightData), false);
    engine.addTask(6, sizeof(ReducedFlightData), true);

    engine.setSink(testHotCallPerformance_sinkResult, sizeof(FlightData));
    engine.start();

    const auto hotCallPerformances = engine.getHotCallPerformanceParams();
    std::string measurementDirName = createMeasurementsDirectory("../../measurements/testHotCallPerformance");
    for (size_t i = 0; i < hotCallPerformances.size(); ++i) {
        if (!engine.isWithHotCall(static_cast<int>(i))) {
            continue;
        }

        std::string filename = "Enclave_" + std::to_string(i);
        std::string fileFullPath = measurementDirName;
        fileFullPath.append("/").append(filename);

        std::ofstream measurementFile;
        measurementFile.open(fileFullPath, std::ios::app);

        std::cout << "Writing measurements for enclave " << i << std::endl;
        for (size_t j = 0; j < hotCallPerformances[i].endTimes.size(); ++j) {
            measurementFile << std::chrono::duration_cast<std::chrono::nanoseconds>(hotCallPerformances[i].endTimes[j] - hotCallPerformances[i].startTimes[j]).count() << "ns\n";
        }

        measurementFile.close();
    }

    printf("Info: Engine successfully returned.\n");
}
