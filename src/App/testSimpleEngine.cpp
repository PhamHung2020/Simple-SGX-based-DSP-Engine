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

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    printf(
        "Sink result:\n\t- Unique carrier: %s\n\t- Count: %d\n\t- Total: %d\n\n",
        reducedFlight->uniqueCarrier,
        reducedFlight->count,
        reducedFlight->total
        );
}

void testSimpleEngine() {
    FastCallEmitter emitter;
    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 200);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    engine.addTask(4, 200);
    engine.addTask(5, sizeof(FlightData));
    engine.addTask(6, sizeof(ReducedFlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(ReducedFlightData));
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