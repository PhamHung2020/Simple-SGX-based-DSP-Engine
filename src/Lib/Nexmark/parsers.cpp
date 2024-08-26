//
// Created by hungpm on 06/06/2024.
//
#include "Nexmark/parsers.h"
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>

PersonParser::PersonParser() {
    this->person_ = new Person();
}

PersonParser::~PersonParser() {
    delete this->person_;
}

void *PersonParser::parseFromString(const std::string &str) {
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
        if (words.size() != 7) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->person_->id = std::stoul(words[0]);
        } else {
            this->person_->id = 0;
        }

        if (!words[1].empty()) {
            strncpy(this->person_->name, words[1].c_str(), PERSON_NAME_SIZE);
        } else {
            strncpy(this->person_->name, "UNKNOWN", PERSON_NAME_SIZE);
        }

        if (!words[2].empty()) {
            strncpy(this->person_->emailAddress, words[2].c_str(), PERSON_EMAIL_SIZE);
        } else {
            strncpy(this->person_->emailAddress, "UNKNOWN", PERSON_EMAIL_SIZE);
        }

        if (!words[3].empty()) {
            strncpy(this->person_->creditCard, words[3].c_str(), PERSON_CREDIT_CARD_SIZE);
        } else {
            strncpy(this->person_->creditCard, "UNKNOWN", PERSON_CREDIT_CARD_SIZE);
        }

        if (!words[4].empty()) {
            strncpy(this->person_->city, words[4].c_str(), PERSON_CITY_SIZE);
        } else {
            strncpy(this->person_->city, "UNKNOWN", PERSON_CITY_SIZE);
        }

        if (!words[5].empty()) {
            strncpy(this->person_->state, words[5].c_str(), PERSON_STATE_SIZE);
        } else {
            strncpy(this->person_->state, "UNKNOWN", PERSON_STATE_SIZE);
        }

        if (!words[6].empty()) {
            this->person_->datetime = std::stoul(words[6]);
        } else {
            this->person_->datetime = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->person_;
}

AuctionParser::AuctionParser() {
    this->auction_ = new Auction();
}

AuctionParser::~AuctionParser() {
    delete this->auction_;
}

void *AuctionParser::parseFromString(const std::string &str) {
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
            this->auction_->id = std::stoul(words[0]);
        } else {
            this->auction_->id = 0;
        }

        if (!words[1].empty()) {
            strncpy(this->auction_->itemName, words[1].c_str(), AUCTION_ITEM_NAME_SIZE);
        } else {
            strncpy(this->auction_->itemName, "UNKNOWN", AUCTION_ITEM_NAME_SIZE);
        }

        if (!words[2].empty()) {
            this->auction_->initialBid = std::stoul(words[2]);
        } else {
            this->auction_->initialBid = 0;
        }

        if (!words[3].empty()) {
            this->auction_->reserve = std::stoul(words[3]);
        } else {
            this->auction_->reserve = 0;
        }

        if (!words[4].empty()) {
            this->auction_->datetime = std::stoul(words[4]);
        } else {
            this->auction_->datetime = 0;
        }

        if (!words[5].empty()) {
            this->auction_->expires = std::stoul(words[5]);
        } else {
            this->auction_->expires = 0;
        }

        if (!words[6].empty()) {
            this->auction_->seller = std::stoul(words[6]);
        } else {
            this->auction_->seller = 0;
        }

        if (!words[7].empty()) {
            this->auction_->category = std::stoul(words[7]);
        } else {
            this->auction_->category = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->auction_;
}

BidParser::BidParser() {
    this->bid_ = new Bid();
}

BidParser::~BidParser() {
    delete this->bid_;
}

void *BidParser::parseFromString(const std::string &str) {
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

        if (words.size() != 4) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->bid_->auction = std::stoul(words[0]);
        } else {
            this->bid_->auction = 0;
        }

        if (!words[1].empty()) {
            this->bid_->bidder = std::stoul(words[1]);
        } else {
            this->bid_->bidder = 0;
        }

        if (!words[2].empty()) {
            this->bid_->price = std::stoul(words[2]);
        } else {
            this->bid_->price = 0;
        }

        if (!words[3].empty()) {
            this->bid_->datetime = std::stoul(words[3]);
        } else {
            this->bid_->datetime = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    this->bid_->auction = 1000;
    return this->bid_;
}

