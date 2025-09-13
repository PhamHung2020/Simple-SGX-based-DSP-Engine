#!/bin/bash

# Path to your compiled program
program="./app"

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi

# Check and create required directories
for dir in "../../source_data/nexmark" "../../timing_measurements/nexmark" "../../sinks/nexmark"; do
    if [ ! -d "$dir" ]; then
        echo "Directory $dir does not exist. Creating..."
        mkdir -p "$dir"
    fi
done

cd build/$mode

$program -r "Nexmark" "Query1" "../../source_data/nexmark/bids.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query2_Filter" "../../source_data/nexmark/bids.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query2_Map" "../../sinks/nexmark/Nexmark_Query2_Filter_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query3_FilterPerson" "../../source_data/nexmark/people.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query3_FilterAuction" "../../source_data/nexmark/auctions.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query3_JoinPersonAuction" "../../sinks/nexmark/Nexmark_Query3_FilterPerson_1.csv" "../../sinks/nexmark/Nexmark_Query3_FilterAuction_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query3_MapJoinResult" "../../sinks/nexmark/Nexmark_Query3_JoinPersonAuction_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query4_JoinAuctionBid" "../../source_data/nexmark/auctions.csv" "../../source_data/nexmark/bids.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query4_MapAuctionBid" "../../sinks/nexmark/Nexmark_Query4_JoinAuctionBid_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query4_Max" "../../sinks/nexmark/Nexmark_Query4_MapAuctionBid_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query4_JoinCategory" "../../sinks/nexmark/Nexmark_Query4_Max_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query4_Average" "../../sinks/nexmark/Nexmark_Query4_JoinCategory_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query5_CountByAuction" "../../source_data/nexmark/bids.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query5_MaxBatch" "../../sinks/nexmark/Nexmark_Query5_CountByAuction_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query6_JoinAuctionBid" "../../source_data/nexmark/auctions.csv" "../../source_data/nexmark/bids.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query6_Filter" "../../sinks/nexmark/Nexmark_Query6_JoinAuctionBid_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query6_Max" "../../sinks/nexmark/Nexmark_Query6_Filter_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1
$program -r "Nexmark" "Query6_Avg" "../../sinks/nexmark/Nexmark_Query6_Max_1.csv" "../../timing_measurements/nexmark" "../../sinks/nexmark" 1