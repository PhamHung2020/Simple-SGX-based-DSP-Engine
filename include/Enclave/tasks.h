//
// Created by hungpm on 24/03/2024.
//

#ifndef TASKS_H
#define TASKS_H

#include <vector>
#include "data_types.h"

#define TASK_COUNT 37

/* === STREAM BOX === */

void TopValuePerKey(void* data);
//void AggregateSensorData(void* data);
//void JoinSyntheticData(void* data);

/* === STREAM BOX === */

//void MapCsvRowToFlight(void* data);
//void FilterFlight(void* data);
//void ReduceFlight(void* data);
//void JoinFlight(void* data);
//void JoinFlight2Stream(void* data);

//void MapFlightToFlight(void* data);
//void FilterCarrierAndDelay(void* data);
//void ReduceDelay(void* data);

//void NexmarkQ1(void* data);
//void NexmarkQ2_Filter(void* data);
//void NexmarkQ2_Map(void* data);
//void NexmarkQ3_FilterPerson(void* data);
//void NexmarkQ3_FilterAuction(void* data);
//void NexmarkQ3_JoinPersonAuction(void* data);
//void NexmarkQ3_MapResult(void* data);
//void NexmarkQ4_JoinAuctionBid(void* data);
//void NexmarkQ4_MapAuctionBid(void* data);
//void NexmarkQ4_MaxAuctionPriceByCategory(void* data);
//void NexmarkQ4_JoinWithCategory(void* data);
//void NexmarkQ4_Average(void* data);

//void NexmarkQ5_CountTotal(void* data);
//void NexmarkQ5_CountByAuction(void* data);
//void NexmarkQ5_MaxBatch(void* data);
//void NexmarkQ5_MaxWindow(void* data);

//void NexmarkQ6_Join(void* data);
//void NexmarkQ6_Filter(void* data);
//void NexmarkQ6_Max(void* data);
//void NexmarkQ6_Avg(void* data);

//void NexmarkQ7_Max(void* data);
//void NexmarkQ7_Join(void* data);
//void NexmarkQ7_MaxJoin(void* data);
//
//void NexmarkQ8_JoinPersonAuction(void* data);
//void NexmarkQ8_Map(void* data);
//
//void NexmarkQA_JoinPersonBid(void* data);
//void testDecryption(void* data);
//void Nexmark_AvgPartition(void* data);

static void (*callbacks[TASK_COUNT])(void*) = {
        TopValuePerKey,
//        AggregateSensorData,
//        JoinSyntheticData

//        MapCsvRowToFlight,
//        FilterFlight,
//        ReduceFlight,
//        JoinFlight,
//        JoinFlight2Stream,
//        MapFlightToFlight,
//        FilterCarrierAndDelay,
//        ReduceDelay,
//        NexmarkQ1,
//        NexmarkQ2_Filter,
//        NexmarkQ2_Map,
//        NexmarkQ3_FilterPerson,
//        NexmarkQ3_FilterAuction,
//        NexmarkQ3_JoinPersonAuction,
//        NexmarkQ3_MapResult,
//        NexmarkQ4_JoinAuctionBid,
//        NexmarkQ4_MapAuctionBid,
//        NexmarkQ4_MaxAuctionPriceByCategory,
//        NexmarkQ4_JoinWithCategory,
//        NexmarkQ4_Average,

//        NexmarkQ5_CountTotal, NOT USED

//        NexmarkQ5_CountByAuction,
//        NexmarkQ5_MaxBatch,

//        NexmarkQ5_MaxWindow, NOT USED
//        NexmarkQ6_Join,
//        NexmarkQ6_Filter,
//        NexmarkQ6_Max,
//        NexmarkQ6_Avg,

//        NexmarkQ7_Max, NOT USED
//        NexmarkQ7_Join, NOT USED

//        NexmarkQ7_MaxJoin,
//        NexmarkQ8_JoinPersonAuction,
//        NexmarkQ8_Map,
//        NexmarkQA_JoinPersonBid,
//        testDecryption,
//        Nexmark_AvgPartition
};

#if defined(__cplusplus)
extern "C" {
#endif
    extern int task3Result;
    extern int task3WindowLength;
    extern int task3Count;

    extern std::vector<ReducedFlightData> reducedDatas;
#if defined(__cplusplus)
}
#endif


#endif //TASKS_H
