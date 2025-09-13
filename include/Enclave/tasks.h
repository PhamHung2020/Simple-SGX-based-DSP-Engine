//
// Created by hungpm on 24/03/2024.
//

#ifndef TASKS_H
#define TASKS_H

#include <vector>
#include "data_types.h"

#define TASK_COUNT 37

/* === STREAM BOX === */

void StreamBox_Query1_TopValuePerKey(void* data);
void StreamBox_Query2_CountTrip(void* data);
void StreamBox_Query3_JoinSyntheticData(void* data);
void StreamBox_Query4_AggregateSensorData(void* data);
void StreamBox_Query5_FilterSyntheticData(void* data);
void StreamBox_Query6_MaxTrip(void* data);

/* === SECURE STREAM === */

void SecureStream_MapCsvRowToFlight(void* data);
void SecureStream_FilterFlight(void* data);
void SecureStream_ReduceFlight(void* data);

/* === NEXMARK === */

void Nexmark_Query1(void* data);
void Nexmark_Query2_Filter(void* data);
void Nexmark_Query2_Map(void* data);
void Nexmark_Query3_FilterPerson(void* data);
void Nexmark_Query3_FilterAuction(void* data);
void Nexmark_Query3_JoinPersonAuction(void* data);
void Nexmark_Query3_MapResult(void* data);
void Nexmark_Query4_JoinAuctionBid(void* data);
void Nexmark_Query4_MapAuctionBid(void* data);
void Nexmark_Query4_MaxAuctionPriceByCategory(void* data);
void Nexmark_Query4_JoinWithCategory(void* data);
void Nexmark_Query4_Average(void* data);

//void NexmarkQ5_CountTotal(void* data);
void Nexmark_Query5_CountByAuction(void* data);
void Nexmark_Query5_MaxBatch(void* data);
//void NexmarkQ5_MaxWindow(void* data);

void Nexmark_Query6_Join(void* data);
void Nexmark_Query6_Filter(void* data);
void Nexmark_Query6_Max(void* data);
void Nexmark_Query6_Avg(void* data);

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
        SecureStream_MapCsvRowToFlight,
        SecureStream_FilterFlight,
        SecureStream_ReduceFlight,
        Nexmark_Query1,
        Nexmark_Query2_Filter,
        Nexmark_Query2_Map,
        Nexmark_Query3_FilterPerson,
        Nexmark_Query3_FilterAuction,
        Nexmark_Query3_JoinPersonAuction,
        Nexmark_Query3_MapResult,
        Nexmark_Query4_JoinAuctionBid,
        Nexmark_Query4_MapAuctionBid,
        Nexmark_Query4_MaxAuctionPriceByCategory,
        Nexmark_Query4_JoinWithCategory,
        Nexmark_Query4_Average,

//        NexmarkQ5_CountTotal, NOT USED

        Nexmark_Query5_CountByAuction,
        Nexmark_Query5_MaxBatch,

//        NexmarkQ5_MaxWindow, NOT USED
        Nexmark_Query6_Join,
        Nexmark_Query6_Filter,
        Nexmark_Query6_Max,
        Nexmark_Query6_Avg,

//        NexmarkQ7_Max, NOT USED
//        NexmarkQ7_Join, NOT USED

//        NexmarkQ7_MaxJoin,
//        NexmarkQ8_JoinPersonAuction,
//        NexmarkQ8_Map,
//        NexmarkQA_JoinPersonBid,
//        testDecryption,
//        Nexmark_AvgPartition

        StreamBox_Query1_TopValuePerKey,
        StreamBox_Query2_CountTrip,
        StreamBox_Query3_JoinSyntheticData,
        StreamBox_Query4_AggregateSensorData,
        StreamBox_Query5_FilterSyntheticData,
        StreamBox_Query6_MaxTrip
//        JoinSyntheticData,
//        FilterSyntheticData
};

#endif //TASKS_H
