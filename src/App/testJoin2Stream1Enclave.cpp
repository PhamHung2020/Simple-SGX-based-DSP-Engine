//
// Created by hungpm on 10/04/2024.
//

#include "App/test.h"
#include "Source/CsvSource.h"
#include "App/utils.h"
#include "DataStructure/circular_buffer.h"
#include "Engine/SimpleEngine.h"
#include "sgx_lib.h"
#include <iostream>
#include <sgx_urts.h>
#include <fstream>
#include "Enclave_u.h"

std::ofstream testJoin2Stream1Enclave_sinkFileStream;

struct FastCallPair2 {
    uint8_t no;
    sgx_enclave_id_t enclaveId;
    FastCallStruct* fastECall1;
    FastCallStruct* fastECall2;
    FastCallStruct* fastOCall;
    uint16_t callId;
    HotCall* hotCall;
};

typedef struct {
    Source* source;
    Emitter* emitter;
} SourceEmitter;

typedef struct {
    pthread_t observedThread;
    circular_buffer* buffer;
    bool isHead;
    uint64_t previousValue;
    uint64_t* noItem;
    std::chrono::_V2::system_clock::time_point* timePoints;
    uint64_t count;
    bool keepPolling;
    std::chrono::_V2::system_clock::time_point startTime;
} testJoin2Stream1Enclave_ObservedData;

testJoin2Stream1Enclave_ObservedData testJoin2Stream1Enclave_tailObservedData;
testJoin2Stream1Enclave_ObservedData testJoin2Stream1Enclave_tailObservedData2;

void testJoin2Stream1Enclave_writeBufferObserverMeasurementToFile(const std::string& pathToFile, testJoin2Stream1Enclave_ObservedData* observedData) {
    std::ofstream measurementFile;
    measurementFile.open(pathToFile, std::ios::app);

    for (size_t i = 0; i < observedData->count; ++i) {
        if (i == 0) {
            measurementFile << observedData->noItem[i] << " " << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData->timePoints[i] - observedData->startTime).count() << std::endl;
        } else {
            measurementFile << observedData->noItem[i] << " " << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData->timePoints[i] - observedData->timePoints[i-1]).count() << std::endl;
        }
    }

    measurementFile.close();
}

