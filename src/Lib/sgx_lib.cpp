//
// Created by hungpm on 25/03/2024.
//

#include <cstdio>
#include <pwd.h>
#include <cstring>
#include <unistd.h>

#include "sgx_urts.h"
#include "sgx_lib.h"

/* Check error conditions for loading enclave */
void print_error_message(const sgx_status_t ret) {
    size_t idx;
    constexpr size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if (ret == sgx_errlist[idx].err) {
            if (nullptr != sgx_errlist[idx].sug)
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
int initialize_enclave(sgx_enclave_id_t *enclaveID) {
    char token_path[MAX_PATH] = {'\0'};
    sgx_launch_token_t token = {0};
    int updated = 0;

    /* Step 1: try to retrieve the launch token saved by last transaction
        *         if there is no token, then create a new one.
        */
    /* try to get the token saved in $HOME */
    const char *home_dir = getpwuid(getuid())->pw_dir;

    if (home_dir != nullptr &&
        (strlen(home_dir) + strlen("/") + sizeof(TOKEN_FILENAME) + 1) <= MAX_PATH) {
        /* compose the token path */
        strncpy(token_path, home_dir, strlen(home_dir));
        strncat(token_path, "/", strlen("/"));
        strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME) + 1);
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
    printf("line: %d\n", __LINE__);
    return 0;
}
