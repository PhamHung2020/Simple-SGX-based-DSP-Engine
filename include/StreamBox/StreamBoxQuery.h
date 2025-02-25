//
// Created by hungpm on 14/02/2025.
//

#ifndef STREAM_BOX_STREAMBOXQUERY_H
#define STREAM_BOX_STREAMBOXQUERY_H

#include "App/utils.h"

class StreamBoxQuery {
public:
    void setMeasurementDirName(std::string& measurementDirName);
    void setResultDirName(std::string& resultDirName);

    void runQuery1(std::string, std::string, std::string);
    void runQuery3(std::string, std::string, std::string, std::string);
    void runQuery4(std::string, std::string, std::string);

protected:
    std::string measurementDirName_;
    std::string resultDirName_;

    void setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName);
    void cleanConfiguration_(ConfigurationTesting* config);
};

#endif //STREAM_BOX_STREAMBOXQUERY_H
