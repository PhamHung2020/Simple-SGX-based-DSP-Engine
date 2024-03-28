//
// Created by hungpm on 27/03/2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string createMeasurementsDirectory(const std::string &pathToDir);

void getTimeStamp(char *timestamp, size_t size);

bool isDirectoryExists(const std::string& path);

#endif //UTILS_H
