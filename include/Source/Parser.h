//
// Created by hungpm on 05/04/2024.
//

#ifndef SIMPLE_DSP_ENGINE_PARSER_H
#define SIMPLE_DSP_ENGINE_PARSER_H

#include <string>

class Parser {
public:
    virtual ~Parser() = default;
    virtual void* parseFromString(const std::string& str) = 0;
};

#endif //SIMPLE_DSP_ENGINE_PARSER_H
