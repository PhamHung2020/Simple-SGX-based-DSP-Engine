//
// Created by hungpm on 07/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_NEXMARK_SINKS_H
#define SIMPLE_DSP_ENGINE_NEXMARK_SINKS_H

#include <fstream>
#include "schemas.h"

#if defined(__cplusplus)
extern "C" {
#endif

    extern std::ofstream nexmarkSinkFileStream;

#if defined(__cplusplus)
}
#endif

std::ofstream* getSinkFileStream();
void sinkBid(void* rawData);
void sinkPerson(void* rawData);
void sinkAuction(void* rawData);
void sinkQ2Result(void* rawData);
void sinkQ3JoinResult(void* rawData);
void sinkQ3Result(void* rawData);
void sinkQ4Join1Result(void* rawData);
void sinkQ4MapResult(void* rawData);
void sinkQ4AverageResult(void* rawData);
void sinkQ5CountTotalResult(void* rawData);
void sinkQ5CountByAuctionResult(void* rawData);
void sinkQ6JoinResult(void* rawData);
void sinkQ6MaxResult(void* rawData);
void sinkQ6AverageResult(void* rawData);
void sinkQ7MaxResult(void* rawData);
void sinkQ8JoinResult(void* rawData);
void sinkQ8MapResult(void* rawData);
void sinkQAJoinPersonBidResult(void* rawData);
void sinkChar(void* rawData);
void sinkUint64(void* rawData);

#endif //SIMPLE_DSP_ENGINE_NEXMARK_SINKS_H
