//
// Created by hungpm on 23/07/2024.
//

#include "SecureSGX/SecureSgxQuery.h"
#include "SecureSGX/sinks.h"
#include "App/utils.h"

#include <iostream>

void SecureSgxQuery::setupConfiguration_(ConfigurationTesting *config, std::string sourceFilePath,
                                         std::string measurementFileName, std::string sinkFileName) {
    if (config == nullptr) {
        return;
    }

    config->resultDirName = this->resultDirName_;
    config->measurementDirName = this->measurementDirName_;

    config->sourceId = 1;
    config->sourceDelay = 0;
    config->sourceHasHeader = false;
    config->sourceCount = -1;

    config->sinkFileName = std::move(sinkFileName);
    config->measurementFileName = std::move(measurementFileName);
    config->sourceFileName = std::move(sourceFilePath);
}

void SecureSgxQuery::cleanConfiguration_(ConfigurationTesting *config) {
    delete config->parser;
    delete config->parser1;
    delete config->parser2;
}

void SecureSgxQuery::setMeasurementDirName(std::string &measurementDirName) {
    this->measurementDirName_ = measurementDirName;
}

void SecureSgxQuery::setResultDirName(std::string &resultDirName) {
    this->resultDirName_ = resultDirName;
}

void SecureSgxQuery::runMapQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.sourceHasHeader = true;
//    config.sourceCount = 500;
    config.taskId = 0;
    config.parser = new FlightFullDataParser();

//    config.taskInputDataSize = 200 + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize = sizeof(FlightFullData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;

    config.taskShouldBeObserved = true;
//    config.outputDataSize = sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.outputDataSize = sizeof(FlightFullData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;

//    config.sink = sinkMap;
    config.sink = sinkFlightFullData;

    config.sinkFileStream = getSecureSgxSinkFileStream();

    std::cout << "Start Map Query\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Map Query\n";
    this->cleanConfiguration_(&config);
}

void SecureSgxQuery::runFilterQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.sourceHasHeader = false;
//    config.sourceCount = 500;
    config.taskId = 1;
    config.parser = new FlightDataIntermediateParser();
    config.parser = new FlightFullDataParser();

//    config.taskInputDataSize = sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize = sizeof(FlightFullData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;

    config.taskShouldBeObserved = true;
//    config.outputDataSize = sizeof(FlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.outputDataSize = sizeof(FlightFullData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;

//    config.sink = sinkMap;
    config.sink = sinkFlightFullData;

    config.sinkFileStream = getSecureSgxSinkFileStream();


    std::cout << "Start Filter Query\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Filter Query\n";
    this->cleanConfiguration_(&config);
}

void SecureSgxQuery::runReduceQuery(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.sourceHasHeader = true;
    config.taskId = 2;
//    config.parser = new FlightDataIntermediateParser();
    config.parser = new FlightFullDataParser();

    config.taskInputDataSize = sizeof(FlightFullData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(ReducedFlightData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkReduce;
    config.sinkFileStream = getSecureSgxSinkFileStream();

    std::cout << "Start Reduce Query\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Reduce Query\n";
    this->cleanConfiguration_(&config);
}
