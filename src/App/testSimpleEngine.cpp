//
// Created by hungpm on 28/03/2024.
//

#include "App/test.h"

#include <iostream>
#include <cstdio>
#include <chrono>
#include <cstring>
#include <fstream>

#include "Source/CsvSource.h"
#include "data_types.h"
#include "Engine/SimpleEngine.h"
#include "Source/Parser.h"
#include "App/utils.h"

class FlightDataParser :  public Parser {
private:
    FlightData* pFlightData_ = nullptr;
public:
    FlightDataParser() {
        this->pFlightData_ = new FlightData;
    }

    void *parseFromString(const std::string& str) override {
        try {
            std::vector<std::string> words;
            std::size_t previousPos = 0;
            std::size_t pos = str.find(',');
            while (pos != std::string::npos)
            {
                std::string word = str.substr(previousPos, pos - previousPos);
                words.push_back(word);

                previousPos = pos + 1;
                pos = str.find(',', previousPos);
            }

            const std::string word = str.substr(pos + 1);
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
        catch(const std::invalid_argument&)
        {
            return nullptr;
        }

        return this->pFlightData_;
    }

    ~FlightDataParser() {
        delete this->pFlightData_;
    }
};

class FlightDataIntermediateParser : public Parser {
private:
    FlightData* pFlightData_ = nullptr;
public:
    FlightDataIntermediateParser() {
        this->pFlightData_ = new FlightData;
    }

    void *parseFromString(const std::string& str) override {
        try {
            std::vector<std::string> words;
            std::size_t previousPos = 0;
            std::size_t pos = str.find(',');
            while (pos != std::string::npos)
            {
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
        catch(const std::invalid_argument& e)
        {
            std::cout << e.what() << std::endl;
            return nullptr;
        }

        return this->pFlightData_;
    }

    ~FlightDataIntermediateParser() {
        delete this->pFlightData_;
    }
};

std::string resultDirName = "../../results/callbacks";
std::string sinkFileName = "map.csv";
std::string sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
std::ofstream fout;

void testSimpleEngine_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*>(rawData);
    fout << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;

//    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
//    fout << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;
}

void testSimpleEngine() {
//    FlightDataParser flightDataParser;
//    FlightDataIntermediateParser parser;
    FastCallEmitter emitter;
    CsvSource source1(1, sourceFileName, 0, true, 100);
//    source1.setParser(&parser);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    // map
    engine.addTask(4, 200);

    // filter
//    engine.addTask(5, sizeof(FlightData));

    // reduce
//     engine.addTask(6, sizeof(FlightData));

    // join
    //engine.addTask(7, sizeof(FlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(FlightData));

    // create directory and file to store processed results
    std::string fileFullPath;
    if (resultDirName == "../../results/callbacks") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    fout.open(fileFullPath, std::ios::out);
    if (fout.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    engine.start();

    fout.close();

    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
    std::cout << "Pipeline time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;

    const int nTask = engine.getNumberOfTask();
    for (int i = 0; i < nTask; ++i) {
        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
    }

    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    printf("Info: Engine successfully returned.\n");
}