# Leaking Queries On Secure Stream Processing Systems

This project provides a simple DSP engine running on Intel SGX using HotCalls, designed to benchmark and analyze query leakage in secure stream processing systems.

## Organization

This repository is organized into the following main directories:
- For the DSP engine and benchmarks:
  - config: Configuration files for compiling SGX projects.
  - include: Header files for the project.
  - src: Source code for the DSP engine and benchmarks.
  - scripts: Scripts to run benchmarks and manage experiments.
  - libraries: External libraries used in the project.
- For data analysis and visualization:
  - notebooks: Jupyter notebooks and Python code for data analysis, visualization, and produce results in the paper.

## Prerequisites

This project is tested on Ubuntu 22.04 LTS, Intel SGX supported, with following dependencies:
- If you want to run the DSP engine:
  - **Intel SGX SDK**: Version 2.24. Ensure you have the Intel SGX SDK installed. Follow the instructions at [Intel SGX SDK Installation Guide](https://download.01.org/intel-sgx/latest/linux-latest/docs/Intel_SGX_SW_Installation_Guide_for_Linux.pdf).
  - **CMake**: Version 3.28.3.
  - **GCC**: Version 13.3.0.
  - **Make**: GNU Make 4.3.
  - **Openssl**: Version 3.0.13.
- If you want to run the notebooks and produce results in the paper:
  - **Python**: Version 3.12.

If you want to run the DSP engine but your machine does not support Intel SGX, you can run the engine in SGX simulation mode. See the section "Running in SGX Simulation Mode" below for more details.
To install all dependencies, run _install.sh_ script with _sudo_.

## Data visualization and produce results in the paper

Notebooks and Python scripts for producing results in the paper are located in the _notebooks_ folder. These notebooks and code are tested with Python 3.12.

Required Python packages are listed in _notebooks/requirements.txt_. You can create a virtual environment using Venv, then install required packages using pip:

```sh
pip install -r notebooks/requirements.txt
```

Or you can use Conda if you prefer.

There are 3 folders:
- **notebooks/cdf**: Contains Jupyter notebooks for CDF related results.
- **notebooks/ts2vec**: Contains Jupyter notebooks for TS2VEC related results.
- **notebooks/utils**: Contains useful Python scripts, used by notebooks in 2 previous folders.

In **notebooks/cdf** and **notebooks/ts2vec** folders, each has following notebooks:
- **classification.ipynb**: Perform operator classifications.
- **cross_classification.ipynb**: Perform cross classification - use models trained on Nexmark dataset to perform classification on Secure Stream dataset.
- **regression.ipynb**: Perform operators' parameters prediction.
- **visualization.ipynb**: Generate figures used in the paper.

If you wish to run these notebooks:
- First download the dataset, which is our timing measurements of the queries in the paper, from the following link: [Timing dataset](https://zenodo.org/records/17115025).
- Unzip the downloaded file, you will see 2 zip files: nexmark_dataset.zip and secure_sgx_dataset.zip. Continue unzipping them, and place them under the _notebooks_ folder. The whole dataset is around 41GB after unzipping.
- Finally, install the required packages and run the notebooks.

## DSP Engine

This prototype DPS engine was used to run the benchmarks in the paper and generate the dataset mentioned above. If you wish to run this tool, follow the instructions below.

### Building the DSP Engine

Clone the repository and navigate to the root directory of the project.
```sh
git clone https://github.com/PhamHung2020/Simple-SGX-based-DSP-Engine
cd Simple-SGX-based-DSP-Engine
```

Install all dependencies by running:
```sh
sudo ./install.sh
```

Build the project using:

```sh
make <mode>
```

where <mode> can be either `debug` or `release`. Default mode is `debug`.
This will compile the application and its dependencies, then produce a runnable file in the folder _build/\<mode>_.

Export the SGX SDK environment variables by running:
```sh
source /opt/intel/sgxsdk/environment
```
Everytime you open a new terminal, remember to run this command to set the environment variables.

### Running queries in the paper

Run the script:
```sh
scripts/download_source_data_for_queries.sh
```

The script will download the datasource used in the experiments from the following links: [source_data](https://github.com/PhamHung2020/Simple-SGX-based-DSP-Engine/releases/download/data_source/source_data.zip), then unzip the file and place the unzipped folder (named _source_data_) under the root directory of the project.

To run benchmarks used in the associated research paper, after building the project, execute the corresponding script for the desired benchmark:
- For `nexmark` benchmark, run **_scripts/run_nexmark_queries.sh_**.
- For `securestream` benchmark, run **_scripts/run_securestream_queries.sh_**.
- For `streambox` benchmark, run **_scripts/run_streambox_queries.sh_**.

Results, which is the output of the queries, will be stored in the _sinks_ folder. Timing measurements will be saved in the _timing_measurements_ folder.
You can modify the scripts to change parameters such as the number of runs or input data files as needed.

### DSP Engine Command Line Usage

After building, change directory to _build/\<mode>_, then you can run the tool with the following commands:

#### Display Help

```sh
./app -h
```
Shows all available commands and their usage.

#### List All Benchmarks

```sh
./app -l
```
Lists all supported benchmark names.

#### List Queries of a Benchmark

```sh
./app -lq <benchmark_name>
```
Lists all available queries for the specified benchmark. Benchmark names include: `nexmark`, `securestream`, and `streambox`.

#### Run a Query

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

### Running in SGX Simulation Mode
To compile and run the application in SGX simulation mode, change the `Makefile` to set `SGX_MODE=SIM` and then build the project. This mode allows you to test the application without requiring actual SGX hardware.
In case your hardware supports SGX, you can set `SGX_MODE=HW` to run the application on real SGX hardware.