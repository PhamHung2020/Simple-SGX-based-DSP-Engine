#include <sgx_defs.h>
#include <iostream>
#include "Enclave_u.h"

#include "App/test.h"
#include "App/utils.h"
#include "utils.h"
#include "Nexmark/schemas.h"
#include "Nexmark/sinks.h"
#include "Nexmark/parsers.h"

using namespace std;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
//     testSimpleEngine();
//    testHotCallPerformance();
//     testObserverPerformance(0);
//    testJoin2Stream1Enclave();
//    testCpuAffinity();

  //  for (int i = 0; i < 5; ++i) {
  //      printf("Test %d.\n", i);
  //      testJoin2Stream1Enclave(i);
  //  }

//    ConfigurationTesting config;
//    config.resultDirName = "../../results/secure-sgx";
//    config.measurementDirName = "../../measurements/secure-sgx";
//
//    config.sinkFileName = "reduce_1500_2005.csv";
//    config.measurementFileName = "process_time_tail_reduce_1500_2005.csv";
////    config.sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
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

    std::string measurementBatchDirName = "../../measurements/testNexmark/crypto/data";
    std::string resultBatchDirName = "../../results/testNexmark/crypto/data";


    for (int i = 0; i < -5; ++i) {
        testNexmark2(i);
    }

    for (int i = 0; i < 1; ++i) {
        std::string measurementDirName = createDirectory(measurementBatchDirName + "/" + std::to_string(i));
        std::string resultDirName = createDirectory(resultBatchDirName + "/" + std::to_string(i));

        ConfigurationTesting config;
        config.resultDirName = resultDirName;
        config.measurementDirName = measurementDirName;

        config.sourceId = 1;
        config.sourceDelay = 0;
        config.sourceHasHeader = true;
        config.sourceCount = -1;

         config.sinkFileName = "q1result_test.csv";
         config.measurementFileName = "process_time_q1_test.csv";
         config.sourceFileName = "../../../other/bids.csv";
         config.taskId = 12;
         config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
         config.taskShouldBeObserved = true;
         config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
         config.parser = new BidParser();
         config.sink = sinkBid;
         config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q2result_filter.csv";
//        config.measurementFileName = "process_time_q2_filter.csv";
//        config.sourceFileName = "../../../other/bids.csv";
//        config.taskId = 13;
//        config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new BidParser();
//        config.sink = sinkBid;
//        config.sinkFileStream = getSinkFileStream();

//       config.sinkFileName = "q2result_map.csv";
//       config.measurementFileName = "process_time_q2_map.csv";
//       config.sourceFileName = config.resultDirName +  "/q2result_filter.csv";
//       config.taskId = 14;
//       config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//       config.taskShouldBeObserved = true;
//       config.outputDataSize = sizeof(Q2Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//       config.parser = new BidParser();
//       config.sink = sinkQ2Result;
//       config.sinkFileStream = getSinkFileStream();

//       config.sinkFileName = "q3result_filter_person_4_state.csv";
//       config.measurementFileName = "process_time_q3_filter_person_4_state.csv";
//       config.sourceFileName = "../../../other/people.csv";
//       config.taskId = 15;
//       config.taskInputDataSize = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//       config.taskShouldBeObserved = true;
//       config.outputDataSize = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//       config.parser = new PersonParser();
//       config.sink = sinkPerson;
//       config.sinkFileStream = getSinkFileStream();

//         config.sinkFileName = "q3result_filter_auction.csv";
//         config.measurementFileName = "process_time_q3_filter_auction.csv";
//         config.sourceFileName = "../../../other/auction.csv";
//         config.taskId = 16;
//         config.taskInputDataSize = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//         config.taskShouldBeObserved = true;
//         config.outputDataSize = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//         config.parser = new AuctionParser();
//         config.sink = sinkAuction;
//         config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q3result_map_600.csv";
//        config.measurementFileName = "process_time_q3_map_600.csv";
//        config.sourceFileName = resultDirName + "/q3result_join_600.csv";
//        config.taskId = 18;
//        config.taskInputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q3Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ3Result;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q3result_map_900.csv";
//        config.measurementFileName = "process_time_q3_map_900.csv";
//        config.sourceFileName = resultDirName + "/q3result_join_900.csv";
//        config.taskId = 18;
//        config.taskInputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q3Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ3Result;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q3result_map_1500.csv";
//        config.measurementFileName = "process_time_q3_map_1500.csv";
//        config.sourceFileName = resultDirName + "/q3result_join_1500.csv";
//        config.taskId = 18;
//        config.taskInputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q3Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ3Result;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q3result_map_3000.csv";
//        config.measurementFileName = "process_time_q3_map_3000.csv";
//        config.sourceFileName = resultDirName + "/q3result_join_3000.csv";
//        config.taskId = 18;
//        config.taskInputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q3Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ3Result;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);


//        config.sinkFileName = "q4result_map1_600.csv";
//        config.measurementFileName = "process_time_q4_map1_600.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_join1_600.csv";
//        config.taskId = 20;
//        config.taskInputDataSize = sizeof(Q4Join1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q4Join1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q4result_map1_900.csv";
//        config.measurementFileName = "process_time_q4_map1_900.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_join1_900.csv";
//        config.taskId = 20;
//        config.taskInputDataSize = sizeof(Q4Join1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q4Join1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q4result_map1_1500.csv";
//        config.measurementFileName = "process_time_q4_map1_1500.csv";
//        config.sourceFileName = config.resultDirName +  "/q4result_join1_1500.csv";
//        config.taskId = 20;
//        config.taskInputDataSize = sizeof(Q4Join1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q4Join1ResultParser();
//        config.sink = sinkQ4MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);


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

//        config.sinkFileName = "q5result_count_total_900.csv";
//        config.measurementFileName = "process_time_q5_count_total_900.csv";
//        config.sourceFileName = "../../../other/bids.csv";
//        config.taskId = 24;
//        config.taskInputDataSize = sizeof(Bid);
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q5CountTotalResult);
//        config.parser = new BidParser();
//        config.sink = sinkQ5CountTotalResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q5result_count_by_auction_1500.csv";
//        config.measurementFileName = "process_time_q5_count_by_auction_1500.csv";
//        config.sourceFileName = "../../../other/bids.csv";
//        config.taskId = 24;
//        config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new BidParser();
//        config.sink = sinkQ5CountByAuctionResult;
//        config.sinkFileStream = getSinkFileStream();
//        config.sourceCount = 5000;

