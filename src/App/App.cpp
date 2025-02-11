#include <sgx_defs.h>
#include <iostream>
#include "Enclave_u.h"

#include "App/test.h"
#include "App/utils.h"
#include "utils.h"
#include "Nexmark/schemas.h"
#include "Nexmark/sinks.h"
#include "Nexmark/parsers.h"
#include "Nexmark/NexmarkQuery.h"
#include "SecureSGX/sinks.h"
#include "SecureSGX/SecureSgxQuery.h"

#include "Source/StringRandomGenerationSource.h"
#include "Crypto/aes_gcm.h"

#include "sgx_lib.h"
#include "sgx_urts.h"
#include <ostream>

#define CACHE_DEFEAT_SIZE 10*1024*1024

using namespace std;

std::vector<uint64_t> recordedStartTime;
std::vector<uint64_t> recordedEndTime;

inline __attribute__((always_inline))  uint64_t rdtscp()
{
    unsigned int low, high;

    asm volatile("rdtscp" : "=a" (low), "=d" (high));

    return low | ((uint64_t)high) << 32;
}

void* testOpenSSLWithoutSGXThread(void* data) {
    uint64_t dataSize = 1000;
    uint64_t numberOfData = 1000000;
    StringRandomGenerationSource source(dataSize, numberOfData);
    source.prepare();
    auto generatedTexts = source.getGeneratedTexts();
    std::vector<uint64_t> measurementTimes;
    measurementTimes.reserve(2000000);

    char encryptedData[10000];
    char decryptedData[10000];

    for (int i = 0; i < numberOfData; ++i) {
//        std::cout << generatedTexts[i] << std::endl;
        aes128GcmEncrypt(
                (unsigned char *) generatedTexts[i].c_str(),
                dataSize, NULL, 0,
                const_cast<unsigned char *>(AES_GCM_KEY),
                const_cast<unsigned char *>(AES_GCM_IV), SGX_AESGCM_IV_SIZE,
                (unsigned char *)(encryptedData + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE),
                (unsigned char *)(encryptedData));

        auto start = rdtscp();

        aes128GcmDecrypt(
                (unsigned char*)encryptedData + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
                dataSize,
                NULL, 0,
                (unsigned char*)encryptedData,
                (unsigned char *)(AES_GCM_KEY),
                (unsigned char*)(AES_GCM_IV),
                SGX_AESGCM_IV_SIZE,
                (unsigned char*)decryptedData);

        auto end = rdtscp();
        measurementTimes.push_back(end - start);
//        decryptedData[dataSize+1] = '\0';
//        std::cout << decryptedData << std::endl;
    }

    ofstream out;
    out.open("../../measurements/testing/openssl_without_sgx/" + std::to_string(dataSize) + ".csv", std::ios::out);
    out << "index,time\n";
    for (int i = 0; i < numberOfData; ++i) {
        out << i << "," << measurementTimes[i] << std::endl;
    }
    out.close();

    return nullptr;
}

void testOpenSSLWithoutSGX() {
    // CPU Affinity
    pthread_attr_t testOpenSSLWithoutSGXThreadAttr;
    cpu_set_t testOpenSSLWithoutSGXThreadCpu;

    pthread_attr_init(&testOpenSSLWithoutSGXThreadAttr);
    CPU_ZERO(&testOpenSSLWithoutSGXThreadCpu);
    CPU_SET(7, &testOpenSSLWithoutSGXThreadCpu);
    pthread_attr_setaffinity_np(&testOpenSSLWithoutSGXThreadAttr, sizeof(cpu_set_t), &testOpenSSLWithoutSGXThreadCpu);

    // Start thread
    pthread_t threadId;
    std::cout << "Starting...\n";
    pthread_create(&threadId, &testOpenSSLWithoutSGXThreadAttr, testOpenSSLWithoutSGXThread, nullptr);
    std::cout << "Started. Waiting for stop....\n";

    // Wait for thread to stop
    pthread_join(threadId, nullptr);
    std::cout << "End.\n";
}

void* testCryptoOCall(void*) {
    sgx_enclave_id_t enclaveId;
    if (initialize_enclave(&enclaveId) != SGX_SUCCESS) {
        std::cout << "Initialize enclave failed. Exiting...\n";
        return nullptr;
    }
    std::cout << "Initialize enclave successfully\n";

    recordedStartTime.reserve(1000005);
    recordedEndTime.reserve(1000005);
    sgx_status_t status = testOpenSslOCall(enclaveId);
    if (status != SGX_SUCCESS) {
        std::cout << "ECall failed. Exiting...\n";
        sgx_destroy_enclave(enclaveId);
        return nullptr;
    }

    if (sgx_destroy_enclave(enclaveId) != SGX_SUCCESS) {
        std::cout << "Destroy enclave failed. Exiting...\n";
        return nullptr;
    }

    ofstream out;
    out.open("../../measurements/testing/cryptosdk_ocall/16.csv", ios::out);
    out << "index,time\n";
    for (int i = 0; i < 1000000; ++i) {
        out << i << "," << recordedEndTime[i] - recordedStartTime[i] << std::endl;
    }
    out.close();

    return nullptr;
}

