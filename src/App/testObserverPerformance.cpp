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

void testObserverPerformance_sinkResult_map_filter(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*>(rawData);
    testObserverPerformance_sinkFileStream << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;
}

void testObserverPerformance_sinkResult_reduce(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    testObserverPerformance_sinkFileStream << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;
}

void testObserverPerformance_sinkResult_join(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto joinedFlightData = static_cast<JoinedFlightData*>(rawData);
    testObserverPerformance_sinkFileStream << joinedFlightData->uniqueCarrier1 << "," << joinedFlightData->uniqueCarrier2 << "," << joinedFlightData->arrDelay << std::endl;
}

void writeBufferObserverMeasurementToFile(const std::string& pathToFile, const EngineWithBufferObserver::ObservedData& observedData) {
    std::ofstream measurementFile;
    measurementFile.open(pathToFile, std::ios::app);

    for (uint64_t i = 0; i < observedData.count; ++i) {
        if (i == 0) {
            measurementFile << observedData.noItem[i] << "," << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.startTime).count() << std::endl;
        } else {
            measurementFile << observedData.noItem[i] << "," << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData.timePoints[i] - observedData.timePoints[i-1]).count() << std::endl;
        }
    }

    measurementFile.close();
}

void testObserverPerformance(int n) {
    //  =============== Define variables =================
//    std::string resultDirName = "../../results/testBufferObserver";
    std::string resultDirName = "../../results/test";
    std::string sinkFileName = std::to_string(n) +  "_reduce.csv";
    std::string sourceFileName = "../../results/test/" + std::to_string(n) + "_filter.csv";
//    std::string sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
//    std::string measurementDirName = "../../measurements/testBufferObserver";
    std::string measurementDirName = "../../measurements/test";
    std::string measurementFileName = "map.csv";
//    std::string throughoutFileName = "throughout_reduce.csv";

    // ================ Set up engine ====================
    CsvSource source1(1, sourceFileName, 0, false, 1000000);
    FlightDataIntermediateParser parser;
    source1.setParser(&parser);

    EngineWithBufferObserver engine;
    engine.setSource(source1);

    FastCallEmitter emitter;
    engine.setEmitter(emitter);

//    engine.addTask(4, 200, true);
//    engine.addTask(5, sizeof(FlightData), true);
    engine.addTask(6, sizeof(FlightData), true);
//    engine.addTask(7, sizeof(FlightData), true);

    engine.setSink(testObserverPerformance_sinkResult_reduce, sizeof(ReducedFlightData));

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

    testObserverPerformance_sinkFileStream.close();

    // ======================= Create measurement folder =======================
    std::string createdMeasurementDirName;
    if (measurementDirName == "../../measurements/testBufferObserver") {
        createdMeasurementDirName = createMeasurementsDirectory(measurementDirName);
    } else {
        createdMeasurementDirName = measurementDirName;
    }

    // ======================= Get throughout =======================
//    std::ofstream throughoutStream;
//    std::string throughoutFullPath = createdMeasurementDirName;
//    throughoutStream.open(throughoutFullPath.append("/").append(throughoutFileName));
//
//    std::cout << "THROUGHOUT\n";
//    std::cout << "Number of records: " << EngineWithBufferObserver::processedCountIndex << std::endl;
//    for (int i = 0; i < EngineWithBufferObserver::processedCountIndex; ++i) {
//        throughoutStream << i+1 << "," << EngineWithBufferObserver::processedPerSecond[i] << std::endl;
//    }
//    throughoutStream.close();

    // ==================== Processing Time =======================
    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
    std::cout << "Pipeline time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;

    const int nTask = engine.getNumberOfTask();
    for (int i = 0; i < nTask; ++i) {
        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
    }

    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    // ====================== Write measurements ==============================
    const size_t nBuffer = engine.getBufferCount();

    for (size_t i = 0; i < nBuffer; ++i) {
        if (!engine.isObserved(static_cast<int>(i))) {
            continue;
        }

//        std::string tailFilename = "Tail_Buffer_" + std::to_string(i);
        std::string tailFilename = std::to_string(n) + "_process_time_tail_reduce.csv";
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