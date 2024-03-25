#include <cstdio>
#include <fstream>
#include <sgx_defs.h>

#include "Enclave_u.h"
#include "Source/CsvSource.h"
#include "data_types.h"

#include "Engine/SimpleEngine.h"

using namespace std;

void sinkResult(void* rawData)
{
    auto* event = static_cast<MyEvent *>(rawData);
    printf(
        "Sink Result: (%lf %d %d %d %s)\n",
        event->timestamp, event->sourceId, event->key, event->data, event->message
    );
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{

    /* =================== DECLARE AND START SOURCES ====================*/
    CsvSource source1(1, "../../test_data.csv", 0);
    SimpleEngine engine;
    engine.setSource(source1);
    engine.setSink(sinkResult);
    engine.addOperator(0);
    engine.addOperator(1);
    engine.start();
    
    printf("Info: Engine successfully returned.\n");

    return 0;
}

