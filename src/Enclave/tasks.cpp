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
#include <stdexcept>

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;
char encryptedData[1000];

void MapCsvRowToFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto row = static_cast<char *>(data);
//    print("%s\n", row);
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

    FastCall_request_encrypt(globalFastOCall, &flightData);
}

void FilterFlight(void* data) {
    if (data == NULL) {
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);

    if (flightData->arrDelay > 0) {
        FastCall_request_encrypt(globalFastOCall, flightData);
    }
}

uint16_t reduceWindow = 1500                                                                                                                                        ;
std::vector<FlightData*> receivedFlightData;
std::vector<ReducedFlightData*> reducedData;
std::vector<ReducedFlightData> reducedDatas;
void ReduceFlight(void* data) {
    if (data == NULL) {
//        for (auto &reduceFlightData: reducedData) {
//            FastCall_request_encrypt(globalFastOCall, reduceFlightData);
//        }
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

    // output reduced data
    for (auto &reduceFlightData: reducedData) {
        FastCall_request_encrypt(globalFastOCall, reduceFlightData);
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
    if (data == NULL) {
        for (auto &reduceFlightData: reducedDatas) {
            strncpy(reducedDelayFlightData.uniqueCarrier, "abc\0", 4);
            FastCall_request(globalFastOCall, &reducedDelayFlightData);
        }
        return;
    }

    const auto flightData = static_cast<FlightData*> (data);
    reducedDelayFlightData.count += 1;
    reducedDelayFlightData.total += flightData->arrDelay;
    reduceDelayCount++;

    if (reduceDelayCount >= reduceDelayWindow) {
        strncpy(reducedDelayFlightData.uniqueCarrier, "abc\0", 4);
        FastCall_request(globalFastOCall, &reducedDelayFlightData);

        reduceDelayCount = 0;
        reducedDelayFlightData.count = 0;
        reducedDelayFlightData.total = 0;
    }
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
    if (bid->auction == 17600 || bid->auction == 27500 || bid->auction == 40700 || bid->auction == 51500) {
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
    if (strcmp(person->state, "or") == 0 || strcmp(person->state, "id") == 0 || strcmp(person->state, "ca") == 0) {
//    if (strcmp(person->state, "or") == 0 || strcmp(person->state, "id") == 0 || strcmp(person->state, "ca") == 0 || strcmp(person->state, "wa") == 0) {
            FastCall_request_encrypt2(globalFastOCall, person, encryptedData);
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
std::vector<std::pair<Person*, Auction*>> joinedResults;
uint64_t timeRange = 3000;
uint64_t personTimeMax = 0;
uint64_t auctionTimeMax = 0;
void NexmarkQ3_JoinPersonAuction(void* data) {
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
        if (person->datetime > personTimeMax) {
            personTimeMax = person->datetime;
            for (auto & storedPerson : people) {
                if (storedPerson->datetime >= personTimeMax - timeRange) {
                    continue;
                }

                removedPersonMap[storedPerson] = true;
                delete storedPerson;
                storedPerson = NULL;
            }

            for (size_t i = 0; i < people.size(); ++i) {
                if (people[i] == NULL) {
                    people[i] = people.back();
                    people.pop_back();
                    --i;
                }
            }
        }

        if (person->datetime >= personTimeMax - timeRange) {
            auto* newPerson = new Person;
            newPerson->id = person->id;
            strncpy(newPerson->name, person->name, PERSON_NAME_SIZE);
            strncpy(newPerson->emailAddress, person->emailAddress, PERSON_EMAIL_SIZE);
            strncpy(newPerson->creditCard, person->creditCard, PERSON_CREDIT_CARD_SIZE);
            strncpy(newPerson->city, person->city, PERSON_CITY_SIZE);
            strncpy(newPerson->state, person->state, PERSON_STATE_SIZE);
            newPerson->datetime = person->datetime;
            people.push_back(newPerson);

            isNewPersonAdded = true;
        }
    }

    if (auction != NULL) {
        if (auction->datetime > auctionTimeMax) {
            auctionTimeMax = auction->datetime;
            for (auto & storedAuction : auctions) {
                if (storedAuction->datetime >= auctionTimeMax - timeRange) {
                    continue;
                }

                removedAuctionMap[storedAuction] = true;
                delete storedAuction;
                storedAuction = NULL;
            }

            for (size_t i = 0; i < auctions.size(); ++i) {
                if (auctions[i] == NULL) {
                    auctions[i] = auctions.back();
                    auctions.pop_back();
                    --i;
                }
            }
        }

        if (auction->datetime >= auctionTimeMax - timeRange) {
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

            isNewAuctionAdded = true;
        }
    }

    for (size_t i = 0; i < joinedResults.size(); ++i) {
        if (removedPersonMap[joinedResults[i].first] || removedAuctionMap[joinedResults[i].second]) {
            joinedResults[i] = joinedResults.back();
            joinedResults.pop_back();
            --i;
        }
    }

    if (isNewPersonAdded) {
        for (const auto& storedAuction: auctions) {
            if (storedAuction->seller == person->id) {
                joinedResults.emplace_back(people.back(), storedAuction);
            }
        }
    }

    if (isNewAuctionAdded) {
        size_t peopleSize = isNewPersonAdded ? people.size() - 1 : people.size();
        for (size_t i = 0; i < peopleSize; ++i) {
            if (people[i]->id == auction->seller) {
                joinedResults.emplace_back(people[i], auctions.back());
            }
        }
    }

    for (const auto& result : joinedResults) {
        Q3JoinResult mapJoinResult{ *result.first, *result.second };
        FastCall_request_encrypt(globalFastOCall, &mapJoinResult);
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
    FastCall_request_encrypt(globalFastOCall, &result);
}

std::vector<Auction*> auctionsQ4;
std::vector<Bid*> bidsQ4;
std::vector<std::pair<Auction*, Bid*>> join1ResultsQ4;
uint64_t timeRangeQ4 = 1500;
uint64_t bidTimeMaxQ4 = 0;
uint64_t auctionTimeMaxQ4 = 0;
void NexmarkQ4_JoinAuctionBid(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto auction = dataGroup->data1 != NULL ? static_cast<Auction*>(dataGroup->data1) : NULL;
    const auto bid = dataGroup->data2 != NULL ? static_cast<Bid*>(dataGroup->data2) : NULL;
    std::map<Auction*, bool> removedAuctionMap;
    std::map<Bid*, bool> removedBidMap;

    bool isNewBidAdded = false;
    bool isNewAuctionAdded = false;

    if (bid != NULL) {
        if (bid->datetime > bidTimeMaxQ4) {
            bidTimeMaxQ4 = bid->datetime;
            for (auto & storedBid : bidsQ4) {
                if (storedBid->datetime >= bidTimeMaxQ4 - timeRangeQ4) {
                    continue;
                }

                removedBidMap[storedBid] = true;
                delete storedBid;
                storedBid = NULL;
            }

            for (size_t i = 0; i < bidsQ4.size(); ++i) {
                if (bidsQ4[i] == NULL) {
                    bidsQ4[i] = bidsQ4.back();
                    bidsQ4.pop_back();
                    --i;
                }
            }
        }

        if (bid->datetime >= bidTimeMaxQ4 - timeRangeQ4) {
            auto* newBid = new Bid;
            newBid->auction = bid->auction;
            newBid->bidder = bid->bidder;
            newBid->price = bid->price;
            newBid->datetime = bid->datetime;
//            print("%lu %lu %lu %lu\n", newBid->auction, newBid->bidder, newBid->price, newBid->datetime);
            bidsQ4.push_back(newBid);

            isNewBidAdded = true;
        }

//        print("After %ld\n", bidsQ4.size());

    }

    if (auction != NULL) {
//        print("%lu\n", auction->datetime);
        if (auction->datetime > auctionTimeMaxQ4) {
            auctionTimeMaxQ4 = auction->datetime;
            for (auto & storedAuction : auctionsQ4) {
                if (storedAuction->datetime >= auctionTimeMaxQ4 - timeRangeQ4) {
                    continue;
                }

                removedAuctionMap[storedAuction] = true;
                delete storedAuction;
                storedAuction = NULL;
            }

            for (size_t i = 0; i < auctionsQ4.size(); ++i) {
                if (auctionsQ4[i] == NULL) {
                    auctionsQ4[i] = auctionsQ4.back();
                    auctionsQ4.pop_back();
                    --i;
                }
            }
        }

        if (auction->datetime >= auctionTimeMaxQ4 - timeRangeQ4) {
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

            isNewAuctionAdded = true;
        }
    }

    for (size_t i = 0; i < join1ResultsQ4.size(); ++i) {
        if (removedAuctionMap[join1ResultsQ4[i].first] || removedBidMap[join1ResultsQ4[i].second]) {
            join1ResultsQ4[i] = join1ResultsQ4.back();
            join1ResultsQ4.pop_back();
            --i;
        }
    }

    if (isNewBidAdded) {
        for (const auto& storedAuction: auctionsQ4) {
            if (storedAuction->id == bid->auction) {
                join1ResultsQ4.emplace_back(storedAuction, bidsQ4.back());
            }
        }
    }

    if (isNewAuctionAdded) {
        size_t bidSize = isNewBidAdded ? bidsQ4.size() - 1 : bidsQ4.size();
        for (size_t i = 0; i < bidSize; ++i) {
            if (bidsQ4[i]->auction == auction->id) {
                join1ResultsQ4.emplace_back(auctionsQ4.back(), bidsQ4[i]);
            }
        }
    }

    for (const auto& result : join1ResultsQ4) {
        Q4Join1Result mapJoinResult{ *result.first, *result.second };
        FastCall_request_encrypt(globalFastOCall, &mapJoinResult);
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

    FastCall_request_encrypt(globalFastOCall, &mapResult);
}

std::map<uint64_t, uint64_t> maxPriceByCategoryQ4;
void NexmarkQ4_MaxAuctionPriceByCategory(void* data) {
    if (data == NULL) {
        return;
    }

    const auto parsedData = static_cast<Q4Map1Result*>(data);
    if (maxPriceByCategoryQ4.find(parsedData->category) != maxPriceByCategoryQ4.end()) {
        if (maxPriceByCategoryQ4[parsedData->category] < parsedData->final) {
            maxPriceByCategoryQ4[parsedData->category] = parsedData->final;
            Q4Map1Result result{};
            result.category = parsedData->category;
            result.final = parsedData->final;
            FastCall_request_encrypt(globalFastOCall, &result);
        }
    } else {
        maxPriceByCategoryQ4[parsedData->category] = parsedData->final;
        Q4Map1Result result{};
        result.category = parsedData->category;
        result.final = parsedData->final;
        FastCall_request_encrypt(globalFastOCall, &result);
    }

//    for (auto & it : maxPriceByCategoryQ4) {
//        Q4Map1Result result{};
//        result.category = it.first;
//        result.final = it.second;
//        FastCall_request(globalFastOCall, &result);
//    }
}

uint64_t categories[10] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
std::vector<std::pair<uint64_t, Q4Map1Result*>> join2ResultsQ4;
const uint64_t windowSizeJoin2Q4 = 100;
std::vector<std::pair<uint64_t, Q4Map1Result*>> map1ResultsQ4;
uint64_t mapResultIdQ4 = 0;
void NexmarkQ4_JoinWithCategory(void* data) {
    if (data == NULL) {
        return;
    }

    const auto mapResult = static_cast<Q4Map1Result*>(data);
    mapResultIdQ4 += 1;
    uint64_t deletedId = -1;
    if (map1ResultsQ4.size() == windowSizeJoin2Q4) {
        deletedId = map1ResultsQ4[0].first;
        delete map1ResultsQ4[0].second;
        map1ResultsQ4.erase(map1ResultsQ4.begin());
    }
    auto newResult = new Q4Map1Result;
    newResult->category = mapResult->category;
    newResult->final = mapResult->final;
    map1ResultsQ4.emplace_back(mapResultIdQ4, newResult);

    if (deletedId != -1) {
        for (size_t i = 0; i < join2ResultsQ4.size(); ++i) {
            if (join2ResultsQ4[i].first == deletedId) {
                join2ResultsQ4[i] = join2ResultsQ4.back();
                join2ResultsQ4.pop_back();
                --i;
            }
        }
    }

    for (const auto& category : categories) {
        if (newResult->category == category) {
            join2ResultsQ4.emplace_back(mapResultIdQ4, newResult);
        }
    }

    for (const auto& join2Result : join2ResultsQ4) {
        FastCall_request(globalFastOCall, join2Result.second);
    }
}

std::map<uint64_t, std::pair<uint64_t, uint64_t>> averagesQ4;
void NexmarkQ4_Average(void* data) {
    if (data == NULL) {
        return;
    }

    const auto maxResult = static_cast<Q4Map1Result*>(data);
    if (averagesQ4.find(maxResult->category) == averagesQ4.end()) {
        averagesQ4[maxResult->category] = std::make_pair(0, 0);
    }

    auto& resultBySeller = averagesQ4[maxResult->category];
    resultBySeller.first += maxResult->final;
    resultBySeller.second += 1;
    const auto avg = resultBySeller.first / resultBySeller.second;

    Q4Map1Result avgResult{};
    avgResult.category = maxResult->category;
    avgResult.final = avg;
    FastCall_request(globalFastOCall, &avgResult);
}

//std::vector<Bid*> bidsCountTotalQ5;
//uint64_t bidTimeMaxCountTotalQ5 = 0;
uint64_t timeRangeQ5 = 600;
//void NexmarkQ5_CountTotal(void* data) {
//    if (data == NULL) {
//        return;
//    }
//
//    const auto bid = static_cast<Bid*>(data);
//    if (bid->datetime > bidTimeMaxCountTotalQ5) {
//        bidTimeMaxCountTotalQ5 = bid->datetime;
//        for (auto & storedBid : bidsCountTotalQ5) {
//            if (storedBid->datetime >= bidTimeMaxCountTotalQ5 - timeRangeQ5) {
//                continue;
//            }
//
//            delete storedBid;
//            storedBid = NULL;
//        }
//
//        for (size_t i = 0; i < bidsCountTotalQ5.size(); ++i) {
//            if (bidsCountTotalQ5[i] == NULL) {
//                bidsCountTotalQ5[i] = bidsCountTotalQ5.back();
//                bidsCountTotalQ5.pop_back();
//                --i;
//            }
//        }
//    }
//
//    if (bid->datetime >= bidTimeMaxCountTotalQ5 - timeRangeQ5) {
//        auto* newBid = new Bid;
//        newBid->auction = bid->auction;
//        newBid->bidder = bid->bidder;
//        newBid->price = bid->price;
//        newBid->datetime = bid->datetime;
//        bidsCountTotalQ5.push_back(newBid);
//    }
//
//    Q5CountTotalResult result{};
//    result.count = bidsCountTotalQ5.size();
//    result.datetime = bid->datetime;
//    FastCall_request(globalFastOCall, &result);
//}

std::vector<Bid*> bidsCountByAuctionQ5;
std::map<uint64_t, uint64_t> countByAuctionResultsQ5;
uint64_t bidTimeMaxCountByAuctionQ5 = 0;
uint64_t countId = 0;
void NexmarkQ5_CountByAuction(void* data) {
    if (data == NULL) {
        return;
    }

//    std::map<uint64_t, bool> changedAuctions;

    const auto bid = static_cast<Bid*>(data);
    if (bid->datetime > bidTimeMaxCountByAuctionQ5) {
        bidTimeMaxCountByAuctionQ5 = bid->datetime;
        for (auto & storedBid : bidsCountByAuctionQ5) {
            if (storedBid->datetime >= bidTimeMaxCountByAuctionQ5 - timeRangeQ5) {
                continue;
            }

            countByAuctionResultsQ5[storedBid->auction] -= 1;
            if (countByAuctionResultsQ5[storedBid->auction] <= 0) {
                countByAuctionResultsQ5.erase(storedBid->auction);
            }
//            changedAuctions[storedBid->auction] = true;
            delete storedBid;
            storedBid = NULL;
        }

        for (size_t i = 0; i < bidsCountByAuctionQ5.size(); ++i) {
            if (bidsCountByAuctionQ5[i] == NULL) {
                bidsCountByAuctionQ5[i] = bidsCountByAuctionQ5.back();
                bidsCountByAuctionQ5.pop_back();
                --i;
            }
        }
    }

    if (bid->datetime >= bidTimeMaxCountByAuctionQ5 - timeRangeQ5) {
        auto* newBid = new Bid;
        newBid->auction = bid->auction;
        newBid->bidder = bid->bidder;
        newBid->price = bid->price;
        newBid->datetime = bid->datetime;
        bidsCountByAuctionQ5.push_back(newBid);

        if (countByAuctionResultsQ5.find(bid->auction) == countByAuctionResultsQ5.end()) {
            countByAuctionResultsQ5[bid->auction] = 1;
        } else {
            countByAuctionResultsQ5[bid->auction] += 1;
        }
//        changedAuctions[bid->auction] = true;
    }

//    if (!changedAuctions.empty()) {
//        for (const auto& changedAuction : changedAuctions) {
//            Q5CountByAuctionResult result{};
//            result.auction = changedAuction.first;
//            result.count = countByAuctionResultsQ5[result.auction];
//            result.datetime = bid->datetime;
//            FastCall_request(globalFastOCall, &result);
//        }
//    }

    for (const auto& countAuctionResult : countByAuctionResultsQ5) {
        Q5CountByAuctionResult result{};
        result.auction = countAuctionResult.first;
        result.count = countAuctionResult.second;
        result.datetime = countId;
        FastCall_request_encrypt(globalFastOCall, &result);
    }
    countId++;
}

std::vector<Q5CountByAuctionResult*> countByAuctionMaxBatchQ5;
uint64_t maxBatchValueQ5 = 0;
void NexmarkQ5_MaxBatch(void* data) {
    if (data == NULL) {
        return;
    }

    const auto countByAuctionResult = static_cast<Q5CountByAuctionResult*>(data);
    if (countByAuctionMaxBatchQ5.empty() || countByAuctionResult->datetime == countByAuctionMaxBatchQ5.back()->datetime) {
        auto result = new Q5CountByAuctionResult;
        result->datetime = countByAuctionResult->datetime;
        result->count = countByAuctionResult->count;
        result->auction = countByAuctionResult->auction;
        countByAuctionMaxBatchQ5.push_back(result);

        if (result->count > maxBatchValueQ5) {
            maxBatchValueQ5 = result->count;
        }
        return;
    }

    std::vector<Q5CountByAuctionResult*> outputResults;
    for (const auto storedResult : countByAuctionMaxBatchQ5) {
        if (storedResult->count >= maxBatchValueQ5) {
            outputResults.push_back(storedResult);
        }
    }

    if (!outputResults.empty()) {
        for (const auto outputResult : outputResults) {
            FastCall_request_encrypt(globalFastOCall, outputResult);
        }
    }

    for (const auto storedResult : countByAuctionMaxBatchQ5) {
        delete storedResult;
    }
    countByAuctionMaxBatchQ5.clear();

    auto result = new Q5CountByAuctionResult;
    result->datetime = countByAuctionResult->datetime;
    result->count = countByAuctionResult->count;
    result->auction = countByAuctionResult->auction;
    countByAuctionMaxBatchQ5.push_back(result);
    maxBatchValueQ5 = result->count;
}

//std::vector<Q5CountByAuctionResult*> countByAuctionFilterQ5;
////std::vector<Q5CountTotalResult*> countTotalFilterQ5;
//std::map<uint64_t, uint64_t> countTotalMappingFilterQ5;
//void NexmarkQ5_Filter(void* data) {
//    if (data == NULL) {
//        return;
//    }
//
//    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
//    const auto countByAuctionResult = dataGroup->data1 != NULL ? static_cast<Q5CountByAuctionResult*>(dataGroup->data1) : NULL;
//    const auto countTotalResult = dataGroup->data2 != NULL ? static_cast<Q5CountTotalResult*>(dataGroup->data2) : NULL;
//
//    if (countByAuctionResult != NULL) {
//        auto result = new Q5CountByAuctionResult;
//        result->datetime = countByAuctionResult->datetime;
//        result->auction = countByAuctionResult->auction;
//        result->count = countByAuctionResult->count;
//        countByAuctionFilterQ5.push_back(result);
//    }
//
//    if (countTotalResult != NULL) {
////        auto result = new Q5CountTotalResult;
////        result->datetime = countTotalResult->datetime;
////        result->count = countTotalResult->count;
////        countTotalFilterQ5.push_back(result);
//
//        countTotalMappingFilterQ5[countTotalResult->datetime] = countTotalResult->count;
//    }
//
//    if ((countByAuctionResult == NULL && countTotalResult == NULL) || countByAuctionFilterQ5.empty() || countTotalMappingFilterQ5.empty()) {
//        print("Return\n");
//        return;
//    }
//
//    uint64_t maxDatetime = countByAuctionFilterQ5.back()->datetime;
//    std::vector<uint64_t> auctionOutputs;
//    for (auto& storedCountByAuctionResult : countByAuctionFilterQ5) {
//        uint64_t currentDatetime = storedCountByAuctionResult->datetime;
//        if (countTotalMappingFilterQ5.find(currentDatetime) != countTotalMappingFilterQ5.end()) {
//            if (storedCountByAuctionResult->count <= countTotalMappingFilterQ5[currentDatetime]) {
//                auctionOutputs.push_back(storedCountByAuctionResult->auction);
//            }
//            storedCountByAuctionResult->datetime = 0;
//        }
////        size_t countTotalFilterSize = countTotalFilterQ5.size();
////        for (int i = (int)countTotalFilterSize - 1; i >= 0; --i) {
////            if (
////                    countTotalFilterQ5[i]->datetime == currentDatetime &&
////                    (i == 0 || countTotalFilterQ5[i-1]->datetime > currentDatetime)) {
////                if (storedCountByAuctionResult->count > countTotalFilterQ5[i]->count) {
////                    auctionOutputs.push_back(storedCountByAuctionResult->auction);
////                }
////                storedCountByAuctionResult->datetime = 0;
////                break;
////            }
////        }
//
////        if (storedCountByAuctionResult->datetime != 0 && storedCountByAuctionResult->datetime < countTotalFilterQ5[0]->datetime) {
////            storedCountByAuctionResult->datetime = 0;
////        }
//    }
//
//    for (size_t i = 0; i < countByAuctionFilterQ5.size(); ++i) {
//        if (countByAuctionFilterQ5[i]->datetime == 0) {
//            countByAuctionFilterQ5.erase(countByAuctionFilterQ5.begin() + (long)i);
//            --i;
//        }
//    }
////    countTotalFilterQ5.clear();
//
//    if (!countByAuctionFilterQ5.empty()) {
//        maxDatetime = countByAuctionFilterQ5[0]->datetime;
//    }
//
//    std::vector<uint64_t> deletedKeys;
//    for (auto & it : countTotalMappingFilterQ5) {
////        print("%lu ", it.first - maxDatetime);
//        if (it.first < maxDatetime) {
//            deletedKeys.push_back(it.first);
////            countTotalMappingFilterQ5.erase(it);
//        }
////        print("\n");
//    }
//
//    for (auto key : deletedKeys) {
//        countTotalMappingFilterQ5.erase(key);
//    }
//
////    if (deletedKeys.empty()) {
////        print("%lu %lu\n", maxDatetime, countTotalMappingFilterQ5.begin()->first);
////    }
//
////    print("%lu %lu\n", countByAuctionFilterQ5.size(), countTotalMappingFilterQ5.size());
//
////    size_t maxIndex = -1;
////    if (countTotalFilterQ5.back()->datetime == maxDatetime) {
////        maxIndex = countTotalFilterQ5.size() - 1;
////    } else if (countTotalFilterQ5.back()->datetime < maxDatetime) {
////        maxIndex = countByAuctionFilterQ5.size();
////    } else {
////        for (int i = (int)countTotalFilterQ5.size() - 2; i >= 0; --i) {
////            print("maxDatetime: %lu, i_Datetime: %lu, i+1_Datetime: %lu\n", maxDatetime, countTotalFilterQ5[i]->datetime, countTotalFilterQ5[i+1]->datetime);
////            if (
////                    (countTotalFilterQ5[i]->datetime < maxDatetime && countTotalFilterQ5[i+1]->datetime >= maxDatetime) ||
////                    (countTotalFilterQ5[i]->datetime == maxDatetime && countTotalFilterQ5[i+1]->datetime == maxDatetime)
////                    ) {
////                maxIndex = i + 1;
////                break;
////            }
////        }
////    }
////
//////    print("%lu\n", maxIndex);
////
////    if (maxIndex > -1) {
////        countTotalFilterQ5.erase(countTotalFilterQ5.begin(), countTotalFilterQ5.begin() + (long)maxIndex);
////    }
////
//////    print("%lu\n", countTotalFilterQ5.size());
//
//    for (auto output : auctionOutputs) {
//        FastCall_request(globalFastOCall, &output);
//    }
//}

std::vector<Auction*> auctionsQ6;
std::vector<Bid*> bidsQ6;
std::vector<std::pair<Auction*, Bid*>> join1ResultsQ6;
uint64_t timeRangeQ6 = 600;
uint64_t bidTimeMaxQ6 = 0;
uint64_t auctionTimeMaxQ6 = 0;
void NexmarkQ6_Join(void* data) {
    if (data == NULL) {
        return;
    }

    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
    const auto auction = dataGroup->data1 != NULL ? static_cast<Auction*>(dataGroup->data1) : NULL;
    const auto bid = dataGroup->data2 != NULL ? static_cast<Bid*>(dataGroup->data2) : NULL;
    std::map<Auction*, bool> removedAuctionMap;
    std::map<Bid*, bool> removedBidMap;

    bool isNewBidAdded = false;
    bool isNewAuctionAdded = false;

    if (bid != NULL) {
        if (bid->datetime > bidTimeMaxQ6) {
            bidTimeMaxQ6 = bid->datetime;
            for (auto & storedBid : bidsQ6) {
                if (storedBid->datetime >= bidTimeMaxQ6 - timeRangeQ6) {
                    continue;
                }

                removedBidMap[storedBid] = true;
                delete storedBid;
                storedBid = NULL;
            }

            for (size_t i = 0; i < bidsQ6.size(); ++i) {
                if (bidsQ6[i] == NULL) {
                    bidsQ6[i] = bidsQ6.back();
                    bidsQ6.pop_back();
                    --i;
                }
            }
        }

        if (bid->datetime >= bidTimeMaxQ6 - timeRangeQ6) {
            auto* newBid = new Bid;
            newBid->auction = bid->auction;
            newBid->bidder = bid->bidder;
            newBid->price = bid->price;
            newBid->datetime = bid->datetime;
            bidsQ6.push_back(newBid);

            isNewBidAdded = true;
        }
    }

    if (auction != NULL) {
        if (auction->datetime > auctionTimeMaxQ6) {
            auctionTimeMaxQ6 = auction->datetime;
            for (auto & storedAuction : auctionsQ6) {
                if (storedAuction->datetime >= auctionTimeMaxQ6 - timeRangeQ6) {
                    continue;
                }

                removedAuctionMap[storedAuction] = true;
                delete storedAuction;
                storedAuction = NULL;
            }

            for (size_t i = 0; i < auctionsQ6.size(); ++i) {
                if (auctionsQ6[i] == NULL) {
                    auctionsQ6[i] = auctionsQ6.back();
                    auctionsQ6.pop_back();
                    --i;
                }
            }
        }

        if (auction->datetime >= auctionTimeMaxQ6 - timeRangeQ6) {
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

            isNewAuctionAdded = true;
        }
    }

    bool shouldPushJoinResult = false;
    if (!removedAuctionMap.empty() || !removedBidMap.empty()) {
        shouldPushJoinResult = true;
        for (size_t i = 0; i < join1ResultsQ6.size(); ++i) {
            if (removedAuctionMap[join1ResultsQ6[i].first] || removedBidMap[join1ResultsQ6[i].second]) {
                join1ResultsQ6[i] = join1ResultsQ6.back();
                join1ResultsQ6.pop_back();
                --i;
            }
        }
    }

    if (isNewBidAdded) {
        shouldPushJoinResult = true;
        for (const auto& storedAuction: auctionsQ6) {
            if (storedAuction->id == bid->auction) {
                join1ResultsQ6.emplace_back(storedAuction, bidsQ6.back());
            }
        }
    }

    if (isNewAuctionAdded) {
        shouldPushJoinResult = true;
        size_t bidSize = isNewBidAdded ? bidsQ6.size() - 1 : bidsQ6.size();
        for (size_t i = 0; i < bidSize; ++i) {
            if (bidsQ6[i]->auction == auction->id) {
                join1ResultsQ6.emplace_back(auctionsQ6.back(), bidsQ6[i]);
            }
        }
    }

    if (shouldPushJoinResult) {
        for (const auto& result : join1ResultsQ6) {
            Q6JoinResult mapJoinResult{ *result.first, *result.second };
            FastCall_request_encrypt(globalFastOCall, &mapJoinResult);
        }
    }
}

void NexmarkQ6_Filter(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q6JoinResult *>(data);
    if (joinResult->bid.datetime < joinResult->auction.expires) {
        FastCall_request_encrypt(globalFastOCall, joinResult);
    }
}

const uint64_t partitionSizeQ6 = 10;
std::map<uint64_t, std::vector<Q6JoinResult*>> partitionsQ6;
void NexmarkQ6_Max(void* data) {
    if (data == NULL) {
        return;
    }

    const auto joinResult = static_cast<Q6JoinResult *>(data);
    uint64_t seller = joinResult->auction.seller;

    if (partitionsQ6.find(seller) != partitionsQ6.end()) {
        auto& resultsInPartition = partitionsQ6[seller];
        if (resultsInPartition.size() >= partitionSizeQ6) {
            delete resultsInPartition[0];
            resultsInPartition.erase(resultsInPartition.begin());
        }
    } else {
        partitionsQ6[seller].reserve(partitionSizeQ6);
    }

    auto& resultsInPartition = partitionsQ6[seller];
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
    resultsInPartition.push_back(newJoinResult);

    std::vector<Q6MaxResult> maxResults;
    for (const auto& partition : partitionsQ6) {
        if (partition.second.size() < partitionSizeQ6) {
            continue;
        }

        uint64_t currentMaxPrice = partition.second[0]->bid.price;
        for (const auto& result : partition.second) {
            if (result->bid.price > currentMaxPrice) {
                currentMaxPrice = result->bid.price;
            }
        }

        Q6MaxResult newMaxResult{};
        newMaxResult.final = currentMaxPrice;
        newMaxResult.seller = partition.first;
        maxResults.emplace_back(newMaxResult);
    }

    if (!maxResults.empty()) {
        for (auto& maxResult : maxResults) {
            FastCall_request_encrypt(globalFastOCall, &maxResult);
        }
    }

//    uint64_t currentMaxPrice = partitionsQ6[seller][0]->bid.price;
//    bool maxPriceChanged = false;
//    for (const auto& resultInPartition : resultsInPartition) {
//        if (resultInPartition->bid.price > currentMaxPrice) {
//            currentMaxPrice = resultInPartition->bid.price;
//            maxPriceChanged = true;
//        }
//    }
//
//    if (maxPriceChanged) {
//        Q6MaxResult maxResult{};
//        maxResult.seller = seller;
//        maxResult.final = currentMaxPrice;
//        FastCall_request(globalFastOCall, &maxResult);
//    }
}

std::map<uint64_t, std::pair<uint64_t, uint64_t>> averagesQ6;
std::vector<Q6MaxResult*> averageMaxResultsQ6;
uint64_t averageWindowSizeQ6 = 100;
void NexmarkQ6_Avg(void* data) {
    if (data == NULL) {
        return;
    }

    const auto maxResult = static_cast<Q6MaxResult*>(data);
    if (averageMaxResultsQ6.size() >= averageWindowSizeQ6) {
        const auto& deleteResult = averageMaxResultsQ6[0];
        averagesQ6[deleteResult->seller].first -= deleteResult->final;
        averagesQ6[deleteResult->seller].second -= 1;
        delete averageMaxResultsQ6[0];
        averageMaxResultsQ6.erase(averageMaxResultsQ6.begin());
    }

    auto* newMaxResult = new Q6MaxResult;
    newMaxResult->seller = maxResult->seller;
    newMaxResult->final = maxResult->final;
    averageMaxResultsQ6.push_back(newMaxResult);

    if (averagesQ6.find(maxResult->seller) == averagesQ6.end()) {
        averagesQ6[maxResult->seller] = std::make_pair(0, 0);
    }

    auto& resultBySeller = averagesQ6[maxResult->seller];
    resultBySeller.first += maxResult->final;
    resultBySeller.second += 1;

    if (averageMaxResultsQ6.size() < averageWindowSizeQ6) {
        return;
    }

    for (const auto& averageResult : averagesQ6) {
        Q6MaxResult avgResult{};
        avgResult.seller = averageResult.first;
        avgResult.final = averageResult.second.first / averageResult.second.second;
        FastCall_request_encrypt(globalFastOCall, &avgResult);
    }
}

//std::vector<Bid*> bidsQ7Max;
//uint64_t bidTimeMaxQ7 = 0;
//uint64_t timeRangeQ7 = 300;
//uint64_t maxPriceQ7 = 0;
//void NexmarkQ7_Max(void* data) {
//    if (data == NULL) {
//        return;
//    }
//
//    const auto bid = static_cast<Bid*>(data);
//
//    if (bid->datetime > bidTimeMaxQ7) {
//        bidTimeMaxQ7 = bid->datetime;
//        for (auto & storedBid : bidsQ7Max) {
//            if (storedBid->datetime >= bidTimeMaxQ7 - timeRangeQ7) {
//                continue;
//            }
//
//            delete storedBid;
//            storedBid = NULL;
//        }
//
//        for (size_t i = 0; i < bidsQ7Max.size(); ++i) {
//            if (bidsQ7Max[i] == NULL) {
//                bidsQ7Max[i] = bidsQ7Max.back();
//                bidsQ7Max.pop_back();
//                --i;
//            }
//        }
//    }
//
//    if (bid->datetime >= bidTimeMaxQ7 - timeRangeQ7) {
//        auto* newBid = new Bid;
//        newBid->auction = bid->auction;
//        newBid->bidder = bid->bidder;
//        newBid->price = bid->price;
//        newBid->datetime = bid->datetime;
//        bidsQ7Max.push_back(newBid);
//    }
//
//    for (const auto& storedBid : bidsQ7Max) {
//        if (storedBid->price > maxPriceQ7) {
//            maxPriceQ7 = storedBid->price;
//        }
//    }
//
//    FastCall_request(globalFastOCall, &maxPriceQ7);
//}
//
//std::vector<Bid*> bidsQ7Join;
//uint64_t maxPriceQ7Join = 0;
//uint64_t bidTimeJoinQ7 = 0;
//void NexmarkQ7_Join(void* data) {
//    if (data == NULL) {
//        return;
//    }
//
//    const auto dataGroup = static_cast<FastCallDataGroup*>(data);
//    Bid *bid = dataGroup->data1 != NULL ? static_cast<Bid*>(dataGroup->data1) : NULL;
//    const auto maxPrice = dataGroup->data2 != NULL ? static_cast<uint64_t *>(dataGroup->data2) : NULL;
//
//    bool hasRemoved = false;
//    bool isNewBidAdded = false;
//    bool isMaxPriceChanged = false;
//
//    if (bid != NULL && bid->datetime > bidTimeJoinQ7) {
//        bidTimeJoinQ7 = bid->datetime;
//        for (auto & storedBid : bidsQ7Join) {
//            if (storedBid->datetime >= bidTimeJoinQ7 - timeRangeQ7) {
//                continue;
//            }
//
//            delete storedBid;
//            storedBid = NULL;
//        }
//
//        for (size_t i = 0; i < bidsQ7Join.size(); ++i) {
//            if (bidsQ7Join[i] == NULL) {
//                hasRemoved = true;
//                bidsQ7Join[i] = bidsQ7Join.back();
//                bidsQ7Join.pop_back();
//                --i;
//            }
//        }
//    }
//
//    if (bid != NULL && bid->datetime >= bidTimeJoinQ7 - timeRangeQ7) {
//        isNewBidAdded = true;
//        auto* newBid = new Bid;
//        newBid->auction = bid->auction;
//        newBid->bidder = bid->bidder;
//        newBid->price = bid->price;
//        newBid->datetime = bid->datetime;
//        bidsQ7Join.push_back(newBid);
//    }
//
//    if (maxPrice != NULL && *maxPrice > maxPriceQ7Join) {
//        isMaxPriceChanged = true;
//        maxPriceQ7Join = *maxPrice;
//    }
//
//    if (!(hasRemoved || isNewBidAdded || isMaxPriceChanged)) {
//        return;
//    }
//
//    std::vector<Bid*> joinResultQ7;
//    for (const auto& storedBid : bidsQ7Join) {
//        if (storedBid->price == maxPriceQ7Join) {
//            joinResultQ7.push_back(storedBid);
//        }
//    }
//
//    if (!joinResultQ7.empty()) {
//        for (const auto& result : joinResultQ7) {
//            FastCall_request(globalFastOCall, result);
//        }
//    }
//}

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
            FastCall_request_encrypt(globalFastOCall, result);
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
uint64_t timeRangeQ8 = 1500;
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
        FastCall_request_encrypt(globalFastOCall, &mapJoinResult);
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
    FastCall_request_encrypt(globalFastOCall, &mapResult);
}