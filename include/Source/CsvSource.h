#ifndef CSV_SOURCE_
#define CSV_SOURCE_

#include "Source/Source.h"

class CsvSource final : Source
{
public:
    CsvSource(int sourceId, const std::string &filename);
    CsvSource(int sourceId, const std::string &filename, int delay);
    int start() override;
    int start(void (*emit)(const MyEvent&)) override;
};

#endif