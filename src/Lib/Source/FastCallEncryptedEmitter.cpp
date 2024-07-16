//
// Created by hungpm on 03/07/2024.
//

#include "Source/FastCallEncryptedEmitter.h"
#include "Crypto/aes_gcm.h"

void FastCallEncryptedEmitter::emit(void *data) {
    if (this->fastCallDataP_->data_buffer != nullptr) {
//        const auto parsedData = static_cast<char *>(data);
//        printf("encrypting: %s\n", parsedData);
//        int encryptedDataLength = SGX_AESGCM_MAC_SIZE + this->dataLength_;
//        char* encryptedData = new char[encryptedDataLength + 1];
//        aes128GcmEncrypt(
//                (unsigned char *) parsedData,
//                this->dataLength_, NULL, 0,
//                const_cast<unsigned char *>(AES_GCM_KEY),
//                const_cast<unsigned char *>(AES_GCM_IV), SGX_AESGCM_IV_SIZE,
//                (unsigned char *)(encryptedData + SGX_AESGCM_MAC_SIZE),
//                (unsigned char *)(encryptedData));
//
//        encryptedData[encryptedDataLength] = '\0';
//        printf("emitting: %s\n", encryptedData);
        FastCall_request_encrypt(this->fastCallDataP_, data);
//        delete[] encryptedData;
    }
}

void FastCallEncryptedEmitter::setDataLength(size_t dataLength) {
    this->dataLength_ = dataLength;
}


