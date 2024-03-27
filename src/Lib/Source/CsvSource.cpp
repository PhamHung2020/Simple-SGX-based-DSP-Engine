#include "Source/CsvSource.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

CsvSource::CsvSource(const int sourceId, const std::string &filename) : Source(sourceId, filename) {}

CsvSource::CsvSource(const int sourceId, const std::string &filename, const int delay) : Source(sourceId, filename, delay) {}

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

    std::vector<std::string> words;
    std::string line, word, temp;
    while (getline(fin, line))
    {
        words.clear();
        std::stringstream s(line);

        while (getline(s, word, ','))
        {
            words.push_back(word);
        }

        if (words.size() != 4)
        {
            continue;
        }

        try
        {
            double timestamp = std::stod(words[0]);
            int key = std::stoi(words[1]);
            int data = std::stoi(words[2]);

            std::cout << timestamp << " " << key << " " << data << " " << words[3] << std::endl;
        }
        catch(const std::invalid_argument&)
        {
            std::cout << "Invalid argument" << std::endl;
            continue;
        }
        
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

    std::vector<std::string> words;
    std::string line, word, temp;
    int count = 0;
    while (getline(fin, line))
    {
        if (count == 0) {
            count++;
            continue;
        }

        char content[200];
        strncpy(content, line.c_str(), 200);
        emitter.emit(content);
        count++;
        if (count > 200)
            break;
        
        if (delay != 0)
        {
            sleep(this->getDelay());
        }
    }

    running = false;
    return 0;
}