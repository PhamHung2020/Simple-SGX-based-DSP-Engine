//
// Created by hungpm on 25/03/2024.
//

#ifndef ENCLAVE_H
#define ENCLAVE_H

#include "fast_call.h"

#if defined(__cplusplus)
extern "C" {
#endif

    extern FastCallStruct* globalFastOCall;
    extern circular_buffer* fastOCallBuffer;

#if defined(__cplusplus)
}
#endif

#endif //ENCLAVE_H
