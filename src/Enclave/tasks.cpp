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

void MapCsvRowToFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto row = static_cast<char *>(data);
    std::string rowStr = row;
    FlightData flightData{};

    try {
        std::vector<std::string> words;
        std::size_t previousPos = 0;
        std::size_t pos = rowStr.find(',');
        while (pos != std::string::npos)
        {
            std::string word = rowStr.substr(previousPos, pos - previousPos);
            words.push_back(word);

            previousPos = pos + 1;
            pos = rowStr.find(',', previousPos);
        }

        const std::string word = rowStr.substr(pos + 1);
        words.push_back(word);

        flightData.arrDelay = static_cast<int>(words.size());
        if (!words[8].empty()) {
            strncpy(flightData.uniqueCarrier, words[8].c_str(), 10);
        } else {
            strncpy(flightData.uniqueCarrier, "UNKNOW", 10);
        }
        if (!words[14].empty() && words[14] != "NA") {
            flightData.arrDelay = std::stoi(words[14]);
        } else {
            flightData.arrDelay = 0;
        }
    }
    catch(const std::invalid_argument&)
    {
        return;
    }

    FastCall_request_encrypt2(globalFastOCall, &flightData, encryptedData);
}

void FilterFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    if (flightData->arrDelay > 0) {
//        FastCall_request(globalFastOCall, flightData);
        FastCall_request_encrypt2(globalFastOCall, flightData, encryptedData);
    }
}

uint16_t reduceWindow = 100;
uint16_t reduceStep = 10;
uint16_t reduceCurrentStep = 10;
std::vector<FlightData*> receivedFlightData;
void ReduceFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    // if window reached max size --> delete old data
    if (receivedFlightData.size() >= reduceWindow) {
        delete receivedFlightData[0];
        receivedFlightData.erase(receivedFlightData.begin());
    }

    // add new data
    const auto newFlightData = new FlightData;
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

constexpr int joinWindow = 100;
FlightData buffer1[joinWindow];
FlightData buffer2[joinWindow];
JoinedFlightData joinedData[joinWindow * joinWindow + 5];
int n1 = 0, n2 = 0, nJoin = 0;
void JoinFlight(void *data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);
    // FastCall_request(globalFastOCall, flightData);
    if (n1 < joinWindow) {
        strncpy(buffer1[n1].uniqueCarrier, flightData->uniqueCarrier, 10);
        buffer1[n1].arrDelay = flightData->arrDelay;
        n1++;
    } else if (n2 < joinWindow) {
        strncpy(buffer2[n2].uniqueCarrier, flightData->uniqueCarrier, 10);
        buffer2[n2].arrDelay = flightData->arrDelay;
        n2++;
    }

    if (n1 == joinWindow && n2 == joinWindow) {
        for (auto & flight1 : buffer1) {
            for (auto & flight2 : buffer2) {
                // if (true) {
//                if (strcmp(flight1.uniqueCarrier, flight2.uniqueCarrier) == 0) {
                 if (flight1.arrDelay == flight2.arrDelay && flight1.arrDelay != 0) {
                    strncpy(joinedData[nJoin].uniqueCarrier1, flight1.uniqueCarrier, 10);
                    strncpy(joinedData[nJoin].uniqueCarrier2, flight2.uniqueCarrier, 10);
                    joinedData[nJoin].arrDelay = flight1.arrDelay;
                    nJoin++;
                }
            }
        }

        if (nJoin > 0) {
            for (int i = 0; i < nJoin; ++i) {
                FastCall_request(globalFastOCall, &joinedData[i]);
            }
            nJoin = 0;
        }

        for (int i = 0; i < joinWindow; ++i) {
            strncpy(buffer2[i].uniqueCarrier, buffer1[i].uniqueCarrier, 10);
            buffer2[i].arrDelay = buffer1[i].arrDelay;
        }
        n1 = 0;
        n2 = joinWindow;
    }
}

void JoinFlight2Stream(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto flightData1 = static_cast<FlightData*>(dataGroup->data1);
    const auto flightData2 = static_cast<FlightData*>(dataGroup->data2);

    if (flightData1 != NULL && n1 < joinWindow) {
        strncpy(buffer1[n1].uniqueCarrier, flightData1->uniqueCarrier, 10);
        buffer1[n1].arrDelay = flightData1->arrDelay;
        n1++;
    } else if (flightData2 != NULL && n2 < joinWindow) {
        strncpy(buffer2[n2].uniqueCarrier, flightData2->uniqueCarrier, 10);
        buffer2[n2].arrDelay = flightData2->arrDelay;
        n2++;
    }

    if (n1 == joinWindow || n2 == joinWindow) {
        for (int i = 0; i < n1; ++i) {
            for (int j = 0; j < n2; ++j) {
//                if (strcmp(buffer1[i].uniqueCarrier, buffer2[j].uniqueCarrier) == 0) {
                if (buffer1[i].arrDelay == buffer2[j].arrDelay && buffer1[i].arrDelay != 0) {
                    strncpy(joinedData[nJoin].uniqueCarrier1, buffer1[i].uniqueCarrier, 10);
                    strncpy(joinedData[nJoin].uniqueCarrier2, buffer2[j].uniqueCarrier, 10);
                    joinedData[nJoin].arrDelay = buffer1[i].arrDelay;
                    nJoin++;
                }
            }
        }

        if (nJoin > 0) {
            for (int i = 0; i < nJoin; ++i) {
                FastCall_request(globalFastOCall, &joinedData[i]);
            }
            nJoin = 0;
        }

        if (n1 == joinWindow) {
            n1 = 0;
        }

        if (n2 == joinWindow) {
            n2 = 0;
        }
    }
}

void MapFlightToFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);
    flightData->arrDelay += 5;
    size_t uniqueCarrierLength = strlen(flightData->uniqueCarrier);
    if (uniqueCarrierLength < 9) {
        flightData->uniqueCarrier[uniqueCarrierLength] = 'H';
        flightData->uniqueCarrier[uniqueCarrierLength + 1] = '\0';
    }
    FastCall_request(globalFastOCall, flightData);
}

void FilterCarrierAndDelay(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);
    if (flightData->uniqueCarrier[0] != 'U' && flightData->arrDelay * 10 > 50) {
        FastCall_request(globalFastOCall, flightData);
    }
}

uint16_t reduceDelayWindow = 200;
uint16_t reduceDelayCount = 0;
ReducedFlightData reducedDelayFlightData;
void ReduceDelay(void* data) {
//    if (data == NULL) {
//        for (auto &reduceFlightData: reducedDatas) {
//            strncpy(reducedDelayFlightData.uniqueCarrier, "abc\0", 4);
//            FastCall_request(globalFastOCall, &reducedDelayFlightData);
//        }
//        return;
//    }
//
//    const auto flightData = static_cast<FlightData*> (data);
//    reducedDelayFlightData.count += 1;
//    reducedDelayFlightData.total += flightData->arrDelay;
//    reduceDelayCount++;
//
//    if (reduceDelayCount >= reduceDelayWindow) {
//        strncpy(reducedDelayFlightData.uniqueCarrier, "abc\0", 4);
//        FastCall_request(globalFastOCall, &reducedDelayFlightData);
//
//        reduceDelayCount = 0;
//        reducedDelayFlightData.count = 0;
//        reducedDelayFlightData.total = 0;
//    }
}

void NexmarkQ1(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);
    // convert dollar to euro
    long double exchangeRate = 0.92;
    bid->price = (uint64_t)(bid->price * exchangeRate);
    FastCall_request_encrypt2(globalFastOCall, bid, encryptedData);
}

void NexmarkQ2_Filter(void* data) {
    // 17600, 27500, 40700, 51500
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);
//    if (bid->auction == 17600 || bid->auction == 27500 || bid->auction == 40700 || bid->auction == 51500) {
//    if (bid->auction == 17600 || bid->auction == 27500 || bid->auction == 40700) {
//    if (bid->auction == 17600 || bid->auction == 27500) {
    if (bid->auction > 1760) {
            FastCall_request_encrypt2(globalFastOCall, bid, encryptedData);
    }
}

void NexmarkQ2_Map(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);
    Q2Result mapBidResult{};
    mapBidResult.auction = bid->auction;
    mapBidResult.price = bid->price;
    FastCall_request_encrypt2(globalFastOCall, &mapBidResult, encryptedData);
}

void NexmarkQ3_FilterPerson(void* data) {
    if (data == NULL) {
        return;
    }

    const auto person = static_cast<Person*>(data);
    // P.state = `OR' OR P.state = `ID' OR P.state = `CA'
//    if (strcmp(person->state, "or") == 0 || strcmp(person->state, "id") == 0 || strcmp(person->state, "ca") == 0) {
//    if (strcmp(person->state, "or") == 0) {
    if (strcmp(person->state, "or") == 0 || strcmp(person->state, "id") == 0 || strcmp(person->state, "ca") == 0 || strcmp(person->state, "wa") == 0) {
            FastCall_request_encrypt2(globalFastOCall, data, encryptedData);
//    FastCall_request(globalFastOCall, data);
    }
}

void NexmarkQ3_FilterAuction(void* data) {
    if (data == NULL) {
        return;
    }

    const auto auction = static_cast<Auction*>(data);
    if (auction->category == 10) {
        FastCall_request_encrypt2(globalFastOCall, auction, encryptedData);
    }
}

std::vector<Person*> people;
std::vector<Auction*> auctions;
const uint64_t q3JoinWindowSize = 200;
const uint64_t q3JoinSlidingStep = 10;
uint64_t q3PeopleCurrentSliding = q3JoinSlidingStep;
uint64_t q3AuctionCurrentSliding = q3JoinSlidingStep;
void NexmarkQ3_JoinPersonAuction(void* data) {
    if (data == NULL) {
        return;
    }

    // get data and cast them to appropriate data type
    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto person = dataGroup->data1 != NULL ? static_cast<Person*>(dataGroup->data1) : NULL;
    const auto auction = dataGroup->data2 != NULL ? static_cast<Auction*>(dataGroup->data2) : NULL;

    // manage person's window
    if (person != NULL) {
        // if window size reached --> delete old data
        if (people.size() >= q3JoinWindowSize) {
            delete people[0];
            people.erase(people.begin());
        }

        // add new data
        auto* newPerson = new Person;
        newPerson->id = person->id;
        strncpy(newPerson->name, person->name, PERSON_NAME_SIZE);
        strncpy(newPerson->emailAddress, person->emailAddress, PERSON_EMAIL_SIZE);
        strncpy(newPerson->creditCard, person->creditCard, PERSON_CREDIT_CARD_SIZE);
        strncpy(newPerson->city, person->city, PERSON_CITY_SIZE);
        strncpy(newPerson->state, person->state, PERSON_STATE_SIZE);
        newPerson->datetime = person->datetime;
        people.push_back(newPerson);

        // increase current sliding step
        q3PeopleCurrentSliding++;
    }

    // manage auction's window
    if (auction != NULL) {
        // if window size reached --> delete old data
        if (auctions.size() >= q3JoinWindowSize) {
            delete auctions[0];
            auctions.erase(auctions.begin());
        }

        // add new data
        auto* newAuction = new Auction;
        newAuction->id = auction->id;
        strncpy(newAuction->itemName, auction->itemName, AUCTION_ITEM_NAME_SIZE);
        newAuction->initialBid = auction->initialBid;
        newAuction->reserve = auction->reserve;
        newAuction->datetime = auction->datetime;
        newAuction->expires = auction->expires;
        newAuction->seller = auction->seller;
        newAuction->category = auction->category;
        auctions.push_back(newAuction);

        // increase current sliding step
        q3AuctionCurrentSliding++;
    }

    bool shouldPeopleBeJoined = people.size() >= q3JoinWindowSize && q3PeopleCurrentSliding >= q3JoinSlidingStep;
    bool shouldAuctionBeJoined = auctions.size() >= q3JoinWindowSize && q3AuctionCurrentSliding >= q3JoinSlidingStep;
    bool shouldJoin = shouldPeopleBeJoined || shouldAuctionBeJoined;

    // if sliding enough --> join
    if (shouldJoin) {
        if (shouldPeopleBeJoined) {
            q3PeopleCurrentSliding = 0;
        }

        if (shouldAuctionBeJoined) {
            q3AuctionCurrentSliding = 0;
        }

        // nested loop join
        std::vector<std::pair<Person*, Auction*>> joinedResults;
        for (auto& storedPerson : people) {
            for (auto& storedAuction : auctions) {
                if (storedPerson->id == storedAuction->seller) {
                    joinedResults.emplace_back(storedPerson, storedAuction);
                }
            }
        }

        // output results
        for (const auto& result : joinedResults) {
            Q3JoinResult mapJoinResult{ *result.first, *result.second };
            FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
        }
    }
}

