#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

# include <unistd.h>
# include <pwd.h>

#include "App.h"
#include "Enclave_u.h"

#include "sgx_urts.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "CsvSource.h"
#include "../include/common.h"
#include "../include/data_types.h"

# define MAX_PATH FILENAME_MAX

sgx_enclave_id_t globalEnclaveID;
std::vector<MyEvent> source;
std::vector<JoinResult> sink;


HotCall hotEcall = HOTCALL_INITIALIZER;
HotCall hotOcall = HOTCALL_INITIALIZER;
const uint16_t requestedCallID = 0;
int nJoinedMatch = 0;

using namespace std;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
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
int initialize_enclave(sgx_enclave_id_t* globalEnclaveID)
{
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;
    
    /* Step 1: try to retrieve the launch token saved by last transaction 
        *         if there is no token, then create a new one.
        */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;
    
    if (home_dir != NULL && 
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
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
    }
    
    if (fp != NULL) {
        /* read the token from saved file */
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            /* if token is invalid, clear the buffer */
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
        }
    }
    
    /* Step 2: call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, globalEnclaveID, NULL);
    if (ret != SGX_SUCCESS) {
        printf("sgx_create_enclave returned 0x%x\n", ret);
        print_error_message(ret);
        if (fp != NULL) fclose(fp);
        return -1;
    }
    
    /* Step 3: save the launch token if it is updated */
    if (updated == FALSE || fp == NULL) {
        /* if the token is not updated, or file handler is invalid, do not perform saving */
        if (fp != NULL) fclose(fp);
        return 0;
    }
    
    /* reopen the file with write capablity */
    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;
    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
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

void test(JoinResult* joinResult)
{
    MyEvent currentEvent1 = joinResult->event1;
    MyEvent currentEvent2 = joinResult->event2;
    printf(
        "(%lf %d %d %d %s) (%lf %d %d %d %s)\n", 
        currentEvent1.timestamp, currentEvent1.sourceId, currentEvent1.key, currentEvent1.data, currentEvent1.message,
        currentEvent2.timestamp, currentEvent2.sourceId, currentEvent2.key, currentEvent2.data, currentEvent2.message
    );
    sink.push_back(*joinResult);
}

void printEvent(MyEvent event)
{
    printf(
        "(%lf %d %d %d %s)\n", 
        event.timestamp, event.sourceId, event.key, event.data, event.message); 
}

void sinkReduceResult(void* rawData)
{
    HotOCallParams* hotOCallParams = (HotOCallParams*) rawData;
    printf("Reduce: %d\n", hotOCallParams->reduceResult);
}

void sinkJoinResult(void* rawData)
{
    HotOCallParams* hotOCallParams = (HotOCallParams*) rawData;
    JoinResult joinResult = hotOCallParams->joinResult;
    MyEvent currentEvent1 = joinResult.event1;
    MyEvent currentEvent2 = joinResult.event2;
    printf(
        "Join: (%lf %d %d %d %s) (%lf %d %d %d %s)\n", 
        currentEvent1.timestamp, currentEvent1.sourceId, currentEvent1.key, currentEvent1.data, currentEvent1.message,
        currentEvent2.timestamp, currentEvent2.sourceId, currentEvent2.key, currentEvent2.data, currentEvent2.message
    );
    nJoinedMatch++;
}

void* EnclaveResponderThread(void* hotEcallAsVoidP)
{
    //To be started in a new thread
    HotCall *hotEcall = (HotCall*)hotEcallAsVoidP;
    sgx_status_t status = EcallStartResponder(globalEnclaveID, hotEcall);
    if (status == SGX_SUCCESS)
    {
        printf("Polling success\n");
    }
    else
    {
        printf("Polling failed\n");
        print_error_message(status);
    }

    return NULL;
}

