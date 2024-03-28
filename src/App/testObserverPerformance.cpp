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

void testObserverPerformance_sinkResult(void* rawData)
{
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

void testObserverPerformance() {
    FastCallEmitter emitter;
    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 200);

    EngineWithBufferObserver engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    engine.addTask(4, 200);
    engine.addTask(5, sizeof(FlightData));
    engine.addTask(6, sizeof(ReducedFlightData));

    engine.setSink(testObserverPerformance_sinkResult, sizeof(FlightData));
    engine.start();

    const EngineWithBufferObserver::ObservedData observedData = engine.getTailObservedData(1);
    printf("Count: %d\n", observedData.count);
    for (int i = 0; i < observedData.count; ++i) {
        if (i == 0)
            printf("%d %ld\n", observedData.noItem[i], std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.startTime).count());
        else
            printf("%d %ld\n", observedData.noItem[i], std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.timePoints[i-1]).count());
    }

    printf("Info: Engine successfully returned.\n");
}