void NexmarkQ3_MapResult(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q3JoinResult*>(data);
    Q3Result result{};
    result.auctionId = joinResult->auction.id;
    strncpy(result.name, joinResult->person.name, PERSON_NAME_SIZE);
    strncpy(result.city, joinResult->person.city, PERSON_CITY_SIZE);
    strncpy(result.state, joinResult->person.state, PERSON_STATE_SIZE);
    FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
}

std::vector<Auction*> auctionsQ4;
std::vector<Bid*> bidsQ4;
const uint64_t q4JoinWindowSize = 100;
const uint64_t q4JoinSlidingStep = 10;
uint64_t q4BidCurrentSliding = q4JoinSlidingStep;
uint64_t q4AuctionCurrentSliding = q4JoinSlidingStep;
void NexmarkQ4_JoinAuctionBid(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto auction = dataGroup->data1 != NULL ? static_cast<Auction*>(dataGroup->data1) : NULL;
    const auto bid = dataGroup->data2 != NULL ? static_cast<Bid*>(dataGroup->data2) : NULL;

    if (bid != NULL) {
        if (bidsQ4.size() >= q4JoinWindowSize) {
            delete bidsQ4[0];
            bidsQ4.erase(bidsQ4.begin());
        }

        auto* newBid = new Bid;
        newBid->auction = bid->auction;
        newBid->bidder = bid->bidder;
        newBid->price = bid->price;
        newBid->datetime = bid->datetime;
        bidsQ4.push_back(newBid);

        q4BidCurrentSliding++;
    }

    if (auction != NULL) {
        if (auctionsQ4.size() >= q4JoinWindowSize) {
            delete auctionsQ4[0];
            auctionsQ4.erase(auctionsQ4.begin());
        }

        auto* newAuction = new Auction;
        newAuction->id = auction->id;
        strncpy(newAuction->itemName, auction->itemName, AUCTION_ITEM_NAME_SIZE);
        newAuction->initialBid = auction->initialBid;
        newAuction->reserve = auction->reserve;
        newAuction->datetime = auction->datetime;
        newAuction->expires = auction->expires;
        newAuction->seller = auction->seller;
        newAuction->category = auction->category;
        auctionsQ4.push_back(newAuction);

        q4AuctionCurrentSliding++;
    }

    bool shouldBidBeJoined = bidsQ4.size() >= q4JoinWindowSize && q4BidCurrentSliding >= q4JoinSlidingStep;
    bool shouldAuctionBeJoined = auctionsQ4.size() >= q4JoinWindowSize && q4AuctionCurrentSliding >= q4JoinSlidingStep;
    bool shouldJoin = shouldBidBeJoined || shouldAuctionBeJoined;

    if (shouldJoin) {
        if (shouldBidBeJoined) {
            q4BidCurrentSliding = 0;
        }

        if (shouldAuctionBeJoined) {
            q4AuctionCurrentSliding = 0;
        }

        std::vector<std::pair<Auction*, Bid*>> joinResultsQ4;
        for (auto& storedAuction : auctionsQ4) {
            for (auto& storedBid: bidsQ4) {
                if (storedAuction->id == storedBid->auction) {
                    joinResultsQ4.emplace_back(storedAuction, storedBid);
                }
            }
        }

        for (const auto& result : joinResultsQ4) {
            Q4Join1Result mapJoinResult{ *result.first, *result.second };
            FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
        }
    }
}

void NexmarkQ4_MapAuctionBid(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q4Join1Result*>(data);
    Q4Map1Result mapResult{};
    mapResult.final = joinResult->bid.price;
    mapResult.category = joinResult->auction.category;

    FastCall_request_encrypt2(globalFastOCall, &mapResult, encryptedData);
}


std::vector<Q4Map1Result*> joinResultsForMaxQ4;
uint64_t q4MaxCurrenSlidingStep = q4JoinSlidingStep;
bool isSetup = false;
void setup() {
    joinResultsForMaxQ4.reserve(q4JoinWindowSize + 2);
    isSetup = true;
}

