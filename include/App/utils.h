//
// Created by hungpm on 27/03/2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "Source/Parser.h"
#include "data_types.h"
#include "ConfigurationTesting.h"
#include "Engine/EngineWithBufferObserver.h"
#include "Engine/Simple2SourceObserverEngine.h"

std::string createMeasurementsDirectory(const std::string &pathToDir);

std::string createDirectory(const std::string &pathToDir);

void getTimeStamp(char *timestamp, size_t size);

bool isDirectoryExists(const std::string& path);

class FlightDataParser :  public Parser {
private:
    FlightData* pFlightData_ = nullptr;
public:
    FlightDataParser();
    void *parseFromString(const std::string& str) override;
    ~FlightDataParser();
};

class FlightFullDataParser :  public Parser {
private:
    FlightFullData* pFlightData_ = nullptr;
public:
    FlightFullDataParser();
    void *parseFromString(const std::string& str) override;
    ~FlightFullDataParser() override;
};

class FlightDataIntermediateParser : public Parser {
private:
    FlightData* pFlightData_ = nullptr;
public:
    FlightDataIntermediateParser();
    void *parseFromString(const std::string& str) override;
    ~FlightDataIntermediateParser();
};

void writeObservedMeasurementToFile(const std::string& pathToFile, const EngineWithBufferObserver::ObservedData& observedData);
void writeObservedMeasurementToFile(const std::string& pathToFile, const Simple2SourceObserverEngine::ObservedData& observedData);

void runEngineWithBufferObserverCrypto(ConfigurationTesting& config, bool shouldCreateMeasurementDirectory = false, bool shouldCreateResultDirectory = false);
void runSimple2SourceObserverEngine(ConfigurationTesting& config, bool shouldCreateMeasurementDirectory = false, bool shouldCreateResultDirectory = false);

#endif //UTILS_H
