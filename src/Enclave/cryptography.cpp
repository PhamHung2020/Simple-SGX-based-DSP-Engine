//
// Created by hungpm on 03/07/2024.
//
#include "Enclave/cryptography.h"

void aesGcmDecrypt(char *encMessageIn, size_t len, char *decMessageOut, size_t lenOut) {
    uint8_t AES_GCM_KEY[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
    uint8_t AES_GCM_IV[] = { 0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84 };

    auto *encMessage = (uint8_t *) encMessageIn;
    uint8_t p_dst[BUFLEN] = {0};

    sgx_aes_gcm_decrypt(
            AES_GCM_KEY,
            SGX_AESGCM_KEY_SIZE,
            encMessage + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
            lenOut,
            p_dst,
            AES_GCM_IV, SGX_AESGCM_IV_SIZE,
            NULL, 0,
            (sgx_aes_gcm_128bit_tag_t *) encMessage
    );

    memcpy(decMessageOut, p_dst, lenOut);
}

void aesGcmEncrypt(char *decMessageIn, size_t len, char *encMessageOut, size_t lenOut)
{
    uint8_t AES_GCM_KEY[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
//    uint8_t AES_GCM_IV[] = { 0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84 };

    auto *origMessage = (uint8_t *) decMessageIn;
    uint8_t p_dst[BUFLEN] = {0};

    // Generate the IV (nonce)
    // sgx_read_rand(p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE);

    sgx_aes_gcm_encrypt(
            AES_GCM_KEY,
            SGX_AESGCM_KEY_SIZE,
            origMessage, len,
            p_dst + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
            p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
            NULL, 0,
            (sgx_aes_gcm_128bit_tag_t *) (p_dst));

//    memcpy(encMessageOut + SGX_AESGCM_MAC_SIZE, AES_GCM_IV, SGX_AESGCM_IV_SIZE);
    memcpy(encMessageOut,p_dst,lenOut);
}
