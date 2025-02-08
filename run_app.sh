#!/bin/bash

mode=$1
rule_name=$2

if [[ -z "$mode" ]]; then
    mode="debug"
fi

cpupower frequency-set -r -u 1000MHz
wrmsr -a 0x1a0 0x4000850089
cd build/$mode
#cat /proc/interrupts > ../../before_43567.txt
#perf stat --cpu=3,4,5,6,7 -A ./app > ../../perf_43567.txt
# -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations,context-switches
#cat /proc/interrupts > ../../after_43567.txt
./app "$rule_name"
# perf stat -B -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations
# \documentclass[sigconf, anonymous]{acmart}
cpupower frequency-set -r -u 4000MHz