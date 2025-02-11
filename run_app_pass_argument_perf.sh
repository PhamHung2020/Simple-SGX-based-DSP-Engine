#!/bin/bash

# List of string arguments
arguments=(
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
  cat /proc/interrupts > ../../perf/rework_cryptosdk_disabled_hyperthread_perf/before_$arg.txt
  perf stat --cpu=1,2,3 -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations,context-switches -A $program "$arg" 2> ../../perf/rework_cryptosdk_disabled_hyperthread_perf/perf_$arg.txt
  cat /proc/interrupts > ../../perf/rework_cryptosdk_disabled_hyperthread_perf/after_$arg.txt
#  $program "$arg"
done

cpupower frequency-set -r -u 4000MHz
