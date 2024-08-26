//
// Created by hungpm on 22/08/2024.
//

#ifndef SIMPLE_DSP_ENGINE_STRINGRANDOMGENERATIONSOURCE_H
#define SIMPLE_DSP_ENGINE_STRINGRANDOMGENERATIONSOURCE_H

#include "Source.h"
#include "Emitter.h"

#include <vector>
#include <string>

class StringRandomGenerationSource : public Source {
protected:
    uint64_t generatedDataSizeInBytes_ = 16;
    uint64_t count_ = 0;
    std::vector<std::string> generatedTexts_;
    const std::string allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+}{";

    std::string generateRandomString_();
public:
    StringRandomGenerationSource();
    StringRandomGenerationSource(uint64_t generatedDataSizeInBytes, uint64_t count);
    void prepare();
    int start(Emitter &emitter) override;
};

#endif //SIMPLE_DSP_ENGINE_STRINGRANDOMGENERATIONSOURCE_H
