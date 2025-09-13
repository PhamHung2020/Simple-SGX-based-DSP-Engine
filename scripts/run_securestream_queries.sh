#!/bin/bash

# Path to your compiled program
program="./app"

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi

# Check and create required directories
for dir in "./source_data/securestream" "./timing_measurements/securestream" "./sinks/securestream"; do
    if [ ! -d "$dir" ]; then
        echo "Directory $dir does not exist. Creating..."
        mkdir -p "$dir"
    fi
done

cd build/$mode

$program -r "SecureStream" "MapQuery" "../../source_data/securestream/2005_100000.csv" "../../timing_measurements/securestream" "../../sinks/securestream" 1
$program -r "SecureStream" "FilterQuery" "../../sinks/securestream/SecureStream_MapQuery_1.csv" "../../timing_measurements/securestream" "../../sinks/securestream" 1
$program -r "SecureStream" "ReduceQuery" "../../sinks/securestream/SecureStream_FilterQuery_1.csv" "../../timing_measurements/securestream" "../../sinks/securestream" 1