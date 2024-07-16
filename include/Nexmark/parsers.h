//
// Created by hungpm on 06/06/2024.
//

#ifndef SIMPLE_DSP_ENGINE_NEXMARK_PARSERS_H
#define SIMPLE_DSP_ENGINE_NEXMARK_PARSERS_H

#include "schemas.h"
#include "../Source/Parser.h"

class PersonParser : public Parser {
private:
    Person* person_ = nullptr;
public:
    PersonParser();
    ~PersonParser();
    void *parseFromString(const std::string &str) override;
};

class AuctionParser : public Parser {
private:
    Auction* auction_ = nullptr;
public:
    AuctionParser();
    ~AuctionParser();
    void *parseFromString(const std::string &str) override;
};

class BidParser : public Parser {
private:
    Bid* bid_ = nullptr;
public:
    BidParser();
    ~BidParser();
    void *parseFromString(const std::string &str) override;
};

class Q3JoinResultParser : public Parser {
private:
    Q3JoinResult* result_ = nullptr;
public:
    Q3JoinResultParser();
    ~Q3JoinResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q4Join1ResultParser : public Parser {
private:
    Q4Join1Result* result_ = nullptr;
public:
    Q4Join1ResultParser();
    ~Q4Join1ResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q4Map1ResultParser : public Parser {
private:
    Q4Map1Result* result_ = nullptr;
public:
    Q4Map1ResultParser();
    ~Q4Map1ResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q5CountTotalResultParser : public Parser {
private:
    Q5CountTotalResult* result_ = nullptr;
public:
    Q5CountTotalResultParser();
    ~Q5CountTotalResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q5CountByAuctionResultParser : public Parser {
private:
    Q5CountByAuctionResult* result_ = nullptr;
public:
    Q5CountByAuctionResultParser();
    ~Q5CountByAuctionResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q6JoinResultParser : public Parser {
private:
    Q6JoinResult* result_ = nullptr;
public:
    Q6JoinResultParser();
    ~Q6JoinResultParser();
    void *parseFromString(const std::string &str) override;
};

class Q6MaxResultParser : public Parser {
private:
    Q6MaxResult* result_ = nullptr;
public:
    Q6MaxResultParser();
    ~Q6MaxResultParser();
    void *parseFromString(const std::string &str) override;
};

class Uint64Parser : public Parser {
private:
    uint64_t* result_ = nullptr;
public:
    Uint64Parser();
    ~Uint64Parser();
    void *parseFromString(const std::string &str) override;
};
#endif //SIMPLE_DSP_ENGINE_NEXMARK_PARSERS_H
