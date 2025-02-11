//
// Created by hungpm on 16/07/2024.
//

#include "Nexmark/NexmarkQuery.h"

#include <utility>
#include <iostream>
#include "Nexmark/parsers.h"
#include "Nexmark/sinks.h"
#include "App/utils.h"

void NexmarkQuery::setMeasurementDirName(std::string &measurementDirName) {
    this->measurementDirName_ = measurementDirName;
}

void NexmarkQuery::setResultDirName(std::string &resultDirName) {
    this->resultDirName_ = resultDirName;
}

void NexmarkQuery::setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    if (config == nullptr) {
        return;
    }

    config->resultDirName = this->resultDirName_;
    config->measurementDirName = this->measurementDirName_;

    config->sourceId = 1;
    config->sourceDelay = 0;
    config->sourceHasHeader = true;
    config->sourceCount = -1;

    config->sinkFileName = std::move(sinkFileName);
    config->measurementFileName = std::move(measurementFileName);
    config->sourceFileName = std::move(sourceFilePath);
}

void NexmarkQuery::runQuery1(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 8;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkBid;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query1\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query1\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery2_Filter(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 9;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkBid;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query2 Filter\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query2 Filter\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery2_Map(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 10;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q2Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkQ2Result;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query2 Map\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query2 Map\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery3_FilterPerson(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 11;
    config.taskInputDataSize = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new PersonParser();
    config.sink = sinkPerson;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query3 Filter Person\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query3 Filter Person\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery3_FilterAuction(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 12;
    config.taskInputDataSize = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new AuctionParser();
    config.sink = sinkAuction;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query3 Filter Auction\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query3 Filter Auction\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery3_JoinPersonAuction(std::string sourceFilePath1, std::string sourceFilePath2, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, "", std::move(measurementFileName), std::move(sinkFileName));

    config.sourceFileName1 = std::move(sourceFilePath1);
    config.sourceId1 = 1;
    config.sourceDelay1 = 0;
    config.sourceHasHeader1 = true;
    config.sourceCount1 = -1;
    config.parser1 = new PersonParser();

    config.sourceFileName2 = std::move(sourceFilePath2);
    config.sourceId2 = 2;
    config.sourceDelay2 = 0;
    config.sourceHasHeader2 = true;
    config.sourceCount2 = -1;
    config.parser2 = new AuctionParser();

    config.taskId = 13;
    config.taskInputDataSize1 = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize2 = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkQ3JoinResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query3 Join\n";
    runSimple2SourceObserverEngine(config);
    std::cout << "End Query3 Join\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery3_MapJoinResult(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 14;
    config.taskInputDataSize = sizeof(Q3JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q3Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q3JoinResultParser();
    config.sink = sinkQ3Result;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query3 Map\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query3 Map\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery4_JoinAuctionBid(std::string sourceFilePath1, std::string sourceFilePath2, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, "", std::move(measurementFileName), std::move(sinkFileName));

    config.sourceFileName1 = std::move(sourceFilePath1);
    config.sourceId1 = 1;
    config.sourceDelay1 = 0;
    config.sourceHasHeader1 = true;
    config.sourceCount1 = -1;
    config.parser1 = new AuctionParser();

    config.sourceFileName2 = std::move(sourceFilePath2);
    config.sourceId2 = 2;
    config.sourceDelay2 = 0;
    config.sourceHasHeader2 = true;
    config.sourceCount2 = -1;
    config.parser2 = new BidParser();

    config.taskId = 15;
    config.taskInputDataSize1 = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize2 = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q4Join1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkQ4Join1Result;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query4 Join\n";
    runSimple2SourceObserverEngine(config);
    std::cout << "End Query4 Join\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery4_MapAuctionBid(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 16;
    config.taskInputDataSize = sizeof(Q4Join1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q4Join1ResultParser();
    config.sink = sinkQ4MapResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query4 Map\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query4 Map\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery4_Max(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 17;
    config.taskInputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q4Map1ResultParser();
    config.sink = sinkQ4MapResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query4 Max\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query4 Max\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery4_JoinCategory(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 18;
    config.taskInputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q4Map1ResultParser();
    config.sink = sinkQ4MapResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query4 Join Category\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query4 Join Category\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery4_Average(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 19;
    config.taskInputDataSize = sizeof(Q4Map1Result) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q4AverageResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q4Map1ResultParser();
    config.sink = sinkQ4AverageResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query4 Average\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query4 Average\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery5_CountByAuction(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 20;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkQ5CountByAuctionResult;
    config.sinkFileStream = getSinkFileStream();
//    config.sourceCount = 20000;

    std::cout << "Start Query5 Count\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query5 Count\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery5_MaxBatch(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 21;
    config.taskInputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q5CountByAuctionResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q5CountByAuctionResultParser();
    config.sink = sinkQ5CountByAuctionResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query5 Max Batch\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query5 Max Batch\n";
    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery6_JoinAuctionBid(std::string sourceFilePath1, std::string sourceFilePath2, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, "", std::move(measurementFileName), std::move(sinkFileName));

    config.sourceFileName1 = std::move(sourceFilePath1);
    config.sourceId1 = 1;
    config.sourceDelay1 = 0;
    config.sourceHasHeader1 = true;
    config.sourceCount1 = -1;
    config.parser1 = new AuctionParser();

    config.sourceFileName2 = std::move(sourceFilePath2);
    config.sourceId2 = 2;
    config.sourceDelay2 = 0;
    config.sourceHasHeader2 = true;
    config.sourceCount2 = -1;
    config.parser2 = new BidParser();

    config.taskId = 22;
    config.taskInputDataSize1 = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize2 = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkQ6JoinResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query6 Join\n";
    runSimple2SourceObserverEngine(config);
    std::cout << "End Query6 Join\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery6_Filter(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 23;
    config.taskInputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q6JoinResultParser();
    config.sink = sinkQ6JoinResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query6 Filter\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query6 Filter\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery6_Max(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 24;
    config.taskInputDataSize = sizeof(Q6JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q6MaxResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q6JoinResultParser();
    config.sink = sinkQ6MaxResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query6 Max\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query6 Max\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery6_Avg(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

//    config.taskId = 25;
    config.taskId = 31;

    config.taskInputDataSize = sizeof(Q6MaxResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.taskInputDataSize = sizeof(Q6MaxResult);

    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q6AverageResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
//    config.outputDataSize = sizeof(Q6AverageResult);

    config.parser = new Q6MaxResultParser();
    config.sink = sinkQ6AverageResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query6 Avg\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query6 Avg\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery7_MaxJoin(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 26;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkBid;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query7 MaxJoin\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query7 MaxJoin\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery8_JoinPersonAuction(std::string sourceFilePath1, std::string sourceFilePath2, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, "", std::move(measurementFileName), std::move(sinkFileName));

    config.sourceFileName1 = std::move(sourceFilePath1);
    config.sourceId1 = 1;
    config.sourceDelay1 = 0;
    config.sourceHasHeader1 = true;
    config.sourceCount1 = -1;
    config.parser1 = new PersonParser();

    config.sourceFileName2 = std::move(sourceFilePath2);
    config.sourceId2 = 2;
    config.sourceDelay2 = 0;
    config.sourceHasHeader2 = true;
    config.sourceCount2 = -1;
    config.parser2 = new AuctionParser();

    config.taskId = 27;
    config.taskInputDataSize1 = sizeof(Person) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize2 = sizeof(Auction) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q8JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkQ8JoinResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query8 Join\n";
    runSimple2SourceObserverEngine(config);
    std::cout << "End Query8 Join\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::runQuery8_MapJoinResult(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 28;
    config.taskInputDataSize = sizeof(Q8JoinResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Q8MapResult) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new Q3JoinResultParser();
    config.sink = sinkQ8MapResult;
    config.sinkFileStream = getSinkFileStream();

    std::cout << "Start Query8 Map\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query8 Map\n";

    this->cleanConfiguration_(&config);
}

void NexmarkQuery::cleanConfiguration_(ConfigurationTesting *config) {
    delete config->parser;
    delete config->parser1;
    delete config->parser2;
}

