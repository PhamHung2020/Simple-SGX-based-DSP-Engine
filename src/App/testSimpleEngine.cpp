//
// Created by hungpm on 28/03/2024.
//

#include "App/test.h"

#include <iostream>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <fstream>

#include "Source/CsvSource.h"
#include "data_types.h"
#include "Engine/SimpleEngine.h"
#include "Source/FastCallEncryptedEmitter.h"
#include "Source/Parser.h"
#include "App/utils.h"
#include "Crypto/aes_gcm.h"

std::string resultDirName = "../../results/callbacks/2024-07-04_10-50-22";
std::string sinkFileName = "map.csv";
std::string sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
//std::string sourceFileName = "../../results/callbacks/2024-04-09_14-21-15/map.csv";
std::ofstream fout;

void testSimpleEngine_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*>(rawData);
    fout << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;

//    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
//    fout << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;

//    const auto joinedFlightData = static_cast<JoinedFlightData*>(rawData);
//    fout << joinedFlightData->uniqueCarrier1 << "," << joinedFlightData->uniqueCarrier2 << "," << joinedFlightData->arrDelay << std::endl;
}

void testSimpleEngine() {
//    FlightDataParser flightDataParser;
//    FlightDataIntermediateParser parser;
//    FastCallEmitter emitter;
    FastCallEncryptedEmitter emitter;
//    emitter.setDataLength(200);
    CsvSource source1(1, sourceFileName, 0, true, 1000000);
//    source1.setParser(&parser);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    // map
    engine.addTask(4, SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 200);

    // filter
//    engine.addTask(5, sizeof(FlightData));

    // reduce
//     engine.addTask(6, sizeof(FlightData));

    // join
//    engine.addTask(7, sizeof(FlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE);

    // create directory and file to store processed results
    std::string fileFullPath;
    if (resultDirName == "../../results/callbacks") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    fout.open(fileFullPath, std::ios::out);
    if (fout.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    engine.start();

    fout.close();

    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
    std::cout << "Pipeline time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;

    const int nTask = engine.getNumberOfTask();
    for (int i = 0; i < nTask; ++i) {
        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
    }

    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    printf("Info: Engine successfully returned.\n");

    engine.clean();
}