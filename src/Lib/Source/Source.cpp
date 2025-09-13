#include "Source/Source.h"

#include <utility>
#include <ctime>

Source::Source()
{
    sourceUri = "";
    delay = 0;
    sourceId = 0;
}

Source::Source(const int sourceId, std::string sourceUri) : sourceUri(std::move(sourceUri)), delay(-1), running(false), sourceId(sourceId)
{
    srand(time(nullptr));
}

Source::Source(const int sourceId, std::string sourceUri, const int delay) : sourceUri(std::move(sourceUri)), delay(delay), running(false), sourceId(sourceId)
{
    srand(time(nullptr));
}

int Source::getSourceId() const {
    return this->sourceId;
}

std::string Source::getSourceUri()
{
    return this->sourceUri;
}

int Source::getDelay()
{
    if (delay >= 0)
    {
        return this->delay;
    }

    return rand() % 3;
}

bool Source::isRunning() const {
    return this->running;
}

int Source::start()
{
    return 0;
}

int Source::start(Emitter &emitter)
{
    return 0;
}