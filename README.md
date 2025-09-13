# Leaking Queries On Secure Stream Processing Systems

This project provides a simple DSP engine running on Intel SGX using HotCalls, designed to benchmark and analyze query leakage in secure stream processing systems.

## Prerequisites

This project is tested on Ubuntu 22.04 LTS, with following dependencies:
- **Intel SGX SDK**: Version 2.24. Ensure you have the Intel SGX SDK installed. Follow the instructions at [Intel SGX SDK Installation Guide](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf).
- **CMake**: Version 3.28.3.
- **GCC**: Version 13.3.0.
- **Make**: GNU Make 4.3.
- **Python**: Version 3.12.

## Building the Tool

Before running any commands, build the project using:

```sh
make <mode>
```

where <mode> can be either `debug` or `release`. Default mode is `debug`.
This will compile the application and its dependencies, then produce a runnable file in the folder _build/\<mode>_.

## Command Line Usage

After building, change directory to _build/\<mode>_, then you can run the tool with the following commands:

### Display Help

```sh
./app -h
```
Shows all available commands and their usage.

### List All Benchmarks

```sh
./app -l
```
Lists all supported benchmark names.

### List Queries of a Benchmark

```sh
./app -lq <benchmark_name>
```
Lists all available queries for the specified benchmark. Benchmark names include: `nexmark`, `securestream`, and `streambox`.

### Run a Query

```sh
./app -r <benchmark_name> <query_name> <source1> [<source2>] <measurement_dir> <result_dir> <number_of_runs>
```
Runs the specified query from the chosen benchmark. Some queries may require two sources; provide both if needed. The results and measurements will be saved in the specified directories, with filenames formatted as `<benchmark>_<query>_<run>.csv`.

**Arguments:**
- `<benchmark_name>`: The benchmark to use (`nexmark`, `securestream`, or `streambox`)
- `<query_name>`: The query to run (see `-lq` for available queries)
- `<source1>`: Path to the first source file
- `[<source2>]`: (Optional) Path to the second source file, if required by the query
- `<measurement_dir>`: Directory to store measurement files
- `<result_dir>`: Directory to store result files
- `<number_of_runs>`: Number of times to run the query; output files will be suffixed with the run number

**Example:**
```sh
./app -r nexmark q1 dataset/nexmark/auctions.csv measurements/nexmark results/nexmark 3
```

## Running in SGX Simulation Mode
To compile and run the application in SGX simulation mode, change the `Makefile` to set `SGX_MODE=SIM` and then build the project. This mode allows you to test the application without requiring actual SGX hardware.

## Running queries in the paper

Download the datasource used in the experiments from the following links: [source_data](https://drive.google.com/file/d/1144xgE9u4ZwF7hgLMe5Wh5k6OUTcEHvr/view?usp=drive_link). Unzip the file and place the unzipped folder (named _source_data_) under the root directory of the project.

To run benchmarks used in the associated research paper, after building the project, execute the corresponding script for the desired benchmark:
- For `nexmark` benchmark, run **_scripts/run_nexmark_queries.sh_**.
- For `securestream` benchmark, run **_scripts/run_securestream_queries.sh_**.
- For `streambox` benchmark, run **_scripts/run_streambox_queries.sh_**.

Results, which is the output of the queries, will be stored in the _sinks_ folder. Timing measurements will be saved in the _timing_measurements_ folder.
You can modify the scripts to change parameters such as the number of runs or input data files as needed.