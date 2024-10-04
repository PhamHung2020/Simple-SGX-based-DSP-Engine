//
// Created by hungpm on 03/07/2024.
//
#include "Enclave/cryptography.h"
#include "tsgxsslio.h"
#include <openssl/bio.h>
#include <openssl/evp.h>

int openSslAes128GcmEncrypt(unsigned char *plaintext, int plaintext_len,
                     unsigned char *aad, int aad_len,
                     unsigned char *key,
                     unsigned char *iv, int iv_len,
                     unsigned char *ciphertext,
                     unsigned char *tag) {
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
//        printf("Cannot create and initialise the context\n");
        return -1;
    }

    /* Initialise the encryption operation. */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL)) {
//        printf("Cannot initialise the encryption operation\n");
        return -1;
    }

    /*
      * Set IV length if default 12 bytes (96 bits) is not appropriate
      */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL)) {
//        printf("Cannot set IV length\n");
        return -1;
    }

    /* Initialise key and IV */
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) {
//        printf("Cannot initialise Key and IV\n");
        return -1;
    }

    /*
      * Provide any AAD data. This can be called zero or more times as
      * required
      */
    // if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
    //     handleErrors();

    /*
      * Provide the message to be encrypted, and obtain the encrypted output.
      * EVP_EncryptUpdate can be called multiple times if necessary
      */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
//        printf("Cannot encrypted message\n");
        return -1;
    }
    ciphertext_len = len;

    /*
      * Finalise the encryption. Normally ciphertext bytes may be written at
      * this stage, but this does not occur in GCM mode
      */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
//        printf("Cannot finalise the encryption\n");
        return -1;
    }
    ciphertext_len += len;

    /* Get the tag */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)) {
//        printf("Cannot get the tag\n");
        return -1;
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int openSslAes128GcmDecrypt(unsigned char *ciphertext, int ciphertext_len,
                     unsigned char *aad, int aad_len,
                     unsigned char *tag,
                     unsigned char *key,
                     unsigned char *iv, int iv_len,
                     unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
//        printf("Cannot initialise the context\n");
        return -1;
    }

    /* Initialise the decryption operation. */
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL)) {
//        printf("Cannot initialise the decryption operation\n");
        return -1;
    }

    /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL)) {
//        printf("Cannot set IV length\n");
        return -1;
    }

    /* Initialise key and IV */
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
//        printf("Cannot initialise key and IV\n");
        return -1;
    }

    /*
     * Provide any AAD data. This can be called zero or more times as
     * required
     */
    // if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
    //     handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
//        printf("Cannot decrypt the message\n");
        return -1;
    }
    plaintext_len = len;

    /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)) {
//        printf("Cannot set tag value\n");
        return -1;
    }

    /*
     * Finalise the decryption. A positive return value indicates success,
     * anything else is a failure - the plaintext is not trustworthy.
     */
    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    if(ret > 0) {
        /* Success */
        plaintext_len += len;
        return plaintext_len;
    } else {
        /* Verify failed */
        return -1;
    }
}

void aesGcmDecrypt(char *encMessageIn, size_t len, char *decMessageOut, size_t lenOut) {
//    uint8_t AES_GCM_KEY[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
//    uint8_t AES_GCM_IV[] = { 0x99,0xaa,0x3e,0x68,0xed,0x81,0x73,0xa0,0xee,0xd0,0x66,0x84 };

    auto *encMessage = (uint8_t *) encMessageIn;
    uint8_t p_dst[BUFLEN] = {0};

    sgx_aes_gcm_decrypt(
            SGX_AES_GCM_KEY,
            SGX_AESGCM_KEY_SIZE,
            encMessage + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
            lenOut,
            p_dst,
            SGX_AES_GCM_IV, SGX_AESGCM_IV_SIZE,
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
