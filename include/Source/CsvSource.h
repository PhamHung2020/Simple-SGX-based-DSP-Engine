#ifndef CSV_SOURCE_
#define CSV_SOURCE_

#include "Source/Source.h"
#include "Source/Parser.h"

class CsvSource final : public Source
{
protected:
    bool hasHeader_;
    uint64_t count_;
    Parser* parser_;
public:
    const static int MAX_CHARACTERS_PER_ROW = 200;
    CsvSource(int sourceId, const std::string &filename, bool hasHeader, uint64_t count);
    CsvSource(int sourceId, const std::string &filename, int delay, bool hasHeader, uint64_t count);

    void setParser(Parser* parser);
    int start() override;
    int start(Emitter &emitter) override;
};

#endif