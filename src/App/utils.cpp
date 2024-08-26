//
// Created by hungpm on 27/03/2024.
//

#include "App/utils.h"
#include <sys/stat.h>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <iostream>

void getTimeStamp(char *timestamp, const size_t size) {
    time_t rawtime;
    time(&rawtime);
    const struct tm *timeinfo = localtime(&rawtime);

    strftime(timestamp, size, "%Y-%m-%d_%H-%M-%S", timeinfo);
}

bool isDirectoryExists(const std::string &path) {
    struct stat st = {0};

    return stat(path.c_str(), &st) != -1;
}


std::string createMeasurementsDirectory(const std::string &pathToDir) {
    char timestamp[100] = {0};
    getTimeStamp(timestamp, 100);
    // printf( "%s\n", timestamp);

    if (!isDirectoryExists(pathToDir)) {
        printf("Creating directory %s\n", pathToDir.c_str());
        mkdir(pathToDir.c_str(), 0700);
    }

    const auto m_measurementsDir = pathToDir + "/" + timestamp;
    if (!isDirectoryExists(m_measurementsDir)) {
        printf("Creating directory %s\n", m_measurementsDir.c_str());
        mkdir(m_measurementsDir.c_str(), 0700);
    }

    return m_measurementsDir;
}

std::string createDirectory(const std::string &pathToDir) {
    if (!isDirectoryExists(pathToDir)) {
        printf("Creating directory %s\n", pathToDir.c_str());
        mkdir(pathToDir.c_str(), 0700);
    }

    return pathToDir;
}

FlightDataParser::FlightDataParser() {
    this->pFlightData_ = new FlightData;
}

void *FlightDataParser::parseFromString(const std::string &str) {
    try {
        std::vector<std::string> words;
        std::size_t previousPos = 0;
        std::size_t pos = str.find(',');
        while (pos != std::string::npos) {
            std::string word = str.substr(previousPos, pos - previousPos);
            words.push_back(word);

            previousPos = pos + 1;
            pos = str.find(',', previousPos);
        }

        const std::string word = str.substr(previousPos);
        words.push_back(word);

        if (!words[8].empty()) {
            strncpy(this->pFlightData_->uniqueCarrier, words[8].c_str(), 10);
        } else {
            strncpy(this->pFlightData_->uniqueCarrier, "UNKNOW", 10);
        }
        if (!words[14].empty() && words[14] != "NA") {
            this->pFlightData_->arrDelay = std::stoi(words[14]);
        } else {
            this->pFlightData_->arrDelay = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->pFlightData_;
}

void writeObservedMeasurementToFile(const std::string& pathToFile, const EngineWithBufferObserver::ObservedData& observedData) {
    std::ofstream measurementFile;
    measurementFile.open(pathToFile, std::ios::out);
    std::cout << "Count " << observedData.count << std::endl;

    measurementFile << "index,time\n";

    for (uint64_t i = 0; i < observedData.count; ++i) {
        if (i == 0) {
            measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[0] - observedData.startTimestamp << std::endl;
        } else {
            measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[i] - observedData.timestampCounterPoints[i-1] << std::endl;
        }
    }

    measurementFile.close();
}

void writeObservedMeasurementToFile(const std::string& pathToFile, const Simple2SourceObserverEngine::ObservedData& observedData) {
    std::ofstream measurementFile;
    measurementFile.open(pathToFile, std::ios::out);
    std::cout << "Count " << observedData.count << std::endl;

    measurementFile << "index,time\n";

    for (uint64_t i = 0; i < observedData.count; ++i) {
        if (i == 0) {
            measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[0] - observedData.startTimestamp << std::endl;
        } else {
            measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[i] - observedData.timestampCounterPoints[i-1] << std::endl;
        }
    }

    measurementFile.close();
}

FlightDataParser::~FlightDataParser() {
    delete this->pFlightData_;
}

FlightDataIntermediateParser::FlightDataIntermediateParser() {
    this->pFlightData_ = new FlightData;
}

void *FlightDataIntermediateParser::parseFromString(const std::string &str) {
    try {
        std::vector<std::string> words;
        std::size_t previousPos = 0;
        std::size_t pos = str.find(',');
        while (pos != std::string::npos) {
            std::string word = str.substr(previousPos, pos - previousPos);
            words.push_back(word);

            previousPos = pos + 1;
            pos = str.find(',', previousPos);
        }

        const std::string word = str.substr(previousPos);
        if (!word.empty())
            words.push_back(word);

        if (words.empty() || words.size() < 2) {
            std::cout << "Row invalid\n";
            return nullptr;
        }

        if (!words[0].empty()) {
            strncpy(this->pFlightData_->uniqueCarrier, words[0].c_str(), 10);
        } else {
            strncpy(this->pFlightData_->uniqueCarrier, "UNKNOW", 10);
        }
        if (!words[1].empty() && words[1] != "NA") {
            this->pFlightData_->arrDelay = std::stoi(words[1]);
        } else {
            this->pFlightData_->arrDelay = 0;
        }
    }
    catch (const std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        return nullptr;
    }

    return this->pFlightData_;
}

FlightDataIntermediateParser::~FlightDataIntermediateParser() {
    delete this->pFlightData_;
}