void* UntrsutedResponserThread(void* hotOcallAsVoidP)
{
    void (*callbacks[2])(void*);
    callbacks[0] = sinkJoinResult;
    callbacks[1] = sinkReduceResult;

    HotCallTable callTable;
    callTable.numEntries = 2;
    callTable.callbacks  = callbacks;

    HotCall_waitForCall((HotCall*)hotOcallAsVoidP, &callTable);

    return NULL;
}

void* EnclaveTransformationThread(void*)
{
    sgx_status_t filterResult = TaskExecutor1(globalEnclaveID);
    if (filterResult == SGX_SUCCESS)
    {
        printf("Transformation Success!\n");
    }
    else
    {
        printf("Transformation Failed!\n");
        print_error_message(filterResult);
    }

    return NULL;
}

void* EnclaveJoinThread(void*)
{
    sgx_status_t joinResult = TaskExecutor2(globalEnclaveID, &hotOcall);
    if (joinResult == SGX_SUCCESS)
    {
        printf("Join Success!\n");
    }
    else
    {
        printf("Join Failed!\n");
        print_error_message(joinResult);
    }

    return NULL;
}

void* EnclaveReduceThread(void*)
{
    sgx_status_t reduceResult = TaskExecutor3(globalEnclaveID, &hotOcall);
    if (reduceResult == SGX_SUCCESS)
    {
        printf("Reduce Success!\n");
    }
    else
    {
        printf("Reduce Failed!\n");
        print_error_message(reduceResult);
    }

    return NULL;
}

void sendToEngine(MyEvent event)
{
    HotCall_requestCall(&hotEcall, requestedCallID, &event);
}


void* startSource(void* sourceAsVoid)
{
    Source* source = (Source*) sourceAsVoid;
    source->start(sendToEngine);
    return NULL;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    /* Initialize the enclave */
    if (initialize_enclave(&globalEnclaveID) < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }

    /* ========================= PREPARE =====================*/
    pthread_create(&hotEcall.responderThread, NULL, EnclaveResponderThread, (void*)&hotEcall);

    OcallParams ocallParams;
    hotOcall.data = &ocallParams;
    pthread_create(&hotOcall.responderThread, NULL, UntrsutedResponserThread, (void*)&hotOcall);

    pthread_t transformationThread;
    pthread_create(&transformationThread, NULL, EnclaveTransformationThread, NULL);

    pthread_t joinThread;
    pthread_create(&joinThread, NULL, EnclaveJoinThread, NULL);

    pthread_t reduceThread;
    pthread_create(&reduceThread, NULL, EnclaveReduceThread, NULL);



    /* =================== DECLARE AND START SOURCES ====================*/
    CsvSource source1(1, "test_data.csv", 1);
    CsvSource source2(2, "test_data2.csv", 1);

    pthread_t sourceThread1, sourceThread2;
    pthread_create(&sourceThread1, NULL, startSource, (void*) &source1);
    pthread_create(&sourceThread2, NULL, startSource, (void*) &source2);

    /* ================== WAIT FOR SOURCES ===================*/
    printf("Start sending events...\n");
    pthread_join(sourceThread1, NULL);
    pthread_join(sourceThread2, NULL);

    /* ================== STOP EXECUTORS =================*/
    printf("Sending STOP event\n");
    MyEvent stopEvent = { 0.0, 0, 0, 0, "Stop" };
    HotCall_requestCall(&hotEcall, requestedCallID, &stopEvent);
    printf("STOP event sent\n");

    StopResponder(&hotEcall);

    pthread_join(hotEcall.responderThread, NULL);
    pthread_join(transformationThread, NULL);
    pthread_join(joinThread, NULL);
    pthread_join(reduceThread, NULL);

    StopResponder(&hotOcall);
    pthread_join(hotOcall.responderThread, NULL);

    printf("Joined data: %d item(s)\n", nJoinedMatch);

    /* Destroy the enclave */
    sgx_destroy_enclave(globalEnclaveID);
    
    printf("Info: SampleEnclave successfully returned.\n");

    return 0;
}

