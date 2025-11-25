# Pi4Flasher Quick Start Guide

Get up and running with Pi4Flasher in 5 minutes!

## Prerequisites

- Raspberry Pi 4 Model B+
- Raspberry Pi OS installed
- Root access (sudo)
- Internet connection

## Installation (5 Steps)

### Step 1: Install Dependencies

```bash
cd Pi4Flasher
sudo bash scripts/install_deps.sh
```

**Time**: ~5 minutes

### Step 2: Build

```bash
mkdir build && cd build
cmake ..
make -j4
```

**Time**: ~1 minute

### Step 3: Install

```bash
sudo make install
```

### Step 4: Setup Hardware

Connect your Pi 4 to Xbox 360 Falcon NAND:

| Pi 4 Pin | Xbox Signal |
|----------|-------------|
| Pin 19 (GPIO 10) | SPI_MOSI |
| Pin 21 (GPIO 9) | SPI_MISO |
| Pin 23 (GPIO 11) | SPI_CLK |
| Pin 24 (GPIO 8) | SPI_SS_N |
| Pin 3 (GPIO 2) | SMC_RST_XDK_N |
| Pin 5 (GPIO 3) | SMC_DBG_EN |
| Pin 6 (GND) | GND |

**⚠️ CRITICAL**: Use a 3.3V level shifter between Pi and Xbox!

### Step 5: Run

```bash
sudo pi4flasher /dev/ttyAMA0
```

## Using with J-Runner

1. Connect PC to Pi's serial port
2. Start pi4flasher on Pi
3. Open J-Runner on PC
4. Select the serial port
5. Use J-Runner normally to read/write NAND

## Troubleshooting

### "Failed to initialize GPIO"
→ Run with `sudo`

### "Failed to initialize serial port"
→ Check device path: `ls /dev/tty*`

### Read errors
→ Check wire connections and level shifter

## What's Next?

- **Full Documentation**: See [README.md](README.md)
- **Build Details**: See [BUILDING.md](BUILDING.md)
- **Testing Guide**: See [TESTING.md](TESTING.md)
- **Technical Details**: See [ProjectDescription.md](ProjectDescription.md)

## Safety Checklist

Before powering on:

- [ ] Level shifter installed
- [ ] All connections verified
- [ ] NAND backup exists (if possible)
- [ ] Using test console (recommended)
- [ ] ESD protection in place

## Support

For issues, check:
1. [TESTING.md](TESTING.md) troubleshooting section
2. [README.md](README.md) for detailed help
3. Verify all connections
4. Check system logs: `sudo journalctl -u pi4flasher`

---

**Ready to Flash!** 🚀

