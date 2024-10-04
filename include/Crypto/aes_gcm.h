//
// Created by hungpm on 03/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_AES_GCM_H
#define SIMPLE_DSP_ENGINE_AES_GCM_H

#include <openssl/bio.h>
#include <openssl/evp.h>

#define BUFLEN 2048
#define SGX_AESGCM_MAC_SIZE 16
#define SGX_AESGCM_IV_SIZE 12
#define KEY_SIZE 16

const unsigned char AES_GCM_KEY[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
const unsigned char AES_GCM_IV[] = { 0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84 };

int aes128GcmEncrypt(unsigned char *plaintext, int plaintext_len,
                unsigned char *aad, int aad_len,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *ciphertext,
                unsigned char *tag);

int aes128GcmDecrypt(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *aad, int aad_len,
                unsigned char *tag,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *plaintext);

#endif //SIMPLE_DSP_ENGINE_AES_GCM_H
