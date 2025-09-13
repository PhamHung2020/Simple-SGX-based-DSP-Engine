#!/bin/bash

# Path to your compiled program
program="./app"

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi

# Check and create required directories
for dir in "./source_data/streambox" "./timing_measurements/streambox" "./sinks/streambox"; do
    if [ ! -d "$dir" ]; then
        echo "Directory $dir does not exist. Creating..."
        mkdir -p "$dir"
    fi
done

cd build/$mode

export LD_LIBRARY_PATH=/opt/intel/sgxsdk/lib64:$LD_LIBRARY_PATH

$program -r "StreamBox" "Query1" "../../source_data/streambox/synthetic_dataset_for_q1.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1
$program -r "StreamBox" "Query2" "../../source_data/streambox/trip_data_first_5k.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1
$program -r "StreamBox" "Query3" "../../source_data/streambox/synthetic_dataset_for_q3_1.csv" "../../source_data/streambox/synthetic_dataset_for_q3_2.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1
$program -r "StreamBox" "Query4" "../../source_data/streambox/data_for_q4.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1
$program -r "StreamBox" "Query5" "../../source_data/streambox/synthetic_dataset_for_q5.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1
$program -r "StreamBox" "Query6" "../../source_data/streambox/trip_data_first_5k.csv" "../../timing_measurements/streambox" "../../sinks/streambox" 1