void NexmarkQ4_MaxAuctionPriceByCategory(void* data) {
//    if (!isSetup) {
//        setup();
//    }

//    if (data == NULL) {
//        return;
//    }

//    const auto parsedData = static_cast<Q4Map1Result*>(data);

    // if window size reached --> delete old data
//    if (joinResultsForMaxQ4.size() >= q4JoinWindowSize) {
//        delete joinResultsForMaxQ4[0];
//        joinResultsForMaxQ4.erase(joinResultsForMaxQ4.begin());
//    }

//    auto* newResult = new Q4Map1Result;
//    newResult->category = parsedData->category;
//    newResult->final = parsedData->final;
//    joinResultsForMaxQ4.push_back(newResult);

//    if (joinResultsForMaxQ4.size() < q4JoinWindowSize) {
//        return;
//    }
//
//    q4MaxCurrenSlidingStep++;

    // if sliding enough --> find the maximum
//    if (q4MaxCurrenSlidingStep >= q4JoinSlidingStep) {
//        q4MaxCurrenSlidingStep = 0;

//        std::unordered_map<uint64_t, uint64_t> maxResultsQ4;
//        maxResultsQ4.reserve(15);
        // iterate all events in window to find the maximum
//        for (auto& storedResult : joinResultsForMaxQ4) {
//            if (maxResultsQ4.find(storedResult->category) == maxResultsQ4.end() || storedResult->final > maxResultsQ4[storedResult->category]) {
//                maxResultsQ4[storedResult->category] = storedResult->final;
//            }
//        }

//        for (auto& maxResult : maxResultsQ4) {
//            Q4Map1Result result{};
//            result.category = maxResult.first;
//            result.final = maxResult.second;
//
//            // output result
//            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
//        }
//    }
}

uint64_t categories[10] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
const uint64_t q4JoinCategoryWindowSize = 500;
const uint64_t q4JoinCategorySlidingStep = 10;
uint64_t q4JoinCategoryCurrentSlidingStep = q4JoinCategorySlidingStep;
std::vector<Q4Map1Result*> maxResultForJoinCategoryQ4;
void NexmarkQ4_JoinWithCategory(void* data) {
    if (data == NULL) {
        return;
    }

    const auto maxResult = static_cast<Q4Map1Result*>(data);

    if (maxResultForJoinCategoryQ4.size() >= q4JoinCategoryWindowSize) {
        delete maxResultForJoinCategoryQ4[0];
        maxResultForJoinCategoryQ4.erase(maxResultForJoinCategoryQ4.begin());
    }

    auto newResult = new Q4Map1Result;
    newResult->category = maxResult->category;
    newResult->final = maxResult->final;
    maxResultForJoinCategoryQ4.push_back(newResult);

    if (maxResultForJoinCategoryQ4.size() < q4JoinCategoryWindowSize) {
        return;
    }

    q4JoinCategoryCurrentSlidingStep++;

    if (q4JoinCategoryCurrentSlidingStep >= q4JoinCategorySlidingStep) {
        q4JoinCategoryCurrentSlidingStep = 0;

        std::vector<std::pair<uint64_t, Q4Map1Result*>> joinCategoryResultsQ4;
        for (const auto& storedResult : maxResultForJoinCategoryQ4) {
            for (const auto& category : categories) {
                if (storedResult->category == category) {
                    joinCategoryResultsQ4.emplace_back(category, storedResult);
                }
            }
        }

        for (const auto& result : joinCategoryResultsQ4) {
            Q4Map1Result mapJoinResult{ result.second->final, result.second->category };
            FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
        }
    }
}

