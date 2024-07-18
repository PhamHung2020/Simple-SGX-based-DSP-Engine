//
// Created by hungpm on 16/07/2024.
//

#include "Nexmark/NexmarkQuery.h"

#include <utility>
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

    runEngineWithBufferObserverCrypto(config);
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

    runEngineWithBufferObserverCrypto(config);
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

    runEngineWithBufferObserverCrypto(config);
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

    runEngineWithBufferObserverCrypto(config);
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

    runEngineWithBufferObserverCrypto(config);
}
