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

#include "sgx_lib.h"
#include "sgx_urts.h"
#include <ostream>

using namespace std;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    /* TESTING DECRYPTION */
//    ConfigurationTesting config;
//    config.taskInputDataSize = 500;
//    config.sourceCount = 1000000;
//    config.taskId = 30;
//    config.taskShouldBeObserved = true;
//    config.sink = sinkBid;
//    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.sinkFileStream = getSinkFileStream();
//    config.resultDirName = "../../results/testing/plain";
//    config.measurementDirName = "../../measurements/testing/plain";
//    config.sinkFileName = "test.csv";
//    config.measurementFileName = "500.csv";
//
//    runEngineWithBufferObserverCrypto(config);


    /* SECURE STREAMS BENCHMARK */
//    std::string resultDirName = "../../results/secure-sgx/data_master";
//    std::string measurementDirName = "../../measurements/secure-sgx/data_master";
//
//    SecureSgxQuery secureSgxQuery;
//    secureSgxQuery.setMeasurementDirName(measurementDirName);
//    secureSgxQuery.setResultDirName(resultDirName);

//    secureSgxQuery.runMapQuery("../../dataset/secure-sgx-dataset/2005.csv", "process_time_map_2005.csv", "map_2005.csv");
//    secureSgxQuery.runFilterQuery(resultDirName +  "/map_2005.csv", "process_time_filter_2005.csv", "filter_2005.csv");
//    secureSgxQuery.runReduceQuery("../../dataset/secure-sgx-dataset/2005.csv", "process_time_reduce_2005_1000.csv", "reduce_2005_1000.csv");
//    secureSgxQuery.runReduceQuery(resultDirName +  "/filter_2005.csv", "process_time_reduce_2005_100_10.csv", "reduce_2005_100_10.csv");


//    config.sinkFileName = "reduce_2005.csv";
//    config.measurementFileName = "process_time_tail_reduce_2005.csv";
////    config.sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
//    config.sourceFileName = "../../results/secure-sgx/data/filter_2005.csv";
//    config.sourceId = 1;
//    config.sourceDelay = 0;
//    config.sourceHasHeader = false;
//    config.sourceCount = 100;
//
//    config.taskId = 2;
//    config.parser = new FlightDataIntermediateParser();
//    config.taskInputDataSize = sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.taskShouldBeObserved = true;
//    config.outputDataSize = sizeof(ReducedFlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.sink = sinkReduce;
//    config.sinkFileStream = getSecureSgxSinkFileStream();
//
//    runEngineWithBufferObserverCrypto(config);

    /* NEXMARK BENCHMARK */

      //  std::string measurementBatchDirName = createMeasurementsDirectory("../../measurements/testNexmark/batch");
      //  std::string resultBatchDirName = createMeasurementsDirectory("../../results/testNexmark/batch");

//      std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-09_23-46-53";
//      std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-09_23-46-53";

