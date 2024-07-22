//
// Created by hungpm on 07/06/2024.
//
#include <iostream>
#include "Nexmark/sinks.h"

std::ofstream nexmarkSinkFileStream;

std::ofstream* getSinkFileStream() {
    return &nexmarkSinkFileStream;
}

void sinkBid(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto bid = static_cast<Bid*>(rawData);
    nexmarkSinkFileStream << bid->auction << "," << bid->bidder << "," << bid->price << "," << bid->datetime << std::endl;
}

void sinkPerson(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto person = static_cast<Person*>(rawData);
    nexmarkSinkFileStream
        << person->id << ","
        << person->name << ","
        << person->emailAddress << ","
        << person->creditCard << ","
        << person->city << ","
        << person->state << ","
        << person->datetime << std::endl;
}

void sinkAuction(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto auction = static_cast<Auction*>(rawData);
    nexmarkSinkFileStream
            << auction->id << ","
            << auction->itemName << ","
            << auction->initialBid << ","
            << auction->reserve << ","
            << auction->datetime << ","
            << auction->expires << ","
            << auction->seller << ","
            << auction->category << std::endl;
}

void sinkQ2Result(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto bid = static_cast<Q2Result*>(rawData);
    nexmarkSinkFileStream << bid->auction << "," << bid->price << std::endl;
}

void sinkQ3JoinResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q3JoinResult*>(rawData);
    nexmarkSinkFileStream
    << result->auction.id << ","
    << result->auction.itemName << ","
    << result->auction.initialBid << ","
    << result->auction.reserve << ","
    << result->auction.datetime << ","
    << result->auction.expires << ","
    << result->auction.seller << ","
    << result->auction.category << ","
    << result->person.id << ","
    << result->person.name << ","
    << result->person.emailAddress << ","
    << result->person.creditCard << ","
    << result->person.city << ","
    << result->person.state << ","
    << result->person.datetime << std::endl;
}

void sinkQ3Result(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q3Result*>(rawData);
    nexmarkSinkFileStream << result->auctionId << "," << result->name << "," << result->city << "," << result->state << std::endl;
}

void sinkQ4Join1Result(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q4Join1Result*>(rawData);
    nexmarkSinkFileStream
            << result->auction.id << ","
            << result->auction.itemName << ","
            << result->auction.initialBid << ","
            << result->auction.reserve << ","
            << result->auction.datetime << ","
            << result->auction.expires << ","
            << result->auction.seller << ","
            << result->auction.category << ","
            << result->bid.auction << ","
            << result->bid.bidder << ","
            << result->bid.price << ","
            << result->bid.datetime << std::endl;
}

void sinkQ4MapResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q4Map1Result*>(rawData);
    nexmarkSinkFileStream << result->category << "," << result->final << std::endl;
}

void sinkQ5CountTotalResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q5CountTotalResult*>(rawData);
    nexmarkSinkFileStream << result->datetime << "," << result->count << std::endl;
}

void sinkQ5CountByAuctionResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q5CountByAuctionResult*>(rawData);
    nexmarkSinkFileStream << result->datetime << "," << result->auction << "," << result->count << std::endl;
}

void sinkQAJoinPersonBidResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<QAJoinPersonBidResult*>(rawData);
    nexmarkSinkFileStream
            << result->person.id << ","
            << result->person.name << ","
            << result->person.emailAddress << ","
            << result->person.creditCard << ","
            << result->person.city << ","
            << result->person.state << ","
            << result->person.datetime << ","
            << result->bid.auction << ","
            << result->bid.bidder << ","
            << result->bid.price << ","
            << result->bid.datetime << std::endl;
}

void sinkQ6JoinResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q6JoinResult*>(rawData);
    nexmarkSinkFileStream
            << result->auction.id << ","
            << result->auction.itemName << ","
            << result->auction.initialBid << ","
            << result->auction.reserve << ","
            << result->auction.datetime << ","
            << result->auction.expires << ","
            << result->auction.seller << ","
            << result->auction.category << ","
            << result->bid.auction << ","
            << result->bid.bidder << ","
            << result->bid.price << ","
            << result->bid.datetime << std::endl;
}

void sinkQ6MaxResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q6MaxResult*>(rawData);
    nexmarkSinkFileStream << result->final << "," << result->seller << std::endl;
}

void sinkQ7MaxResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<uint64_t *>(rawData);
    nexmarkSinkFileStream << *result << std::endl;
}

void sinkQ8JoinResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q8JoinResult*>(rawData);
    nexmarkSinkFileStream
            << result->auction.id << ","
            << result->auction.itemName << ","
            << result->auction.initialBid << ","
            << result->auction.reserve << ","
            << result->auction.datetime << ","
            << result->auction.expires << ","
            << result->auction.seller << ","
            << result->auction.category << ","
            << result->person.id << ","
            << result->person.name << ","
            << result->person.emailAddress << ","
            << result->person.creditCard << ","
            << result->person.city << ","
            << result->person.state << ","
            << result->person.datetime << std::endl;
}

void sinkQ8MapResult(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<Q8MapResult*>(rawData);
    nexmarkSinkFileStream
            << result->personId << ","
            << result->personName << ","
            << result->auctionReserve << std::endl;
}

void sinkChar(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<char *>(rawData);
    nexmarkSinkFileStream << result << std::endl;
}

void sinkUint64(void* rawData) {
    if (rawData == NULL) {
        return;
    }

    const auto result = static_cast<uint64_t *>(rawData);
    nexmarkSinkFileStream << *result << std::endl;
}