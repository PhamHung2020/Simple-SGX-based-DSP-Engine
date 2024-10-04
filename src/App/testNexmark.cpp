//
// Created by hungpm on 06/06/2024.
//
#include "App/test.h"
#include "Source/CsvSource.h"
#include "Engine/EngineWithBufferObserver.h"
#include "Source/FastCallEncryptedEmitter.h"
#include "App/utils.h"
#include "Nexmark/schemas.h"

#include <iostream>
#include <fstream>

namespace TestNexmark {
    std::ofstream nexmarkSinkFileStream;

    void writeMeasurementToFile(const std::string& pathToFile, const EngineWithBufferObserver::ObservedData& observedData) {
        std::ofstream measurementFile;
        measurementFile.open(pathToFile, std::ios::out);
        std::cout << "Count " << observedData.count << std::endl;

        measurementFile << "index,time\n";

        for (uint64_t i = 0; i < observedData.count; ++i) {
            if (i == 0) {
                measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[0] - observedData.startTimestamp << std::endl;
            } else {
                measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[i] - observedData.timestampCounterPoints[i-1] << std::endl;
            }
        }

        measurementFile.close();
    }

    void sinkBid(void* rawData) {
        if (rawData == NULL) {
            return;
        }

        const auto bid = static_cast<Bid*>(rawData);
        nexmarkSinkFileStream << bid->auction << "," << bid->bidder << "," << bid->price << "," << bid->datetime << std::endl;
    }
}

void testNexmark(const ConfigurationTesting& config) {
    // ================ Set up engine ====================
    CsvSource source(config.sourceId, config.sourceFileName, config.sourceDelay, config.sourceHasHeader, config.sourceCount);
    source.setParser(config.parser);

    EngineWithBufferObserver engine;
    engine.setSource(source);

//    FastCallEmitter emitter;
    FastCallEncryptedEmitter emitter;
    engine.setEmitter(emitter);

    engine.addTask(config.taskId, config.taskInputDataSize, config.taskShouldBeObserved);
    engine.setSink(config.sink, config.outputDataSize);
//    engine.setSink(TestNexmark::sinkBid, config.outputDataSize);

// =================== Create directory and file to store processed results =========================
    std::string fileFullPath;
    if (config.resultDirName == "../../results/testNexmark") {
        fileFullPath = createMeasurementsDirectory(config.resultDirName);
    } else {
        fileFullPath = config.resultDirName;
    }

    fileFullPath.append("/").append(config.sinkFileName);
    config.sinkFileStream->open(fileFullPath, std::ios::out);
    if (config.sinkFileStream->fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    // ======================= Create measurement folder =======================
    std::string createdMeasurementDirName;
    if (config.measurementDirName == "../../measurements/testNexmark") {
        createdMeasurementDirName = createMeasurementsDirectory(config.measurementDirName);
    } else {
        createdMeasurementDirName = config.measurementDirName;
    }

    // ======================= Start engine ==========================
    engine.start();

    config.sinkFileStream->close();

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

        std::string tailFilename = "tail_" + std::to_string(i) + "_" + config.measurementFileName;
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << i << std::endl;
        TestNexmark::writeMeasurementToFile(tailFileFullPath, engine.getTailObservedData(static_cast<int>(i)));

        std::string headFilename = "head_" + std::to_string(i+1) + "_" + config.measurementFileName;
        std::string headFileFullPath = createdMeasurementDirName;
        headFileFullPath.append("/").append(headFilename);

        std::cout << "Writing measurements for head buffer " << i << std::endl;
        TestNexmark::writeMeasurementToFile(headFileFullPath, engine.getHeadObservedData(static_cast<int>(i+1)));
    }

    printf("Info: Engine successfully returned.\n");

    engine.clean();
}