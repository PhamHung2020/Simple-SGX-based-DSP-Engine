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

protected:
    std::string measurementDirName_;
    std::string resultDirName_;

    void setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
};

#endif //SIMPLE_DSP_ENGINE_NEXMARK_QUERY_H
