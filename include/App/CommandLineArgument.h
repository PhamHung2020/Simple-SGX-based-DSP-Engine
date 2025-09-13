//
// Created by hungpm on 11/09/2025.
//

#ifndef SIMPLE_DSP_ENGINE_COMMANDLINEARGUMENT_H
#define SIMPLE_DSP_ENGINE_COMMANDLINEARGUMENT_H

#include <string>
#include <cstdint>

class CommandLineArgument {
protected:
    std::string benchmarkName_;
    std::string queryName_;
    std::string sourceFilePath_;
    std::string sourceFilePath2_;
    std::string measurementDirName_;
    std::string resultDirName_;
    uint32_t numberOfRuns_;
public:
    CommandLineArgument(int argc, char *argv[]);
    std::string getBenchmarkName() const;
    std::string getQueryName() const;
    std::string getSourceFilePath() const;
    std::string getSourceFilePath2() const;
    std::string getMeasurementDirName() const;
    std::string getResultDirName() const;
    uint32_t getNumberOfRuns() const;
};

#endif //SIMPLE_DSP_ENGINE_COMMANDLINEARGUMENT_H
