#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure and build
echo "Configuring project with CMake..."
cmake ..

echo "Building project..."
make

echo "Build complete. Running application..."
echo "Defaults to Publisher mode with default channel and log_file name"
echo "--------------------------------------------------------"
./app