std::vector<Q4Map1Result*> joinCategoryResultForAverageQ4;
uint64_t q4AverageCurrentSlidingStep = q4JoinSlidingStep;
void NexmarkQ4_Average(void* data) {
    if (data == NULL) {
        return;
    }

    const auto parsedData = static_cast<Q4Map1Result*>(data);

    if (joinCategoryResultForAverageQ4.size() >= q4JoinWindowSize) {
        delete joinCategoryResultForAverageQ4[0];
        joinCategoryResultForAverageQ4.erase(joinCategoryResultForAverageQ4.begin());
    }

    auto newResult = new Q4Map1Result;
    newResult->category = parsedData->category;
    newResult->final = parsedData->final;
    joinCategoryResultForAverageQ4.push_back(newResult);

    if (joinCategoryResultForAverageQ4.size() < q4JoinWindowSize) {
        return;
    }

    q4AverageCurrentSlidingStep++;

    if (q4AverageCurrentSlidingStep >= q4JoinSlidingStep) {
        q4AverageCurrentSlidingStep = 0;

        std::map<uint64_t, std::pair<uint64_t, uint64_t>> averageResults;
        for (const auto& storedResult : joinCategoryResultForAverageQ4) {
            const auto category = storedResult->category;
            const auto final = storedResult->final;
            if (averageResults.find(category) == averageResults.end()) {
                averageResults[category] = std::make_pair(final, 0);
            } else {
                averageResults[category].first += final;
                averageResults[category].second += 1;
            }
        }

        for (auto& averageResult : averageResults) {
            Q4AverageResult result{};
            result.category = averageResult.first;
            result.average = static_cast<double>(averageResult.second.first) * 1.0 / averageResult.second.second;

            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}

std::vector<Bid*> bidsCountByAuctionQ5;
const uint64_t q5WindowSize = 200;
const uint64_t q5SlidingStep = 10;
uint64_t q5BidCurrentSliding = q5SlidingStep;
void NexmarkQ5_CountByAuction(void* data) {
//    if (data == NULL) {
//        return;
//    }

//    const auto bid = static_cast<Bid*>(data);

    // if window size reached --> delete old data
//    if (bidsCountByAuctionQ5.size() >= q5WindowSize) {
//        delete bidsCountByAuctionQ5[0];
//        bidsCountByAuctionQ5.erase(bidsCountByAuctionQ5.begin());
//    }
//
//    // add new data
//    auto* newBid = new Bid;
//    newBid->auction = bid->auction;
//    newBid->bidder = bid->bidder;
//    newBid->price = bid->price;
//    newBid->datetime = bid->datetime;
//    bidsCountByAuctionQ5.push_back(newBid);
//
//    if (bidsCountByAuctionQ5.size() < q5WindowSize) {
//        return;
//    }
//
//    q5BidCurrentSliding++;
//
//    // if sliding enough --> count using a hash table<auction_id, count>
//    if (q5BidCurrentSliding >= q5SlidingStep) {
//        q5BidCurrentSliding = 0;
//
//        std::map<uint64_t, uint64_t> countByAuctionResultsQ5;
////        countByAuctionResultsQ5.reserve(60000);
//        for (auto& storedAuction : bidsCountByAuctionQ5) {
//            // if auction not appeared before ==> count = 1
//            if (countByAuctionResultsQ5.find(storedAuction->auction) == countByAuctionResultsQ5.end()) {
//                countByAuctionResultsQ5[storedAuction->auction] = 1;
//            } else { // else, increase count of that auction to 1
//                countByAuctionResultsQ5[storedAuction->auction] += 1;
//            }
//        }
//
//        // output results
//        for (auto& countAuctionResult : countByAuctionResultsQ5) {
//            Q5CountByAuctionResult result{};
//            result.auction = countAuctionResult.first;
//            result.count = countAuctionResult.second;
//            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
//        }
//    }
}

std::vector<Q5CountByAuctionResult*> countByAuctionMaxBatchQ5;
uint64_t q5MaxBatchCurrenSlidingStep = q5SlidingStep;
void NexmarkQ5_MaxBatch(void* data) {
    if (data == NULL) {
        return;
    }

    const auto countByAuctionResult = static_cast<Q5CountByAuctionResult*>(data);

    // if window size reached --> delete old data
    if (countByAuctionMaxBatchQ5.size() >= q5WindowSize) {
        delete countByAuctionMaxBatchQ5[0];
        countByAuctionMaxBatchQ5.erase(countByAuctionMaxBatchQ5.begin());
    }

    // add new data
    auto result = new Q5CountByAuctionResult;
    result->datetime = countByAuctionResult->datetime;
    result->auction = countByAuctionResult->auction;
    countByAuctionMaxBatchQ5.push_back(result);

    if (countByAuctionMaxBatchQ5.size() < q5WindowSize) {
        return;
    }

    q5MaxBatchCurrenSlidingStep++;

    // if sliding enough --> find the maximum
    if (q5MaxBatchCurrenSlidingStep >= q5SlidingStep) {
        q5MaxBatchCurrenSlidingStep = 0;

        Q5CountByAuctionResult maxResult{};
        maxResult.count = countByAuctionMaxBatchQ5[0]->count;
        // iterate all events in window to find the maximum
        for (auto& storedResult : countByAuctionMaxBatchQ5) {
            if (storedResult->count > maxResult.count) {
                maxResult.count = storedResult->count;
                maxResult.datetime = storedResult->datetime;
                maxResult.auction = storedResult->auction;
            }
        }

        // output result
        FastCall_request_encrypt2(globalFastOCall, &maxResult, encryptedData);
    }
}

std::vector<Auction*> auctionsQ6;
std::vector<Bid*> bidsQ6;
const uint64_t q6JoinWindowSize = 200;
const uint64_t q6JoinSlidingStep = 10;
uint64_t q6BidCurrentSliding = q6JoinSlidingStep;
uint64_t q6AuctionCurrentSliding = q6JoinSlidingStep;
void NexmarkQ6_Join(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto auction = dataGroup->data1 != NULL ? static_cast<Auction*>(dataGroup->data1) : NULL;
    const auto bid = dataGroup->data2 != NULL ? static_cast<Bid*>(dataGroup->data2) : NULL;

    if (bid != NULL) {
        if (bidsQ6.size() >= q6JoinWindowSize) {
            delete bidsQ6[0];
            bidsQ6.erase(bidsQ6.begin());
        }

        auto* newBid = new Bid;
        newBid->auction = bid->auction;
        newBid->bidder = bid->bidder;
        newBid->price = bid->price;
        newBid->datetime = bid->datetime;
        bidsQ6.push_back(newBid);

        q6BidCurrentSliding++;
    }

    if (auction != NULL) {
        if (auctionsQ6.size() >= q6JoinWindowSize) {
            delete auctionsQ6[0];
            auctionsQ6.erase(auctionsQ6.begin());
        }

        auto* newAuction = new Auction;
        newAuction->id = auction->id;
        strncpy(newAuction->itemName, auction->itemName, AUCTION_ITEM_NAME_SIZE);
        newAuction->initialBid = auction->initialBid;
        newAuction->reserve = auction->reserve;
        newAuction->datetime = auction->datetime;
        newAuction->expires = auction->expires;
        newAuction->seller = auction->seller;
        newAuction->category = auction->category;
        auctionsQ6.push_back(newAuction);

        q6AuctionCurrentSliding++;
    }

    bool shouldBidBeJoined = bidsQ6.size() >= q6JoinWindowSize && q6BidCurrentSliding >= q6JoinSlidingStep;
    bool shouldAuctionBeJoined = auctionsQ6.size() >= q6JoinWindowSize && q6AuctionCurrentSliding >= q6JoinSlidingStep;
    bool shouldJoin = shouldBidBeJoined || shouldAuctionBeJoined;

    if (shouldJoin) {
        if (shouldBidBeJoined) {
            q6BidCurrentSliding = 0;
        }

        if (shouldAuctionBeJoined) {
            q6AuctionCurrentSliding = 0;
        }

        std::vector<std::pair<Auction*, Bid*>> joinResultsQ6;
        for (auto& storedAuction : auctionsQ6) {
            for (auto& storedBid: bidsQ6) {
                if (storedAuction->id == storedBid->auction) {
                    joinResultsQ6.emplace_back(storedAuction, storedBid);
                }
            }
        }

        for (const auto& result : joinResultsQ6) {
            Q4Join1Result mapJoinResult{ *result.first, *result.second };
            FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
        }
    }
}

void NexmarkQ6_Filter(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q6JoinResult *>(data);
    if (joinResult->bid.datetime < joinResult->auction.expires) {
        FastCall_request_encrypt2(globalFastOCall, joinResult, encryptedData);
    }
}

std::vector<Q6JoinResult*> joinResultsForMaxQ6;
uint64_t q6MaxCurrenSlidingStep = q6JoinSlidingStep;
void NexmarkQ6_Max(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q6JoinResult *>(data);

    // if window size reached --> delete old data
    if (joinResultsForMaxQ6.size() >= q6JoinWindowSize) {
        delete joinResultsForMaxQ6[0];
        joinResultsForMaxQ6.erase(joinResultsForMaxQ6.begin());
    }

    auto* newJoinResult = new Q6JoinResult;
    newJoinResult->auction.id = joinResult->auction.id;
    strncpy(newJoinResult->auction.itemName, joinResult->auction.itemName, AUCTION_ITEM_NAME_SIZE);
    newJoinResult->auction.initialBid = joinResult->auction.initialBid;
    newJoinResult->auction.reserve = joinResult->auction.reserve;
    newJoinResult->auction.datetime = joinResult->auction.datetime;
    newJoinResult->auction.expires = joinResult->auction.expires;
    newJoinResult->auction.seller = joinResult->auction.seller;
    newJoinResult->auction.category = joinResult->auction.category;
    newJoinResult->bid.datetime = joinResult->bid.datetime;
    newJoinResult->bid.price = joinResult->bid.price;
    newJoinResult->bid.auction = joinResult->bid.auction;
    newJoinResult->bid.bidder = joinResult->bid.bidder;
    joinResultsForMaxQ6.push_back(newJoinResult);

    if (joinResultsForMaxQ6.size() < q6JoinWindowSize) {
        return;
    }

    q6MaxCurrenSlidingStep++;

    // if sliding enough --> find the maximum
    if (q6MaxCurrenSlidingStep >= q6JoinSlidingStep) {
        q6MaxCurrenSlidingStep = 0;

        std::map<uint64_t, uint64_t> maxResultsQ6;
        // iterate all events in window to find the maximum
        for (auto& storedResult : joinResultsForMaxQ6) {
            const auto seller = storedResult->auction.seller;
            const auto price = storedResult->bid.price;
            if (maxResultsQ6.find(seller) == maxResultsQ6.end() || price > maxResultsQ6[seller]) {
                maxResultsQ6[seller] = price;
            }
        }

        for (auto& maxResult : maxResultsQ6) {
            Q6MaxResult result{};
            result.seller = maxResult.first;
            result.final = maxResult.second;

            // output result
            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}

const uint64_t averagePartitionSizeQ6 = 200;
std::map<uint64_t, std::vector<Q6MaxResult*>> averagePartitionsQ6;
void NexmarkQ6_Avg(void* data) {
    if (data == NULL) {
        return;
    }

    const auto maxResult = static_cast<Q6MaxResult*>(data);
    const auto seller = maxResult->seller;
    if (averagePartitionsQ6.find(seller) == averagePartitionsQ6.end()) {
        averagePartitionsQ6[seller].reserve(averagePartitionSizeQ6);
    }

    auto& currentPartition = averagePartitionsQ6[seller];
    if (currentPartition.size() >= averagePartitionSizeQ6) {
        delete currentPartition[0];
        currentPartition.erase(currentPartition.begin());
    }

    auto* newMaxResult = new Q6MaxResult;
    newMaxResult->seller = maxResult->seller;
    newMaxResult->final = maxResult->final;
    currentPartition.push_back(newMaxResult);

    // processing
    std::vector<Q6AverageResult> averageResults;
    for (const auto& partition : averagePartitionsQ6) {
        if (partition.second.size() < averagePartitionSizeQ6) {
            continue;
        }

        uint64_t sum_final = 0;
        for (const auto& e : partition.second) {
            sum_final += e->final;
        }

        Q6AverageResult newAverageResult{};
        newAverageResult.seller = partition.first;
        newAverageResult.final = static_cast<double>(sum_final) * 1.0 / partition.second.size();
        averageResults.push_back(newAverageResult);
    }

    if (!averageResults.empty()) {
        for (auto& averageResult : averageResults) {
            FastCall_request(globalFastOCall, &averageResult);
//            FastCall_request_encrypt2(globalFastOCall, &averageResult, encryptedData);
        }
    }
}

std::vector<Bid*> bidsQ7MaxJoin;
uint64_t bidTimeMaxJoinQ7 = 0;
uint64_t timeRangeMaxJoinQ7 = 1500;
void NexmarkQ7_MaxJoin(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);

    bool hasRemoved = false;
    bool isNewBidAdded = false;

    if (bid->datetime > bidTimeMaxJoinQ7) {
        bidTimeMaxJoinQ7 = bid->datetime;
        for (auto & storedBid : bidsQ7MaxJoin) {
            if (storedBid->datetime >= bidTimeMaxJoinQ7 - timeRangeMaxJoinQ7) {
                continue;
            }

            delete storedBid;
            storedBid = NULL;
        }

        for (size_t i = 0; i < bidsQ7MaxJoin.size(); ++i) {
            if (bidsQ7MaxJoin[i] == NULL) {
                hasRemoved = true;
                bidsQ7MaxJoin[i] = bidsQ7MaxJoin.back();
                bidsQ7MaxJoin.pop_back();
                --i;
            }
        }
    }

    if (bid->datetime >= bidTimeMaxJoinQ7 - timeRangeMaxJoinQ7) {
        isNewBidAdded = true;
        auto* newBid = new Bid;
        newBid->auction = bid->auction;
        newBid->bidder = bid->bidder;
        newBid->price = bid->price;
        newBid->datetime = bid->datetime;
        bidsQ7MaxJoin.push_back(newBid);
    }

    if (!(hasRemoved || isNewBidAdded)) {
        return;
    }

    uint64_t currentMaxPrice = 0;
    for (const auto& storeBid : bidsQ7MaxJoin) {
        if (storeBid->price > currentMaxPrice) {
            currentMaxPrice = storeBid->price;
        }
    }

    std::vector<Bid*> joinResultQ7;
    for (const auto& storedBid : bidsQ7MaxJoin) {
        if (storedBid->price == currentMaxPrice) {
            joinResultQ7.push_back(storedBid);
        }
    }

    if (!joinResultQ7.empty()) {
        for (const auto& result : joinResultQ7) {
            FastCall_request_encrypt2(globalFastOCall, result, encryptedData);
        }
    }
}

std::vector<Person*> peopleQA;
std::vector<Bid*> bidsQA;
std::vector<std::pair<Person*, Bid*>> joinResultsQA;
uint64_t timeRangeQA = 600;
uint64_t bidTimeMaxQA = 0;
uint64_t personTimeMaxQA = 0;
void NexmarkQA_JoinPersonBid(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto person = static_cast<Person*>(dataGroup->data1);
    const auto bid = static_cast<Bid*>(dataGroup->data2);
    std::map<Person*, bool> removedPersonMap;
    std::map<Bid*, bool> removedBidMap;

    bool isNewBidAdded = false;
    bool isNewPersonAdded = false;

    if (bid != NULL) {
        if (bid->datetime > bidTimeMaxQA) {
            bidTimeMaxQA = bid->datetime;
            for (auto & storedBid : bidsQA) {
                if (storedBid->datetime >= bidTimeMaxQA - timeRangeQA) {
                    continue;
                }

                removedBidMap[storedBid] = true;
                delete storedBid;
                storedBid = NULL;
            }

            for (size_t i = 0; i < bidsQA.size(); ++i) {
                if (bidsQA[i] == NULL) {
                    bidsQA[i] = bidsQA.back();
                    bidsQA.pop_back();
                    --i;
                }
            }
        }

        if (bid->datetime >= bidTimeMaxQA - timeRangeQA) {
            auto* newBid = new Bid;
            newBid->auction = bid->auction;
            newBid->bidder = bid->bidder;
            newBid->price = bid->price;
            newBid->datetime = bid->datetime;
            bidsQA.push_back(newBid);

            isNewBidAdded = true;
        }
    }

    if (person != NULL) {
        if (person->datetime > personTimeMaxQA) {
            personTimeMaxQA = person->datetime;
            for (auto & storedPerson : peopleQA) {
                if (storedPerson->datetime >= personTimeMaxQA - timeRangeQA) {
                    continue;
                }

                removedPersonMap[storedPerson] = true;
                delete storedPerson;
                storedPerson = NULL;
            }

            for (size_t i = 0; i < peopleQA.size(); ++i) {
                if (peopleQA[i] == NULL) {
                    peopleQA[i] = peopleQA.back();
                    peopleQA.pop_back();
                    --i;
                }
            }
        }

        if (person->datetime >= personTimeMaxQA - timeRangeQA) {
            auto* newPerson = new Person;
            newPerson->id = person->id;
            strncpy(newPerson->name, person->name, PERSON_NAME_SIZE);
            strncpy(newPerson->emailAddress, person->emailAddress, PERSON_EMAIL_SIZE);
            strncpy(newPerson->creditCard, person->creditCard, PERSON_CREDIT_CARD_SIZE);
            strncpy(newPerson->city, person->city, PERSON_CITY_SIZE);
            strncpy(newPerson->state, person->state, PERSON_STATE_SIZE);
            newPerson->datetime = person->datetime;
            peopleQA.push_back(newPerson);

            isNewPersonAdded = true;
        }
    }

    for (size_t i = 0; i < joinResultsQA.size(); ++i) {
        if (removedPersonMap[joinResultsQA[i].first] || removedBidMap[joinResultsQA[i].second]) {
            joinResultsQA[i] = joinResultsQA.back();
            joinResultsQA.pop_back();
            --i;
        }
    }

    if (isNewBidAdded) {
        for (const auto& storedPeople: peopleQA) {
            if (storedPeople->id == bid->bidder) {
                joinResultsQA.emplace_back(storedPeople, bidsQA.back());
            }
        }
    }

    if (isNewPersonAdded) {
        size_t bidSize = isNewBidAdded ? bidsQA.size() - 1 : bidsQA.size();
        for (size_t i = 0; i < bidSize; ++i) {
            if (bidsQA[i]->bidder == person->id) {
                joinResultsQA.emplace_back(peopleQA.back(), bidsQA[i]);
            }
        }
    }

    for (const auto& result : joinResultsQA) {
        QAJoinPersonBidResult mapJoinResult{ *result.first, *result.second };
        FastCall_request(globalFastOCall, &mapJoinResult);
    }
}


std::vector<Person*> peopleQ8;
std::vector<Auction*> auctionsQ8;
std::vector<std::pair<Person*, Auction*>> joinResultsQ8;
uint64_t timeRangeQ8 = 600;
uint64_t auctionTimeMaxQ8 = 0;
uint64_t personTimeMaxQ8 = 0;
void NexmarkQ8_JoinPersonAuction(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto person = dataGroup->data1 != NULL ? static_cast<Person*>(dataGroup->data1) : NULL;
    const auto auction = dataGroup->data2 != NULL ? static_cast<Auction*>(dataGroup->data2) : NULL;
    std::map<Person*, bool> removedPersonMap;
    std::map<Auction*, bool> removedAuctionMap;

    bool isNewPersonAdded = false;
    bool isNewAuctionAdded = false;
    if (person != NULL) {
        if (person->datetime > personTimeMaxQ8) {
            personTimeMaxQ8 = person->datetime;
            for (auto & storedPerson : peopleQ8) {
                if (storedPerson->datetime >= personTimeMaxQ8 - timeRangeQ8) {
                    continue;
                }

                removedPersonMap[storedPerson] = true;
                delete storedPerson;
                storedPerson = NULL;
            }

            for (size_t i = 0; i < peopleQ8.size(); ++i) {
                if (peopleQ8[i] == NULL) {
                    peopleQ8[i] = peopleQ8.back();
                    peopleQ8.pop_back();
                    --i;
                }
            }
        }

        if (person->datetime >= personTimeMaxQ8 - timeRangeQ8) {
            auto* newPerson = new Person;
            newPerson->id = person->id;
            strncpy(newPerson->name, person->name, PERSON_NAME_SIZE);
            strncpy(newPerson->emailAddress, person->emailAddress, PERSON_EMAIL_SIZE);
            strncpy(newPerson->creditCard, person->creditCard, PERSON_CREDIT_CARD_SIZE);
            strncpy(newPerson->city, person->city, PERSON_CITY_SIZE);
            strncpy(newPerson->state, person->state, PERSON_STATE_SIZE);
            newPerson->datetime = person->datetime;
            peopleQ8.push_back(newPerson);

            isNewPersonAdded = true;
        }
    }

    if (auction != NULL) {
        if (auction->datetime > auctionTimeMaxQ8) {
            auctionTimeMaxQ8 = auction->datetime;
            for (auto & storedAuction : auctionsQ8) {
                if (storedAuction->datetime >= auctionTimeMaxQ8 - timeRangeQ8) {
                    continue;
                }

                removedAuctionMap[storedAuction] = true;
                delete storedAuction;
                storedAuction = NULL;
            }

            for (size_t i = 0; i < auctionsQ8.size(); ++i) {
                if (auctionsQ8[i] == NULL) {
                    auctionsQ8[i] = auctionsQ8.back();
                    auctionsQ8.pop_back();
                    --i;
                }
            }
        }

        if (auction->datetime >= auctionTimeMaxQ8 - timeRangeQ8) {
            auto* newAuction = new Auction;
            newAuction->id = auction->id;
            strncpy(newAuction->itemName, auction->itemName, AUCTION_ITEM_NAME_SIZE);
            newAuction->initialBid = auction->initialBid;
            newAuction->reserve = auction->reserve;
            newAuction->datetime = auction->datetime;
            newAuction->expires = auction->expires;
            newAuction->seller = auction->seller;
            newAuction->category = auction->category;
            auctionsQ8.push_back(newAuction);

            isNewAuctionAdded = true;
        }
    }

    for (size_t i = 0; i < joinResultsQ8.size(); ++i) {
        if (removedPersonMap[joinResultsQ8[i].first] || removedAuctionMap[joinResultsQ8[i].second]) {
            joinResultsQ8[i] = joinResultsQ8.back();
            joinResultsQ8.pop_back();
            --i;
        }
    }

    if (isNewPersonAdded) {
        for (const auto& storedAuction: auctionsQ8) {
            if (storedAuction->seller == person->id) {
                joinResultsQ8.emplace_back(peopleQ8.back(), storedAuction);
            }
        }
    }

    if (isNewAuctionAdded) {
        size_t peopleSize = isNewPersonAdded ? peopleQ8.size() - 1 : peopleQ8.size();
        for (size_t i = 0; i < peopleSize; ++i) {
            if (peopleQ8[i]->id == auction->seller) {
                joinResultsQ8.emplace_back(peopleQ8[i], auctionsQ8.back());
            }
        }
    }

    for (const auto& result : joinResultsQ8) {
        Q8JoinResult mapJoinResult{*result.first, *result.second };
        FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
    }
}

void NexmarkQ8_Map(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q8JoinResult*>(data);
    Q8MapResult mapResult{};
    mapResult.personId = joinResult->person.id;
    strncpy(mapResult.personName, joinResult->person.name, PERSON_NAME_SIZE);
    mapResult.auctionReserve = joinResult->auction.reserve;
    FastCall_request_encrypt2(globalFastOCall, &mapResult, encryptedData);
}

void testDecryption(void* data) {

}