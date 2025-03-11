//
// Created by hungpm on 14/02/2025.
//

#include "StreamBox/parsers.h"

#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>

SensorDataParser::SensorDataParser() {
    this->data_ = new SensorData();
}

SensorDataParser::~SensorDataParser() {
    delete this->data_;
}

void *SensorDataParser::parseFromString(const std::string &str) {
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
        if (words.size() != 8) {
            return nullptr;
        }

        if (!words[0].empty()) {
            strncpy(this->data_->date, words[0].c_str(), 16);
        } else {
            strncpy(this->data_->date, "UNKNOWN", 16);
        }

        if (!words[1].empty()) {
            strncpy(this->data_->time, words[1].c_str(), 16);
        } else {
            strncpy(this->data_->time, "UNKNOWN", 16);
        }

        if (!words[2].empty()) {
            this->data_->epoch = std::stoul(words[2]);
        } else {
            this->data_->epoch = 0;
        }

        if (!words[3].empty()) {
            this->data_->moteId = std::stoul(words[3]);
        } else {
            this->data_->moteId = 0;
        }

        if (!words[4].empty()) {
            this->data_->temperature = std::stod(words[4]);
        } else {
            this->data_->temperature = 0;
        }

        if (!words[5].empty()) {
            this->data_->humidity = std::stod(words[5]);
        } else {
            this->data_->humidity = 0;
        }

        if (!words[6].empty()) {
            this->data_->light = std::stod(words[6]);
        } else {
            this->data_->light = 0;
        }

        if (!words[7].empty()) {
            this->data_->voltage = std::stod(words[7]);
        } else {
            this->data_->voltage = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->data_;
}

SyntheticDataParser::SyntheticDataParser() {
    this->data_ = new SyntheticData();
}

SyntheticDataParser::~SyntheticDataParser() {
    delete this->data_;
}

void *SyntheticDataParser::parseFromString(const std::string &str) {
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
        if (words.size() != 2) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->data_->key = std::stoi(words[0]);
        } else {
            return nullptr;
//            this->data_->key = 0;
        }

        if (!words[1].empty()) {
            this->data_->value = std::stoi(words[1]);
        } else {
            return nullptr;
//            this->data_->value = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->data_;
}

TripParser::TripParser() {
    this->data_ = new TripData();
}

TripParser::~TripParser() {
    delete this->data_;
}

void *TripParser::parseFromString(const std::string &str) {
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
        if (words.size() != 14) {
            return nullptr;
        }

        if (!words[0].empty()) {
            strncpy(this->data_->medallion, words[0].c_str(), 64);
        } else {
            strncpy(this->data_->medallion, "UNKNOWN", 64);
        }

        if (!words[1].empty()) {
            strncpy(this->data_->hackLicense, words[1].c_str(), 64);
        } else {
            strncpy(this->data_->hackLicense, "UNKNOWN", 64);
        }

        if (!words[5].empty()) {
            strncpy(this->data_->pickupDateTime, words[5].c_str(), 64);
        } else {
            strncpy(this->data_->pickupDateTime, "UNKNOWN", 64);
        }

        if (!words[6].empty()) {
            strncpy(this->data_->dropOffDateTime, words[6].c_str(), 64);
        } else {
            strncpy(this->data_->dropOffDateTime, "UNKNOWN", 64);
        }

        if (!words[8].empty()) {
            this->data_->tripTimeInSecs = std::stoul(words[8]);
        } else {
            this->data_->tripTimeInSecs = 0;
        }

        if (!words[9].empty()) {
            this->data_->tripDistance = std::stod(words[9]);
        } else {
            this->data_->tripDistance = -1;
        }

        if (!words[10].empty()) {
            this->data_->pickupLongitude = std::stod(words[10]);
        } else {
            this->data_->pickupLongitude = 0;
        }

        if (!words[11].empty()) {
            this->data_->pickupLatitude = std::stod(words[11]);
        } else {
            this->data_->pickupLatitude = 0;
        }

        if (!words[12].empty()) {
            this->data_->dropOffLongitude = std::stod(words[12]);
        } else {
            this->data_->dropOffLatitude = 0;
        }

        if (!words[13].empty()) {
            this->data_->dropOffLatitude = std::stod(words[13]);
        } else {
            this->data_->dropOffLatitude = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->data_;
}