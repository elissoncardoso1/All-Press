#!/bin/bash

# Development script for All Press C++
# Builds in debug mode and runs with verbose logging

set -e

echo "=================================="
echo "  All Press C++ Development Mode"
echo "=================================="

# Create build directory
mkdir -p build
cd build

# Configure for debug
echo "Configuring for debug build..."
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
echo "Building..."
ninja

# Run with debug flags
echo "Starting server in debug mode..."
echo ""
./all_press_server --debug

