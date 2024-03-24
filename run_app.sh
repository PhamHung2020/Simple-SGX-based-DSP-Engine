#!/bin/bash

mode=$1

if [[ -z "$mode" ]]; then
    mode="debug"
fi

cd build/$mode
./app