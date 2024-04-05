//
// Created by hungpm on 28/03/2024.
//

#include "App/test.h"

#include <iostream>
#include <cstdio>
#include <chrono>
#include <cstring>

#include "Source/CsvSource.h"
#include "data_types.h"
#include "Engine/SimpleEngine.h"
#include "Source/Parser.h"

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

            this->pFlightData_->arrDelay = static_cast<int>(words.size());
            if (!words[0].empty()) {
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
        printf("Parser log: %s %d\n", this->pFlightData_->uniqueCarrier, this->pFlightData_->arrDelay);


        return this->pFlightData_;
    }

    ~FlightDataParser() {
        delete this->pFlightData_;
    }
};

void testSimpleEngine_sinkResult(void* rawData)
{
    if (rawData == NULL) {
        return;
    }

     const auto flightData = static_cast<FlightData*>(rawData);
     printf("Sink result:\n\t- Unique carrier: %s\n\t- Delay: %d\n\n", flightData->uniqueCarrier, flightData->arrDelay);

    // const auto reducedFlight = static_cast<ReducedFlightData*> (rawData);
    // printf(
    //     "Sink result:\n\t- Unique carrier: %s\n\t- Count: %d\n\t- Total: %d\n\n",
    //     reducedFlight->uniqueCarrier,
    //     reducedFlight->count,
    //     reducedFlight->total
    //     );

//    const auto joinedFlightData = static_cast<JoinedFlightData*> (rawData);
//    printf(
//        "Sink result:\n\t- Unique carrier 1: %s\n\t- Unique carrier 2: %s\n\t- Delay: %d\n\n",
//        joinedFlightData->uniqueCarrier1,
//        joinedFlightData->uniqueCarrier2,
//        joinedFlightData->arrDelay
//        );
}

void testSimpleEngine() {
    FlightDataParser flightDataParser;
    FastCallEmitter emitter;
    CsvSource source1(1, "../../dataset/secure-sgx-dataset/2005.csv", 0, true, 100);
    source1.setParser(&flightDataParser);

    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);

    // map
//    engine.addTask(4, 200);

    // filter
    engine.addTask(5, sizeof(FlightData));

    // reduce
    // engine.addTask(6, sizeof(FlightData));

    // join
//    engine.addTask(7, sizeof(FlightData));

    engine.setSink(testSimpleEngine_sinkResult, sizeof(FlightData));
    engine.start();

    std::cout << "Source time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndSourceTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;
    // std::cout << "Pipline time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(SimpleEngine::getEndPipelineTime() - SimpleEngine::getStartSourceTime()).count() << std::endl;

    const int nTask = engine.getNumberOfTask();
    for (int i = 0; i < nTask; ++i) {
        std::cout << "Enclave " << i << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(i) - SimpleEngine::getStartEnclaveTime(i)).count() << std::endl;
    }

    std::cout << "Processing time: " << std::chrono::duration_cast<std::chrono::microseconds>(SimpleEngine::getEndEnclaveTime(nTask - 1) - SimpleEngine::getStartSourceTime()).count() << std::endl;

    printf("Info: Engine successfully returned.\n");
}