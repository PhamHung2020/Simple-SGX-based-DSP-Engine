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




};

#endif //SIMPLE_DSP_ENGINE_CONFIGURATIONTESTING_H
