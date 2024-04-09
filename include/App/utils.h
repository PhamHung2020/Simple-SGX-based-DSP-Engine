//
// Created by hungpm on 27/03/2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "Source/Parser.h"
#include "data_types.h"

std::string createMeasurementsDirectory(const std::string &pathToDir);

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

class FlightDataIntermediateParser : public Parser {
private:
    FlightData* pFlightData_ = nullptr;
public:
    FlightDataIntermediateParser();
    void *parseFromString(const std::string& str) override;
    ~FlightDataIntermediateParser();
};

#endif //UTILS_H
