//
// Created by hungpm on 23/07/2024.
//

#ifndef SIMPLE_DSP_ENGINE_SINKS_H
#define SIMPLE_DSP_ENGINE_SINKS_H

#include <fstream>

#if defined(__cplusplus)
extern "C" {
#endif

extern std::ofstream secureSgxSinkFileStream;

#if defined(__cplusplus)
}
#endif

std::ofstream* getSecureSgxSinkFileStream();
void sinkMap(void* rawData);
void sinkReduce(void* rawData);

#endif //SIMPLE_DSP_ENGINE_SINKS_H
