//
// Created by hungpm on 03/07/2024.
//

#include "Source/FastCallEncryptedEmitter.h"
#include "Crypto/aes_gcm.h"

void FastCallEncryptedEmitter::emit(void *data) {
    if (this->fastCallDataP_->data_buffer != nullptr) {
        FastCall_request_encrypt(this->fastCallDataP_, data);
    }
}

void FastCallEncryptedEmitter::setDataLength(size_t dataLength) {
    this->dataLength_ = dataLength;
}


