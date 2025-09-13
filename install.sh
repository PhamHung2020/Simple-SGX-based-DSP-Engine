#!/bin/bash
# Install dependencies for Leaking Queries On Secure Stream Processing Systems

set -e

# Install SGX SDK (Intel SGX)
if [ ! -d "/opt/intel/sgxsdk" ]; then
    echo "Installing Intel SGX SDK..."
    wget https://download.01.org/intel-sgx/sgx-linux/2.24/distro/ubuntu22.04-server/sgx_linux_x64_sdk_2.24.100.3.bin -O sgx_sdk.bin
    chmod +x sgx_sdk.bin
    echo -e "no\n/opt/intel" | ./sgx_sdk.bin
    rm sgx_sdk.bin
    echo "SGX SDK installed at /opt/intel/sgxsdk"
else
    echo "SGX SDK already installed."
fi

# Install CMake (>=3.10)
if ! command -v cmake &> /dev/null; then
    echo "Installing CMake..."
    sudo apt-get update
    sudo apt-get install -y cmake
else
    echo "CMake already installed."
fi

# Install GCC (>=7)
if ! command -v gcc &> /dev/null; then
    echo "Installing GCC..."
    sudo apt-get update
    sudo apt-get install -y build-essential
else
    echo "GCC already installed."
fi

# Install Make
if ! command -v make &> /dev/null; then
    echo "Installing Make..."
    sudo apt-get update
    sudo apt-get install -y make
else
    echo "Make already installed."
fi

# Install OpenSSL
if ! command -v make &> /dev/null; then
    echo "Installing OpenSSL..."
    sudo apt-get update
    sudo apt-get install -y libssl-dev
else
    echo "OpenSSL already installed."
fi

# Install Python (>=3.6)
if ! command -v python3 &> /dev/null; then
    echo "Installing Python3..."
    sudo apt-get update
    sudo apt-get install -y python3 python3-pip
else
    echo "Python3 already installed."
fi

# Check Python version
PYTHON_VERSION=$(python3 -c 'import sys; print("{}.{}".format(sys.version_info[0], sys.version_info[1]))')
REQUIRED_VERSION="3.6"
if [[ "${PYTHON_VERSION}" < "${REQUIRED_VERSION}" ]]; then
    echo "Python version must be >= 3.6. Found ${PYTHON_VERSION}. Please upgrade manually."
    exit 1
fi

# Copy libraries to sgxsdk
SGX_LIB_DIR="/opt/intel/sgxsdk/lib64"
SGX_INCLUDE_DIR="/opt/intel/sgxsdk/include"

sudo cp libraries/* "$SGX_LIB_DIR"
sudo chmod 644 "$SGX_LIB_DIR"/*

sudo cp -R include/sgxssl/ "$SGX_INCLUDE_DIR"
sudo chmod -R 755 "$SGX_INCLUDE_DIR"

echo "All dependencies installed."

