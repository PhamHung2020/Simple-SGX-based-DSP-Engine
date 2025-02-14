#!/bin/bash

# List of string arguments
arguments=(
#  "3-4-5-6-7"
#  "3-4-5-7-6"
#  "3-4-6-5-7"
#  "3-4-6-7-5"
#  "3-4-7-5-6"
#  "3-4-7-6-5"
#  "3-5-4-6-7"
#  "3-5-4-7-6"
#  "3-5-6-4-7"
#  "3-5-6-7-4"
#  "3-5-7-4-6"
#  "3-5-7-6-4"
#  "3-6-4-5-7"
#  "3-6-4-7-5"
#  "3-6-5-4-7"
#  "3-6-5-7-4"
#  "3-6-7-4-5"
#  "3-6-7-5-4"
#  "3-7-4-5-6"
#  "3-7-4-6-5"
#  "3-7-5-4-6"
#  "3-7-5-6-4"
#  "3-7-6-4-5"
#  "3-7-6-5-4"
#  "4-3-5-6-7"
#  "4-3-5-7-6"
#  "4-3-6-5-7"
#  "4-3-6-7-5"
#  "4-3-7-5-6"
#  "4-3-7-6-5"
#  "4-5-3-6-7"
#  "4-5-3-7-6"
#  "4-5-6-3-7"
#  "4-5-6-7-3"
#  "4-5-7-3-6"
#  "4-5-7-6-3"
#  "4-6-3-5-7"
#  "4-6-3-7-5"
#  "4-6-5-3-7"
#  "4-6-5-7-3"
#  "4-6-7-3-5"
#  "4-6-7-5-3"
#  "4-7-3-5-6"
#  "4-7-3-6-5"
#  "4-7-5-3-6"
#  "4-7-5-6-3"
#  "4-7-6-3-5"
#  "4-7-6-5-3"
#  "5-3-4-6-7"
#  "5-3-4-7-6"
#  "5-3-6-4-7"
#  "5-3-6-7-4"
#  "5-3-7-4-6"
#  "5-3-7-6-4"
#  "5-4-3-6-7"
#  "5-4-3-7-6"
#  "5-4-6-3-7"
#  "5-4-6-7-3"
#  "5-4-7-3-6"
#  "5-4-7-6-3"
#  "5-6-3-4-7"
#  "5-6-3-7-4"
#  "5-6-4-3-7"
#  "5-6-4-7-3"
#  "5-6-7-3-4"
#  "5-6-7-4-3"
#  "5-7-3-4-6"
#  "5-7-3-6-4"
#  "5-7-4-3-6"
#  "5-7-4-6-3"
#  "5-7-6-3-4"
#  "5-7-6-4-3"
#  "6-3-4-5-7"
#  "6-3-4-7-5"
#  "6-3-5-4-7"
#  "6-3-5-7-4"
#  "6-3-7-4-5"
#  "6-3-7-5-4"
#  "6-4-3-5-7"
#  "6-4-3-7-5"
#  "6-4-5-3-7"
#  "6-4-5-7-3"
#  "6-4-7-3-5"
#  "6-4-7-5-3"
#  "6-5-3-4-7"
#  "6-5-3-7-4"
#  "6-5-4-3-7"
#  "6-5-4-7-3"
#  "6-5-7-3-4"
#  "6-5-7-4-3"
#  "6-7-3-4-5"
#  "6-7-3-5-4"
#  "6-7-4-3-5"
#  "6-7-4-5-3"
#  "6-7-5-3-4"
#  "6-7-5-4-3"
#  "7-3-4-5-6"
#  "7-3-4-6-5"
#  "7-3-5-4-6"
#  "7-3-5-6-4"
#  "7-3-6-4-5"
#  "7-3-6-5-4"
#  "7-4-3-5-6"
#  "7-4-3-6-5"
#  "7-4-5-3-6"
#  "7-4-5-6-3"
#  "7-4-6-3-5"
#  "7-4-6-5-3"
#  "7-5-3-4-6"
#  "7-5-3-6-4"
#  "7-5-4-3-6"
#  "7-5-4-6-3"
#  "7-5-6-3-4"
#  "7-5-6-4-3"
#  "7-6-3-4-5"
#  "7-6-3-5-4"
#  "7-6-4-3-5"
#  "7-6-4-5-3"
#  "7-6-5-3-4"
#  "7-6-5-4-3"

#  "3-4-5-6-7"
#  "3-5-4-6-7"
#  "4-3-5-6-7"
#  "4-5-3-6-7"
#  "5-3-4-6-7"
#  "5-4-3-6-7"
#  "3-6-7-4-5"
#  "3-7-6-4-5"
#  "6-3-7-4-5"
#  "6-7-3-4-5"
#  "7-3-6-4-5"
#  "7-6-3-4-5"
  "1-2-3-6-7"
  "1-3-2-6-7"
  "2-1-3-6-7"
  "2-3-1-6-7"
  "3-1-2-6-7"
  "3-2-1-6-7"
)

# Path to your compiled program
program="./app"

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi
cd build/$mode

cpupower frequency-set -r -u 1000MHz
wrmsr -a 0x1a0 0x4000850089

# Iterate over the arguments and pass each one to the C++ program
for arg in "${arguments[@]}"; do
#  cat /proc/interrupts > ../../perf/rework_cryptosdk_coldcache_perf/2/before_$arg.txt
#  perf stat --cpu=3,4,5,6,7 -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations,context-switches -A $program "$arg" 2> ../../perf/rework_cryptosdk_coldcache_perf/2/perf_$arg.txt
#  cat /proc/interrupts > ../../perf/rework_cryptosdk_coldcache_perf/2/after_$arg.txt
  $program "$arg"
done

cpupower frequency-set -r -u 4000MHz