void * testJoin2Stream1Enclave_observationThread(void *observedDataAsVoidP) {
    const auto observedData = static_cast<testJoin2Stream1Enclave_ObservedData *>(observedDataAsVoidP);

    const auto buffer = observedData->buffer;
    observedData->startTime = std::chrono::high_resolution_clock::now();
    if (observedData->isHead) {
        while (true) {
            const int value = buffer->head;
            if (value != observedData->previousValue) {
                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
    } else {
        while (true) {
            const int value = buffer->tail;
            if (value != observedData->previousValue) {
//                std::cout << observedData->noItem[0] << std::endl;
                observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                observedData->noItem[observedData->count] = value;
                observedData->count += 1;
                observedData->previousValue = value;
            }

            if (!observedData->keepPolling)
                break;
        }
    }

    return nullptr;
}

void testJoin2Stream1Enclave_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    const auto joinedFlightData = static_cast<JoinedFlightData*>(rawData);
    testJoin2Stream1Enclave_sinkFileStream << joinedFlightData->uniqueCarrier1 << "," << joinedFlightData->uniqueCarrier2 << "," << joinedFlightData->arrDelay << std::endl;
}

void* testJoin2Stream1Enclave_appResponderThread(void* fastOCallAsVoidP)
{
    const auto fastOCall = static_cast<FastCallStruct *>(fastOCallAsVoidP);

    void (*callbacks[1])(void*);
    callbacks[0] = testJoin2Stream1Enclave_sinkResult;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    FastCall_wait(fastOCall, &callTable, 0);

    return nullptr;
}

void *testJoin2Stream1Enclave_enclaveResponderThread(void *fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair2 *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall1 = fastCallPair->fastECall1;
    FastCallStruct *fastEcall2 = fastCallPair->fastECall2;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;

    const sgx_status_t status = EcallStartResponder2(fastCallPair->enclaveId, fastEcall1, fastEcall2, fastOcall, fastCallPair->callId);

    if (status == SGX_SUCCESS)
    {
        printf("Polling success\n");
    }
    else
    {
        printf("Polling failed\n");
        print_error_message(status);
    }

    return nullptr;
}

void* testJoin2Stream1Enclave_startSource(void* sourceEmitterPairAsVoid) {
    const auto sourceEmitterPair = static_cast<SourceEmitter *>(sourceEmitterPairAsVoid);
    const auto source = sourceEmitterPair->source;
    const auto emitter = sourceEmitterPair->emitter;
    source->start(*emitter);
    return nullptr;
}

void testJoin2Stream1Enclave() {
    sgx_enclave_id_t enclaveId;
    FastCallEmitter emitter1;
    FastCallEmitter emitter2;
    pthread_t sourceThread1, sourceThread2;
    std::string resultDirName = "../../results/2stream";
    std::string sinkFileName = "join2stream.csv";
    std::string measurementDirName = "../../measurements/2stream";


    FlightDataIntermediateParser parser1;
    CsvSource sourceMap = CsvSource(
            1,
            "../../results/testBufferObserver/2024-04-09_10-03-43/map.csv",
            0,
            false,
            -1);
    sourceMap.setParser(&parser1);

    FlightDataIntermediateParser parser2;
    CsvSource sourceFilter = CsvSource(
            2,
            "../../results/testBufferObserver/2024-04-09_10-03-43/filter.csv",
            0,
            false,
            -1);
    sourceFilter.setParser(&parser2);

    circular_buffer buffer1 = {
            new char[MAX_BUFFER_SIZE * sizeof(FlightData)],
            0,
            0,
            MAX_BUFFER_SIZE,
            sizeof(FlightData),
            SGX_SPINLOCK_INITIALIZER,
            0
    };

    circular_buffer buffer2 = {
            new char[MAX_BUFFER_SIZE * sizeof(FlightData)],
            0,
            0,
            MAX_BUFFER_SIZE,
            sizeof(FlightData),
            SGX_SPINLOCK_INITIALIZER,
            0
    };

    circular_buffer buffer3 = {
            new char[MAX_BUFFER_SIZE * sizeof(JoinedFlightData)],
            0,
            0,
            MAX_BUFFER_SIZE,
            sizeof(JoinedFlightData),
            SGX_SPINLOCK_INITIALIZER,
            0
    };

    FastCallStruct fastCallData1 = {
            SGX_SPINLOCK_INITIALIZER,
            0,
            &buffer1,
            true
    };

    FastCallStruct fastCallData2 = {
            SGX_SPINLOCK_INITIALIZER,
            0,
            &buffer2,
            true
    };

    FastCallStruct fastCallData3 = {
            SGX_SPINLOCK_INITIALIZER,
            0,
            &buffer3,
            true
    };

    testJoin2Stream1Enclave_tailObservedData.observedThread = 0;
    testJoin2Stream1Enclave_tailObservedData.buffer = &buffer1;
    testJoin2Stream1Enclave_tailObservedData.isHead = false;
    testJoin2Stream1Enclave_tailObservedData.count = 0;
    testJoin2Stream1Enclave_tailObservedData.keepPolling = true;
    testJoin2Stream1Enclave_tailObservedData.previousValue = 0;
    testJoin2Stream1Enclave_tailObservedData.noItem = new uint64_t[8000000];
    testJoin2Stream1Enclave_tailObservedData.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];

    testJoin2Stream1Enclave_tailObservedData2.observedThread = 0;
    testJoin2Stream1Enclave_tailObservedData2.buffer = &buffer2;
    testJoin2Stream1Enclave_tailObservedData2.isHead = false;
    testJoin2Stream1Enclave_tailObservedData2.count = 0;
    testJoin2Stream1Enclave_tailObservedData2.keepPolling = true;
    testJoin2Stream1Enclave_tailObservedData2.previousValue = 0;
    testJoin2Stream1Enclave_tailObservedData2.noItem = new uint64_t[8000000];
    testJoin2Stream1Enclave_tailObservedData2.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];

    if (initialize_enclave(&enclaveId) != SGX_SUCCESS) {
        std::cout << "Initialize enclave failed\n";
        return;
    }

    FastCallPair2 fastCallPair2 = {
            1,
            enclaveId,
            &fastCallData1,
            &fastCallData2,
            &fastCallData3,
            8,
            nullptr
    };
    pthread_create(&testJoin2Stream1Enclave_tailObservedData.observedThread, nullptr, testJoin2Stream1Enclave_observationThread, &testJoin2Stream1Enclave_tailObservedData);
    pthread_create(&testJoin2Stream1Enclave_tailObservedData2.observedThread, nullptr, testJoin2Stream1Enclave_observationThread, &testJoin2Stream1Enclave_tailObservedData2);

    std::cout << "Started observer thread\n";

    pthread_create(&fastCallData1.responderThread, nullptr, testJoin2Stream1Enclave_enclaveResponderThread, &fastCallPair2);
    fastCallData2.responderThread = fastCallData1.responderThread;
    std::cout << "Started enclave\n";

    std::string fileFullPath;
    if (resultDirName == "../../results/2stream") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    testJoin2Stream1Enclave_sinkFileStream.open(fileFullPath, std::ios::out);
    if (testJoin2Stream1Enclave_sinkFileStream.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }
    pthread_create(&fastCallData3.responderThread, nullptr, testJoin2Stream1Enclave_appResponderThread, &fastCallData3);
    std::cout << "Started sink\n";

    emitter1.setFastCallData(&fastCallData1);
    SourceEmitter sourceEmitterPair1 = {&sourceMap, &emitter1 };
    pthread_create(&sourceThread1, nullptr, testJoin2Stream1Enclave_startSource, &sourceEmitterPair1);
    printf("Start source 1...\n");

    emitter2.setFastCallData(&fastCallData2);
    SourceEmitter sourceEmitterPair2 = { &sourceFilter, &emitter2 };
    pthread_create(&sourceThread2, nullptr, testJoin2Stream1Enclave_startSource, &sourceEmitterPair2);
    printf("Start source 2...\n");

    std::cout << "Waiting for source 1...\n";
    pthread_join(sourceThread1, nullptr);

    std::cout << "Waiting for source 2...\n";
    pthread_join(sourceThread2, nullptr);

    std::cout << "Waiting for enclave...\n";
    StopFastCallResponder(&fastCallData1);
    StopFastCallResponder(&fastCallData2);
    pthread_join(fastCallData1.responderThread, nullptr);

    std::cout << "Waiting for sink...\n";
    StopFastCallResponder(&fastCallData3);
    pthread_join(fastCallData3.responderThread, nullptr);
    testJoin2Stream1Enclave_sinkFileStream.close();

    std::cout << "Waiting for observer thread...\n";
    testJoin2Stream1Enclave_tailObservedData.keepPolling = false;
    pthread_join(testJoin2Stream1Enclave_tailObservedData.observedThread, nullptr);
    testJoin2Stream1Enclave_tailObservedData2.keepPolling = false;
    pthread_join(testJoin2Stream1Enclave_tailObservedData2.observedThread, nullptr);

    if (sgx_destroy_enclave(enclaveId) != SGX_SUCCESS) {
        std::cout << "Destroying enclave failed...\n";
        return;
    }

    delete[] static_cast<FlightData*>(buffer1.buffer);
    delete[] static_cast<FlightData*>(buffer2.buffer);
    delete[] static_cast<JoinedFlightData*>(buffer3.buffer);

    // ====================== Write measurements ==============================
    std::string createdMeasurementDirName;
    if (measurementDirName == "../../measurements/2stream") {
        createdMeasurementDirName = createMeasurementsDirectory(measurementDirName);
    } else {
        createdMeasurementDirName = measurementDirName;
    }

    {
        std::string tailFilename = "Tail_Stream_1";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        testJoin2Stream1Enclave_writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                     &testJoin2Stream1Enclave_tailObservedData);
    }
    {
        std::string tailFilename = "Tail_Stream_2";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        testJoin2Stream1Enclave_writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                     &testJoin2Stream1Enclave_tailObservedData2);
    }

    delete[] testJoin2Stream1Enclave_tailObservedData.noItem;
    delete[] testJoin2Stream1Enclave_tailObservedData.timePoints;
    delete[] testJoin2Stream1Enclave_tailObservedData2.noItem;
    delete[] testJoin2Stream1Enclave_tailObservedData2.timePoints;

    std::cout << "Success";

}
