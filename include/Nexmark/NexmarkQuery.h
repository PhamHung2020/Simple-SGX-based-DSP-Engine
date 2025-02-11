//
// Created by hungpm on 16/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_NEXMARK_QUERY_H
#define SIMPLE_DSP_ENGINE_NEXMARK_QUERY_H

#include "App/utils.h"

class NexmarkQuery {
public:
    void setMeasurementDirName(std::string& measurementDirName);
    void setResultDirName(std::string& resultDirName);

    void runQuery1(std::string, std::string, std::string);

    void runQuery2_Filter(std::string, std::string, std::string);
    void runQuery2_Map(std::string, std::string, std::string);

    void runQuery3_FilterPerson(std::string, std::string, std::string);
    void runQuery3_FilterAuction(std::string, std::string, std::string);
    void runQuery3_JoinPersonAuction(std::string, std::string, std::string, std::string);
    void runQuery3_MapJoinResult(std::string, std::string, std::string);

    void runQuery4_JoinAuctionBid(std::string, std::string, std::string, std::string);
    void runQuery4_MapAuctionBid(std::string, std::string, std::string);
    void runQuery4_Max(std::string, std::string, std::string);
    void runQuery4_JoinCategory(std::string, std::string, std::string);
    void runQuery4_Average(std::string, std::string, std::string);

    void runQuery5_CountByAuction(std::string, std::string, std::string);
    void runQuery5_MaxBatch(std::string, std::string, std::string);

    void runQuery6_JoinAuctionBid(std::string, std::string, std::string, std::string);
    void runQuery6_Filter(std::string, std::string, std::string);
    void runQuery6_Max(std::string, std::string, std::string);
    void runQuery6_Avg(std::string, std::string, std::string);

    void runQuery7_MaxJoin(std::string, std::string, std::string);

    void runQuery8_JoinPersonAuction(std::string, std::string, std::string, std::string);
    void runQuery8_MapJoinResult(std::string, std::string, std::string);

protected:
    std::string measurementDirName_;
    std::string resultDirName_;

    void setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
    void cleanConfiguration_(ConfigurationTesting* config);
};

#endif //SIMPLE_DSP_ENGINE_NEXMARK_QUERY_H
