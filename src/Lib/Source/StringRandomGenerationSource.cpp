//
// Created by hungpm on 22/08/2024.
//

#include "Source/StringRandomGenerationSource.h"

#include <random>
#include <cstring>
#include <iostream>

StringRandomGenerationSource::StringRandomGenerationSource() = default;

StringRandomGenerationSource::StringRandomGenerationSource(uint64_t generatedDataSizeInBytes, uint64_t count) {
    this->generatedDataSizeInBytes_ = generatedDataSizeInBytes;
    this->count_ = count;
}

void StringRandomGenerationSource::prepare() {
//    std::cout << "Data size: " << this->generatedDataSizeInBytes_ << std::endl;
    this->generatedTexts_.clear();
    this->generatedTexts_.reserve(this->count_);

    for (uint64_t i = 0; i < this->count_; ++i) {
        this->generatedTexts_.push_back(this->generateRandomString_());
//        std::cout << i << ". " << this->generatedTexts_[i] << std::endl;
    }
}

int StringRandomGenerationSource::start(Emitter &emitter) {
    const uint64_t MAX_CHARACTERS = this->generatedDataSizeInBytes_ + 1;
    char* content = new char[MAX_CHARACTERS];
    for (uint64_t i = 0; i < this->count_; ++i) {
        strncpy(content, this->generatedTexts_[i].c_str(), MAX_CHARACTERS);
        emitter.emit(content);
    }
    return 0;
}

std::string StringRandomGenerationSource::generateRandomString_() {
    std::string randomString;

    // Use a random device to seed the generator
    std::random_device rd;
    std::mt19937 generator(rd());

    // Create a distribution to select characters from the characters string
    std::uniform_int_distribution<> distribution(0, this->allowedCharacters.size() - 1);

    for (int i = 0; i < this->generatedDataSizeInBytes_; i++) {
        randomString += this->allowedCharacters[distribution(generator)];
    }

    return randomString;
}


