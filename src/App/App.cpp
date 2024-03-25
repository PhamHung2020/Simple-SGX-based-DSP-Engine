#include <cstdio>
#include <fstream>
#include <unistd.h>
#include "sgx_urts.h"

#include "Enclave_u.h"
#include "sgx_lib.h"
#include "Source/CsvSource.h"
#include "fast_call.h"
#include "data_types.h"

using namespace std;

constexpr uint16_t requestedCallID = 0;
constexpr int bufferSize = 128;
auto bufferECall = new MyEvent[128];
circular_buffer circular_buffer_ecall = 
{ 
    .buffer = bufferECall,
    .head = 0, 
    .tail = 0,
    .maxlen = bufferSize,
    .data_size = sizeof(MyEvent)
};

FastCallStruct fastECallData = 
{ 
    .responderThread = 0, 
    .data_buffer = &circular_buffer_ecall, 
    .keepPolling = true 
};

MyEvent globalEvent;

void sinkResult(void* rawData)
{
    // HotOCallParams* hotOCallParams = (HotOCallParams*) rawData;
    auto* event = static_cast<MyEvent *>(rawData);
    printf(
        "Sink Result: (%lf %d %d %d %s)\n", 
        event->timestamp, event->sourceId, event->key, event->data, event->message
    );
}

void* EnclaveResponderThread(void* fastCallPairAsVoidP)
{
    const auto* fastCallPair = static_cast<FastCallPair *>(fastCallPairAsVoidP);
    FastCallStruct *fastEcall = fastCallPair->fastECall;
    FastCallStruct *fastOcall = fastCallPair->fastOCall;
    const sgx_status_t status = EcallStartResponder(fastCallPair->enclaveId, fastEcall, fastOcall, fastCallPair->callId);
    if (status == SGX_SUCCESS)
    {
        printf("Polling success\n");
    }
    else
    {
        printf("Polling failed\n");
        print_error_message(status);
    }

    return nullptr;
}

void* UntrsutedResponserThread(void* fastOCallAsVoidP)
{
    void (*callbacks[1])(void*);
    callbacks[0] = sinkResult;

    FastCallTable callTable;
    callTable.numEntries = 1;
    callTable.callbacks  = callbacks;

    FastCall_wait(static_cast<FastCallStruct *>(fastOCallAsVoidP), &callTable, 0);
    return nullptr;
}

void sendToEngine(const MyEvent &event)
{
    globalEvent = event;
    FastCall_request(&fastECallData, &globalEvent);
}

void* startSource(void* sourceAsVoid)
{
    const auto source = static_cast<Source *>(sourceAsVoid);
    source->start(sendToEngine);
    return nullptr;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    /* Initialize the enclave */
    sgx_enclave_id_t filterEnclaveId;
    sgx_enclave_id_t mapEnclaveId;

    if (initialize_enclave(&filterEnclaveId) < 0)
    {
        printf("Initialize filter enclave failed...\n");
        getchar();
        return -1;
    }

    if (initialize_enclave(&mapEnclaveId) < 0)
    {
        printf("Initialize map enclave failed...\n");
        getchar();
        return -1;
    }
    

    /* ========================= PREPARE & START RESPONDERS =====================*/

    const auto bufferECall2 = new MyEvent[128];
    circular_buffer circular_buffer_ecall2 =
    {
        .buffer = bufferECall2,
        .head = 0,
        .tail = 0,
        .maxlen = bufferSize,
        .data_size = sizeof(MyEvent)
    };

    FastCallStruct fastECallData2 =
    {
        .responderThread = 0,
        .data_buffer = &circular_buffer_ecall2,
        .keepPolling = true
    };

    const auto bufferOCall = new MyEvent[128];
    circular_buffer circular_buffer_ocall = 
    { 
        .buffer = bufferOCall,
        .head = 0,
        .tail = 0,
        .maxlen = bufferSize,
        .data_size = sizeof(MyEvent)
    };

    FastCallStruct fastOCallData = 
    { 
        .responderThread = 0,
        .data_buffer = &circular_buffer_ocall,
        .keepPolling = true 
    };

    FastCallPair fastCallPair = 
    { 
        .enclaveId = filterEnclaveId,
        .fastECall = &fastECallData,
        .fastOCall = &fastECallData2,
        .callId = 0
    };
    pthread_create(&fastECallData.responderThread, nullptr, EnclaveResponderThread, (void*)&fastCallPair);

    FastCallPair fastCallPair2 =
    {
        .enclaveId = mapEnclaveId,
        .fastECall = &fastECallData2,
        .fastOCall = &fastOCallData,
        .callId = 1
    };
    pthread_create(&fastECallData2.responderThread, nullptr, EnclaveResponderThread, (void*)&fastCallPair2);

    pthread_create(&fastOCallData.responderThread, nullptr, UntrsutedResponserThread, (void*)&fastOCallData);


    /* =================== DECLARE AND START SOURCES ====================*/
    CsvSource source1(1, "../../test_data.csv", 0);

    pthread_t sourceThread1;
    pthread_create(&sourceThread1, nullptr, startSource, (void*) &source1);

    /* ================== WAIT FOR SOURCES ===================*/
    printf("Start sending events...\n");

    pthread_join(sourceThread1, nullptr);

    printf("Stopped source\n");

    /* ================== STOP RESPONDERS =================*/
    sleep(5);
    StopResponder(&fastECallData);
    pthread_join(fastECallData.responderThread, nullptr);

    StopResponder(&fastECallData2);
    pthread_join(fastECallData2.responderThread, nullptr);

    StopResponder(&fastOCallData);
    pthread_join(fastOCallData.responderThread, nullptr);

    /* ================== DESTROY ENCLAVE =================*/
    sgx_destroy_enclave(filterEnclaveId);
    sgx_destroy_enclave(mapEnclaveId);

    delete[] bufferECall;
    delete[] bufferOCall;
    
    printf("Info: SampleEnclave successfully returned.\n");

    return 0;
}

