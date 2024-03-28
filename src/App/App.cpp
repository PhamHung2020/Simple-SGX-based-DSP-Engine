#include <iostream>
#include <cstdio>
#include <fstream>
#include <sgx_defs.h>

#include "Enclave_u.h"
#include "Source/CsvSource.h"
#include "data_types.h"

#include "Engine/SimpleEngine.h"
#include "Engine/EngineWithBufferObserver.h"
#include "Engine/EngineWithMultipleSources.h"
#include "Source/FastCallPerformanceEmitter.h"
#include "App/test.h"

using namespace std;

// std::chrono::_V2::system_clock::time_point endTimesList[1000];
// int countEndTime = 0;
// endTimesList[countEndTime] = std::chrono::high_resolution_clock::now();
// countEndTime++;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    // if (countEndTime == emitter.getCount()) {
    //     for (int i = 0; i < countEndTime; ++i) {
    //         std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(endTimesList[i] - emitter.getStartTime(i)).count() << "ns\n";
    //     }
    // }
    // printf("Info: Engine successfully returned.\n");

    // testHotCallPerformance();
    testObserverPerformance();

    return 0;
}

