//
// Created by hungpm on 14/02/2025.
//

#include "StreamBox/StreamBoxQuery.h"

#include <utility>
#include <iostream>
#include "StreamBox/parsers.h"
#include "StreamBox/sinks.h"
#include "App/utils.h"

void StreamBoxQuery::setMeasurementDirName(std::string &measurementDirName) {
    this->measurementDirName_ = measurementDirName;
}

void StreamBoxQuery::setResultDirName(std::string &resultDirName) {
    this->resultDirName_ = resultDirName;
}

void StreamBoxQuery::setupConfiguration_(ConfigurationTesting* config, std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
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

void StreamBoxQuery::cleanConfiguration_(ConfigurationTesting *config) {
    delete config->parser;
    delete config->parser1;
    delete config->parser2;
}

void StreamBoxQuery::runQuery4(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 0;
    config.taskInputDataSize = sizeof(SensorData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(SensorAggregationData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new SensorDataParser();
    config.sink = sinkSensorAggregationData;
    config.sinkFileStream = getStreamBoxSinkFileStream();

    std::cout << "Start Query 4 (Sensor Query)\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query 4 (Sensor Query)\n";
    this->cleanConfiguration_(&config);
}

void StreamBoxQuery::runQuery3(std::string sourceFilePath1, std::string sourceFilePath2, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, "", std::move(measurementFileName), std::move(sinkFileName));

    config.sourceFileName1 = std::move(sourceFilePath1);
    config.sourceId1 = 1;
    config.sourceDelay1 = 0;
    config.sourceHasHeader1 = true;
    config.sourceCount1 = -1;
    config.parser1 = new SyntheticDataParser();

    config.sourceFileName2 = std::move(sourceFilePath2);
    config.sourceId2 = 2;
    config.sourceDelay2 = 0;
    config.sourceHasHeader2 = true;
    config.sourceCount2 = -1;
    config.parser2 = new SyntheticDataParser();

    config.taskId = 1;
    config.taskInputDataSize1 = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskInputDataSize2 = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(SyntheticJoinData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.sink = sinkSyntheticJoinData;
    config.sinkFileStream = getStreamBoxSinkFileStream();

    std::cout << "Start Query3 (Temporal Join) Join\n";
    runSimple2SourceObserverEngine(config);
    std::cout << "End Query3 (Temporal Join) Join\n";

    this->cleanConfiguration_(&config);
}

void StreamBoxQuery::runQuery1(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 0;
    config.taskInputDataSize = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new SyntheticDataParser();
    config.sink = sinkSyntheticData;
    config.sinkFileStream = getStreamBoxSinkFileStream();

    std::cout << "Start Query 1 (Top Value per Key)\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query 1 (Top Value per Key)\n";
    this->cleanConfiguration_(&config);
}

void StreamBoxQuery::runQuery5(std::string sourceFilePath, std::string measurementFileName, std::string sinkFileName) {
    ConfigurationTesting config;
    this->setupConfiguration_(&config, std::move(sourceFilePath), std::move(measurementFileName), std::move(sinkFileName));

    config.taskId = 0;
    config.taskInputDataSize = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.taskShouldBeObserved = true;
    config.outputDataSize = sizeof(SyntheticData) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE + 4;
    config.parser = new SyntheticDataParser();
    config.sink = sinkSyntheticData;
    config.sinkFileStream = getStreamBoxSinkFileStream();

    std::cout << "Start Query 5 (Filter)\n";
    runEngineWithBufferObserverCrypto(config);
    std::cout << "End Query 5 (Filter)\n";
    this->cleanConfiguration_(&config);
}
