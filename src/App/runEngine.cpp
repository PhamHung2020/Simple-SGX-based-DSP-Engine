//
// Created by hungpm on 16/07/2024.
//

#include <iostream>

#include "App/utils.h"
#include "Engine/EngineWithBufferObserver.h"
#include "Engine/EngineWithBufferObserverCrypto.h"
#include "Source/CsvSource.h"
#include "Source/FastCallEncryptedEmitter.h"
#include "Engine/Simple2SourceObserverEngine.h"
#include "Source/StringRandomGenerationSource.h"

void runEngineWithBufferObserverCrypto(ConfigurationTesting& config, bool shouldCreateMeasurementDirectory, bool shouldCreateResultDirectory) {
    // ================ Set up engine ====================
    CsvSource source(config.sourceId, config.sourceFileName, config.sourceDelay, config.sourceHasHeader, config.sourceCount);
    source.setParser(config.parser);

//    StringRandomGenerationSource source(config.taskInputDataSize, config.sourceCount);
//    source.prepare();

    EngineWithBufferObserverCrypto engine;
    EngineWithBufferObserverCrypto::encryptInput(false);
    EngineWithBufferObserverCrypto::encryptOutput(false);
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

void runSimple2SourceObserverEngine(ConfigurationTesting& config, bool shouldCreateMeasurementDirectory, bool shouldCreateResultDirectory) {
    // ================ Set up engine ====================
    CsvSource source1(config.sourceId1, config.sourceFileName1, config.sourceDelay1, config.sourceHasHeader1, config.sourceCount1);
    source1.setParser(config.parser1);

    CsvSource source2(config.sourceId2, config.sourceFileName2, config.sourceDelay2, config.sourceHasHeader2, config.sourceCount2);
    source2.setParser(config.parser2);

    Simple2SourceObserverEngine engine;
    engine.setSource1(source1);
    engine.setSource2(source2);

    FastCallEncryptedEmitter emitter1, emitter2;
    engine.setEmitter(emitter1, emitter2);

    engine.setTask(config.taskId, config.taskInputDataSize1, config.taskInputDataSize2);
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

    // ====================== Write measurements ==============================
    std::string tailFilename = "tail_0_0_process_time_" + config.measurementFileName;
    std::string tailFileFullPath = createdMeasurementDirName;
    tailFileFullPath.append("/").append(tailFilename);

    std::cout << "Writing measurements for tail buffer 1" << std::endl;
    writeObservedMeasurementToFile(tailFileFullPath, engine.tailObservedData1_);

    tailFilename = "tail_0_1_process_time_" + config.measurementFileName;
    tailFileFullPath = createdMeasurementDirName;
    tailFileFullPath.append("/").append(tailFilename);

    std::cout << "Writing measurements for tail buffer 2" << std::endl;
    writeObservedMeasurementToFile(tailFileFullPath, engine.tailObservedData2_);

    std::string headFilename = "head_1_process_time_" + config.measurementFileName;
    std::string headFileFullPath = createdMeasurementDirName;
    headFileFullPath.append("/").append(headFilename);

    std::cout << "Writing measurements for head buffer " << std::endl;
    writeObservedMeasurementToFile(headFileFullPath, engine.headObservedData_);

    printf("Info: Engine successfully returned.\n");

    engine.clean();

}