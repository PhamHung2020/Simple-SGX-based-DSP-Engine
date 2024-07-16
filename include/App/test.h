//
// Created by hungpm on 27/03/2024.
//

#ifndef TEST_H
#define TEST_H

#include "App/ConfigurationTesting.h"

void testSimpleEngine();
void testHotCallPerformance();
void testObserverPerformance(const ConfigurationTesting&);
void testJoin2Stream1Enclave(int n);
void testCpuAffinity();
void testNexmark(const ConfigurationTesting&);
void testNexmark2(int n);
#endif //TEST_H
