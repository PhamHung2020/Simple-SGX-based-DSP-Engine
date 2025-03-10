//
// Created by hungpm on 24/03/2024.
//

#include "Enclave/tasks.h"
#include "Enclave/Enclave.h"
#include "fast_call.h"
#include "StreamBox/schemas.h"
#include "sgx_trts_exception.h"

#include <cmath>
#include <stdexcept>

using namespace std;

FastCallStruct* globalFastOCall;
circular_buffer* fastOCallBuffer;
char encryptedData[1000];

bool isPalindrome(int64_t num) {
    num = abs(num);
    int64_t reversed = 0, original = num;

    while (num > 0) {
        reversed = reversed * 10 + (num % 10);
        num /= 10;
    }

    return original == reversed;
}

bool isPrime(int64_t num) {
    num = abs(num); // Take the absolute value

    if (num < 2) return false; // 0 and 1 are not prime numbers
    if (num == 2 || num == 3) return true; // 2 and 3 are prime
    if (num % 2 == 0 || num % 3 == 0) return false; // Eliminate even numbers and multiples of 3

    // Check divisibility from 5 to sqrt(num) using 6k ± 1 optimization
    for (int64_t i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }

    return true;
}


// Function to check if a number is a perfect square
bool isPerfectSquare(int64_t num) {
    int64_t s = static_cast<int64_t>(std::sqrt(num));
    return s * s == num;
}

// Function to check if |num| is a Fibonacci number
bool isFibonacci(int64_t num) {
    num = abs(num); // Take the absolute value

    // A number is Fibonacci if and only if (5*n^2 + 4) or (5*n^2 - 4) is a perfect square
    return isPerfectSquare(5 * num * num + 4) || isPerfectSquare(5 * num * num - 4);
}

bool FilterCondition(SyntheticData* syntheticData) {
    if (!isFibonacci(syntheticData->key) || syntheticData->value >= 500) return false;
    return true;
}

bool isPowerOf2(int64_t num) {
    num = abs(num); // Take absolute value

    return (num > 0) && ((num & (num - 1)) == 0);
}

void FilterSyntheticData(void* data) {
    if (data == NULL) {
        return;
    }

    const auto syntheticData = static_cast<SyntheticData*>(data);
    if (!FilterCondition(syntheticData)) {
        FastCall_request_encrypt2(globalFastOCall, syntheticData, encryptedData);
    }
}