//   std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-12_09-58-38";
//   std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-12_09-58-38";

    std::string measurementBatchDirName = "../../measurements/testNexmark/crypto/data_20240801";
    std::string resultBatchDirName = "../../results/testNexmark/crypto/data_20240801";
    srand((unsigned)time(nullptr));

    for (int i = 0; i < 20; ++i) {
        std::string measurementDirName = createDirectory(measurementBatchDirName + "/" + std::to_string(i));
        std::string resultDirName = createDirectory(resultBatchDirName + "/" + std::to_string(0));

        NexmarkQuery nexmarkQuery;
        nexmarkQuery.setMeasurementDirName(measurementDirName);
        nexmarkQuery.setResultDirName(resultDirName);

//        nexmarkQuery.runQuery1("../../../other/nexmark-data/bids.csv", "process_time_q1.csv", "q1result.csv");

//        nexmarkQuery.runQuery2_Filter("../../../other/nexmark-data/bids.csv", "process_time_q2_filter_greater_2.csv", "q2result_filter_greater_2.csv");
//        nexmarkQuery.runQuery2_Map("../../../other/nexmark-data/bids.csv", "process_time_q2_map.csv", "q2result_map.csv");
//        nexmarkQuery.runQuery2_Map(resultDirName +  "/q2result_filter_4.csv", "process_time_q2_map.csv", "q2result_map.csv");

//        nexmarkQuery.runQuery3_FilterPerson("../../../other/nexmark-data/people_100000.csv", "process_time_q3_filter_person_4.csv", "q3result_filter_person_4.csv");
//        nexmarkQuery.runQuery3_FilterAuction("../../../other/nexmark-data/auction.csv", "process_time_q3_filter_auction.csv", "q3result_filter_auction.csv");
//        nexmarkQuery.runQuery3_JoinPersonAuction("../../../other/nexmark-data/people.csv", "../../../other/nexmark-data/auction.csv", "q3_join_200_10.csv", "q3result_join_200_10.csv");
//        nexmarkQuery.runQuery3_MapJoinResult(resultDirName + "/q3result_join_200_10.csv", "process_time_q3_map.csv", "q3result_map.csv");

//        nexmarkQuery.runQuery4_JoinAuctionBid("../../../other/nexmark-data/auction.csv", "../../../other/nexmark-data/bids.csv", "q4_join1_200_10.csv", "q4result_join1_200_10.csv");
//        nexmarkQuery.runQuery4_MapAuctionBid(resultDirName +  "/q4result_join1.csv", "process_time_q4_map1.csv", "q4result_map1.csv");
//        nexmarkQuery.runQuery4_Max(resultDirName +  "/q4result_map1.csv", "process_time_q4_max_parsed_100_10.csv", "q4result_max_parsed_100_10.csv");
//        nexmarkQuery.runQuery4_JoinCategory(resultDirName +  "/q4result_map1.csv", "process_time_q4_join_category_500_10.csv", "q4result_join_category_500_10.csv");
//        nexmarkQuery.runQuery4_Average(resultDirName +  "/q4result_map1.csv", "process_time_q4_average_500_10.csv", "q4result_average_500_10.csv");

//        nexmarkQuery.runQuery5_CountByAuction("../../../other/nexmark-data/bids.csv", "process_time_q5_count_by_auction_parsed_200_10.csv", "q5result_count_by_auction_parsed_200_10.csv");
//        nexmarkQuery.runQuery5_MaxBatch(resultDirName + "/q5result_count_by_auction_100_10.csv", "process_time_q5_max_test_200_10.csv", "q5result_max_test_200_10.csv");

//        nexmarkQuery.runQuery6_JoinAuctionBid("../../../other/nexmark-data/auction.csv", "../../../other/nexmark-data/bids.csv", "q6_join_200_10.csv", "q6result_join_200_10.csv");
//        nexmarkQuery.runQuery6_Filter(resultDirName + "/q6result_join_200_10.csv", "process_time_q6_filter.csv", "q6result_filter.csv");
//        nexmarkQuery.runQuery6_Max(resultDirName + "/q6result_filter.csv", "process_time_q6_max_500_10.csv", "q6result_max_500_10.csv");
        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6_avg_input_54seller.csv", "process_time_q6_avg_200_nocrypto_54seller.csv", "q6result_avg_200_nocrypto_54seller.csv");

//        nexmarkQuery.runQuery7_MaxJoin("../../../other/bids.csv", "process_time_q7_maxjoin_1500.csv", "q7result_maxjoin_1500.csv");

//        nexmarkQuery.runQuery8_JoinPersonAuction("../../../other/people.csv", "../../../other/auction.csv", "q8_join_600.csv", "q8result_join_600.csv");
//        nexmarkQuery.runQuery8_MapJoinResult(resultDirName +  "/q8result_join_600.csv", "process_time_q8_map_600.csv", "q8result_map_600.csv");

//        config.sinkFileName = "q4result_max_simple_600.csv";
//        config.measurementFileName = "process_time_q4_max_simple.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_map1_600.csv";
//        config.taskId = 21;
//        config.taskInputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q4Map1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q4result_join2.csv";
//        config.measurementFileName = "process_time_q4_join2.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_max_simple.csv";
//        config.taskId = 22;
//        config.taskInputDataSize = sizeof(Q4Map1Result);
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result);
//        config.parser = new Q4Map1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q4result_avg.csv";
//        config.measurementFileName = "process_time_q4result_avg.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_join2.csv";
//        config.taskId = 23;
//        config.taskInputDataSize = sizeof(Q4Map1Result);
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result);
//        config.parser = new Q4Map1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();

    }


    return 0;
}

