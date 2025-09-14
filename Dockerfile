# Use Ubuntu 22.04 as base image
FROM ubuntu:22.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && \
    apt-get install -y sudo git wget unzip build-essential cmake python3 python3-pip && \
    rm -rf /var/lib/apt/lists/*

# Create a working directory
WORKDIR /app

# Clone the repository from GitHub
RUN git clone https://github.com/PhamHung2020/Simple-SGX-based-DSP-Engine.git

# Change to the repository directory
WORKDIR /app/Simple-SGX-based-DSP-Engine

# Make install.sh executable and run it with admin privileges
RUN chmod +x install.sh && sudo ./install.sh

# Build the tool using make
RUN make

# Download source_data.zip and unzip it inside the repository
RUN wget -O source_data.zip https://github.com/PhamHung2020/Simple-SGX-based-DSP-Engine/releases/download/data_source/source_data.zip && \
    unzip source_data.zip && \
    rm source_data.zip

# Set default command (can be changed as needed)
CMD ["/bin/bash"]
