//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "data_types.h"
#include "Enclave/enclave_operators.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"
//#include "Nexmark/schemas.h"
#include "StreamBox/schemas.h"
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

const uint64_t syntheticDataWindowSize = 5500;const uint64_t syntheticDataSlidingStep = 4100;
// const uint64_t syntheticDataWindowSize = 100;
// const uint64_t syntheticDataSlidingStep = 10;
uint64_t syntheticDataCurrentSlidingStep = syntheticDataSlidingStep;
std::vector<SyntheticData*> syntheticDataArr;
void TopValuePerKey(void* data) {
    if (data == NULL) {
        return;
    }

    const auto parsedData = static_cast<SyntheticData*>(data);

//     if window size reached --> delete old data
    if (syntheticDataArr.size() >= syntheticDataWindowSize) {
        delete syntheticDataArr[0];
        syntheticDataArr.erase(syntheticDataArr.begin());
    }

    auto* newResult = new SyntheticData;
    newResult->key = parsedData->key;
    newResult->value = parsedData->value;
    syntheticDataArr.push_back(newResult);

    if (syntheticDataArr.size() < syntheticDataWindowSize) {
        return;
    }

    syntheticDataCurrentSlidingStep++;

//     if sliding enough --> find the maximum
    if (syntheticDataCurrentSlidingStep >= syntheticDataSlidingStep) {
        syntheticDataCurrentSlidingStep = 0;

        std::map<int64_t, int64_t> maxResults;

//         iterate all events in window to find the maximum
        for (auto& storedData : syntheticDataArr) {
//            auto key = std::make_pair(storedData->auctionId, storedData->category);
            if (maxResults.find(storedData->key) == maxResults.end() || storedData->value > maxResults[storedData->key]) {
                maxResults[storedData->key] = storedData->value;
            }
        }

        for (auto& maxResult : maxResults) {
            SyntheticData result{};
            result.key = maxResult.first;
            result.value = maxResult.second;

            // output result
            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}