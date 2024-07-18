//
// Created by hungpm on 08/05/2024.
//

#ifndef SIMPLE_DSP_ENGINE_CONFIGURATIONTESTING_H
#define SIMPLE_DSP_ENGINE_CONFIGURATIONTESTING_H

#include <string>
#include <fstream>
#include "Source/Parser.h"

class ConfigurationTesting {
public:
    std::string resultDirName;
    std::string measurementDirName;

    std::string sinkFileName;
    std::string sourceFileName;
    std::string measurementFileName;

    int sourceId = 1;
    int sourceDelay = 0;
    bool sourceHasHeader = false;
    int sourceCount = 1000000;

    int taskId;
    int taskInputDataSize;
    int taskShouldBeObserved;

    int outputDataSize;
    void (*sink)(void *);
    std::ofstream* sinkFileStream;

    Parser* parser = nullptr;

    int sourceId1 = 1;
    int sourceDelay1 = 0;
    bool sourceHasHeader1 = false;
    int sourceCount1 = 1000000;
    std::string sourceFileName1;

    int sourceId2 = 1;
    int sourceDelay2 = 0;
    bool sourceHasHeader2 = false;
    int sourceCount2 = 1000000;
    std::string sourceFileName2;

    Parser* parser1 = nullptr;
    Parser* parser2 = nullptr;

    int taskInputDataSize1;
    int taskInputDataSize2;


};

#endif //SIMPLE_DSP_ENGINE_CONFIGURATIONTESTING_H