void testingDecryption(std::string cpus = "") {
    // cpus format: source-enclave-observer-headObserver-sink

    ConfigurationTesting config;
    if (cpus.length() >= 9) {
        config.sourceCPU = cpus[0] - 48;
        config.enclaveCPU = cpus[2] - 48;
        config.observerCPU = cpus[4] - 48;
        config.headObserverCPU = cpus[6] - 48;
        config.sinkCPU = cpus[8] - 48;
    }

    config.taskInputDataSize = 16 + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.taskInputDataSize = 16;
    config.sourceCount = 1000000;
    config.taskId = 30;
    config.taskShouldBeObserved = true;
    config.sink = sinkChar;
//    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.outputDataSize = 16;
    config.sinkFileStream = getSinkFileStream();
    config.resultDirName = "../../results/testing/rework_cryptosdk_disabled_hyperthread";
    config.measurementDirName = "../../measurements/testing/rework_cryptosdk_disabled_hyperthread_perf";
    config.sinkFileName = "test.csv";
    config.measurementFileName = "16_" + cpus + ".csv";

    runEngineWithBufferObserverCrypto(config);
}

void testSecureStreamBenchmark(std::string ruleName) {
//    std::string resultDirName = "../../results/secure-sgx/data_master";
//    std::string measurementDirName = "../../measurements/secure-sgx/data_master";

    std::string resultDirName = "../../results/secure_streams_dataset/reduce";
    std::string measurementDirName = "../../measurements/secure_streams_dataset/reduce";

    SecureSgxQuery secureSgxQuery;
    secureSgxQuery.setMeasurementDirName(measurementDirName);
    secureSgxQuery.setResultDirName(resultDirName);

//    secureSgxQuery.runMapQuery("../../dataset/secure-sgx-dataset/2005_100000.csv", "process_time_map_2005.csv", "flight_data_2005_100000.csv");
//    secureSgxQuery.runMapQuery(resultDirName +  "/../flight_data_2005_100000.csv", ruleName + ".csv", ruleName + ".csv");

//    secureSgxQuery.runFilterQuery(resultDirName +  "/map_2005.csv", "process_time_filter_2005.csv", "filter_2005.csv");
//    secureSgxQuery.runFilterQuery(resultDirName +  "/../flight_data_2005_100000.csv", ruleName + ".csv", ruleName + ".csv");

//    secureSgxQuery.runReduceQuery("../../dataset/secure-sgx-dataset/2005.csv", "process_time_reduce_2005_1000.csv", "reduce_2005_1000.csv");
    secureSgxQuery.runReduceQuery(resultDirName +  "/../flight_data_2005_100000.csv", ruleName + ".csv", ruleName + ".csv");
}

