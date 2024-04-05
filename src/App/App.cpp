#include <cstdio>
#include <sgx_defs.h>

#include "Enclave_u.h"

#include "App/test.h"

using namespace std;

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
     testSimpleEngine();
//    testHotCallPerformance();
    // testObserverPerformance();

    return 0;
}

