# Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4 B+

Pi4Flasher is a port of the [PicoFlasher](https://github.com/X360Tools/PicoFlasher) Xbox 360 NAND reader/writer to the Raspberry Pi 4 Model B+. It enables reading and writing NAND flash memory from Xbox 360 Falcon motherboards for modding and repair purposes.

## Features

- Full NAND read/write support for Xbox 360 Falcon motherboards (16MB NAND)
- Compatible with J-Runner host software
- Hardware SPI for reliable high-speed communication
- Direct BCM2711 register access for minimal OS jitter
- Serial protocol compatible with PicoFlasher

## Hardware Requirements

- **Raspberry Pi 4 Model B+** (or compatible)
- **Xbox 360 Falcon Motherboard** (16MB NAND)
- **3.3V Logic Level Converter** (MANDATORY - protects console from damage)
- **USB-Serial adapter** or use built-in UART (`/dev/ttyAMA0`)
- Jumper wires for GPIO connections

## Pin Connections

Connect the Raspberry Pi 4 GPIO pins to the Xbox 360 Falcon motherboard NAND signals:

| Falcon NAND Signal | Pi 4 Physical Pin | BCM GPIO | Function |
|-------------------|-------------------|----------|----------|
| SPI_MOSI (Data Out) | Pin 19 | GPIO 10 | Hardware SPI0 MOSI |
| SPI_MISO (Data In) | Pin 21 | GPIO 9 | Hardware SPI0 MISO |
| SPI_CLK (Clock) | Pin 23 | GPIO 11 | Hardware SPI0 SCK |
| SPI_SS_N (Chip Select) | Pin 24 | GPIO 8 | Hardware SPI0 CE0 |
| SMC_RST_XDK_N (Reset) | Pin 3 | GPIO 2 | Control Signal |
| SMC_DBG_EN (Debug Enable) | Pin 5 | GPIO 3 | Control Signal |
| GND | Pin 6 (or any GND) | GND | Ground |

**⚠️ CRITICAL:** Always use a 3.3V logic level converter between the Pi and the Xbox 360 motherboard to prevent damage!

## Installation

### 1. Install Dependencies

Run the dependency installation script:

```bash
cd Pi4Flasher
sudo bash scripts/install_deps.sh
```

This will install:
- Build tools (gcc, cmake, make)
- BCM2835 library for GPIO/SPI access

### 2. Build Pi4Flasher

```bash
mkdir build
cd build
cmake ..
make
```

### 3. Install (Optional)

```bash
sudo make install
```

This installs `pi4flasher` to `/usr/local/bin`.

## Usage

### Running Pi4Flasher

Pi4Flasher requires root privileges to access `/dev/mem` for direct hardware control:

```bash
sudo ./pi4flasher [serial_device]
```

Examples:
```bash
# Use hardware UART (default)
sudo ./pi4flasher /dev/ttyAMA0

# Use USB-serial adapter
sudo ./pi4flasher /dev/ttyUSB0
```

### Connecting with J-Runner

1. Connect your PC to the Raspberry Pi's serial port
2. Start Pi4Flasher on the Raspberry Pi
3. Open J-Runner on your PC
4. Select the serial port connected to the Pi
5. Use J-Runner's NAND read/write functions as normal

## Command Protocol

Pi4Flasher implements the PicoFlasher command protocol:

| Command | Code | Description |
|---------|------|-------------|
| `GET_VERSION` | 0x00 | Get firmware version |
| `GET_FLASH_CONFIG` | 0x01 | Read NAND configuration |
| `READ_FLASH` | 0x02 | Read single NAND block (512 bytes + 16 spare) |
| `WRITE_FLASH` | 0x03 | Write single NAND block |
| `READ_FLASH_STREAM` | 0x04 | Stream read multiple blocks |

## Troubleshooting

### "Failed to initialize GPIO"

- Make sure you're running with `sudo`
- Check that BCM2835 library is properly installed

### "Failed to initialize serial port"

- Verify the serial device path (check `ls /dev/tty*`)
- Ensure the serial port isn't being used by another process
- For hardware UART, disable console on serial in `raspi-config`

### NAND Read Errors

- Check all wire connections
- Verify you're using a logic level converter
- Ensure the Xbox 360 is powered appropriately
- Try lowering the SPI clock speed in `pi4_spi.c`

### OS Jitter / Timing Issues

If you experience data corruption:

1. **Increase process priority:**
   ```bash
   sudo chrt -f 99 ./pi4flasher
   ```

2. **Isolate CPU core:**
   Add `isolcpus=3` to `/boot/cmdline.txt` and reboot

3. **Install RT_PREEMPT kernel** (advanced):
   See [Real-Time Linux guide](https://wiki.linuxfoundation.org/realtime/start)

## Performance

Expected performance on Raspberry Pi 4:
- Single block read: ~1-2 ms
- Full 16MB NAND dump: ~3-5 minutes
- SPI clock: 28 MHz (adjustable)

## Technical Details

Pi4Flasher uses:
- **BCM2835 library** for direct hardware register access
- **Hardware SPI0** peripheral with DMA for reliable timing
- **POSIX serial I/O** for J-Runner communication
- **LSB-first bit ordering** (via lookup table) for Falcon compatibility

## License

This program is free software; you can redistribute it and/or modify it under the terms and conditions of the GNU General Public License, version 2.

Original PicoFlasher by Balázs Triszka <balika011@gmail.com>

## Credits

- **PicoFlasher**: Balázs Triszka (balika011)
- **Pi4Flasher Port**: Based on the technical plan and porting effort
- **BCM2835 Library**: Mike McCauley

## References

- [PicoFlasher Source Code](https://github.com/X360Tools/PicoFlasher)
- [BCM2711 Peripherals Datasheet](https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf)
- [pigpio Library Documentation](http://abyz.me.uk/rpi/pigpio/)
- [Xbox 360 RGH Guide](https://xbox360hub.com/guides/rgh-3-guide/)

## Safety Warning

⚠️ **IMPORTANT SAFETY NOTES:**

1. Always use proper ESD protection when working with electronics
2. The 3.3V logic level converter is MANDATORY
3. Double-check all connections before powering on
4. Backup your NAND before attempting writes
5. Modifying Xbox 360 firmware may void warranties and violate terms of service

## Support

For issues, questions, or contributions:
- Review the troubleshooting section
- Check the PicoFlasher documentation
- Ensure all hardware connections are correct
- Verify BCM2835 library installation

