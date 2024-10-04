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

namespace TestJoin2Stream1Enclave {
    std::ofstream sinkFileStream;
    bool shouldContinue = true;
    uint64_t processedPerSecond[1000];
    int processedCountIndex = 0;

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
        uint64_t* timestampCounterPoints;
        uint64_t count;
        bool keepPolling;
        std::chrono::_V2::system_clock::time_point startTime;
        uint64_t startTimestamp;
    } ObservedData;

    ObservedData tailObservedData;
    ObservedData tailObservedData2;
    ObservedData headObservedData;
    ObservedData headObservedData2;

    inline __attribute__((always_inline))  uint64_t rdtscp()
    {
        unsigned int low, high;

        asm volatile("rdtscp" : "=a" (low), "=d" (high));

        return low | ((uint64_t)high) << 32;
    }

    void *processedCountThread(void *circularBufferAsVoidP) {
        const auto buffer = (circular_buffer*) circularBufferAsVoidP;
        uint64_t value;
        while (shouldContinue) {
            sgx_spin_lock(&buffer->lock_count);
            value = buffer->popped_count;
            buffer->popped_count = 0;
            sgx_spin_unlock(&buffer->lock_count);

            processedPerSecond[processedCountIndex] = value;
            processedCountIndex++;
            sleep(1);
        }

        return nullptr;
    }

    void writeBufferObserverMeasurementToFile(const std::string& pathToFile, ObservedData* observedData) {
        std::ofstream measurementFile;
        measurementFile.open(pathToFile, std::ios::out);

        measurementFile << "index,time\n";
        for (size_t i = 0; i < observedData->count; ++i) {
//            if (i == 0) {
//                measurementFile << observedData->noItem[i] << "," << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData->timePoints[i] - observedData->startTime).count() << std::endl;
//            } else {
//                measurementFile << observedData->noItem[i] << "," << std::chrono::duration_cast<std::chrono::nanoseconds>(observedData->timePoints[i] - observedData->timePoints[i-1]).count() << std::endl;
//            }

            if (i == 0) {
                measurementFile << observedData->noItem[i] << "," << observedData->timestampCounterPoints[0] - observedData->startTimestamp << std::endl;
            } else {
                measurementFile << observedData->noItem[i] << "," << observedData->timestampCounterPoints[i] - observedData->timestampCounterPoints[i-1] << std::endl;
            }
        }

        measurementFile.close();
    }

    void * observationThread(void *observedDataAsVoidP) {
        const auto observedData = static_cast<ObservedData *>(observedDataAsVoidP);

        const auto buffer = observedData->buffer;
//        observedData->startTime = std::chrono::high_resolution_clock::now();
        observedData->startTimestamp = rdtscp();
        if (observedData->isHead) {
            while (true) {
                const int value = buffer->head;
                if (value != observedData->previousValue) {
//                    observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                    uint64_t timestamp = rdtscp();
                    observedData->timestampCounterPoints[observedData->count] = timestamp;
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
//                    observedData->timePoints[observedData->count] = std::chrono::high_resolution_clock::now();
                    uint64_t timestamp = rdtscp();
                    observedData->timestampCounterPoints[observedData->count] = timestamp;
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

    void sinkResult(void* rawData)
    {
        if (rawData == NULL) {
            return;
        }

        const auto joinedFlightData = static_cast<JoinedFlightData*>(rawData);
        sinkFileStream << joinedFlightData->uniqueCarrier1 << "," << joinedFlightData->uniqueCarrier2 << "," << joinedFlightData->arrDelay << std::endl;
    }

    void* appResponderThread(void* fastOCallAsVoidP)
    {
        const auto fastOCall = static_cast<FastCallStruct *>(fastOCallAsVoidP);

        void (*callbacks[1])(void*);
        callbacks[0] = sinkResult;

        FastCallTable callTable;
        callTable.numEntries = 1;
        callTable.callbacks  = callbacks;

        FastCall_wait(fastOCall, &callTable, 0);

        return nullptr;
    }

    void *enclaveResponderThread(void *fastCallPairAsVoidP)
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

    void* startSource(void* sourceEmitterPairAsVoid) {
        const auto sourceEmitterPair = static_cast<SourceEmitter *>(sourceEmitterPairAsVoid);
        const auto source = sourceEmitterPair->source;
        const auto emitter = sourceEmitterPair->emitter;
        source->start(*emitter);
        return nullptr;
    }
}


void testJoin2Stream1Enclave(int n) {
    sgx_enclave_id_t enclaveId;
    FastCallEmitter emitter1;
    FastCallEmitter emitter2;
    pthread_t sourceThread1, sourceThread2, processedPerSecondThread;
//    std::string resultDirName = "../../results";
//    std::string sinkFileName = "join2stream_200.csv";
//    std::string measurementDirName = "../../measurements";
//    std::string throughoutFileName = "throughout.csv";

    std::string resultDirName = "../../results/testBufferObserver/batch/2024-05-30_12-44-04/" + std::to_string(n);
    std::string sinkFileName = "join2stream_affinity.csv";
    std::string measurementDirName = "../../measurements/testBufferObserver/batch/2024-05-30_12-44-04/" + std::to_string(n);

    pthread_attr_t enclaveAttr;
    cpu_set_t enclaveCpu;
    pthread_attr_t sourceAttr;
    cpu_set_t sourceCpu;
    pthread_attr_t sinkAttr;
    cpu_set_t sinkCpu;
    pthread_attr_t observerAttr;
    cpu_set_t observerCpu;
    pthread_attr_t observerAttr2;
    cpu_set_t observerCpu2;
    pthread_attr_t headObserverAttr;
    cpu_set_t headObserverCpu;
    pthread_attr_t headObserverAttr2;
    cpu_set_t headObserverCpu2;


    FlightDataIntermediateParser parser1;
    CsvSource sourceMap = CsvSource(
            1,
//            "../../results/testBufferObserver/batch/2024-05-08_11-36-04/0/map.csv",
//            resultDirName + "/map.csv",
            "/home/hungpm/Work/LeackyStream/simple-dsp-engine/results/map_2005.csv",
            0,
            false,
            8000000);
    sourceMap.setParser(&parser1);

    FlightDataIntermediateParser parser2;
    CsvSource sourceFilter = CsvSource(
            2,
//            "../../results/testBufferObserver/batch/2024-05-08_11-36-04/0/filter.csv",
            "/home/hungpm/Work/LeackyStream/simple-dsp-engine/results/map_2006.csv",
            0,
            false,
            8000000);
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

    TestJoin2Stream1Enclave::tailObservedData.observedThread = 0;
    TestJoin2Stream1Enclave::tailObservedData.buffer = &buffer1;
    TestJoin2Stream1Enclave::tailObservedData.isHead = false;
    TestJoin2Stream1Enclave::tailObservedData.count = 0;
    TestJoin2Stream1Enclave::tailObservedData.keepPolling = true;
    TestJoin2Stream1Enclave::tailObservedData.previousValue = 0;
    TestJoin2Stream1Enclave::tailObservedData.noItem = new uint64_t[8000000];
//    TestJoin2Stream1Enclave::tailObservedData.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];
    TestJoin2Stream1Enclave::tailObservedData.timestampCounterPoints = new uint64_t[8000000];

    TestJoin2Stream1Enclave::tailObservedData2.observedThread = 0;
    TestJoin2Stream1Enclave::tailObservedData2.buffer = &buffer2;
    TestJoin2Stream1Enclave::tailObservedData2.isHead = false;
    TestJoin2Stream1Enclave::tailObservedData2.count = 0;
    TestJoin2Stream1Enclave::tailObservedData2.keepPolling = true;
    TestJoin2Stream1Enclave::tailObservedData2.previousValue = 0;
    TestJoin2Stream1Enclave::tailObservedData2.noItem = new uint64_t[8000000];
//    TestJoin2Stream1Enclave::tailObservedData2.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];
    TestJoin2Stream1Enclave::tailObservedData2.timestampCounterPoints = new uint64_t[8000000];

    TestJoin2Stream1Enclave::headObservedData.observedThread = 0;
    TestJoin2Stream1Enclave::headObservedData.buffer = &buffer1;
    TestJoin2Stream1Enclave::headObservedData.isHead = true;
    TestJoin2Stream1Enclave::headObservedData.count = 0;
    TestJoin2Stream1Enclave::headObservedData.keepPolling = true;
    TestJoin2Stream1Enclave::headObservedData.previousValue = 0;
    TestJoin2Stream1Enclave::headObservedData.noItem = new uint64_t[8000000];
//    TestJoin2Stream1Enclave::tailObservedData.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];
    TestJoin2Stream1Enclave::headObservedData.timestampCounterPoints = new uint64_t[8000000];

    TestJoin2Stream1Enclave::headObservedData2.observedThread = 0;
    TestJoin2Stream1Enclave::headObservedData2.buffer = &buffer2;
    TestJoin2Stream1Enclave::headObservedData2.isHead = true;
    TestJoin2Stream1Enclave::headObservedData2.count = 0;
    TestJoin2Stream1Enclave::headObservedData2.keepPolling = true;
    TestJoin2Stream1Enclave::headObservedData2.previousValue = 0;
    TestJoin2Stream1Enclave::headObservedData2.noItem = new uint64_t[8000000];
//    TestJoin2Stream1Enclave::tailObservedData2.timePoints = new std::chrono::_V2::system_clock::time_point[8000000];
    TestJoin2Stream1Enclave::headObservedData2.timestampCounterPoints = new uint64_t[8000000];

    if (initialize_enclave(&enclaveId) != SGX_SUCCESS) {
        std::cout << "Initialize enclave failed\n";
        return;
    }

    TestJoin2Stream1Enclave::FastCallPair2 fastCallPair2 = {
            1,
            enclaveId,
            &fastCallData1,
            &fastCallData2,
            &fastCallData3,
            8,
            nullptr
    };

    pthread_attr_init(&observerAttr);
    CPU_ZERO(&observerCpu);
    CPU_SET(5, &observerCpu);
    pthread_attr_setaffinity_np(&observerAttr, sizeof(cpu_set_t), &observerCpu);
    pthread_create(&TestJoin2Stream1Enclave::tailObservedData.observedThread, &observerAttr, TestJoin2Stream1Enclave::observationThread, &TestJoin2Stream1Enclave::tailObservedData);

    pthread_attr_init(&observerAttr2);
    CPU_ZERO(&observerCpu2);
    CPU_SET(6, &observerCpu2);
    pthread_attr_setaffinity_np(&observerAttr2, sizeof(cpu_set_t), &observerCpu2);
    pthread_create(&TestJoin2Stream1Enclave::tailObservedData2.observedThread, &observerAttr2, TestJoin2Stream1Enclave::observationThread, &TestJoin2Stream1Enclave::tailObservedData2);

    pthread_attr_init(&headObserverAttr);
    CPU_ZERO(&headObserverCpu);
    CPU_SET(3, &headObserverCpu);
    pthread_attr_setaffinity_np(&headObserverAttr, sizeof(cpu_set_t), &headObserverCpu);
    pthread_create(&TestJoin2Stream1Enclave::headObservedData.observedThread, &headObserverAttr, TestJoin2Stream1Enclave::observationThread, &TestJoin2Stream1Enclave::headObservedData);

    pthread_attr_init(&headObserverAttr2);
    CPU_ZERO(&headObserverCpu2);
    CPU_SET(4, &headObserverCpu2);
    pthread_attr_setaffinity_np(&headObserverAttr2, sizeof(cpu_set_t), &headObserverCpu2);
    pthread_create(&TestJoin2Stream1Enclave::headObservedData2.observedThread, &headObserverAttr2, TestJoin2Stream1Enclave::observationThread, &TestJoin2Stream1Enclave::headObservedData2);
    std::cout << "Started observer thread\n";
//    pthread_create(&processedPerSecondThread, nullptr, TestJoin2Stream1Enclave::processedCountThread, &buffer1);
//    std::cout << "Start processed-per-second thread\n";

    pthread_attr_init(&enclaveAttr);
    CPU_ZERO(&enclaveCpu);
    CPU_SET(7, &enclaveCpu);
    pthread_attr_setaffinity_np(&enclaveAttr, sizeof(cpu_set_t), &enclaveCpu);
    pthread_create(&fastCallData1.responderThread, &enclaveAttr, TestJoin2Stream1Enclave::enclaveResponderThread, &fastCallPair2);
    fastCallData2.responderThread = fastCallData1.responderThread;
    std::cout << "Started enclave\n";

    std::string fileFullPath;
    if (resultDirName == "../../results/2stream") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    TestJoin2Stream1Enclave::sinkFileStream.open(fileFullPath, std::ios::out);
    if (TestJoin2Stream1Enclave::sinkFileStream.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    pthread_attr_init(&sinkAttr);
    CPU_ZERO(&sinkCpu);
    CPU_SET(1, &sinkCpu);
    pthread_attr_setaffinity_np(&sinkAttr, sizeof(cpu_set_t), &sinkCpu);
    pthread_create(&fastCallData3.responderThread, &sinkAttr, TestJoin2Stream1Enclave::appResponderThread, &fastCallData3);
    std::cout << "Started sink\n";

    emitter1.setFastCallData(&fastCallData1);
    TestJoin2Stream1Enclave::SourceEmitter sourceEmitterPair1 = {&sourceMap, &emitter1 };

    pthread_attr_init(&sourceAttr);
    CPU_ZERO(&sourceCpu);
    CPU_SET(2, &sourceCpu);
    pthread_attr_setaffinity_np(&sourceAttr, sizeof(cpu_set_t), &sourceCpu);
    pthread_create(&sourceThread1, &sourceAttr, TestJoin2Stream1Enclave::startSource, &sourceEmitterPair1);
    printf("Start source 1...\n");

    emitter2.setFastCallData(&fastCallData2);
    TestJoin2Stream1Enclave::SourceEmitter sourceEmitterPair2 = { &sourceFilter, &emitter2 };
    pthread_create(&sourceThread2, nullptr, TestJoin2Stream1Enclave::startSource, &sourceEmitterPair2);
    printf("Start source 2...\n");

    std::cout << "Waiting for source 1...\n";
    pthread_join(sourceThread1, nullptr);

    std::cout << "Waiting for source 2...\n";
    pthread_join(sourceThread2, nullptr);

    std::cout << "Waiting for enclave...\n";
    StopFastCallResponder(&fastCallData1);
    StopFastCallResponder(&fastCallData2);
    pthread_join(fastCallData1.responderThread, nullptr);

//    TestJoin2Stream1Enclave::shouldContinue = false;
//    pthread_join(processedPerSecondThread, nullptr);

    std::cout << "Waiting for sink...\n";
    StopFastCallResponder(&fastCallData3);
    pthread_join(fastCallData3.responderThread, nullptr);
    TestJoin2Stream1Enclave::sinkFileStream.close();

    std::cout << "Waiting for observer thread...\n";
    TestJoin2Stream1Enclave::tailObservedData.keepPolling = false;
    pthread_join(TestJoin2Stream1Enclave::tailObservedData.observedThread, nullptr);
    TestJoin2Stream1Enclave::tailObservedData2.keepPolling = false;
    pthread_join(TestJoin2Stream1Enclave::tailObservedData2.observedThread, nullptr);

    if (sgx_destroy_enclave(enclaveId) != SGX_SUCCESS) {
        std::cout << "Destroying enclave failed...\n";
        return;
    }

    delete[] static_cast<FlightData*>(buffer1.buffer);
    delete[] static_cast<FlightData*>(buffer2.buffer);
    delete[] static_cast<JoinedFlightData*>(buffer3.buffer);

    // ======================= Get throughout =======================

    std::string createdMeasurementDirName;
    if (measurementDirName == "../../measurements/2stream") {
        createdMeasurementDirName = createMeasurementsDirectory(measurementDirName);
    } else {
        createdMeasurementDirName = measurementDirName;
    }

//    std::cout << "THROUGHOUT\n";
//    std::cout << "Number of records: " << TestJoin2Stream1Enclave::processedCountIndex << std::endl;
//    std::ofstream throughoutStream;
//    std::string throughoutFullPath = createdMeasurementDirName;
//    throughoutStream.open(throughoutFullPath.append("/").append(throughoutFileName));
//    for (int i = 0; i < TestJoin2Stream1Enclave::processedCountIndex; ++i) {
//        std::cout << i << " " << TestJoin2Stream1Enclave::processedPerSecond[i] << std::endl;
//        throughoutStream << i << "," << TestJoin2Stream1Enclave::processedPerSecond[i] << std::endl;
//    }
//    throughoutStream.close();

    // ====================== Write measurements ==============================
    {
        std::string tailFilename = "process_time_tail_join_1_affinity_100.csv";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        TestJoin2Stream1Enclave::writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                     &TestJoin2Stream1Enclave::tailObservedData);
    }
    {
        std::string tailFilename = "process_time_tail_join_2_affinity_100.csv";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        TestJoin2Stream1Enclave::writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                     &TestJoin2Stream1Enclave::tailObservedData2);
    }
    {
        std::string tailFilename = "process_time_head_join_1_affinity_100.csv";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        TestJoin2Stream1Enclave::writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                      &TestJoin2Stream1Enclave::headObservedData);
    }
    {
        std::string tailFilename = "process_time_head_join_2_affinity_100.csv";
        std::string tailFileFullPath = createdMeasurementDirName;
        tailFileFullPath.append("/").append(tailFilename);

        std::cout << "Writing measurements for tail buffer " << std::endl;
        TestJoin2Stream1Enclave::writeBufferObserverMeasurementToFile(tailFileFullPath,
                                                                      &TestJoin2Stream1Enclave::headObservedData);
    }

    delete[] TestJoin2Stream1Enclave::tailObservedData.noItem;
    delete[] TestJoin2Stream1Enclave::tailObservedData.timePoints;
    delete[] TestJoin2Stream1Enclave::tailObservedData2.noItem;
    delete[] TestJoin2Stream1Enclave::tailObservedData2.timePoints;

    std::cout << "Success";

}
