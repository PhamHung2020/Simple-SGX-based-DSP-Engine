# Leaking Queries On Secure Stream Processing Systems

This project provides a simple DSP engine running on Intel SGX using HotCalls, designed to benchmark and analyze query leakage in secure stream processing systems.

## Organization

- config: Configuration files for compiling SGX projects.
- include: Header files for the project.
- src: Source code for the DSP engine and benchmarks.
- scripts: Scripts to run benchmarks and manage experiments.
- libraries: External libraries used in the project.
- notebooks: Jupyter notebooks and Python code for data analysis, visualization, and produce results in the paper.

## Prerequisites

This project is tested on Ubuntu 22.04 LTS, with following dependencies:
- **Intel SGX SDK**: Version 2.24. Ensure you have the Intel SGX SDK installed. Follow the instructions at [Intel SGX SDK Installation Guide](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf).
- **CMake**: Version 3.28.3.
- **GCC**: Version 13.3.0.
- **Make**: GNU Make 4.3.
- **Openssl**: Version 3.0.13.
- **Python**: Version 3.12.

To install the required packages, run _install.sh_ script with _sudo_.

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

## Running notebooks for data analysis, visualization, and produce results in the paper

Notebooks and Python scripts for data analysis and visualization are located in the _notebooks_ folder. Required Python packages are listed in _notebooks/requirements.txt_. You can install them using pip:

```sh
pip install -r notebooks/requirements.txt
```

However, these notebooks are still in the process of being cleaned, organized, and documented. Below is a brief description of each notebook and Pyhton files:
- **notebooks/load_dataset.py**: Python script to preprocess and load datasets.
- **notebooks/ml_regression.py**: Python script containing functions for performing regression.
- **notebooks/cdf.ipynb**: Jupyter notebook to calculate and perform classification based on CDF.
- **notebooks/cross.ipynb**: Jupyter notebook to use a model trained on a benchmark to perform classification on another benchmark (cross classification).
- **notebooks/ts2vec_classification.ipynb**: Jupyter notebook to use the ts2vec model for classification.
- *notebooks/ts2vec_regression.ipynb*: Jupyter notebook to use the ts2vec model for regression.
- **notebooks/ts2vec_visualize.ipynb**: Jupyter notebook to use the ts2vec model for visualization.

If you wish to run these notebooks, first download the dataset, which is the output of the queries in the paper, from the following link: [output_data](https://drive.google.com/file/d/1JHk3Yk9bX4eX4F7hgLMe5Wh5k6OUTcEHvr/view?usp=drive_link). Unzip the file and place under the _notebooks_ folder. Then, install the required packages and run the notebooks.
This file will be updated regularly to provide detail instructions.