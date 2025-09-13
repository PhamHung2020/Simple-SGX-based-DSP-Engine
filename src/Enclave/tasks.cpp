//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "data_types.h"
#include "Enclave/enclave_operators.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"
#include "Nexmark/schemas.h"
#include "StreamBox/schemas.h"
#include "sgx_trts_exception.h"

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <unordered_set>

using namespace std;

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;
char encryptedData[1000];

void SecureStream_MapCsvRowToFlight(void* data) {
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

void SecureStream_FilterFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*>(data);
    if (flightData->arrDelay > 0) {
        FastCall_request_encrypt2(globalFastOCall, flightData, encryptedData);
    }
}

uint16_t reduceWindow = 1000;
uint16_t reduceStep = 100;
uint16_t reduceCurrentStep = 0;
std::vector<FlightData*> receivedFlightData;
std::vector<ReducedFlightData*> reducedData;
std::vector<ReducedFlightData> reducedDatas;
void SecureStream_ReduceFlight(void* data) {
    if (data == NULL) {
        for (auto &reduceFlightData: reducedData) {
            if (reduceFlightData->count > 0) {
                FastCall_request_encrypt2(globalFastOCall, reduceFlightData, encryptedData);
            }
        }
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    // add new data
    const auto newFlightData = new FlightData;
    strncpy(newFlightData->uniqueCarrier, flightData->uniqueCarrier, 10);
    newFlightData->arrDelay = flightData->arrDelay;
    receivedFlightData.push_back(newFlightData);

    if (receivedFlightData.size() < reduceWindow) {
        return;
    }

    // delete old data if window size is reached
    else if (receivedFlightData.size() > reduceWindow) {
        const auto deletedFlightData = receivedFlightData[0];
        for (auto &reduceFlightData: reducedData) {
            if (strcmp(deletedFlightData->uniqueCarrier, reduceFlightData->uniqueCarrier) == 0) {
                reduceFlightData->count -= 1;
                reduceFlightData->total -= deletedFlightData->arrDelay;
                break;
            }
        }
        receivedFlightData.erase(receivedFlightData.begin());
        delete deletedFlightData;
    }

    // reducing
    bool found = false;
    for (auto &reduceFlightData: reducedData) {
        if (strcmp(newFlightData->uniqueCarrier, reduceFlightData->uniqueCarrier) == 0) {
            reduceFlightData->count += 1;
            reduceFlightData->total += newFlightData->arrDelay;

            found = true;
            break;
        }
    }

    if (!found) {
        auto* reducedFlightData = new ReducedFlightData;
        strncpy(reducedFlightData->uniqueCarrier, newFlightData->uniqueCarrier, 10);
        reducedFlightData->count = 1;
        reducedFlightData->total = newFlightData->arrDelay;
        reducedData.push_back(reducedFlightData);
    }

    if (reduceCurrentStep < reduceStep) {
        reduceCurrentStep += 1;
        return;
    }

    reduceCurrentStep = 0;

    // output reduced data
    for (auto &reduceFlightData: reducedData) {
        if (reduceFlightData->count > 0) {
            FastCall_request_encrypt2(globalFastOCall, reduceFlightData, encryptedData);
        }
    }
}

void Nexmark_Query1(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);

    // convert dollar to euro
    long double exchangeRate = 0.92;
    bid->price = (uint64_t)(bid->price * exchangeRate);

    FastCall_request_encrypt2(globalFastOCall, bid, encryptedData);
}

void Nexmark_Query2_Filter(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);
    if (bid->auction == 17600 || bid->auction == 27500 || bid->auction == 40700 || bid->auction == 51500) {
        FastCall_request_encrypt2(globalFastOCall, bid, encryptedData);
    }
}

void Nexmark_Query2_Map(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);
    Q2Result mapBidResult{};
    mapBidResult.auction = bid->auction;
    mapBidResult.price = bid->price;
    FastCall_request_encrypt2(globalFastOCall, &mapBidResult, encryptedData);
}

