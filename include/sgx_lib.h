//
// Created by hungpm on 25/03/2024.
//

#ifndef SGX_LIB_H
#define SGX_LIB_H

#include "sgx_error.h"
#include "sgx_eid.h"     /* sgx_enclave_id_t */

#define MAX_PATH FILENAME_MAX
#define TOKEN_FILENAME   "enclave.token"
#define ENCLAVE_FILENAME "enclave.signed.so"

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

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
void print_error_message(sgx_status_t ret);
int initialize_enclave(sgx_enclave_id_t* enclaveID);

#endif //SGX_LIB_H
