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

using namespace std;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
//    ConfigurationTesting config;
//    config.resultDirName = "../../results/secure-sgx";
//    config.measurementDirName = "../../measurements/secure-sgx";
//
//    config.sinkFileName = "reduce_1500_2005.csv";
//    config.measurementFileName = "process_time_tail_reduce_1500_2005.csv";
//    config.sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
//    config.sourceFileName = "../../results/secure-sgx/filter_2005.csv";
//    config.sourceId = 1;
//    config.sourceDelay = 0;
//    config.sourceHasHeader = false;
//    config.sourceCount = 300000;
//
//    config.taskId = 6;
//    config.taskInputDataSize = sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//    config.taskShouldBeObserved = true;
//    config.outputDataSize = sizeof(ReducedFlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//
//    testObserverPerformance(config);

      //  std::string measurementBatchDirName = createMeasurementsDirectory("../../measurements/testNexmark/batch");
      //  std::string resultBatchDirName = createMeasurementsDirectory("../../results/testNexmark/batch");

//      std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-09_23-46-53";
//      std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-09_23-46-53";

//   std::string measurementBatchDirName = "../../measurements/testNexmark/batch/2024-06-12_09-58-38";
//   std::string resultBatchDirName = "../../results/testNexmark/batch/2024-06-12_09-58-38";

    std::string measurementBatchDirName = "../../measurements/testNexmark/crypto/data2";
    std::string resultBatchDirName = "../../results/testNexmark/crypto/data2";

    for (int i = 0; i < 2; ++i) {
        std::string measurementDirName = createDirectory(measurementBatchDirName + "/" + std::to_string(i));
        std::string resultDirName = createDirectory(resultBatchDirName + "/" + std::to_string(i));

        NexmarkQuery nexmarkQuery;
        nexmarkQuery.setMeasurementDirName(measurementDirName);
        nexmarkQuery.setResultDirName(resultDirName);

//        nexmarkQuery.runQuery1("../../../other/bids.csv", "process_time_q1.csv", "q1result.csv");

//        nexmarkQuery.runQuery2_Filter("../../../other/bids.csv", "process_time_q2_filter.csv", "q2result_filter.csv");
//        nexmarkQuery.runQuery2_Map(resultDirName +  "/q2result_filter.csv", "process_time_q2_map.csv", "q2result_map.csv");

//        nexmarkQuery.runQuery3_FilterPerson("../../../other/people.csv", "process_time_q3_filter_person_1_state.csv", "q3result_filter_person_1_state.csv");
//        nexmarkQuery.runQuery3_FilterAuction("../../../other/auction.csv", "process_time_q3_filter_auction.csv", "q3result_filter_auction.csv");
//        nexmarkQuery.runQuery3_JoinPersonAuction("../../../other/people.csv", "../../../other/auction.csv", "q3_join_1500.csv", "q3result_join_1500.csv");
//        nexmarkQuery.runQuery3_MapJoinResult(resultDirName + "/q3result_join_600.csv", "process_time_q3_map_600.csv", "q3result_map_600.csv");

//        nexmarkQuery.runQuery4_JoinAuctionBid("../../../other/auction.csv", "../../../other/bids.csv", "q4_join1_1500.csv", "q4result_join1_1500.csv");
//        nexmarkQuery.runQuery4_MapAuctionBid(resultDirName +  "/q4result_join1_600.csv", "process_time_q4_map1_600.csv", "q4result_map1_600.csv");

        nexmarkQuery.runQuery5_CountByAuction("../../../other/bids.csv", "process_time_q5_count_by_auction_1500.csv", "q5result_count_by_auction_1500.csv");
        nexmarkQuery.runQuery5_MaxBatch(resultDirName + "/q5result_count_by_auction_1500.csv", "process_time_q5_maxbatch_1500.csv", "q5result_maxbatch_1500.csv");

//        nexmarkQuery.runQuery6_JoinAuctionBid("../../../other/auction.csv", "../../../other/bids.csv", "q6_join_1500.csv", "q6result_join_1500.csv");
//        nexmarkQuery.runQuery6_Filter(resultDirName + "/q6result_join_900.csv", "process_time_q6_filter_900.csv", "q6result_filter_900.csv");
//        nexmarkQuery.runQuery6_Max(resultDirName + "/q6result_filter_600.csv", "process_time_q6_max_partition_500.csv", "q6result_max_partition_500.csv");
//        nexmarkQuery.runQuery6_Avg(resultDirName + "/q6result_max_partition.csv", "process_time_q6_avg_500.csv", "q6result_avg_500.csv");

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

