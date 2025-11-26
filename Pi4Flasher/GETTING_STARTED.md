# Getting Started with Pi4Flasher

This guide walks you through the complete setup process once you have the Pi4Flasher directory on your Raspberry Pi 4.

## Prerequisites

- Raspberry Pi 4 Model B+
- Raspberry Pi OS installed and running
- Internet connection (for downloading dependencies)
- SSH access or direct terminal access
- Pi4Flasher directory copied to home folder (`~/Pi4Flasher`)

---

## Initial Setup (Software Only)

### Step 1: SSH into Your Pi and Navigate

```bash
ssh pi@raspberrypi
cd ~/Pi4Flasher
```

If you're already on the Pi, just navigate to the directory:
```bash
cd ~/Pi4Flasher
```

### Step 2: Install Dependencies (~5 minutes)

```bash
sudo bash scripts/install_deps.sh
```

**What this does:**
- Updates package lists
- Installs build tools (gcc, cmake, make)
- Downloads and compiles the BCM2835 library from source
- Configures library paths with ldconfig

**Wait for completion** - you'll see:
```
================================
Dependencies installed successfully!
================================
```

**If installation fails:**
- Check internet connection
- Ensure you have sudo privileges
- Try running again (script is idempotent)

### Step 3: Build Pi4Flasher (~1 minute)

```bash
mkdir build
cd build
cmake ..
make -j4
```

**Expected successful output:**
```
-- Pi4Flasher build configuration:
--   C Compiler: /usr/bin/gcc
--   C Flags: -Wall -Wextra -O2
--   BCM2835 Library: /usr/local/lib/libbcm2835.a
[ 16%] Building C object CMakeFiles/pi4flasher.dir/src/main.c.o
[ 33%] Building C object CMakeFiles/pi4flasher.dir/src/pi4_gpio.c.o
[ 50%] Building C object CMakeFiles/pi4flasher.dir/src/pi4_spi.c.o
[ 66%] Building C object CMakeFiles/pi4flasher.dir/src/spiex.c.o
[ 83%] Building C object CMakeFiles/pi4flasher.dir/src/xbox.c.o
[100%] Linking C executable pi4flasher
[100%] Built target pi4flasher
```

**If build fails:**
- Check `install_deps.sh` completed successfully
- Run `sudo ldconfig` and try again
- See [BUILDING.md](BUILDING.md) for troubleshooting

### Step 4: Test the Build (No Hardware Needed)

Test that the binary was created and runs:

```bash
sudo ./pi4flasher
```

**Expected output (this is GOOD):**
```
Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
Version: 4

Error opening /dev/ttyAMA0: No such file or directory
```

This error is **expected** if you haven't configured the serial port yet. The important part is that the program starts and shows the version.

Press `Ctrl+C` to exit.

**Alternative test with a valid serial port:**
```bash
sudo ./pi4flasher /dev/null
```