//        config.sinkFileName = "q5result_maxbatch_1500.csv";
//        config.measurementFileName = "process_time_q5_maxbatch_1500.csv";
//        config.sourceFileName = resultDirName + "/q5result_count_by_auction_1500.csv";
//        config.taskId = 25;
//        config.taskInputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q5CountByAuctionResultParser();
//        config.sink = sinkQ5CountByAuctionResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q6result_filter_600.csv";
//        config.measurementFileName = "process_time_q6_filter_600.csv";
//        config.sourceFileName = resultDirName + "/q6result_join_600.csv";
//        config.taskId = 27;
//        config.taskInputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q6JoinResultParser();
//        config.sink = sinkQ6JoinResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q6result_max_partition.csv";
//        config.measurementFileName = "process_time_q6_max_partition.csv";
//        config.sourceFileName = resultDirName + "/q6result_filter_600.csv";
//        config.taskId = 28;
//        config.taskInputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q6MaxResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q6JoinResultParser();
//        config.sink = sinkQ6MaxResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q6result_avg.csv";
//        config.measurementFileName = "process_time_q6_avg.csv";
//        config.sourceFileName = resultDirName + "/q6result_max_partition.csv";
//        config.taskId = 29;
//        config.taskInputDataSize = sizeof(Q6MaxResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q6MaxResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q6MaxResultParser();
//        config.sink = sinkQ6MaxResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q7result_max_900.csv";
//        config.measurementFileName = "process_time_q7_max_900.csv";
//        config.sourceFileName = "../../../other/bids.csv";
//        config.taskId = 31;
//        config.taskInputDataSize = sizeof(Bid);
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(uint64_t);
//        config.parser = new BidParser();
//        config.sink = sinkQ7MaxResult;
//        config.sinkFileStream = getSinkFileStream();

//        config.sinkFileName = "q7result_maxjoin_1500.csv";
//        config.measurementFileName = "process_time_q7_maxjoin_1500.csv";
//        config.sourceFileName = "../../../other/bids.csv";
//        config.taskId = 30;
//        config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new BidParser();
//        config.sink = sinkBid;
//        config.sinkFileStream = getSinkFileStream();
//        config.sourceCount = 200000;