void testNexmarkBenchmark(std::string ruleName) {
    //  std::string measurementBatchDirName = createMeasurementsDirectory("../../measurements/testNexmark/batch");
    //  std::string resultBatchDirName = createMeasurementsDirectory("../../results/testNexmark/batch");

//      std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-09_23-46-53";
//      std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-09_23-46-53";

//   std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-12_09-58-38";
//   std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-12_09-58-38";

//    std::string measurementBatchDirName = "../../measurements/testNexmark/crypto/data_20240801";
//    std::string resultBatchDirName = "../../results/testNexmark/crypto/data_20240801";

    std::string measurementBatchDirName = "../../measurements/dataset/q4/avg";
    std::string resultBatchDirName = "../../results/dataset/q4/avg";
//    srand((unsigned) time(nullptr));

    for (int i = 0; i < 1; ++i) {
//        std::string measurementDirName = createDirectory(measurementBatchDirName + "/" + std::to_string(i));
//        std::string resultDirName = createDirectory(resultBatchDirName + "/" + std::to_string(0));

        std::string measurementDirName = createDirectory(measurementBatchDirName);
        std::string resultDirName = createDirectory(resultBatchDirName);

        NexmarkQuery nexmarkQuery;
        nexmarkQuery.setMeasurementDirName(measurementDirName);
        nexmarkQuery.setResultDirName(resultDirName);

//        nexmarkQuery.runQuery1("../../../other/nexmark-data/bids.csv", ruleName + ".csv", ruleName + ".csv");

//        nexmarkQuery.runQuery2_Filter("../../../other/nexmark-data/bids.csv", "process_time_q2_filter_greater_2.csv", "q2result_filter_greater_2.csv");
//        nexmarkQuery.runQuery2_Filter("../../../other/nexmark-data/bids.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery2_Map("../../../other/nexmark-data/bids.csv", "process_time_q2_map.csv", "q2result_map.csv");
//        nexmarkQuery.runQuery2_Map("../../../other/nexmark-data/bids.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery2_Map(resultDirName +  "/q2result_filter_4.csv", "process_time_q2_map.csv", "q2result_map.csv");

//        nexmarkQuery.runQuery3_FilterPerson("../../../other/nexmark-data/people.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery3_FilterAuction("../../../other/nexmark-data/auction.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery3_JoinPersonAuction("../../../other/nexmark-data/people.csv", "../../../other/nexmark-data/auction.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery3_JoinPersonAuction("../../../other/nexmark-data/people.csv", "../../../other/nexmark-data/auction.csv", "q3_join_100_10_3-2-1.csv", "test.csv");
//        nexmarkQuery.runQuery3_MapJoinResult(resultDirName + "/q3result_join_200_10.csv", "process_time_q3_map.csv", "q3result_map.csv");

//        nexmarkQuery.runQuery4_JoinAuctionBid("../../../other/nexmark-data/auction.csv", "../../../other/nexmark-data/bids.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery4_MapAuctionBid(resultDirName +  "/q4result_join1_200_10.csv", "process_time_q4_map1.csv", "q4result_map1.csv");
//        nexmarkQuery.runQuery4_MapAuctionBid("../../results/dataset/q4/600_10.csv", ruleName + ".csv", ruleName + ".csv");

//        nexmarkQuery.runQuery4_Max("../../results/dataset/q4/48_add_auction_and_bidder_to_price+48_negate_all_uint64_fields.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery4_Max(resultDirName +  "/q4result_map1.csv", "process_time_q4_max_500_10.csv", "q4result_max_500_10.csv");
//        nexmarkQuery.runQuery4_Max(resultDirName +  "/q4result_map1.csv", "q4_max_100_10_3-2-1.csv", "test.csv");
//        nexmarkQuery.runQuery4_JoinCategory(resultDirName +  "/q4result_map1.csv", "process_time_q4_join_category_500_10.csv", "q4result_join_category_500_10.csv");
//        nexmarkQuery.runQuery4_JoinCategory("../../results/dataset/q4/max_600_200.csv", ruleName + ".csv", ruleName + ".csv");

        nexmarkQuery.runQuery4_Average("../../results/dataset/q4/join_category_500_100.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery4_Average(resultDirName +  "/q4result_join_category_500_10.csv", "process_time_q4_average_200_10.csv", "q4result_average_200_10.csv");
//        nexmarkQuery.runQuery4_Average(resultDirName +  "/q4result_join_category_500_10.csv", "q4_average_100_10_3-2-1.csv", "test.csv");

//        nexmarkQuery.runQuery5_CountByAuction("../../../other/nexmark-data/bids.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery5_MaxBatch("../../results/dataset/q5/count_by_auction/" + ruleName + ".csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery5_MaxBatch(resultDirName + "/q5result_count_by_auction_100_10.csv", "q5_max_100_10_3-2-1.csv", "test.csv");

//        nexmarkQuery.runQuery6_JoinAuctionBid("../../../other/nexmark-data/auction.csv", "../../../other/nexmark-data/bids.csv", "q6_join_100_10.csv", "q6result_join_100_10.csv");
//        nexmarkQuery.runQuery6_Filter(resultDirName + "/q6result_join_100_10.csv", "process_time_q6_filter.csv", "q6result_filter.csv");
//        nexmarkQuery.runQuery6_Filter("../../results/dataset/q4/600_10.csv", ruleName + ".csv", ruleName + ".csv");

//        nexmarkQuery.runQuery6_Max("../../results/dataset/q6/103_filter_price_not_divisible_by_any_field+103_filter_duration_is_not_between_reserve_and_initialBid.csv", ruleName + ".csv", ruleName + ".csv");
//        nexmarkQuery.runQuery6_Max(resultDirName + "/q6result_filter.csv", "process_time_q6_max_100_10.csv", "q6result_max_100_10.csv");
//        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6result_max_100_10.csv", "process_time_q6_avg_100.csv", "q6result_avg_100.csv");
//        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6result_max_100_10.csv", "q6_avg_100_3-2-1.csv", "test.csv");
//        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6result_max_100_10.csv", "test.csv", "result_partition_100_10.csv");
//        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6result_max_100_10.csv", "process_time_partition_avg_500.csv", "result_partition_avg_500.csv");
//        nexmarkQuery.runQuery6_Avg("../../dataset/nexmark/q6_avg_input_1seller.csv", ruleName + "_1seller.csv", ruleName + "_1seller.csv");

//        nexmarkQuery.runQuery7_MaxJoin("../../../other/bids.csv", "process_time_q7_maxjoin_1500.csv", "q7result_maxjoin_1500.csv");

//        nexmarkQuery.runQuery8_JoinPersonAuction("../../../other/people.csv", "../../../other/auction.csv", "q8_join_600.csv", "q8result_join_600.csv");
//        nexmarkQuery.runQuery8_MapJoinResult(resultDirName +  "/q8result_join_600.csv", "process_time_q8_map_600.csv", "q8result_map_600.csv");

//        ofstream out;
//        std::cout << "Writing to cryptosdk_engine_ocall\n";
//        out.open("../../measurements/testing/rework_nexmark/" + std::to_string(i) + "/tail_0_process_time_q1_ocall.csv", ios::out);
//        out << "index,time\n";
//        for (int j = 0; j < recordedEndTime.size(); ++j) {
//            out << j << "," << recordedEndTime[j] - recordedStartTime[j] << std::endl;
//        }
//        out.close();
    }
}


void oCallRecordStartTime() {
    recordedStartTime.push_back(rdtscp());
}

void oCallRecordEndTime() {
    recordedEndTime.push_back(rdtscp());
}

inline __attribute__((always_inline)) uint64_t clearcache(void *buf, size_t size) {
    uint64_t sum;
    size_t count = size / sizeof(uint64_t);
    volatile uint64_t *p = (uint64_t *)buf;
    while (count--) {
        sum += *p;
        *p++ = 0;
    }
    asm volatile ("mfence");
    return sum;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
//    std::string argument = argv[1];
//    std::cout << "Argument: " << argument << std::endl;

//    recordedStartTime.reserve(8000000);
//    recordedEndTime.reserve(8000000);

    /* GENERATE RANDOM STRING */
//    StringRandomGenerationSource source(1000, 100);
//    source.prepare();
//    auto generatedTexts = source.getGeneratedTexts();
//    ofstream out;
//    out.open("../../measurements/testing/openssl_ocall/1000.txt", ios::out);
//    for (int i = 0; i < 100; ++i) {
//        out << "\"" << generatedTexts[i] << "\"" << ',' << std::endl;
//    }
//    out.close();

    /* CRYPTO INSIDE SGX WITH OCALL */
    // CPU Affinity
//    pthread_attr_t testThreadAttr;
//    cpu_set_t testThreadCpu;
//
//    pthread_attr_init(&testThreadAttr);
//    CPU_ZERO(&testThreadCpu);
//    CPU_SET(7, &testThreadCpu);
//    pthread_attr_setaffinity_np(&testThreadAttr, sizeof(cpu_set_t), &testThreadCpu);
//
//    // Start thread
//    pthread_t threadId;
//    std::cout << "Starting...\n";
//    pthread_create(&threadId, &testThreadAttr, testCryptoOCall, nullptr);
//    std::cout << "Started. Waiting for stop....\n";
//
//    // Wait for thread to stop
//    pthread_join(threadId, nullptr);
//    std::cout << "End.\n";

    /* OPENSSL WITHOUT SGX */
//    testOpenSSLWithoutSGX();

    /* TESTING DECRYPTION */
//    void *defeat;
//    if (posix_memalign(&defeat, 64, CACHE_DEFEAT_SIZE) != 0 || !defeat) {
//        printf("defeat buffer allocation failure\n");
//        return -1;
//    }
//    void *defeat0 __attribute__((unused)) = malloc(CACHE_DEFEAT_SIZE);
//    void *defeat1 __attribute__((unused)) = malloc(CACHE_DEFEAT_SIZE);
//#define DEFEATCACHE do { memset_s(defeat0, CACHE_DEFEAT_SIZE, 0, CACHE_DEFEAT_SIZE); memcpy(defeat1, defeat0, CACHE_DEFEAT_SIZE); asm volatile ("mfence"); } while (0)
//
//    void *buffer;
//    if (posix_memalign(&buffer, 64, 2048) != 0 || !buffer) {
//        printf("aligned buffer allocation failure\n");
//        return -1;
//    }
//    memset(buffer, 0, 2048);
//    clearcache(defeat, CACHE_DEFEAT_SIZE);
//
//    testingDecryption(argument);
//
//    free(defeat);
//    free(defeat0);
//    free(defeat1);

    /* SECURE STREAMS BENCHMARK */
//    testSecureStreamBenchmark();


    /* NEXMARK BENCHMARK */
    if (argc < 2) {
        std::cerr << "Error: No rule name provided." << std::endl;
        return 1;
    }

    std::string ruleName = argv[1];
//    testNexmarkBenchmark(ruleName);
    testSecureStreamBenchmark(ruleName);
    return 0;
}

