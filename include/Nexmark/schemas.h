//
// Created by hungpm on 06/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_NEXMARK_SCHEMAS_H
#define SIMPLE_DSP_ENGINE_NEXMARK_SCHEMAS_H

#include <inttypes.h>

#define PERSON_NAME_SIZE 32
#define PERSON_EMAIL_SIZE 32
#define PERSON_CREDIT_CARD_SIZE 32
#define PERSON_CITY_SIZE 32
#define PERSON_STATE_SIZE 16
#define AUCTION_ITEM_NAME_SIZE 32

struct Person {
    uint64_t id;
    char name[PERSON_NAME_SIZE];
    char emailAddress[PERSON_EMAIL_SIZE];
    char creditCard[PERSON_CREDIT_CARD_SIZE];
    char city[PERSON_CITY_SIZE];
    char state[PERSON_STATE_SIZE];
    uint64_t datetime;
};

struct Auction {
    uint64_t id;
    char itemName[AUCTION_ITEM_NAME_SIZE];
    uint64_t initialBid;
    uint64_t reserve;
    uint64_t datetime;
    uint64_t expires;
    uint64_t seller;
    uint16_t category;
};

struct Bid {
    uint64_t auction;
    uint64_t bidder;
    uint64_t datetime;
    uint64_t price;
};

struct Q2Result {
    uint64_t auction;
    uint64_t price;
};

struct Q3JoinResult {
    Person person;
    Auction auction;
};

struct Q3Result {
    char name[PERSON_NAME_SIZE];
    char city[PERSON_CITY_SIZE];
    char state[PERSON_STATE_SIZE];
    uint64_t auctionId;
};

struct Q4Join1Result {
    Auction auction;
    Bid bid;
};

struct Q4Map1Result {
    uint64_t final;
    uint64_t category;
};

struct Q4AverageResult {
    double average;
    uint64_t category;
};

struct Q5CountTotalResult {
    uint64_t datetime;
    uint64_t count;
};

struct Q5CountByAuctionResult {
    uint64_t datetime;
    uint64_t auction;
    uint64_t count;
};

struct Q6JoinResult {
    Auction auction;
    Bid bid;
};

struct Q6MaxResult {
    uint64_t final;
    uint64_t seller;
};

struct Q6AverageResult {
    double final;
    uint64_t seller;
};

struct Q8JoinResult {
    Person person;
    Auction auction;
};

struct Q8MapResult {
    uint64_t personId;
    char personName[PERSON_NAME_SIZE];
    uint64_t auctionReserve;
};

struct QAJoinPersonBidResult {
    Person person;
    Bid bid;
};

#endif //SIMPLE_DSP_ENGINE_NEXMARK_SCHEMAS_H
