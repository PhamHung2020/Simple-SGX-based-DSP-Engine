#!/bin/bash

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi

cpupower frequency-set -r -u 1000MHz
wrmsr -a 0x1a0 0x4000850089
cd build/$mode
./app

cpupower frequency-set -r -u 4000MHz