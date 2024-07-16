//
// Created by hungpm on 16/07/2024.
//

#include "Nexmark/NexmarkQuery.h"

#include <utility>
#include "Nexmark/parsers.h"
#include "Nexmark/sinks.h"
#include "App/utils.h"

void NexmarkQuery::runQuery1(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    config.resultDirName = this->resultDirName_;
    config.measurementDirName = this->measurementDirName_;

    config.sourceId = 1;
    config.sourceDelay = 0;
    config.sourceHasHeader = true;
    config.sourceCount = -1;

    config.sinkFileName = std::move(sinkFileName);
    config.measurementFileName = std::move(measurementFileName);
    config.sourceFileName = std::move(sourceFilePath);
    config.taskId = 8;
    config.taskInputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(Bid) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new BidParser();
    config.sink = sinkBid;
    config.sinkFileStream = getSinkFileStream();

    runEngineWithBufferObserverCrypto(config);
}

void NexmarkQuery::setMeasurementDirName(std::string &measurementDirName) {
    this->measurementDirName_ = measurementDirName;
}

void NexmarkQuery::setResultDirName(std::string &resultDirName) {
    this->resultDirName_ = resultDirName;
}