void *Q3JoinResultParser::parseFromString(const std::string &str) {
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
        if (words.size() != 15) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->result_->auction.id = std::stoul(words[0]);
        } else {
            this->result_->auction.id = 0;
        }

        if (!words[1].empty()) {
            strncpy(this->result_->auction.itemName, words[1].c_str(), AUCTION_ITEM_NAME_SIZE);
        } else {
            strncpy(this->result_->auction.itemName, "UNKNOWN", AUCTION_ITEM_NAME_SIZE);
        }

        if (!words[2].empty()) {
            this->result_->auction.initialBid = std::stoul(words[2]);
        } else {
            this->result_->auction.initialBid = 0;
        }

        if (!words[3].empty()) {
            this->result_->auction.reserve = std::stoul(words[3]);
        } else {
            this->result_->auction.reserve = 0;
        }

        if (!words[4].empty()) {
            this->result_->auction.datetime = std::stoul(words[4]);
        } else {
            this->result_->auction.datetime = 0;
        }

        if (!words[5].empty()) {
            this->result_->auction.expires = std::stoul(words[5]);
        } else {
            this->result_->auction.expires = 0;
        }

        if (!words[6].empty()) {
            this->result_->auction.seller = std::stoul(words[6]);
        } else {
            this->result_->auction.seller = 0;
        }

        if (!words[7].empty()) {
            this->result_->auction.category = std::stoul(words[7]);
        } else {
            this->result_->auction.category = 0;
        }

        if (!words[8].empty()) {
            this->result_->person.id = std::stoul(words[8]);
        } else {
            this->result_->person.id = 0;
        }

        if (!words[9].empty()) {
            strncpy(this->result_->person.name, words[9].c_str(), PERSON_NAME_SIZE);
        } else {
            strncpy(this->result_->person.name, "UNKNOWN", PERSON_NAME_SIZE);
        }

        if (!words[10].empty()) {
            strncpy(this->result_->person.emailAddress, words[10].c_str(), PERSON_EMAIL_SIZE);
        } else {
            strncpy(this->result_->person.emailAddress, "UNKNOWN", PERSON_EMAIL_SIZE);
        }

        if (!words[11].empty()) {
            strncpy(this->result_->person.creditCard, words[11].c_str(), PERSON_CREDIT_CARD_SIZE);
        } else {
            strncpy(this->result_->person.creditCard, "UNKNOWN", PERSON_CREDIT_CARD_SIZE);
        }

        if (!words[12].empty()) {
            strncpy(this->result_->person.city, words[12].c_str(), PERSON_CITY_SIZE);
        } else {
            strncpy(this->result_->person.city, "UNKNOWN", PERSON_CITY_SIZE);
        }

        if (!words[13].empty()) {
            strncpy(this->result_->person.state, words[13].c_str(), PERSON_STATE_SIZE);
        } else {
            strncpy(this->result_->person.state, "UNKNOWN", PERSON_STATE_SIZE);
        }

        if (!words[14].empty()) {
            this->result_->person.datetime = std::stoul(words[14]);
        } else {
            this->result_->person.datetime = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}

Q3JoinResultParser::Q3JoinResultParser() {
    this->result_ = new Q3JoinResult();
}

Q3JoinResultParser::~Q3JoinResultParser() {
    delete this->result_;
}

Q4Join1ResultParser::Q4Join1ResultParser() {
    this->result_ = new Q4Join1Result();
}

Q4Join1ResultParser::~Q4Join1ResultParser() {
    delete this->result_;
}

void *Q4Join1ResultParser::parseFromString(const std::string &str) {
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
        if (words.size() != 12) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->result_->auction.id = std::stoul(words[0]);
        } else {
            this->result_->auction.id = 0;
        }

        if (!words[1].empty()) {
            strncpy(this->result_->auction.itemName, words[1].c_str(), AUCTION_ITEM_NAME_SIZE);
        } else {
            strncpy(this->result_->auction.itemName, "UNKNOWN", AUCTION_ITEM_NAME_SIZE);
        }

        if (!words[2].empty()) {
            this->result_->auction.initialBid = std::stoul(words[2]);
        } else {
            this->result_->auction.initialBid = 0;
        }

        if (!words[3].empty()) {
            this->result_->auction.reserve = std::stoul(words[3]);
        } else {
            this->result_->auction.reserve = 0;
        }

        if (!words[4].empty()) {
            this->result_->auction.datetime = std::stoul(words[4]);
        } else {
            this->result_->auction.datetime = 0;
        }

        if (!words[5].empty()) {
            this->result_->auction.expires = std::stoul(words[5]);
        } else {
            this->result_->auction.expires = 0;
        }

        if (!words[6].empty()) {
            this->result_->auction.seller = std::stoul(words[6]);
        } else {
            this->result_->auction.seller = 0;
        }

        if (!words[7].empty()) {
            this->result_->auction.category = std::stoul(words[7]);
        } else {
            this->result_->auction.category = 0;
        }

        if (!words[8].empty()) {
            this->result_->bid.auction = std::stoul(words[8]);
        } else {
            this->result_->bid.auction = 0;
        }

        if (!words[9].empty()) {
            this->result_->bid.bidder = std::stoul(words[9]);
        } else {
            this->result_->bid.bidder = 0;
        }

        if (!words[10].empty()) {
            this->result_->bid.price = std::stoul(words[10]);
        } else {
            this->result_->bid.price = 0;
        }

        if (!words[11].empty()) {
            this->result_->bid.datetime = std::stoul(words[11]);
        } else {
            this->result_->bid.datetime = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}


Q4Map1ResultParser::Q4Map1ResultParser() {
    this->result_ = new Q4Map1Result();
}

Q4Map1ResultParser::~Q4Map1ResultParser() {
    delete this->result_;
}

