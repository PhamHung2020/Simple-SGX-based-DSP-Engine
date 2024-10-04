//
// Created by hungpm on 24/03/2024.
//
#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "Enclave_t.h"  /* print_string */
#include "data_types.h"

void print(const char *fmt, ...)
{
    char buf[BUFSIZ] = {};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}
