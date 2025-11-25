# Building Pi4Flasher

This document provides detailed instructions for building Pi4Flasher from source on Raspberry Pi 4.

## Prerequisites

### Hardware
- Raspberry Pi 4 Model B+ (2GB RAM or higher recommended)
- MicroSD card with Raspberry Pi OS (64-bit recommended)
- Internet connection for downloading dependencies

### Software
- Raspberry Pi OS Lite or Desktop (Bullseye or newer)
- Root/sudo access

## Quick Build

For a quick automated build:

```bash
cd Pi4Flasher

# Install dependencies
sudo bash scripts/install_deps.sh

# Build the project
mkdir build
cd build
cmake ..
make -j4

# Install (optional)
sudo make install
```

## Detailed Build Steps

### Step 1: Install System Dependencies

Update your system and install build tools:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget
```

### Step 2: Install BCM2835 Library

Pi4Flasher requires the BCM2835 C library for direct hardware access:

```bash
cd /tmp
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.73.tar.gz
tar zxvf bcm2835-1.73.tar.gz
cd bcm2835-1.73
./configure
make
sudo make check
sudo make install
sudo ldconfig
```

### Step 3: Build Pi4Flasher

```bash
cd ~/Pi4Flasher  # or wherever you cloned the repo
mkdir build
cd build
cmake ..
make -j4
```

The `-j4` flag uses all 4 CPU cores for faster compilation.

### Step 4: Test the Build

Run Pi4Flasher without hardware to verify it starts:

```bash
sudo ./pi4flasher
```

You should see initialization messages. Press Ctrl+C to exit.

### Step 5: Install System-Wide (Optional)

To install pi4flasher to `/usr/local/bin`:

```bash
sudo make install
```

Now you can run `pi4flasher` from anywhere:

```bash
sudo pi4flasher
```

## Build Options

### Debug Build

For debugging with symbols:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4
```

### Release Build (Optimized)

For maximum performance:

```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

### Cross-Compilation (Advanced)

To cross-compile from a PC for Raspberry Pi:

```bash
# Install cross-compiler
sudo apt-get install gcc-aarch64-linux-gnu

# Configure CMake for cross-compilation
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake ..
make
```

(Note: Toolchain file not included - create as needed)

## Troubleshooting Build Issues

### "bcm2835 library not found"

Make sure you've installed the BCM2835 library:

```bash
sudo ldconfig
ldconfig -p | grep bcm2835
```

If not found, reinstall the library following Step 2.

### "Permission denied" errors during build

Don't run cmake or make with sudo. Only use sudo for:
- Installing dependencies
- Running the final binary
- Installing with `make install`

### CMake version too old

Update CMake:

```bash
sudo apt-get install --only-upgrade cmake
```

Or install from snap:

```bash
sudo snap install cmake --classic
```

### Out of memory during compilation

If compilation fails due to low memory:

```bash
# Use single-threaded compilation
make -j1

# Or increase swap space
sudo dphys-swapfile swapoff
sudo nano /etc/dphys-swapfile  # Set CONF_SWAPSIZE=2048
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
```

## Cleaning the Build

To clean and rebuild:

```bash
cd build
make clean
make -j4
```

To completely reset:

```bash
rm -rf build
mkdir build
cd build
cmake ..
make -j4
```

## Installing as a System Service

After building and installing, you can set up Pi4Flasher to run automatically:

```bash
cd scripts
sudo bash setup_service.sh
```

See the [README.md](README.md) for more details on service management.

## Verifying the Build

### Check Binary Information

```bash
file ./pi4flasher
# Expected: ELF 64-bit LSB executable, ARM aarch64...

ldd ./pi4flasher
# Should show libbcm2835.so linkage
```

### Run Basic Tests

```bash
# Test GPIO initialization (requires root)
sudo ./pi4flasher --version  # Should print version

# Test with verbose output
sudo ./pi4flasher -v  # If implemented
```

## Performance Tips

1. **Overclock (Optional)**: Edit `/boot/config.txt` to increase CPU speed
   ```
   over_voltage=2
   arm_freq=1750
   ```

2. **Disable GUI**: Use Raspberry Pi OS Lite for lower overhead

3. **CPU Governor**: Set to performance mode
   ```bash
   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

## Next Steps

- Read [README.md](README.md) for usage instructions
- Review [ProjectDescription.md](ProjectDescription.md) for technical details
- Check the troubleshooting section for common issues

## Build System Details

Pi4Flasher uses CMake for build configuration:

- **Minimum CMake version**: 3.10
- **C Standard**: C11
- **Compiler flags**: `-Wall -Wextra -O2`
- **Dependencies**: bcm2835 (dynamically linked)

### Project Structure

```
Pi4Flasher/
├── CMakeLists.txt          # Main build configuration
├── src/                    # Source files
│   ├── main.c             # Application entry point
│   ├── pi4_gpio.c/h       # GPIO abstraction
│   ├── pi4_spi.c/h        # SPI abstraction
│   ├── spiex.c/h          # Xbox SPI protocol
│   ├── xbox.c/h           # NAND operations
│   ├── protocol.h         # Command definitions
│   └── pins.h             # Pin mappings
├── scripts/               # Helper scripts
└── build/                 # Build output (created by user)
```

