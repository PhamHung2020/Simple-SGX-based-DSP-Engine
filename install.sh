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
#if ! command -v make &> /dev/null; then
echo "Installing OpenSSL..."
sudo apt-get update
sudo apt-get install -y libssl-dev
#else
#    echo "OpenSSL already installed."
#fi

# Install Python3.12
if ! command -v python3.12 &> /dev/null; then
    echo "Installing Python3.12..."
    sudo apt update && sudo apt upgrade -y
    sudo add-apt-repository ppa:deadsnakes/ppa
    sudo apt update
    sudo apt install python3.12 python3.12-distutils python3.12-venv
else
    echo "Python3.12 already installed."
fi

# Copy libraries to sgxsdk
echo "Copying necessary libraries to SGX SDK..."
sudo cp libraries/* /opt/intel/sgxsdk/lib64
sudo cp -R include/sgxssl/ /opt/intel/sgxsdk/include
echo "Libraries copied."

echo "All dependencies installed."