void *Q4Map1ResultParser::parseFromString(const std::string &str) {
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
            this->result_->category = std::stoul(words[0]);
        } else {
            this->result_->category = 0;
        }

        if (!words[1].empty()) {
            this->result_->final = std::stoul(words[1]);
        } else {
            this->result_->final = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}

Uint64Parser::Uint64Parser() {
    this->result_ = new uint64_t;
}

Uint64Parser::~Uint64Parser() {
    delete this->result_;
}

void *Uint64Parser::parseFromString(const std::string &str) {
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
        if (words.empty()) {
            return nullptr;
        }

        if (!words[0].empty()) {
            *this->result_ = std::stoul(words[0]);
        } else {
            throw std::invalid_argument("Must be an uint64");
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}


Q6JoinResultParser::Q6JoinResultParser() {
    this->result_ = new Q6JoinResult();
}

Q6JoinResultParser::~Q6JoinResultParser() {
    delete this->result_;
}

void *Q6JoinResultParser::parseFromString(const std::string &str) {
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
        if (words.size() != 12) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->result_->auction.id = std::stoul(words[0]);
        } else {
            this->result_->auction.id = 0;
        }

        if (!words[1].empty()) {
            strncpy(this->result_->auction.itemName, words[1].c_str(), AUCTION_ITEM_NAME_SIZE);
        } else {
            strncpy(this->result_->auction.itemName, "UNKNOWN", AUCTION_ITEM_NAME_SIZE);
        }

        if (!words[2].empty()) {
            this->result_->auction.initialBid = std::stoul(words[2]);
        } else {
            this->result_->auction.initialBid = 0;
        }

        if (!words[3].empty()) {
            this->result_->auction.reserve = std::stoul(words[3]);
        } else {
            this->result_->auction.reserve = 0;
        }

        if (!words[4].empty()) {
            this->result_->auction.datetime = std::stoul(words[4]);
        } else {
            this->result_->auction.datetime = 0;
        }

        if (!words[5].empty()) {
            this->result_->auction.expires = std::stoul(words[5]);
        } else {
            this->result_->auction.expires = 0;
        }

        if (!words[6].empty()) {
            this->result_->auction.seller = std::stoul(words[6]);
        } else {
            this->result_->auction.seller = 0;
        }

        if (!words[7].empty()) {
            this->result_->auction.category = std::stoul(words[7]);
        } else {
            this->result_->auction.category = 0;
        }

        if (!words[8].empty()) {
            this->result_->bid.auction = std::stoul(words[8]);
        } else {
            this->result_->bid.auction = 0;
        }

        if (!words[9].empty()) {
            this->result_->bid.bidder = std::stoul(words[9]);
        } else {
            this->result_->bid.bidder = 0;
        }

        if (!words[10].empty()) {
            this->result_->bid.price = std::stoul(words[10]);
        } else {
            this->result_->bid.price = 0;
        }

        if (!words[11].empty()) {
            this->result_->bid.datetime = std::stoul(words[11]);
        } else {
            this->result_->bid.datetime = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}

Q5CountTotalResultParser::Q5CountTotalResultParser() {
    this->result_ = new Q5CountTotalResult;
}

Q5CountTotalResultParser::~Q5CountTotalResultParser() {
    delete this->result_;
}

void *Q5CountTotalResultParser::parseFromString(const std::string &str) {
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
            this->result_->datetime = std::stoul(words[0]);
        } else {
            this->result_->datetime = 0;
        }

        if (!words[1].empty()) {
            this->result_->count = std::stoul(words[1]);
        } else {
            this->result_->count = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}


Q5CountByAuctionResultParser::Q5CountByAuctionResultParser() {
    this->result_ = new Q5CountByAuctionResult;
}

Q5CountByAuctionResultParser::~Q5CountByAuctionResultParser() {
    delete this->result_;
}

void *Q5CountByAuctionResultParser::parseFromString(const std::string &str) {
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
        if (words.size() != 3) {
            return nullptr;
        }

        if (!words[0].empty()) {
            this->result_->datetime = std::stoul(words[0]);
        } else {
            this->result_->datetime = 0;
        }

        if (!words[1].empty()) {
            this->result_->auction = std::stoul(words[1]);
        } else {
            this->result_->auction = 0;
        }

        if (!words[2].empty()) {
            this->result_->count = std::stoul(words[2]);
        } else {
            this->result_->count = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

    return this->result_;
}


Q6MaxResultParser::Q6MaxResultParser() {
    this->result_ = new Q6MaxResult;
}

Q6MaxResultParser::~Q6MaxResultParser() {
    delete this->result_;
}

void *Q6MaxResultParser::parseFromString(const std::string &str) {
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
            this->result_->final = std::stoul(words[0]);
        } else {
            this->result_->final = 0;
        }

        if (!words[1].empty()) {
            this->result_->seller = std::stoul(words[1]);
        } else {
            this->result_->seller = 0;
        }
    }
    catch (const std::invalid_argument &) {
        return nullptr;
    }

//    if (this->count_ < 10000) {
//        result_->seller = 1000;
//        this->count_++;
//    } else {
//        int i = (rand()%70)+1;
////    std::cout << i << std::endl;
//        result_->seller = 1000 + i;
//    }

    return this->result_;
}




