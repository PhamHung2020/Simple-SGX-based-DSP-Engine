#ifndef SOURCE_
#define SOURCE_

#include<cstdlib>
#include <string>
#include <unistd.h>

#include "data_types.h"
#include "Source/Emitter.h"

class Source
{
protected:
    std::string sourceUri;
    int delay{};
    bool running{};
    int sourceId{};

public:
    virtual ~Source() = default;

    Source();
    Source(int sourceId, std::string sourceUri);
    Source(int sourceId, std::string sourceUri, int delay);

    int getSourceId() const;
    std::string getSourceUri();
    virtual int getDelay();
    bool isRunning() const;

    virtual int start();
    virtual int start(Emitter &emitter);
};

#endif