void Nexmark_Query3_FilterPerson(void* data) {
    if (data == NULL) {
        return;
    }

    const auto person = static_cast<Person*>(data);
    if (strcmp(person->state, "or") == 0 || strcmp(person->state, "id") == 0 || strcmp(person->state, "ca") == 0 || strcmp(person->state, "wa") == 0) {
        FastCall_request_encrypt2(globalFastOCall, data, encryptedData);
    }
}

void Nexmark_Query3_FilterAuction(void* data) {
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
const uint64_t q3JoinWindowSize = 100;
const uint64_t q3JoinSlidingStep = 10;
uint64_t q3PeopleCurrentSliding = q3JoinSlidingStep;
uint64_t q3AuctionCurrentSliding = q3JoinSlidingStep;
void Nexmark_Query3_JoinPersonAuction(void* data) {
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

void Nexmark_Query3_MapResult(void* data) {
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
void Nexmark_Query4_JoinAuctionBid(void* data) {
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

void Nexmark_Query4_MapAuctionBid(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q4Join1Result*>(data);
    Q4Map1Result mapResult{};
    mapResult.auctionId = joinResult->auction.id;
    mapResult.final = joinResult->bid.price;
    mapResult.category = joinResult->auction.category;

    FastCall_request_encrypt2(globalFastOCall, &mapResult, encryptedData);
}

std::vector<Q4Map1Result*> joinResultsForMaxQ4;
uint64_t q4MaxCurrenSlidingStep = q4JoinSlidingStep;
void Nexmark_Query4_MaxAuctionPriceByCategory(void* data) {
    if (data == NULL) {
        return;
    }

    const auto parsedData = static_cast<Q4Map1Result*>(data);

//     if window size reached --> delete old data
    if (joinResultsForMaxQ4.size() >= q4JoinWindowSize) {
        delete joinResultsForMaxQ4[0];
        joinResultsForMaxQ4.erase(joinResultsForMaxQ4.begin());
    }

    auto* newResult = new Q4Map1Result;
    newResult->category = parsedData->category;
    newResult->final = parsedData->final;
    joinResultsForMaxQ4.push_back(newResult);

    if (joinResultsForMaxQ4.size() < q4JoinWindowSize) {
        return;
    }

    q4MaxCurrenSlidingStep++;

//     if sliding enough --> find the maximum
    if (q4MaxCurrenSlidingStep >= q4JoinSlidingStep) {
        q4MaxCurrenSlidingStep = 0;

//        std::map<std::pair<uint64_t, uint64_t>, uint64_t> maxResultsQ4;
        std::map<uint64_t, uint64_t> maxResultsQ4;

//         iterate all events in window to find the maximum
        for (auto& storedResult : joinResultsForMaxQ4) {
//            auto key = std::make_pair(storedResult->auctionId, storedResult->category);
            if (maxResultsQ4.find(storedResult->category) == maxResultsQ4.end() || storedResult->final > maxResultsQ4[storedResult->category]) {
                maxResultsQ4[storedResult->category] = storedResult->final;
            }
        }

        for (auto& maxResult : maxResultsQ4) {
            Q4Map1Result result{};
            result.category = maxResult.first;
            result.final = maxResult.second;

            // output result
            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}

uint64_t categories[10] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
const uint64_t q4JoinCategoryWindowSize = 500;
const uint64_t q4JoinCategorySlidingStep = 10;
uint64_t q4JoinCategoryCurrentSlidingStep = q4JoinCategorySlidingStep;
std::vector<Q4Map1Result*> maxResultForJoinCategoryQ4;
void Nexmark_Query4_JoinWithCategory(void* data) {
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
void Nexmark_Query4_Average(void* data) {
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
const uint64_t q5WindowSize = 100;
const uint64_t q5SlidingStep = 10;
uint64_t q5BidCurrentSliding = q5SlidingStep;
void Nexmark_Query5_CountByAuction(void* data) {
    if (data == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(data);

//     if window size reached --> delete old data
    if (bidsCountByAuctionQ5.size() >= q5WindowSize) {
        delete bidsCountByAuctionQ5[0];
        bidsCountByAuctionQ5.erase(bidsCountByAuctionQ5.begin());
    }

    // add new data
    auto* newBid = new Bid;
    newBid->auction = bid->auction;
    newBid->bidder = bid->bidder;
    newBid->price = bid->price;
    newBid->datetime = bid->datetime;
    bidsCountByAuctionQ5.push_back(newBid);

    if (bidsCountByAuctionQ5.size() < q5WindowSize) {
        return;
    }

    q5BidCurrentSliding++;

    // if sliding enough --> count using a hash table<auction_id, count>
    if (q5BidCurrentSliding >= q5SlidingStep) {
        q5BidCurrentSliding = 0;

        std::map<uint64_t, uint64_t> countByAuctionResultsQ5;
        for (auto& storedAuction : bidsCountByAuctionQ5) {
            // if auction not appeared before ==> count = 1
            if (countByAuctionResultsQ5.find(storedAuction->auction) == countByAuctionResultsQ5.end()) {
                countByAuctionResultsQ5[storedAuction->auction] = 1;
            } else { // else, increase count of that auction to 1
                countByAuctionResultsQ5[storedAuction->auction] += 1;
            }
        }

        // output results
        for (auto& countAuctionResult : countByAuctionResultsQ5) {
            Q5CountByAuctionResult result{};
            result.auction = countAuctionResult.first;
            result.count = countAuctionResult.second;
            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}

std::vector<Q5CountByAuctionResult*> countByAuctionMaxBatchQ5;
uint64_t q5MaxBatchCurrenSlidingStep = q5SlidingStep;
void Nexmark_Query5_MaxBatch(void* data) {
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
const uint64_t q6JoinWindowSize = 100;
const uint64_t q6JoinSlidingStep = 10;
uint64_t q6BidCurrentSliding = q6JoinSlidingStep;
uint64_t q6AuctionCurrentSliding = q6JoinSlidingStep;
void Nexmark_Query6_Join(void* data) {
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

void Nexmark_Query6_Filter(void* data) {
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
void Nexmark_Query6_Max(void* data) {
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

const uint64_t averagePartitionSizeQ6 = 100;
std::map<uint64_t, std::vector<Q6MaxResult*>> averagePartitionsQ6;
void Nexmark_Query6_Avg(void* data) {
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
//            FastCall_request(globalFastOCall, &averageResult);
            FastCall_request_encrypt2(globalFastOCall, &averageResult, encryptedData);
        }
    }
}

const uint64_t syntheticDataWindowSize = 100;
const uint64_t syntheticDataSlidingStep = 10;
uint64_t syntheticDataCurrentSlidingStep = syntheticDataSlidingStep;
std::vector<SyntheticData*> syntheticDataArr;
void StreamBox_Query1_TopValuePerKey(void* data) {
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

std::vector<TripData*> trips;
const uint64_t tripWindowSize = 100;
const uint64_t tripSlidingStep = 100;
uint64_t tripCurrentSliding = tripSlidingStep;
void StreamBox_Query2_CountTrip(void* data) {
    if (data == NULL) {
        return;
    }

    const auto tripData = static_cast<TripData*>(data);

//     if window size reached --> delete old data
    if (trips.size() >= tripWindowSize) {
        delete trips[0];
        trips.erase(trips.begin());
    }

    // add new data
    auto* newTrip = new TripData;
    strncpy(newTrip->medallion, tripData->medallion, 64);
    trips.push_back(newTrip);

    if (trips.size() < tripWindowSize) {
        return;
    }

    tripCurrentSliding++;

    // if sliding enough --> count using a hash table<auction_id, count>
    if (tripCurrentSliding >= tripSlidingStep) {
        tripCurrentSliding = 0;

        std::map<std::string, uint64_t> tripCountByID;
        for (auto& storedTrip : trips) {
            std::string tripID = std::string(storedTrip->medallion);
            if (tripCountByID.find(tripID) == tripCountByID.end()) {
                tripCountByID[tripID] = 1;
            } else { // else, increase count of that auction to 1
                tripCountByID[tripID] += 1;
            }
        }

        // output results
        for (auto& countTripResult : tripCountByID) {
            CountTripData result{};
            strncpy(result.medallion, countTripResult.first.c_str(), 64);
            // result.medallion[countTripResult.first.length() - 1] = '\0';
            result.count = countTripResult.second;
            FastCall_request_encrypt2(globalFastOCall, &result, encryptedData);
        }
    }
}

std::vector<SyntheticData*> syntheticData1Arr;
std::vector<SyntheticData*> syntheticData2Arr;
const uint64_t syntheticDataJoinWindowSize = 50;
const uint64_t syntheticDataJoinSlidingStep = 100;
uint64_t syntheticData1CurrentSliding = syntheticDataJoinSlidingStep;
uint64_t syntheticData2CurrentSliding = syntheticDataJoinSlidingStep;
void StreamBox_Query3_JoinSyntheticData(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto syntheticData1 = dataGroup->data1 != NULL ? static_cast<SyntheticData*>(dataGroup->data1) : NULL;
    const auto syntheticData2 = dataGroup->data2 != NULL ? static_cast<SyntheticData*>(dataGroup->data2) : NULL;

    if (syntheticData2 != NULL) {
        if (syntheticData2Arr.size() >= syntheticDataJoinWindowSize) {
            delete syntheticData2Arr[0];
            syntheticData2Arr.erase(syntheticData2Arr.begin());
        }

        auto* newSyntheticData2 = new SyntheticData;
        newSyntheticData2->key = syntheticData2->key;
        newSyntheticData2->value = syntheticData2->value;
        syntheticData2Arr.push_back(newSyntheticData2);

        syntheticData2CurrentSliding++;
    }

    if (syntheticData1 != NULL) {
        if (syntheticData1Arr.size() >= syntheticDataJoinWindowSize) {
            delete syntheticData1Arr[0];
            syntheticData1Arr.erase(syntheticData1Arr.begin());
        }

        auto* newSyntheticData1 = new SyntheticData;
        newSyntheticData1->key = syntheticData1->key;
        newSyntheticData1->value = syntheticData1->value;
        syntheticData1Arr.push_back(newSyntheticData1);

        syntheticData1CurrentSliding++;
    }

    bool shouldSyntheticData2BeJoined = syntheticData2Arr.size() >= syntheticDataJoinWindowSize && syntheticData2CurrentSliding >= syntheticDataJoinSlidingStep;
    bool shouldSyntheticData1BeJoined = syntheticData1Arr.size() >= syntheticDataJoinWindowSize && syntheticData1CurrentSliding >= syntheticDataJoinSlidingStep;
    bool shouldJoin = shouldSyntheticData2BeJoined || shouldSyntheticData1BeJoined;

    if (shouldJoin) {
        if (shouldSyntheticData2BeJoined) {
            syntheticData2CurrentSliding = 0;
        }

        if (shouldSyntheticData1BeJoined) {
            syntheticData1CurrentSliding = 0;
        }

        std::vector<std::pair<SyntheticData*, SyntheticData*>> joinResults;
        for (auto& storedSyntheticData1 : syntheticData1Arr) {
            for (auto& storedSyntheticData2: syntheticData2Arr) {
                if (storedSyntheticData1->key == storedSyntheticData2->key) {
                    joinResults.emplace_back(storedSyntheticData1, storedSyntheticData2);
                }
            }
        }

        for (const auto& result : joinResults) {
            SyntheticJoinData mapJoinResult{ *result.first, *result.second };
            FastCall_request_encrypt2(globalFastOCall, &mapJoinResult, encryptedData);
        }
    }

}

std::vector<SensorData*> sensorDataWindow;
const uint64_t sensorWindowSize = 100;
const uint64_t sensorSlidingStep = 10;
uint64_t sensorCurrentSliding = sensorSlidingStep;
void StreamBox_Query4_AggregateSensorData(void* data) {
    if (data == NULL) {
        return;
    }

    const auto sensorData = static_cast<SensorData*>(data);

    // if window size reached --> delete old data
    if (sensorDataWindow.size() >= sensorWindowSize) {
        delete sensorDataWindow[0];
        sensorDataWindow.erase(sensorDataWindow.begin());
    }

    auto* newData = new SensorData;
    strncpy(newData->date, sensorData->date, 16);
    strncpy(newData->time, sensorData->time, 16);
    newData->epoch = sensorData->epoch;
    newData->moteId = sensorData->moteId;
    newData->temperature = sensorData->temperature;
    newData->humidity = sensorData->humidity;
    newData->light = sensorData->light;
    newData->voltage = sensorData->voltage;

    sensorDataWindow.push_back(newData);

    if (sensorDataWindow.size() < sensorWindowSize) {
        return;
    }

    sensorCurrentSliding++;

    // if sliding enough --> find the maximum
    if (sensorCurrentSliding >= sensorSlidingStep) {
        sensorCurrentSliding = 0;

        SensorAggregationData aggregationData{};
        aggregationData.temperature = 0;
        aggregationData.humidity = 0;
        aggregationData.light = 0;
        aggregationData.voltage = 0;

        // iterate all events in window to find the maximum
        for (auto& storedData : sensorDataWindow) {
            aggregationData.temperature += storedData->temperature;
            aggregationData.humidity += storedData->humidity;
            aggregationData.light = storedData->light;
            aggregationData.voltage += storedData->voltage;
        }

        // output result
        FastCall_request_encrypt2(globalFastOCall, &aggregationData, encryptedData);
    }

}

void StreamBox_Query5_FilterSyntheticData(void* data) {
    if (data == NULL) {
        return;
    }

    const auto syntheticData = static_cast<SyntheticData*>(data);
    if (syntheticData->value > 0 && syntheticData->value < 730222209) {
        FastCall_request_encrypt2(globalFastOCall, syntheticData, encryptedData);
    }
}

void StreamBox_Query6_MaxTrip(void* data) {
    if (data == NULL) {
        return;
    }

    const auto tripData = static_cast<TripData*>(data);

//     if window size reached --> delete old data
    if (trips.size() >= tripWindowSize) {
        delete trips[0];
        trips.erase(trips.begin());
    }

    // add new data
    auto* newTrip = new TripData;
    strncpy(newTrip->medallion, tripData->medallion, 64);
    strncpy(newTrip->hackLicense, tripData->hackLicense, 64);
    strncpy(newTrip->pickupDateTime, tripData->pickupDateTime, 64);
    strncpy(newTrip->dropOffDateTime, tripData->dropOffDateTime, 64);
    newTrip->tripTimeInSecs = tripData->tripTimeInSecs;
    newTrip->tripDistance = tripData->tripDistance;
    newTrip->pickupLongitude = tripData->pickupLongitude;
    newTrip->pickupLatitude = tripData->pickupLatitude;
    newTrip->dropOffLongitude = tripData->dropOffLongitude;
    newTrip->dropOffLatitude = tripData->dropOffLatitude;

    trips.push_back(newTrip);

    if (trips.size() < tripWindowSize) {
        return;
    }

    tripCurrentSliding++;

    // if sliding enough --> count using a hash table<auction_id, count>
    if (tripCurrentSliding >= tripSlidingStep) {
        tripCurrentSliding = 0;

        TripData* maxData = trips[0];
        for (auto& storedTrip : trips) {
            if (storedTrip->tripTimeInSecs > maxData->tripTimeInSecs) {
                maxData = storedTrip;
            }
        }

        FastCall_request_encrypt2(globalFastOCall, maxData, encryptedData);
    }
}