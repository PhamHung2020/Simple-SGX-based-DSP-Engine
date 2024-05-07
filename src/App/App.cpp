#include <cstdio>
#include <sgx_defs.h>

#include "Enclave_u.h"

#include "App/test.h"

using namespace std;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
//     testSimpleEngine();
//    testHotCallPerformance();
//     testObserverPerformance();
//    testJoin2Stream1Enclave();
//    testCpuAffinity();

    for (int i = 0; i < 10; ++i) {
        printf("Test %d.\n", i);
        testObserverPerformance(i);
    }

    return 0;
}

