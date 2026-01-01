#!/bin/bash

# Create build directory
mkdir -p build
cd build

# Configure with tests enabled
echo "Configuring project with CMake (tests enabled)..."
cmake -DBUILD_TESTS=ON ..

echo "Building project and tests..."
make

echo "Running unit tests..."
echo "========================================="
./run_tests

echo ""
echo "Test execution complete."