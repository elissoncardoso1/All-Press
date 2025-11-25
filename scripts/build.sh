#!/bin/bash

# Build script for All Press C++

set -e

echo "=================================="
echo "  All Press C++ Build Script"
echo "=================================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if build directory exists
if [ -d "build" ]; then
    echo -e "${YELLOW}Build directory exists. Cleaning...${NC}"
    rm -rf build
fi

# Create build directory
echo -e "${GREEN}Creating build directory...${NC}"
mkdir -p build
cd build

# Check for Conan
if command -v conan &> /dev/null; then
    echo -e "${GREEN}Installing dependencies with Conan...${NC}"
    conan install .. --build=missing
else
    echo -e "${YELLOW}Conan not found. Skipping dependency installation.${NC}"
    echo -e "${YELLOW}Install Conan with: pip install conan${NC}"
fi

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
if command -v ninja &> /dev/null; then
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
    BUILD_TOOL="ninja"
else
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
    BUILD_TOOL="make"
fi

# Build
echo -e "${GREEN}Building All Press C++...${NC}"
if [ "$BUILD_TOOL" = "ninja" ]; then
    ninja
else
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

# Success
echo -e "${GREEN}=================================="
echo -e "  Build completed successfully!"
echo -e "==================================${NC}"
echo ""
echo -e "Run the server with: ${GREEN}./build/all_press_server${NC}"
echo ""
