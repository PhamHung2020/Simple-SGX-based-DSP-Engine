//
// Created by hungpm on 06/06/2024.
//
#include "App/test.h"
#include "Source/CsvSource.h"
#include "Engine/Simple2SourceObserverEngine.h"
#include "Source/FastCallEncryptedEmitter.h"
#include "App/utils.h"
#include "Nexmark/schemas.h"
#include "Nexmark/parsers.h"

#include <iostream>
#include <fstream>

namespace TestNexmark2 {
    std::ofstream nexmarkSinkFileStream;

    void writeMeasurementToFile(const std::string& pathToFile, const Simple2SourceObserverEngine::ObservedData& observedData) {
        std::ofstream measurementFile;
        measurementFile.open(pathToFile, std::ios::out);
        std::cout << "Count " << observedData.count << std::endl;

        measurementFile << "index,time\n";

        for (uint64_t i = 0; i < observedData.count; ++i) {
            if (i == 0) {
                measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[0] - observedData.startTimestamp << std::endl;
            } else {
                measurementFile << observedData.noItem[i] << "," << observedData.timestampCounterPoints[i] - observedData.timestampCounterPoints[i-1] << std::endl;
            }
        }

        measurementFile.close();
    }

    void sinkBid(void* rawData) {
        if (rawData == NULL) {
            return;
        }

        const auto bid = static_cast<Bid*>(rawData);
        nexmarkSinkFileStream << bid->auction << "," << bid->bidder << "," << bid->price << "," << bid->datetime << std::endl;
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

    void sinkUint64(void* rawData) {
        if (rawData == NULL) {
            return;
        }

        const auto result = static_cast<uint64_t *>(rawData);
        nexmarkSinkFileStream << *result << std::endl;
    }
}

void testNexmark2(int n) {
//    std::string resultDirName = "../../results/testNexmark/batch/2024-06-09_23-46-53/" + std::to_string(n);
//    std::string measurementDirName = "../../measurements/testNexmark/batch/2024-06-09_23-46-53/" + std::to_string(n);
//    std::string resultDirName = "../../results/testNexmark/batch/2024-06-12_09-58-38/" + std::to_string(n);
//    std::string measurementDirName = "../../measurements/testNexmark/batch/2024-06-12_09-58-38/" + std::to_string(n);
    std::string resultDirName = "../../results/testNexmark/crypto/data/" + std::to_string(n);
    std::string measurementDirName = "../../measurements/testNexmark/crypto/data/" + std::to_string(n);

//    std::string sinkFileName = "q3result_join_3000.csv";
//    std::string measureFileName = "q3_join_3000.csv";

//    std::string sinkFileName = "q4result_join1_1500.csv";
//    std::string measureFileName = "q4_join1_1500.csv";

    std::string sinkFileName = "q6result_join_600.csv";
    std::string measureFileName = "q6_join_600.csv";

//    std::string sinkFileName = "q8result_join_1500.csv";
//    std::string measureFileName = "q8_join_1500.csv";

    // ================ Set up engine ====================
//    CsvSource source1(1, "../../../other/bids.csv", 0, true, 1000000);
//    CsvSource source1(1, resultDirName + "/q3result_filter_person_4_state.csv", 0, true, -1);
//    CsvSource source1(1, "../../../other/auction.csv", 0, true, -1);
//    CsvSource source1(1, "../../../other/people.csv", 0, true, -1);
    CsvSource source1(1, "../../../other/auction.csv", 0, true, -1);

//    PersonParser personParser;
    AuctionParser auctionParser;
    BidParser bidParser;
//    Uint64Parser uint64Parser;
//    Q5CountByAuctionResultParser q5CountByAuctionResultParser;
//    Q5CountTotalResultParser q5CountTotalResultParser;
    source1.setParser(&auctionParser);

//    CsvSource source2(2, "../../../other/bids.csv", 0, true, 1000000);
//    CsvSource source2(2, resultDirName + "/q3result_filter_auction.csv", 0, true, -1);
    CsvSource source2(2, "../../../other/bids.csv", 0, true, 200000);
//    CsvSource source2(2, "../../../other/auction.csv", 0, true, -1);

    source2.setParser(&bidParser);

    Simple2SourceObserverEngine engine;
    engine.setSource1(source1);
    engine.setSource2(source2);

//    FastCallEmitter emitter1, emitter2;
    FastCallEncryptedEmitter emitter1, emitter2;
    engine.setEmitter(emitter1, emitter2);

    engine.setTask(26, sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE, sizeof(Bid)  + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE);
    engine.setSink(TestNexmark2::sinkQ6JoinResult, sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE);

// =================== Create directory and file to store processed results =========================
    std::string fileFullPath;
    if (resultDirName == "../../results/testNexmark") {
        fileFullPath = createMeasurementsDirectory(resultDirName);
    } else {
        fileFullPath = resultDirName;
    }

    fileFullPath.append("/").append(sinkFileName);
    TestNexmark2::nexmarkSinkFileStream.open(fileFullPath, std::ios::out);
    if (TestNexmark2::nexmarkSinkFileStream.fail()) {
        std::cout << "Open out file failed.\n";
        return;
    }

    // ======================= Create measurement folder =======================
    std::string createdMeasurementDirName;
    if (measurementDirName == "../../measurements/testNexmark") {
        createdMeasurementDirName = createMeasurementsDirectory(measurementDirName);
    } else {
        createdMeasurementDirName = measurementDirName;
    }

    // ======================= Start engine ==========================
    engine.start();

    TestNexmark2::nexmarkSinkFileStream.close();

    // ====================== Write measurements ==============================
//    std::string tailFilename = "tail_0_0_process_time_q3_join.csv";
    std::string tailFilename = "tail_0_0_process_time_" + measureFileName;
    std::string tailFileFullPath = createdMeasurementDirName;
    tailFileFullPath.append("/").append(tailFilename);

    std::cout << "Writing measurements for tail buffer 1" << std::endl;
    TestNexmark2::writeMeasurementToFile(tailFileFullPath, engine.tailObserveredData1_);

//    tailFilename = "tail_0_1_process_time_q3_join.csv";
    tailFilename = "tail_0_1_process_time_" + measureFileName;
    tailFileFullPath = createdMeasurementDirName;
    tailFileFullPath.append("/").append(tailFilename);

    std::cout << "Writing measurements for tail buffer 2" << std::endl;
    TestNexmark2::writeMeasurementToFile(tailFileFullPath, engine.tailObserveredData2_);

//    std::string headFilename = "head_1_process_time_q3_join.csv";
    std::string headFilename = "head_1_process_time_" + measureFileName;
    std::string headFileFullPath = createdMeasurementDirName;
    headFileFullPath.append("/").append(headFilename);

    std::cout << "Writing measurements for head buffer " << std::endl;
    TestNexmark2::writeMeasurementToFile(headFileFullPath, engine.headObserveredData_);

    printf("Info: Engine successfully returned.\n");

    engine.clean();
}