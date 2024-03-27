#include <iostream>
#include <cstdio>
#include <fstream>
#include <sgx_defs.h>

#include "Enclave_u.h"
#include "Source/CsvSource.h"
#include "data_types.h"

#include "Engine/SimpleEngine.h"
#include "Source/FastCallPerformanceEmitter.h"

using namespace std;

// std::chrono::_V2::system_clock::time_point endTimesList[1000];
// int countEndTime = 0;

void sinkResult(void* rawData)
{
    // endTimesList[countEndTime] = std::chrono::high_resolution_clock::now();
    // countEndTime++;

    auto* event = static_cast<MyEvent *>(rawData);
    printf(
        "Sink Result: (%lf %d %d %d %s)\n",
        event->timestamp, event->sourceId, event->key, event->data, event->message
    );
    // auto* data = static_cast<int*> (rawData);
    // printf("Sink result: %d\n", *data);
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{

    /* =================== DECLARE AND START SOURCES ====================*/
    FastCallEmitter emitter;
    CsvSource source1(1, "../../test_data.csv", 0);
    SimpleEngine engine;
    engine.setSource(source1);
    engine.setEmitter(emitter);
    engine.setSink(sinkResult, sizeof(MyEvent));
    // engine.addTask(3, 32);
    engine.addTask(0, sizeof(MyEvent));
    engine.addTask(1, sizeof(MyEvent));
    engine.start();

    const auto hotCallPerformances = engine.getHotCallPerformanceParams();
    for (size_t i = 0; i < hotCallPerformances.size(); ++i) {
        printf("Enclave %lu\n", i);
        std::cout << "Enclave " << i << ":\n";
        for (size_t j = 0; j < hotCallPerformances[i].endTimes.size(); ++j) {
            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(hotCallPerformances[i].endTimes[j] - hotCallPerformances[i].startTimes[j]).count() << "ns\n";
        }
    }

    // if (countEndTime == emitter.getCount()) {
    //     for (int i = 0; i < countEndTime; ++i) {
    //         std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(endTimesList[i] - emitter.getStartTime(i)).count() << "ns\n";
    //     }
    // }
    printf("Info: Engine successfully returned.\n");

    return 0;
}

