//
// Created by hungpm on 01/05/2024.
//
#include <iostream>
#include <sgx_urts.h>
#include <pthread.h>
#include <unistd.h>

#include "App/test.h"
#include "sgx_lib.h"
#include "Enclave_u.h"

sgx_enclave_id_t testCpuAffinity_enclaveId;

void *testCpuAffinity_ecallSum(void*)
{
    int a = 5;
    int b = 4;
    int result = 0;

    std::cout << "ID: " << pthread_self() << " , CPU: " << sched_getcpu() << std::endl;

    const sgx_status_t status = ecallSum(testCpuAffinity_enclaveId, &result, a, b);
    if (status != SGX_SUCCESS) {
        std::cout << "Call ecall failed...\n";
    } else {
        std::cout << "Result: " << result << std::endl;
    };

    return nullptr;
}

void testCpuAffinity() {
    if (initialize_enclave(&testCpuAffinity_enclaveId) != SGX_SUCCESS) {
        std::cout << "Initialize enclave failed\n";
        return;
    }

    pthread_t threadId;
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
    CPU_ZERO(&cpus);
    CPU_SET(0, &cpus);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

    pthread_create(&threadId, &attr, testCpuAffinity_ecallSum, nullptr);
    pthread_join(threadId, nullptr);

    if (sgx_destroy_enclave(testCpuAffinity_enclaveId) != SGX_SUCCESS) {
        std::cout << "Destroying enclave failed...\n";
        return;
    }

    std::cout << "DONE\n";
}