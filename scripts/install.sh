#!/bin/bash

# All Press C++ - Installation Script
# This script installs dependencies and builds the application

set -e

echo "=== All Press C++ Installation ==="
echo ""

# Detect OS
OS=""
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Detected OS: $OS"
echo ""

# Install dependencies based on OS
if [[ "$OS" == "macOS" ]]; then
    echo "Installing dependencies via Homebrew..."
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Install dependencies
    brew install cmake
    brew install conan
    brew install cups
    brew install imagemagick
    brew install poppler
    brew install ghostscript
    brew install sqlite3
    brew install boost
    brew install openssl
    
elif [[ "$OS" == "Linux" ]]; then
    echo "Installing dependencies via apt (Ubuntu/Debian)..."
    
    sudo apt-get update
    sudo apt-get install -y \
        cmake \
        build-essential \
        libcups2-dev \
        libcupsimage2-dev \
        libmagick++-dev \
        libpoppler-cpp-dev \
        libsqlite3-dev \
        libboost-all-dev \
        libssl-dev \
        ghostscript \
        python3-pip
    
    # Install Conan
    pip3 install conan
fi

echo ""
echo "=== Installing Conan dependencies ==="
cd "$(dirname "$0")/.."

# Configure Conan profile if needed
if [ ! -d "$HOME/.conan" ]; then
    conan profile detect --force
fi

# Set CUPS pkg-config path for macOS
if [[ "$OS" == "macOS" ]]; then
    export PKG_CONFIG_PATH="/opt/homebrew/opt/cups/lib/pkgconfig:$PKG_CONFIG_PATH"
fi

# Install dependencies
conan install . --output-folder=build --build=missing -s build_type=Release

echo ""
echo "=== Building All Press C++ ==="

# Create build directory
mkdir -p build
cd build

# Configure with CMake (use Conan's toolchain)
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
         -DCMAKE_PREFIX_PATH="$(pwd)"

# Build
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo ""
echo "=== Installation Complete ==="
echo ""
echo "To run All Press C++:"
echo "  cd build"
echo "  ./all_press_c"
echo ""
echo "Or use the dev script:"
echo "  ./scripts/dev.sh"
echo ""
