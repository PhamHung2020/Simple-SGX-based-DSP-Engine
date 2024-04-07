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

std::string measurementDirName = "../../measurements/callbacks/2024-04-07_22-54-22";
std::string measurementFilename = "reduce_Enclave_0";
std::string resultDirName = "../../results/callbacks/2024-04-07_22-54-20";
std::string sinkFileName = "reduce.csv";
//std::string sourceFileName = "../../dataset/secure-sgx-dataset/2005.csv";
std::string sourceFileName = "../../results/callbacks/2024-04-07_22-54-20/filter.csv";
std::ofstream fout;

void testSimpleEngine_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

//    const auto flightData = static_cast<FlightData*>(rawData);
//    fout << flightData->uniqueCarrier << "," << flightData->arrDelay << std::endl;

    const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    fout << reducedFlight->uniqueCarrier << "," << reducedFlight->count << "," << reducedFlight->total << std::endl;
}

void writeMeasurement() {
    std::string fileFullPath;
    if (measurementDirName == "../../measurements/callbacks") {
        fileFullPath = createMeasurementsDirectory(measurementDirName);;
    } else {
        fileFullPath = measurementDirName;
    }
    fileFullPath.append("/").append(measurementFilename);

    std::ofstream measurementFile;
    measurementFile.open(fileFullPath, std::ios::app);

    std::cout << "Writing measurements for enclave 0" << std::endl;

    for (size_t i = 1; i < SimpleEngine::timePoints.size(); ++i) {
        measurementFile << std::chrono::duration_cast<std::chrono::nanoseconds>(SimpleEngine::timePoints[i] - SimpleEngine::timePoints[i-1]).count() << std::endl;
    }

    measurementFile.close();
}

void testSimpleEngine() {
//    FlightDataParser flightDataParser;
    FlightDataIntermediateParser parser;
    FastCallEmitter emitter;
    CsvSource source1(1, sourceFileName, 0, false, 100);
    source1.setParser(&parser);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    // map
//    engine.addTask(4, 200);

    // filter
//    engine.addTask(5, sizeof(FlightData));

    // reduce
     engine.addTask(6, sizeof(FlightData));

    // join
    //engine.addTask(7, sizeof(FlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(ReducedFlightData));

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

    writeMeasurement();


//    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
//    std::cout << "Pipline time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
//
//    const int nTask = engine.getNumberOfTask();
//    for (int i = 0; i < nTask; ++i) {
//        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
//    }

//    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    printf("Info: Engine successfully returned.\n");
}