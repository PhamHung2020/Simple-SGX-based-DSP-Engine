#ifndef __CSV_SOURCE_
#define __CSV_SOURCE_

#include "Source/Source.h"

class CsvSource : Source
{
public:
    CsvSource(int sourceId, std::string filename);
    CsvSource(int sourceId, std::string filename, int delay);
    int start();
    int start(void (*send)(MyEvent));
};

#endif