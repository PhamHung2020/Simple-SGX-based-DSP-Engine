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

std::ofstream testObserverPerformance_sinkFileStream;

void testObserverPerformance_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

//    const auto flightData = static_cast<FlightData*>(rawData);
//    testObserverPerformance_sinkFileStream << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    testObserverPerformance_sinkFileStream << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;
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
    //  =============== Define variables =================
    std::string resultDirName = "../../results/testBufferObserver/2024-04-09_10-03-43";
    std::string sinkFileName = "reduce.csv";
    std::string sourceFileName = "../../results/testBufferObserver/2024-04-09_10-03-43/filter.csv";
    std::string measurementDirName = "../../measurements/testBufferObserver/2024-04-09_10-03-46";

    // ================ Set up engine ====================
    CsvSource source1(1, sourceFileName, 0, false, 10000);
    FlightDataIntermediateParser parser;
    source1.setParser(&parser);

    EngineWithBufferObserver engine;
    engine.setSource(source1);

    FastCallEmitter emitter;
    engine.setEmitter(emitter);

//    engine.addTask(4, 200, true);
//    engine.addTask(5, sizeof(FlightData), true);
    engine.addTask(6, sizeof(ReducedFlightData), true);

    engine.setSink(testObserverPerformance_sinkResult, sizeof(ReducedFlightData));

    // =================== Create directory and file to store processed results =========================
    std::string fileFullPath;
    if (resultDirName == "../../results/testBufferObserver") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    testObserverPerformance_sinkFileStream.open(fileFullPath, std::ios::out);
    if (testObserverPerformance_sinkFileStream.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    // ======================= Start engine ==========================
    engine.start();

    // ====================== Write measurements ==============================
    std::string createdMeasurementDirName;
    if (measurementDirName == "../../measurements/testBufferObserver") {
        createdMeasurementDirName = createMeasurementsDirectory(measurementDirName);
    } else {
        createdMeasurementDirName = measurementDirName;
    }
    const size_t nBuffer = engine.getBufferCount();

    for (size_t i = 0; i < nBuffer; ++i) {
        if (!engine.isObserved(static_cast<int>(i))) {
            continue;
        }

//        std::string tailFilename = "Tail_Buffer_" + std::to_string(i);
        std::string tailFilename = "Tail_Reduce";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << i << std::endl;
        writeBufferObserverMeasurementToFile(tailFileFullPath, engine.getTailObservedData(static_cast<int>(i)));

//        std::string headFilename = "Head_Buffer_" + std::to_string(i+1);
//        std::string headFilename = "Head_Map";
//        std::string headFileFullPath = createdMeasurementDirName;
//        headFileFullPath.append("/").append(headFilename);
//
//        std::cout << "Writing measurements for head buffer " << i+1 << std::endl;
//        writeBufferObserverMeasurementToFile(headFileFullPath, engine.getHeadObservedData(static_cast<int>(i+1)));
    }

    printf("Info: Engine successfully returned.\n");
}