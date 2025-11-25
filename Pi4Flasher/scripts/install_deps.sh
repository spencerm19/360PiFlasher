#!/bin/bash
#
# Pi4Flasher Dependency Installation Script
# Installs required libraries and tools for building Pi4Flasher
#

set -e

echo "================================"
echo "Pi4Flasher Dependency Installer"
echo "================================"
echo ""

# Check if running on Raspberry Pi
if [ ! -f /proc/device-tree/model ]; then
    echo "Warning: This doesn't appear to be a Raspberry Pi"
    echo "Press Ctrl+C to cancel or Enter to continue anyway..."
    read
fi

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root (use sudo)"
    exit 1
fi

echo "Updating package lists..."
apt-get update

echo ""
echo "Installing build tools..."
apt-get install -y build-essential cmake git

echo ""
echo "Checking for bcm2835 library..."
if ! dpkg -l | grep -q libbcm2835-dev; then
    echo "Installing bcm2835 library from source..."
    
    # Download and build bcm2835 library
    cd /tmp
    wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.73.tar.gz
    tar zxvf bcm2835-1.73.tar.gz
    cd bcm2835-1.73
    ./configure
    make
    make check
    make install
    ldconfig
    cd ..
    rm -rf bcm2835-1.73 bcm2835-1.73.tar.gz
    
    echo "bcm2835 library installed successfully"
else
    echo "bcm2835 library already installed"
fi

echo ""
echo "================================"
echo "Dependencies installed successfully!"
echo "================================"
echo ""
echo "Next steps:"
echo "  1. cd to the Pi4Flasher directory"
echo "  2. Run: mkdir build && cd build"
echo "  3. Run: cmake .."
echo "  4. Run: make"
echo "  5. Run: sudo ./pi4flasher"
echo ""

