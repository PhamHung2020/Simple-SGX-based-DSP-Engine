//
// Created by hungpm on 14/02/2025.
//

#ifndef STREAM_BOX_PARSERS_H
#define STREAM_BOX_PARSERS_H

#include "schemas.h"
#include "../Source/Parser.h"

class SensorDataParser : public Parser {
private:
    SensorData* data_ = nullptr;
public:
    SensorDataParser();
    ~SensorDataParser() override;
    void *parseFromString(const std::string &str) override;
};

//class SensorAggregationDataParser : public Parser {
//private:
//    SensorData* data_ = nullptr;
//public:
//    SensorAggregationData();
//    ~SensorAggregationData();
//    void *parseFromString(const std::string &str) override;
//};

class SyntheticDataParser : public Parser {
private:
    SyntheticData* data_ = nullptr;
public:
    SyntheticDataParser();
    ~SyntheticDataParser() override;
    void *parseFromString(const std::string &str) override;
};

class TripParser : public Parser {
private:
    TripData* data_ = nullptr;
public:
    TripParser();
    ~TripParser() override;
    void *parseFromString(const std::string &str) override;
};

#endif //STREAM_BOX_PARSERS_H
