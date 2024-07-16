//
// Created by hungpm on 03/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_CRYPTOGRAPHY_H
#define SIMPLE_DSP_ENGINE_CRYPTOGRAPHY_H

#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BUFLEN 2048

void aesGcmEncrypt(char *decMessageIn, size_t len, char *encMessageOut, size_t lenOut);
void aesGcmDecrypt(char *encMessageIn, size_t len, char *decMessageOut, size_t lenOut);

#endif //SIMPLE_DSP_ENGINE_CRYPTOGRAPHY_H
