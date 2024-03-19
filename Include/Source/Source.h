#ifndef __SOURCE_
#define __SOURCE_

#include<cstdlib>
#include <string>
#include <unistd.h>

#include "../../Include/data_types.h"

class Source
{
protected:
    std::string sourceUri;
    int delay;
    bool running;
    int sourceId;

public:
    Source(int sourceId, std::string sourceUri);
    Source(int sourceId, std::string sourceUri, int delay);

    int getSourceId();
    std::string getSourceUri();
    virtual int getDelay();
    bool isRunning();

    virtual int start();
    virtual int start(void (*send)(MyEvent));
};

#endif