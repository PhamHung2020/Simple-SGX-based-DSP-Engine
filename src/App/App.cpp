#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <pwd.h>

#include "App.h"
#include "Enclave_u.h"

#include "sgx_urts.h"

#include <fstream>

#include "Source/CsvSource.h"
#include "fast_call.h"
#include "data_types.h"

#define MAX_PATH FILENAME_MAX

sgx_enclave_id_t globalEnclaveID;

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

using namespace std;

typedef struct
{
    sgx_enclave_id_t enclaveId;
    FastCallStruct* fastECall;
    FastCallStruct* fastOCall;
    uint16_t callId;
} FastCallPair;

typedef struct sgx_errlist_t
{
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] =
{
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        nullptr
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        nullptr
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        nullptr
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        nullptr
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        nullptr
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        nullptr
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        nullptr
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        nullptr
    },
};

/* Check error conditions for loading enclave */
void print_error_message(const sgx_status_t ret)
{
    size_t idx;
    constexpr size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(nullptr != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
        printf("Error: Unexpected error occurred (%d).\n", ret);
}


/* Initialize the enclave:
    *   Step 1: try to retrieve the launch token saved by last transaction
    *   Step 2: call sgx_create_enclave to initialize an enclave instance
    *   Step 3: save the launch token if it is updated
    */
int initialize_enclave(sgx_enclave_id_t* enclaveID)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    int updated = 0;
    
    /* Step 1: try to retrieve the launch token saved by last transaction 
        *         if there is no token, then create a new one.
        */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;
    
    if (home_dir != nullptr &&
        (strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
    } else {
        /* if token path is too long or $HOME is NULL */
        strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
    }
    
    FILE *fp = fopen(token_path, "rb");
    if (fp == nullptr && (fp = fopen(token_path, "wb")) == nullptr) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }
    
    if (fp != nullptr) {
        /* read the token from saved file */
        const size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
    
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    const sgx_status_t ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, enclaveID, nullptr);
    if (ret != SGX_SUCCESS) {
        printf("sgx_create_enclave returned 0x%x\n", ret);
        print_error_message(ret);
        if (fp != nullptr) fclose(fp);
        return -1;
    }
    
    /* Step 3: save the launch token if it is updated */
    if (updated == FALSE || fp == nullptr) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != nullptr) fclose(fp);
        return 0;
    }
    
    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == nullptr) return 0;
    const size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t))
        printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
    fclose(fp);
    printf("line: %d\n", __LINE__ );
    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

void sinkResult(void* rawData)
{
    // HotOCallParams* hotOCallParams = (HotOCallParams*) rawData;
    auto* event = static_cast<MyEvent *>(rawData);
    printf(
        "Sink Result: (%lf %d %d %d %s)\n", 
        event->timestamp, event->sourceId, event->key, event->data, event->message
    );
}

void printEvent(MyEvent event)
{
    printf(
        "(%lf %d %d %d %s)\n", 
        event.timestamp, event.sourceId, event.key, event.data, event.message); 
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
    

    // /* ========================= PREPARE & START RESPONDERS =====================*/

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


    // /* =================== DECLARE AND START SOURCES ====================*/
    CsvSource source1(1, "../../test_data.csv", 0);

    pthread_t sourceThread1;
    pthread_create(&sourceThread1, nullptr, startSource, (void*) &source1);

    // /* ================== WAIT FOR SOURCES ===================*/
    printf("Start sending events...\n");

    pthread_join(sourceThread1, nullptr);

    printf("Stopped source\n");

    // /* ================== STOP RESPONDERS =================*/
    sleep(5);
    StopResponder(&fastECallData);
    pthread_join(fastECallData.responderThread, nullptr);

    StopResponder(&fastECallData2);
    pthread_join(fastECallData2.responderThread, nullptr);

    StopResponder(&fastOCallData);
    pthread_join(fastOCallData.responderThread, nullptr);

    // /* ================== DESTROY ENCLAVE =================*/
    sgx_destroy_enclave(filterEnclaveId);
    sgx_destroy_enclave(mapEnclaveId);

    delete[] bufferECall;
    delete[] bufferOCall;
    
    printf("Info: SampleEnclave successfully returned.\n");

    return 0;
}

