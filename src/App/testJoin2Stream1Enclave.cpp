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

    if (sgx_destroy_enclave(enclaveId) != SGX_SUCCESS) {
        std::cout << "Destroying enclave failed...\n";
        return;
    }

    delete[] static_cast<FlightData*>(buffer1.buffer);
    delete[] static_cast<FlightData*>(buffer2.buffer);
    delete[] static_cast<JoinedFlightData*>(buffer3.buffer);

    std::cout << "Success";

}
