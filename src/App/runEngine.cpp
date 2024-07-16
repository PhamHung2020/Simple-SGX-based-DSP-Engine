//
// Created by hungpm on 16/07/2024.
//

#include <iostream>

#include "App/utils.h"
#include "Engine/EngineWithBufferObserver.h"
#include "Engine/EngineWithBufferObserverCrypto.h"
#include "Source/CsvSource.h"

void runEngineWithBufferObserverCrypto(ConfigurationTesting& config, bool shouldCreateMeasurementDirectory, bool shouldCreateResultDirectory) {
    // ================ Set up engine ====================
    CsvSource source(config.sourceId, config.sourceFileName, config.sourceDelay, config.sourceHasHeader, config.sourceCount);
    source.setParser(config.parser);

    EngineWithBufferObserverCrypto engine;
    EngineWithBufferObserverCrypto::encryptInput(true);
    EngineWithBufferObserverCrypto::encryptOutput(true);
    engine.setSource(source);

    engine.addTask(config.taskId, config.taskInputDataSize, config.taskShouldBeObserved);
    engine.setSink(config.sink, config.outputDataSize);

    // =================== Create directory and file to store processed results =========================
    std::string fileFullPath;
    if (shouldCreateResultDirectory) {
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
    if (shouldCreateMeasurementDirectory) {
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
        writeObservedMeasurementToFile(tailFileFullPath, engine.getTailObservedData(static_cast<int>(i)));

        std::string headFilename = "head_" + std::to_string(i+1) + "_" + config.measurementFileName;
        std::string headFileFullPath = createdMeasurementDirName;
        headFileFullPath.append("/").append(headFilename);

        std::cout << "Writing measurements for head buffer " << i << std::endl;
        writeObservedMeasurementToFile(headFileFullPath, engine.getHeadObservedData(static_cast<int>(i+1)));
    }

    std::cout << "Info: Engine successfully returned.\n";

    engine.clean();
}
