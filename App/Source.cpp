#include "Source.h"

Source::Source(int sourceId, std::string sourceUri) : sourceId(sourceId), sourceUri(sourceUri), delay(-1), running(false)
{
    srand(time(NULL));
};

Source::Source(int sourceId, std::string sourceUri, int delay) : sourceId(sourceId), sourceUri(sourceUri), delay(delay), running(false)
{
    srand(time(NULL));
};

int Source::getSourceId()
{
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

bool Source::isRunning()
{
    return this->running;
}

int Source::start()
{
    return 0;
}

int Source::start(void (*send)(MyEvent))
{
    MyEvent event = { 0.0, 0, 0, 0, "" };
    send(event);
    return 0;
}