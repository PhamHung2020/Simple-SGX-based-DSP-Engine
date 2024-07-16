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
protected:
    std::string measurementDirName_;
    std::string resultDirName_;
};

#endif //SIMPLE_DSP_ENGINE_NEXMARK_QUERY_H
