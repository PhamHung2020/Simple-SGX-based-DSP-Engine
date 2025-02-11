//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "data_types.h"
#include "Enclave/enclave_operators.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"
#include "Nexmark/schemas.h"
#include "sgx_trts_exception.h"

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdexcept>

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;
char encryptedData[1000];

bool isPalindrome(uint64_t num) {
    uint64_t original = num;
    uint64_t reversed = 0;

    while (num > 0) {
        uint64_t digit = num % 10;
        reversed = reversed * 10 + digit;
        num /= 10;
    }

    return original == reversed;
}

bool isPrime(uint64_t number)
{
    uint64_t sqrt_number = static_cast<uint64_t>(std::sqrt(number));
    for(uint64_t i=2; i<=sqrt_number; i++)
    {
       if(number%i==0)
          return false;
    }  
    return true;
}

void MapCsvRowToFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightFullData*> (data);

    FastCall_request_encrypt2(globalFastOCall, flightData, encryptedData);
}

bool FilterFlight_condition(FlightFullData* flightData) {
    return true;

}

void FilterFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightFullData*> (data);

    if (FilterFlight_condition(flightData)) {
//        FastCall_request(globalFastOCall, flightData);
        FastCall_request_encrypt2(globalFastOCall, flightData, encryptedData);
    }
}

const uint64_t reduceWindow = 5000;const uint64_t reduceStep = 3600;
// uint16_t reduceWindow = 100;
// uint16_t reduceStep = 10;
uint16_t reduceCurrentStep = reduceStep;
std::vector<FlightFullData*> receivedFlightData;
void ReduceFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightFullData*> (data);

    // if window reached max size --> delete old data
    if (receivedFlightData.size() >= reduceWindow) {
        delete receivedFlightData[0];
        receivedFlightData.erase(receivedFlightData.begin());
    }

    // add new data
    const auto newFlightData = new FlightFullData;
    strncpy(newFlightData->uniqueCarrier, flightData->uniqueCarrier, 10);
    newFlightData->arrDelay = flightData->arrDelay;
    receivedFlightData.push_back(newFlightData);

    if (receivedFlightData.size() < reduceWindow) {
        return;
    }

    reduceCurrentStep++;

    if (reduceCurrentStep >= reduceStep) {
        reduceCurrentStep = 0;

        std::unordered_map<std::string, std::pair<uint32_t, int>> reducedDataMap;
        for (const auto& storedFlight : receivedFlightData) {
            const auto carrier = std::string(storedFlight->uniqueCarrier);
            if (reducedDataMap.find(carrier) != reducedDataMap.end()) {
                reducedDataMap[carrier].first += 1;
                reducedDataMap[carrier].second += storedFlight->arrDelay;
            } else {
                reducedDataMap[carrier].first = 1;
                reducedDataMap[carrier].second = storedFlight->arrDelay;
            }
        }

        for (auto& reducedData : reducedDataMap) {
            ReducedFlightData reducedFlightData{};
            strncpy(reducedFlightData.uniqueCarrier, reducedData.first.c_str(), 8);
            reducedFlightData.count = reducedData.second.first;
            reducedFlightData.total = reducedData.second.second;

//            FastCall_request(globalFastOCall, &reducedFlightData);
            FastCall_request_encrypt2(globalFastOCall, &reducedFlightData, encryptedData);
        }
    }
}