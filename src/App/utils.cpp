//
// Created by hungpm on 27/03/2024.
//

#include "App/utils.h"
#include <sys/stat.h>

void getTimeStamp(char *timestamp, const size_t size) {
    time_t rawtime;
    time (&rawtime);
    const struct tm *timeinfo = localtime(&rawtime);

    strftime(timestamp, size, "%Y-%m-%d_%H-%M-%S", timeinfo);
}

bool isDirectoryExists(const std::string& path) {
    struct stat st = {0};

    return stat(path.c_str(), &st) != -1;
}


std::string createMeasurementsDirectory(const std::string &pathToDir) {
    char timestamp[100] = {0};
    getTimeStamp(timestamp, 100);
    // printf( "%s\n", timestamp);

    if(!isDirectoryExists(pathToDir)) {
        printf( "Creating directory %s\n", pathToDir.c_str());
        mkdir(pathToDir.c_str(), 0700);
    }

    const auto m_measurementsDir = pathToDir + "/" + timestamp;
    if(!isDirectoryExists(m_measurementsDir)) {
        printf( "Creating directory %s\n", m_measurementsDir.c_str() );
        mkdir(m_measurementsDir.c_str(), 0700);
    }

    return m_measurementsDir;
}
