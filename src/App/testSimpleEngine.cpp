//
// Created by hungpm on 28/03/2024.
//

#include "App/test.h"

#include <iostream>
#include <cstdio>
#include <fstream>
#include <chrono>

#include "Source/CsvSource.h"
#include "data_types.h"
#include "Engine/SimpleEngine.h"

void testSimpleEngine_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    // const auto flightData = static_cast<FlightData*>(rawData);
    // printf("Sink result:\n\t- Unique carrier: %s\n\t- Delay: %d\n\n", flightData->uniqueCarrier, flightData->arrDelay);

    // const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    // printf(
    //     "Sink result:\n\t- Unique carrier: %s\n\t- Count: %d\n\t- Total: %d\n\n",
    //     reducedFlight->uniqueCarrier,
    //     reducedFlight->count,
    //     reducedFlight->total
    //     );

    const auto joinedFlightData = static_cast<JoinedFlightData*> (rawData);
    printf(
        "Sink result:\n\t- Unique carrier 1: %s\n\t- Unique carrier 2: %s\n\t- Delay: %d\n\n",
        joinedFlightData->uniqueCarrier1,
        joinedFlightData->uniqueCarrier2,
        joinedFlightData->arrDelay
        );
}

void testSimpleEngine() {
    FastCallEmitter emitter;
    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 100);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    // map
    engine.addTask(4, 200);

    // filter
    engine.addTask(5, sizeof(FlightData));

    // reduce
    // engine.addTask(6, sizeof(FlightData));

    // join
    engine.addTask(7, sizeof(FlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(JoinedFlightData));
    engine.start();

    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
    // std::cout << "Pipline time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;

    const int nTask = engine.getNumberOfTask();
    for (int i = 0; i < nTask; ++i) {
        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
    }

    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    printf("Info: Engine successfully returned.\n");
}