This should initialize without serial errors (though it won't be functional).

### Step 5: Configure Serial Port

#### Option A: Using Hardware UART (Recommended)

Enable the hardware UART on Raspberry Pi:

```bash
sudo raspi-config
```

Navigate through the menu:
1. Select **"Interface Options"**
2. Select **"Serial Port"**
3. "Would you like a login shell accessible over serial?" → Select **NO**
4. "Would you like the serial port hardware enabled?" → Select **YES**
5. Select **"Finish"**
6. Reboot when prompted: **"Yes"**

After reboot, verify the serial port exists:
```bash
ls -l /dev/ttyAMA0
```

Should show: `crw-rw---- 1 root dialout ...`

#### Option B: Using USB-Serial Adapter

If using a USB-serial adapter:

1. Plug in the USB-serial adapter
2. Check which device it created:
   ```bash
   ls /dev/ttyUSB*
   ```
3. Use that device when running pi4flasher (e.g., `/dev/ttyUSB0`)

#### Add User to Dialout Group (Optional)

To run without sudo (not recommended for GPIO access, but useful for testing):

```bash
sudo usermod -a -G dialout $USER
```

Logout and login for changes to take effect.

### Step 6: Install System-Wide (Optional)

After building, you can install pi4flasher to `/usr/local/bin`:

```bash
cd ~/Pi4Flasher/build
sudo make install
```

Now you can run from anywhere:
```bash
sudo pi4flasher /dev/ttyAMA0
```

### Step 7: Test Software Initialization

Run Pi4Flasher to verify all software components initialize correctly:

```bash
cd ~/Pi4Flasher/build
sudo ./pi4flasher /dev/ttyAMA0
```

**Expected successful output:**
```
Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
Version: 4

Serial port /dev/ttyAMA0 opened successfully
Xbox NAND interface initialized
SPIEX initialized for Xbox NAND communication
Pi4Flasher ready. Waiting for commands...
```

**✅ If you see this, the software is working correctly!**

Press `Ctrl+C` to exit gracefully.

### Step 8: Verify GPIO Initialization (Optional)

With Pi4Flasher running in one terminal, open another SSH session and check GPIO states:

```bash
# Install gpio utility if not present
sudo apt-get install -y wiringpi

# View all GPIO pins
gpio readall
```

**Expected GPIO states:**

| Physical Pin | BCM GPIO | Name | Mode | Value |
|-------------|----------|------|------|-------|
| Pin 3 | GPIO 2 | SMC_RST_XDK_N | OUT | HIGH (1) |
| Pin 5 | GPIO 3 | SMC_DBG_EN | OUT | HIGH (1) |
| Pin 24 | GPIO 8 | SPI_SS_N | OUT | HIGH (1) |
| Pin 19 | GPIO 10 | SPI_MOSI | ALT0 | - |
| Pin 21 | GPIO 9 | SPI_MISO | ALT0 | - |
| Pin 23 | GPIO 11 | SPI_CLK | ALT0 | - |

**With a multimeter** (even better), measure voltage between these pins and GND:
- GPIO 2: Should read ~3.3V
- GPIO 3: Should read ~3.3V  
- GPIO 8: Should read ~3.3V

---

## Hardware Connection (Xbox 360 Falcon)

### ⚠️ CRITICAL SAFETY REQUIREMENTS

**Before connecting any hardware, you MUST have:**

1. ✅ **3.3V Bidirectional Logic Level Shifter**
   - Examples: SparkFun BOB-12009, Adafruit 757, TXS0108E module
   - Must support bidirectional communication
   - Both LV (Low Voltage) and HV (High Voltage) sides at 3.3V
   - **NEVER connect Pi directly to Xbox - WILL DAMAGE CONSOLE!**

2. ✅ **Quality Jumper Wires**
   - 24-26 AWG recommended
   - Keep wires short (< 6 inches ideal)
   - Use different colors for identification
   - Ensure solid connections

3. ✅ **Anti-Static Protection**
   - ESD wrist strap connected to ground
   - Anti-static work mat (recommended)
   - Touch grounded metal before handling components

4. ✅ **Multimeter for Verification**
   - Test voltages before connecting
   - Check for shorts
   - Verify ground connections

5. ✅ **Xbox 360 Backup** (if possible)
   - If you have another working flasher, backup first
   - Keep multiple copies
   - This is your safety net!

### Hardware Setup Steps

#### Step 1: Power Everything OFF

- Power off Raspberry Pi
- Unplug Xbox 360 from AC power
- Wait 30 seconds for capacitors to discharge

#### Step 2: Identify Xbox 360 Falcon NAND Points

Refer to the [Xbox 360 RGH Guide](https://xbox360hub.com/guides/rgh-3-guide/) for:
- Visual solder point locations on Falcon motherboard
- Proper soldering techniques
- NAND pinout confirmation

**Falcon NAND signals you need:**
- SPI_MOSI (Data Out)
- SPI_MISO (Data In)
- SPI_CLK (Clock)
- SPI_SS_N (Chip Select)
- SMC_RST_XDK_N (Reset)
- SMC_DBG_EN (Debug Enable)
- GND (Ground)

#### Step 3: Connect Level Shifter to Raspberry Pi

**Level Shifter LV (Low Voltage) Side → Pi 4:**

| Level Shifter LV | Pi 4 Physical Pin | BCM GPIO | Signal Name |
|-----------------|-------------------|----------|-------------|
| LV1 | Pin 19 | GPIO 10 | SPI_MOSI |
| LV2 | Pin 21 | GPIO 9 | SPI_MISO |
| LV3 | Pin 23 | GPIO 11 | SPI_CLK |
| LV4 | Pin 24 | GPIO 8 | SPI_SS_N |
| LV5 | Pin 3 | GPIO 2 | SMC_RST_XDK_N |
| LV6 | Pin 5 | GPIO 3 | SMC_DBG_EN |
| LV (VCC) | Pin 1 | 3.3V | Power for LV side |
| GND | Pin 6 | GND | Ground |

#### Step 4: Connect Level Shifter to Xbox 360

**Level Shifter HV (High Voltage) Side → Xbox 360 Falcon:**

| Level Shifter HV | Xbox 360 Signal | Notes |
|-----------------|-----------------|-------|
| HV1 | SPI_MOSI | Refer to RGH guide for location |
| HV2 | SPI_MISO | Refer to RGH guide for location |
| HV3 | SPI_CLK | Refer to RGH guide for location |
| HV4 | SPI_SS_N | Refer to RGH guide for location |
| HV5 | SMC_RST_XDK_N | Refer to RGH guide for location |
| HV6 | SMC_DBG_EN | Refer to RGH guide for location |
| HV (VCC) | 3.3V source | From Xbox or external |
| GND | GND | Common ground |

#### Step 5: Verify Connections

**Before applying power:**

1. **Visual Inspection**
   - Check all connections are secure
   - Verify no crossed wires
   - Ensure proper color coding matches

2. **Continuity Test** (with multimeter, power OFF)
   - Test continuity from Pi GPIO to Level Shifter LV
   - Test continuity from Level Shifter HV to Xbox points
   - Verify GND is common across all components

3. **Short Circuit Check**
   - Check no shorts between adjacent pins
   - Verify 3.3V doesn't short to GND
   - Check signal lines don't short to power/ground

#### Step 6: Power-Up Sequence

**IMPORTANT: Follow this order!**

1. Power on Raspberry Pi
2. Wait for Pi to fully boot
3. Start Pi4Flasher: `sudo pi4flasher /dev/ttyAMA0`
4. Verify initialization messages
5. Power on Xbox 360 (if needed - some RGH setups don't need full power)

### First Hardware Test

#### Test 1: Read Flash Configuration

From Pi4Flasher terminal or a second SSH session:

```bash
# Send GET_FLASH_CONFIG command (0x01)
echo -ne '\x01\x00\x00\x00\x00' > /dev/ttyAMA0

# Read response (4 bytes)
dd if=/dev/ttyAMA0 bs=4 count=1 2>/dev/null | xxd
```

**Expected output** (example, your values will vary):
```
00000000: 0044 3203                             .D2.
```

**What this means:**
- Non-zero response = SUCCESS! NAND is communicating
- All zeros or no response = Check connections
- Response format: 32-bit flash configuration register

**Common flash config values:**
- `0x00443203` - Falcon 16MB NAND (common)
- Values vary by NAND type and motherboard revision

#### Test 2: Read Single NAND Block

```bash
# Send READ_FLASH command (0x02) for block 0
echo -ne '\x02\x00\x00\x00\x00' > /dev/ttyAMA0

# Read response (532 bytes: 4 status + 512 data + 16 spare)
dd if=/dev/ttyAMA0 bs=532 count=1 2>/dev/null | xxd | head -20
```

**Expected output:**
- First 4 bytes: `00 00 00 00` (status = success)
- Next 512 bytes: NAND data (should NOT be all 0x00 or all 0xFF)
- Last 16 bytes: Spare/ECC data

**Success indicators:**
- Status is 0x00000000
- Data looks reasonable (mix of values)
- Consistent on repeated reads

#### Test 3: Stability Test

Read the same block 10 times and verify consistency:

```bash
for i in {1..10}; do
  echo "Read $i:"
  echo -ne '\x02\x00\x00\x00\x00' > /dev/ttyAMA0
  dd if=/dev/ttyAMA0 bs=532 count=1 2>/dev/null | md5sum
  sleep 0.5
done
```

**Expected result:**
- All 10 reads should have **identical MD5 checksums**
- If checksums vary, you have connection/timing issues

---

## Using with J-Runner (Recommended Method)

### Setup J-Runner Connection

#### Step 1: Connect PC to Raspberry Pi Serial

**Option A: USB-Serial Adapter**

1. Connect USB-serial adapter to Pi's GPIO UART pins:
   - Adapter TX → Pi GPIO 14 (Pin 8)
   - Adapter RX → Pi GPIO 15 (Pin 10)
   - Adapter GND → Pi GND (Pin 6)

2. Plug USB-serial adapter into Windows PC

3. Open Device Manager and note the COM port (e.g., COM3)

**Option B: Direct USB Connection**

Some USB-serial adapters can connect directly from PC to Pi's USB ports with appropriate drivers.

#### Step 2: Start Pi4Flasher on Pi

```bash
sudo pi4flasher /dev/ttyAMA0
```

Keep this running in the background.

#### Step 3: Configure J-Runner

1. Open J-Runner on Windows PC
2. Go to **Tools** → **Options** → **Programmer**
3. Settings:
   - Programmer Type: **Serial Programmer**
   - COM Port: Select your COM port (e.g., COM3)
   - Baud Rate: **115200**
   - Data Bits: **8**
   - Parity: **None**
   - Stop Bits: **1**
4. Click **Apply** and **OK**

#### Step 4: Test Connection

1. In J-Runner, click **"Read NAND Info"** or **"Get Version"**
2. Should detect Pi4Flasher and show version 4

#### Step 5: Read Full NAND Dump

1. Click **"Read NAND"**
2. Choose save location (e.g., `falcon_nand_backup.bin`)
3. Click **Start**
4. Progress bar will show read progress
5. **Expected time: 3-5 minutes for 16MB**

**Monitor Pi4Flasher terminal** for progress messages.

#### Step 6: Verify NAND Dump

1. J-Runner will automatically verify CRC
2. Check for bad blocks (some bad blocks are normal)
3. Save this dump as your backup!

#### Step 7: Read Again and Compare

1. Read NAND a second time to different file
2. Compare the two dumps:
   ```bash
   # On Windows in PowerShell
   Get-FileHash falcon_nand_backup1.bin
   Get-FileHash falcon_nand_backup2.bin
   ```
3. Hashes should be **identical**

---

## Setup as System Service (Optional)

To run Pi4Flasher automatically on boot:

```bash
cd ~/Pi4Flasher/scripts
sudo bash setup_service.sh
```

**Service Management Commands:**

```bash
# Start service
sudo systemctl start pi4flasher

# Stop service  
sudo systemctl stop pi4flasher

# Restart service
sudo systemctl restart pi4flasher

# Check status
sudo systemctl status pi4flasher

# View live logs
sudo journalctl -u pi4flasher -f

# Enable auto-start on boot
sudo systemctl enable pi4flasher

# Disable auto-start
sudo systemctl disable pi4flasher
```

**Benefits of running as service:**
- Automatic start on boot
- Real-time scheduling priority (better timing)
- Automatic restart on failure
- Centralized logging

---

## Troubleshooting

### Build Issues

**Error: "bcm2835 library not found"**

Solution:
```bash
sudo ldconfig
ldconfig -p | grep bcm2835

# If not found, reinstall:
cd ~/Pi4Flasher
sudo bash scripts/install_deps.sh
```

**Error: "CMake version too old"**

Solution:
```bash
sudo apt-get update
sudo apt-get install --only-upgrade cmake
cmake --version  # Should be 3.10 or higher
```

**Error: "Permission denied" during build**

Solution:
```bash
# Don't use sudo for build, only for running
cd ~/Pi4Flasher/build
make clean
make -j4
```

### Runtime Issues

**Error: "Failed to initialize GPIO"**

Solution:
```bash
# Must run with sudo
sudo ./pi4flasher /dev/ttyAMA0
```

**Error: "Failed to open serial port"**

Solutions:
```bash
# Check if device exists
ls -l /dev/ttyAMA0

# If using USB-serial:
ls -l /dev/ttyUSB*

# Add user to dialout group
sudo usermod -a -G dialout $USER
# Logout and login again

# Disable serial console (if using hardware UART)
sudo raspi-config
# Interface Options → Serial Port → Disable login shell, Enable hardware
```

**Error: "Serial port /dev/ttyAMA0: Device or resource busy"**

Solution:
```bash
# Check what's using the port
sudo lsof /dev/ttyAMA0

# Kill the process or disable it
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
```

### Hardware Issues

**Problem: Flash config reads as all zeros (0x00000000)**

Possible causes:
- Level shifter not powered
- Incorrect wiring
- Poor solder connections on Xbox
- Xbox not powered

Debugging steps:
```bash
# Check Pi GPIO voltages (with pi4flasher running)
gpio readall

# Check with multimeter:
# - GPIO 2 should be 3.3V
# - GPIO 3 should be 3.3V
# - GPIO 8 should be 3.3V

# Check level shifter outputs with multimeter
# Verify signal integrity with oscilloscope/logic analyzer if available
```

**Problem: Inconsistent reads (different data each time)**

Possible causes:
- Loose connections
- Interference on wires
- Power supply noise
- OS jitter (timing issues)

Solutions:
```bash
# Use real-time priority
sudo chrt -f 99 ./pi4flasher /dev/ttyAMA0

# Check connections
# - Ensure wires are short and secure
# - Add decoupling capacitors near Xbox NAND
# - Keep wires away from power supplies

# Reduce SPI clock speed (edit src/pi4_spi.c)
# Change from 28MHz to 15MHz or lower
```

**Problem: Read errors (non-zero status codes)**

Debugging:
```bash
# Watch pi4flasher output for error details
# Status code format: 0x8000 | NAND_STATUS

# Common NAND status codes:
# 0x01 = Busy (shouldn't see this)
# 0x02 = Error bit set
# 0x04 = Write protect

# Check Xbox NAND power and connections
```

### J-Runner Issues

**Problem: J-Runner doesn't detect programmer**

Solutions:
- Verify COM port in Windows Device Manager
- Check baud rate is 115200 in J-Runner
- Ensure Pi4Flasher is running: `sudo systemctl status pi4flasher`
- Test serial loopback (connect TX to RX on adapter)
- Try different USB-serial adapter
- Check USB cable quality

**Problem: Read operation times out**

Solutions:
- Check Pi4Flasher is running and not crashed
- Verify serial connection is stable
- Check for USB disconnections in Device Manager
- Increase timeout in J-Runner settings (if available)
- Monitor Pi CPU temperature (add heatsinks if throttling)

**Problem: Read succeeds but verification fails**

Causes:
- Data corruption during transfer
- Bad NAND blocks (some are normal)
- Connection issues

Solutions:
- Read again and compare
- Check Pi4Flasher logs for errors
- Verify all hardware connections
- Some bad blocks are normal in used NANDs

---

## Performance Optimization

### Enable Real-Time Scheduling

For better timing and reduced jitter:

```bash
# Run with real-time priority (requires systemd service)
sudo systemctl edit pi4flasher

# Add these lines:
[Service]
CPUSchedulingPolicy=fifo
CPUSchedulingPriority=99
```

Or run manually:
```bash
sudo chrt -f 99 ./pi4flasher /dev/ttyAMA0
```

### CPU Isolation (Advanced)

To dedicate a CPU core to Pi4Flasher:

```bash
# Edit boot config
sudo nano /boot/cmdline.txt

# Add to the end of the line (no newlines):
isolcpus=3

# Reboot
sudo reboot

# Run pi4flasher pinned to isolated core
sudo taskset -c 3 chrt -f 99 ./pi4flasher /dev/ttyAMA0
```

### Disable CPU Frequency Scaling

For consistent performance:

```bash
# Set CPU governor to performance
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Make permanent (add to /etc/rc.local):
sudo nano /etc/rc.local
# Add before "exit 0":
echo performance > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

---

## Quick Reference Commands

```bash
# Build
cd ~/Pi4Flasher/build
cmake .. && make -j4

# Run (manual)
sudo ./pi4flasher /dev/ttyAMA0

# Run with real-time priority
sudo chrt -f 99 ./pi4flasher /dev/ttyAMA0

# Install system-wide
sudo make install

# Setup service
cd ~/Pi4Flasher/scripts
sudo bash setup_service.sh

# Service management
sudo systemctl start pi4flasher
sudo systemctl stop pi4flasher
sudo systemctl status pi4flasher
sudo journalctl -u pi4flasher -f

# Test flash config read
echo -ne '\x01\x00\x00\x00\x00' > /dev/ttyAMA0
dd if=/dev/ttyAMA0 bs=4 count=1 2>/dev/null | xxd

# Check GPIO status
gpio readall
```

---

## Safety Checklist

Before connecting to Xbox 360, verify:

- [ ] Level shifter installed and powered
- [ ] All connections verified with continuity test
- [ ] No short circuits detected
- [ ] Voltages measured and correct (3.3V)
- [ ] Pi4Flasher software tested and working
- [ ] ESD protection in place
- [ ] Backup NAND available (if possible)
- [ ] Working on test console (recommended)
- [ ] All wires secure and labeled
- [ ] Power-up sequence planned

---

## Next Steps

1. ✅ Complete software setup (Steps 1-8 above)
2. ✅ Verify GPIO initialization
3. ✅ Acquire level shifter and wiring materials
4. ✅ Prepare Xbox 360 Falcon motherboard
5. ✅ Connect hardware following safety procedures
6. ✅ Test flash config read
7. ✅ Verify read stability
8. ✅ Setup J-Runner connection
9. ✅ Perform full NAND dump
10. ✅ Verify and backup NAND dumps

---

## Additional Resources

- **[README.md](README.md)** - Main documentation and feature overview
- **[BUILDING.md](BUILDING.md)** - Detailed build instructions
- **[TESTING.md](TESTING.md)** - Comprehensive testing procedures
- **[ProjectDescription.md](ProjectDescription.md)** - Technical specifications
- **[QUICKSTART.md](QUICKSTART.md)** - Quick 5-minute reference

---

## Support

If you encounter issues not covered here:

1. Check [TESTING.md](TESTING.md) troubleshooting section
2. Review system logs: `sudo journalctl -u pi4flasher -n 100`
3. Verify hardware with multimeter
4. Review [Xbox 360 RGH Guide](https://xbox360hub.com/guides/rgh-3-guide/)
5. Check PicoFlasher documentation for protocol details

---

**You're now ready to use Pi4Flasher!** 🎮🔧

Good luck with your Xbox 360 NAND operations. Remember to always prioritize safety and backup your data!

