//
// Created by hungpm on 23/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_SECURE_SGX_QUERY_H
#define SIMPLE_DSP_ENGINE_SECURE_SGX_QUERY_H

#include "App/ConfigurationTesting.h"

class SecureSgxQuery {
public:
    void setMeasurementDirName(std::string& measurementDirName);
    void setResultDirName(std::string& resultDirName);

    void runMapQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
    void runFilterQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
    void runReduceQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);

protected:
    std::string measurementDirName_;
    std::string resultDirName_;

    void setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
    void cleanConfiguration_(ConfigurationTesting* config);
};

#endif //SIMPLE_DSP_ENGINE_SECURE_SGX_QUERY_H
