#include <sgx_defs.h>
#include <iostream>
#include "Enclave_u.h"

#include "App/CommandLineArgument.h"
#include "App/utils.h"
#include "utils.h"
#include "Nexmark/schemas.h"
#include "Nexmark/NexmarkQuery.h"
#include "SecureSGX/SecureSgxQuery.h"
#include "StreamBox/StreamBoxQuery.h"

#include <ostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

std::vector<uint64_t> recordedStartTime;
std::vector<uint64_t> recordedEndTime;

inline __attribute__((always_inline))  uint64_t rdtscp()
{
    unsigned int low, high;

    asm volatile("rdtscp" : "=a" (low), "=d" (high));

    return low | ((uint64_t)high) << 32;
}

void oCallRecordStartTime() {
    recordedStartTime.push_back(rdtscp());
}

void oCallRecordEndTime() {
    recordedEndTime.push_back(rdtscp());
}

bool dirExists(const std::string& path) {
    struct stat info{};
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

void testSecureStreamBenchmark(const CommandLineArgument& cmdArg) {
    if (!dirExists(cmdArg.getMeasurementDirName())) {
        std::cout << "Measurement directory does not exist: " << cmdArg.getMeasurementDirName() << std::endl;
        exit(1);
    }
    if (!dirExists(cmdArg.getResultDirName())) {
        std::cout << "Result directory does not exist: " << cmdArg.getResultDirName() << std::endl;
        exit(1);
    }
    SecureSgxQuery secureSgxQuery;
    secureSgxQuery.setMeasurementDirName(cmdArg.getMeasurementDirName());
    secureSgxQuery.setResultDirName(cmdArg.getResultDirName());
    const std::string& q = cmdArg.getQueryName();
    for (uint32_t i = 1; i <= cmdArg.getNumberOfRuns(); ++i) {
        std::string measurementFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        std::string sinkFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        if (q == "MapQuery") {
            secureSgxQuery.runMapQuery(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "FilterQuery") {
            secureSgxQuery.runFilterQuery(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "ReduceQuery") {
            secureSgxQuery.runReduceQuery(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else {
            std::cout << "Unknown SecureStream query: " << q << std::endl;
        }
    }
}

void testNexmarkBenchmark(const CommandLineArgument& cmdArg) {
    if (!dirExists(cmdArg.getMeasurementDirName())) {
        std::cout << "Measurement directory does not exist: " << cmdArg.getMeasurementDirName() << std::endl;
        exit(1);
    }
    if (!dirExists(cmdArg.getResultDirName())) {
        std::cout << "Result directory does not exist: " << cmdArg.getResultDirName() << std::endl;
        exit(1);
    }
    NexmarkQuery nexmarkQuery;
    nexmarkQuery.setMeasurementDirName(cmdArg.getMeasurementDirName());
    nexmarkQuery.setResultDirName(cmdArg.getResultDirName());
    const std::string& q = cmdArg.getQueryName();
    for (uint32_t i = 1; i <= cmdArg.getNumberOfRuns(); ++i) {
        std::string measurementFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        std::string sinkFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        if (q == "Query1") {
            nexmarkQuery.runQuery1(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query2_Filter") {
            nexmarkQuery.runQuery2_Filter(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query2_Map") {
            nexmarkQuery.runQuery2_Map(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query3_FilterPerson") {
            nexmarkQuery.runQuery3_FilterPerson(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query3_FilterAuction") {
            nexmarkQuery.runQuery3_FilterAuction(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query3_JoinPersonAuction") {
            nexmarkQuery.runQuery3_JoinPersonAuction(cmdArg.getSourceFilePath(), cmdArg.getSourceFilePath2(), measurementFile, sinkFile);
        } else if (q == "Query3_MapJoinResult") {
            nexmarkQuery.runQuery3_MapJoinResult(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query4_JoinAuctionBid") {
            nexmarkQuery.runQuery4_JoinAuctionBid(cmdArg.getSourceFilePath(), cmdArg.getSourceFilePath2(), measurementFile, sinkFile);
        } else if (q == "Query4_MapAuctionBid") {
            nexmarkQuery.runQuery4_MapAuctionBid(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query4_Max") {
            nexmarkQuery.runQuery4_Max(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query4_JoinCategory") {
            nexmarkQuery.runQuery4_JoinCategory(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query4_Average") {
            nexmarkQuery.runQuery4_Average(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query5_CountByAuction") {
            nexmarkQuery.runQuery5_CountByAuction(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query5_MaxBatch") {
            nexmarkQuery.runQuery5_MaxBatch(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query6_JoinAuctionBid") {
            nexmarkQuery.runQuery6_JoinAuctionBid(cmdArg.getSourceFilePath(), cmdArg.getSourceFilePath2(), measurementFile, sinkFile);
        } else if (q == "Query6_Filter") {
            nexmarkQuery.runQuery6_Filter(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query6_Max") {
            nexmarkQuery.runQuery6_Max(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query6_Avg") {
            nexmarkQuery.runQuery6_Avg(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query7_MaxJoin") {
            nexmarkQuery.runQuery7_MaxJoin(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query8_JoinPersonAuction") {
            nexmarkQuery.runQuery8_JoinPersonAuction(cmdArg.getSourceFilePath(), cmdArg.getSourceFilePath2(), measurementFile, sinkFile);
        } else if (q == "Query8_MapJoinResult") {
            nexmarkQuery.runQuery8_MapJoinResult(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else {
            std::cout << "Unknown Nexmark query: " << q << std::endl;
        }
    }
}

void testStreamBoxBenchmark(const CommandLineArgument& cmdArg) {
    if (!dirExists(cmdArg.getMeasurementDirName())) {
        std::cout << "Measurement directory does not exist: " << cmdArg.getMeasurementDirName() << std::endl;
        exit(1);
    }
    if (!dirExists(cmdArg.getResultDirName())) {
        std::cout << "Result directory does not exist: " << cmdArg.getResultDirName() << std::endl;
        exit(1);
    }
    StreamBoxQuery query;
    query.setMeasurementDirName(cmdArg.getMeasurementDirName());
    query.setResultDirName(cmdArg.getResultDirName());
    const std::string& q = cmdArg.getQueryName();
    for (uint32_t i = 1; i <= cmdArg.getNumberOfRuns(); ++i) {
        std::string measurementFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        std::string sinkFile = cmdArg.getBenchmarkName() + "_" + q + "_" + std::to_string(i) + ".csv";
        if (q == "Query1") {
            query.runQuery1(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query2") {
            query.runQuery2(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query3") {
            query.runQuery3(cmdArg.getSourceFilePath(), cmdArg.getSourceFilePath2(), measurementFile, sinkFile);
        } else if (q == "Query4") {
            query.runQuery4(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query5") {
            query.runQuery5(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else if (q == "Query6") {
            query.runQuery6(cmdArg.getSourceFilePath(), measurementFile, sinkFile);
        } else {
            std::cout << "Unknown StreamBox query: " << q << std::endl;
        }
    }
}

void runBenchmark(const CommandLineArgument& cmdArg) {
    std::cout << "Running benchmark: " << cmdArg.getBenchmarkName() << ", query: " << cmdArg.getQueryName() << std::endl;
    if (cmdArg.getBenchmarkName() == "Nexmark") {
        testNexmarkBenchmark(cmdArg);
    } else if (cmdArg.getBenchmarkName() == "SecureStream") {
        testSecureStreamBenchmark(cmdArg);
    } else if (cmdArg.getBenchmarkName() == "StreamBox") {
        testStreamBoxBenchmark(cmdArg);
    } else {
        std::cout << "Unknown benchmark name." << std::endl;
    }
}

// Hardcoded benchmark and query lists
const std::vector<std::string> benchmarks = {"Nexmark", "SecureStream", "StreamBox"};
const std::vector<std::string> nexmarkQueries = {
    "Query1",
    "Query2_Filter",
    "Query2_Map",
    "Query3_FilterPerson",
    "Query3_FilterAuction",
    "Query3_JoinPersonAuction",
    "Query3_MapJoinResult",
    "Query4_JoinAuctionBid",
    "Query4_MapAuctionBid",
    "Query4_Max",
    "Query4_JoinCategory",
    "Query4_Average",
    "Query5_CountByAuction",
    "Query5_MaxBatch",
    "Query6_JoinAuctionBid",
    "Query6_Filter",
    "Query6_Max",
    "Query6_Avg",
    "Query7_MaxJoin",
    "Query8_JoinPersonAuction",
    "Query8_MapJoinResult"
};
const std::vector<std::string> secureStreamQueries = {
    "MapQuery",
    "FilterQuery",
    "ReduceQuery"
};
const std::vector<std::string> streamBoxQueries = {
    "Query1",
    "Query2",
    "Query3",
    "Query4",
    "Query5",
    "Query6"
};

void printHelp() {
    std::cout << "Usage:\n";
    std::cout << "  -h: display all possible commands\n";
    std::cout << "  -l: list all benchmark names\n";
    std::cout << "  -lq <benchmark_name>: list all queries of a benchmark\n";
    std::cout << "  -r <benchmark_name> <query_name> <source> [<source2>] <measurement_dir> <result_dir> <number_of_runs>: run a benchmark query\n";
    std::cout << "    Queries requiring two sources: Nexmark: Query3_JoinPersonAuction, Query4_JoinAuctionBid, Query6_JoinAuctionBid, Query8_JoinPersonAuction; StreamBox: Query3\n";
}

void listBenchmarks() {
    std::cout << "Available benchmarks:\n";
    for (const auto& b : benchmarks) {
        std::cout << "  " << b << std::endl;
    }
}

void listQueries(const std::string& benchmark) {
    std::cout << "Available queries for " << benchmark << ":\n";
    if (benchmark == "Nexmark") {
        for (const auto& q : nexmarkQueries) std::cout << "  " << q << std::endl;
    } else if (benchmark == "SecureStream") {
        for (const auto& q : secureStreamQueries) std::cout << "  " << q << std::endl;
    } else if (benchmark == "StreamBox") {
        for (const auto& q : streamBoxQueries) std::cout << "  " << q << std::endl;
    } else {
        std::cout << "  Unknown benchmark name." << std::endl;
    }
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    if (argc < 2) {
        printHelp();
        return 0;
    }
    std::string cmd = argv[1];
    if (cmd == "-h") {
        printHelp();
        return 0;
    } else if (cmd == "-l") {
        listBenchmarks();
        return 0;
    } else if (cmd == "-lq") {
        if (argc < 3) {
            std::cout << "Please provide a benchmark name." << std::endl;
            return 1;
        }
        listQueries(argv[2]);
        return 0;
    } else if (cmd == "-r") {
        // Queries requiring two sources
        std::vector<std::string> twoSourceQueries = {
            "Query3_JoinPersonAuction", "Query4_JoinAuctionBid", "Query6_JoinAuctionBid", "Query8_JoinPersonAuction", "Query3"
        };
        if (argc < 8) {
            std::cout << "Usage: -r <benchmark_name> <query_name> <source> [<source2>] <measurement_dir> <result_dir> <number_of_runs>" << std::endl;
            return 1;
        }
        std::string queryName = argv[3];
        bool needsTwoSources = std::find(twoSourceQueries.begin(), twoSourceQueries.end(), queryName) != twoSourceQueries.end();
        if (needsTwoSources && argc < 9) {
            std::cout << "Error: Query '" << queryName << "' requires two source files." << std::endl;
            printHelp();
            return 1;
        }
        CommandLineArgument cmdArg(argc, argv);
        runBenchmark(cmdArg);
        return 0;
    } else {
        printHelp();
        return 1;
    }
}
