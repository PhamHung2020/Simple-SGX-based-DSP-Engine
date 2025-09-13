//
// Created by hungpm on 11/09/2025.
//
#include "App/CommandLineArgument.h"

#include <iostream>

CommandLineArgument::CommandLineArgument(int argc, char *argv[]) {
    // Two-source queries: <benchmark_name> <query_name> <source1> <source2> <measurement_dir_name> <result_dir_name> [number_of_runs]
    // Single-source queries: <benchmark_name> <query_name> <source1> <measurement_dir_name> <result_dir_name> [number_of_runs]
    if (argc < 7) {
        std::cout << "Usage: " << argv[1] << " <benchmark_name> <query_name> <source_file_path> [<source_file_path2>] <measurement_dir_name> <result_dir_name> [number_of_runs]" << std::endl;
        exit(1);
    }
    benchmarkName_ = argv[2];
    queryName_ = argv[3];
    // If two sources, parse accordingly
    if (argc >= 9) {
        sourceFilePath_ = argv[4];
        sourceFilePath2_ = argv[5];
        measurementDirName_ = argv[6];
        resultDirName_ = argv[7];
        numberOfRuns_ = static_cast<uint32_t>(std::stoi(argv[8]));
    } else {
        sourceFilePath_ = argv[4];
        sourceFilePath2_ = "";
        measurementDirName_ = argv[5];
        resultDirName_ = argv[6];
        if (argc >= 8) {
            numberOfRuns_ = static_cast<uint32_t>(std::stoi(argv[7]));
        } else {
            numberOfRuns_ = 1;
        }
    }
}

std::string CommandLineArgument::getBenchmarkName() const {
    return benchmarkName_;
}

std::string CommandLineArgument::getQueryName() const {
    return queryName_;
}

std::string CommandLineArgument::getSourceFilePath() const {
    return sourceFilePath_;
}

std::string CommandLineArgument::getSourceFilePath2() const {
    return sourceFilePath2_;
}

std::string CommandLineArgument::getMeasurementDirName() const {
    return measurementDirName_;
}

std::string CommandLineArgument::getResultDirName() const {
    return resultDirName_;
}


uint32_t CommandLineArgument::getNumberOfRuns() const {
    return numberOfRuns_;
}