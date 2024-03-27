#include "Source/CsvSource.h"
#include <cstring>
#include <fstream>
#include <iostream>

CsvSource::CsvSource(const int sourceId, const std::string &filename, const bool hasHeader=false, const uint64_t count=-1) : Source(sourceId, filename) {
    this->hasHeader_ = hasHeader;
    this->count_ = count;
}

CsvSource::CsvSource(const int sourceId, const std::string &filename, const int delay, const bool hasHeader=false, const uint64_t count=-1) : Source(sourceId, filename, delay) {
    this->hasHeader_ = hasHeader;
    this->count_ = count;
}

int CsvSource::start()
{
    running = true;

    std::ifstream fin;
    fin.open(this->sourceUri, std::ios::in);
    if (fin.fail())
    {
        // Get the error code
        std::ios_base::iostate state = fin.rdstate();

        // Check for specific error bits
        if (state & std::ios_base::eofbit)
        {
            std::cout << "End of file reached." << std::endl;
        }
        if (state & std::ios_base::failbit)
        {
            std::cout << "Non-fatal I/O error occurred." << std::endl;
        }
        if (state & std::ios_base::badbit)
        {
            std::cout << "Fatal I/O error occurred." << std::endl;
        }

        return -1;
    }

    std::string line;
    uint64_t lineCount = 0;
    if (this->count_ == 0) {
        this->running = false;
        return 0;
    }

    while (getline(fin, line))
    {
        if (this->count_ > 0 && lineCount == this->count_)
            break;

        if (lineCount == 0 && this->hasHeader_) {
            lineCount++;
            continue;
        }

        lineCount++;
        std::cout << line << std::endl;
        
        if (delay != 0)
        {
            sleep(this->getDelay());
        }
    }

    running = false;
    return 0;
}

int CsvSource::start(Emitter &emitter)
{
    running = true;

    std::ifstream fin;
    fin.open(this->sourceUri, std::ios::in);
    if (fin.fail())
    {
        // Get the error code
        const std::ios_base::iostate state = fin.rdstate();

        // Check for specific error bits
        if (state & std::ios_base::eofbit)
        {
            std::cout << "End of file reached." << std::endl;
        }
        if (state & std::ios_base::failbit)
        {
            std::cout << "Non-fatal I/O error occurred." << std::endl;
        }
        if (state & std::ios_base::badbit)
        {
            std::cout << "Fatal I/O error occurred." << std::endl;
        }

        return -1;
    }

    std::string line;
    if (this->count_ == 0) {
        this->running = false;
        return 0;
    }

    uint64_t lineCount = 0;
    while (getline(fin, line))
    {
        if (this->count_ > 0 && lineCount == this->count_)
            break;

        if (lineCount == 0 && this->hasHeader_) {
            lineCount++;
            continue;
        }

        lineCount++;

        char content[200];
        strncpy(content, line.c_str(), 200);
        emitter.emit(content);

        if (delay != 0)
        {
            sleep(this->getDelay());
        }
    }

    running = false;
    return 0;
}