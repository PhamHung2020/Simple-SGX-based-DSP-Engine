//
// Created by hungpm on 25/03/2024.
//
#include "Enclave_u.h"
#include <cstdio>
#include "data_types.h"

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
}
