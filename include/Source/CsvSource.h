#ifndef CSV_SOURCE_
#define CSV_SOURCE_

#include "Source/Source.h"

class CsvSource final : public Source
{
protected:
    bool hasHeader_;
    uint64_t count_;
public:
    CsvSource(int sourceId, const std::string &filename, bool hasHeader, uint64_t count);
    CsvSource(int sourceId, const std::string &filename, int delay, bool hasHeader, uint64_t count);
    int start() override;
    int start(Emitter &emitter) override;
};

#endif