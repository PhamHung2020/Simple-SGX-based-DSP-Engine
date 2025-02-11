//
// Created by hungpm on 07/05/2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <cinttypes>

inline __attribute__((always_inline))  uint64_t rdtscp()
{
    unsigned int low, high;

    asm volatile("rdtscp" : "=a" (low), "=d" (high));

    return low | ((uint64_t)high) << 32;
}

#endif //UTILS_H