//        config.sinkFileName = "q8result_map_600.csv";
//        config.measurementFileName = "process_time_q8_map_600.csv";
//        config.sourceFileName = config.resultDirName +  "/q8result_join_600.csv";
//        config.taskId = 32;
//        config.taskInputDataSize = sizeof(Q8JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q8MapResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ8MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
        testNexmark(config);
//
//        config.sinkFileName = "q8result_map_900.csv";
//        config.measurementFileName = "process_time_q8_map_900.csv";
//        config.sourceFileName = config.resultDirName +  "/q8result_join_900.csv";
//        config.taskId = 32;
//        config.taskInputDataSize = sizeof(Q8JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q8MapResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ8MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);
//
//        config.sinkFileName = "q8result_map_1500.csv";
//        config.measurementFileName = "process_time_q8_map_1500.csv";
//        config.sourceFileName = config.resultDirName +  "/q8result_join_1500.csv";
//        config.taskId = 32;
//        config.taskInputDataSize = sizeof(Q8JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.taskShouldBeObserved = true;
//        config.outputDataSize = sizeof(Q8MapResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
//        config.parser = new Q3JoinResultParser();
//        config.sink = sinkQ8MapResult;
//        config.sinkFileStream = getSinkFileStream();
//
//        testNexmark(config);

        //  config.sinkFileName = "map.csv";
        //  config.measurementFileName = "process_time_map.csv";
        //  config.sourceFileName = "../../dataset/secure-sgx-dataset/2006.csv";
        //  config.sourceId = 1;
        //  config.sourceDelay = 0;
        //  config.sourceHasHeader = true;
        //  config.sourceCount = 8000000;

        //  config.taskId = 4;
        //  config.taskInputDataSize = 200;
        //  config.taskShouldBeObserved = true;
        //  config.outputDataSize = sizeof(FlightData);

        //  testObserverPerformance(config);

      //  config.sinkFileName = "filter.csv";
      //  config.measurementFileName = "process_time_filter.csv";
      //  config.sourceFileName = config.resultDirName + "/map.csv";
      //  config.sourceHasHeader = false;
      //  config.sourceCount = 8000000;

      //  config.taskId = 5;
      //  config.taskInputDataSize = sizeof(FlightData);
      //  config.taskShouldBeObserved = true;
      //  config.outputDataSize = sizeof(FlightData);

      //  testObserverPerformance(config);

      // config.sinkFileName = "reduce_50.csv";
      // config.measurementFileName = "process_time_reduce_50.csv";
      // config.sourceFileName = config.resultDirName + "/filter.csv";
      // config.sourceHasHeader = false;
      // config.sourceCount = 8000000;

      // config.taskId = 6;
      // config.taskInputDataSize = sizeof(FlightData);
      // config.taskShouldBeObserved = true;
      // config.outputDataSize = sizeof(ReducedFlightData);

      // testObserverPerformance(config);

        //   config.sinkFileName = "map_flight_to_flight.csv";
        //   config.measurementFileName = "process_time_map_flight_to_flight.csv";
        //   config.sourceFileName = config.resultDirName + "/map.csv";
        //   config.sourceHasHeader = false;
        //   config.sourceCount = 8000000;

        //   config.taskId = 9;
        //   config.taskInputDataSize = sizeof(FlightData);
        //   config.taskShouldBeObserved = true;
        //   config.outputDataSize = sizeof(FlightData);

        //   testObserverPerformance(config);

//       config.sinkFileName = "filter_carrier_delay_2.csv";
//       config.measurementFileName = "process_time_filter_carrier_delay_2.csv";
//       config.sourceFileName = config.resultDirName + "/map.csv";
//       config.sourceHasHeader = false;
//       config.sourceCount = 8000000;
//
//       config.taskId = 10;
//       config.taskInputDataSize = sizeof(FlightData);
//       config.taskShouldBeObserved = true;
//       config.outputDataSize = sizeof(FlightData);
//
//       testObserverPerformance(config);

//       config.sinkFileName = "reduce_delay_200.csv";
//       config.measurementFileName = "process_time_reduce_delay_200.csv";
//       config.sourceFileName = config.resultDirName + "/map.csv";
//       config.sourceHasHeader = false;
//       config.sourceCount = 8000000;
//
//       config.taskId = 11;
//       config.taskInputDataSize = sizeof(FlightData);
//       config.taskShouldBeObserved = true;
//       config.outputDataSize = sizeof(ReducedFlightData);
//
//       testObserverPerformance(config);
    }


    return 0;
}

