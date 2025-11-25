# 🛠️ Technical Project Plan: PicoFlasher Port to Raspberry Pi 4 B+

**Target Console:** Xbox 360 Phat (Falcon Motherboard, 16MB NAND)  
**Target Flasher:** Raspberry Pi 4 Model B+ (BCM2711 SoC)  
**Source Code Reference:** [X360Tools/PicoFlasher](https://github.com/X360Tools/PicoFlasher)  
**Host Software:** J-Runner (expects a serial programmer)

---

## 1. 🎯 Project Goal & Core Technical Challenge

The goal is to emulate the high-speed, dedicated I/O of the RP2040 (Pico) on the Raspberry Pi 4 running Linux. The key challenge is **OS Jitter**, where the Linux kernel's non-real-time nature interrupts critical GPIO timing sequences, leading to NAND read/write corruption.

**Solution:** Bypass standard Linux I/O (`sysfs`, `RPi.GPIO`) and use **Direct Memory Access (DMA)** or **Hardware Timer/PWM** techniques (via libraries like `pigpio` or `bcm2835`) to drive the BCM2711 GPIO pins with microsecond precision.

---

## 2. ⚙️ Phase 1: Environment Setup & Hardware Mapping

### 2.1. Pi 4 Host System Requirements

| Task | Detail | Rationale |
|------|--------|-----------|
| **Operating System** | Raspberry Pi OS Lite (64-bit) | Minimize OS overhead and background services. |
| **I/O Library** | `pigpio` or `bcm2835` C Library | These libraries communicate directly with the BCM2711's GPIO and clock registers for high-speed control. |
| **Real-Time Patching** | Optional but recommended: `PREEMPT_RT` kernel patch. | Reduces the worst-case timing latency caused by the scheduler. |
| **Level Shifter (CRITICAL)** | MANDATORY: Use a 3.3V logic level converter. | Protects the Falcon NAND, which operates at 3.3V, from the Pi 4's 3.3V logic (though technically both are 3.3V, a buffer is recommended to protect the sensitive console signals). |

### 2.2. Hardware Mapping: Falcon NAND to Pi 4 GPIO

We target the BCM2711's **Hardware SPI0** pins for the highest possible speed.

| Falcon NAND Signal | Pico Flasher Pin | Pi 4 B+ GPIO Pin | BCM Pin Number | Function |
|-------------------|------------------|------------------|----------------|----------|
| SPI_MOSI (Data Out) | GP19 | Pin 19 | GPIO 10 | Hardware SPI0 MOSI |
| SPI_MISO (Data In) | GP16 | Pin 21 | GPIO 9 | Hardware SPI0 MISO |
| SPI_CLK (Clock) | GP18 | Pin 23 | GPIO 11 | Hardware SPI0 SCK |
| SPI_SS_N (Chip Select) | GP17 | Pin 24 | GPIO 8 | Hardware SPI0 CE0 |
| SMC_RST_XDK_N (Reset) | GP21 | Pin 3 | GPIO 2 | Control Signal |
| SMC_DBG_EN (Debug Enable) | GP20 | Pin 5 | GPIO 3 | Control Signal |

---

## 3. 💻 Phase 2: Code Porting and Low-Level Driver Implementation

The primary objective is to replace the PicoFlasher's `pio_spi.c` and associated low-level functions with BCM2711-specific code.

### 3.1. I/O Driver Conversion (Replacing `pio_spi.c`)

The Pi 4 must generate a clock signal and perform synchronized data transfer.

#### Direct Register Access (Hardcore Method):
- Map the BCM2711 peripheral base address (`0xFE000000`) into user space memory using `/dev/mem`.
- Manually implement bit-banging functions by writing directly to the GPIO set/clear registers.
- **Risk:** Highly complex and prone to system instability.

#### Hardware SPI Driver (Recommended Method):
- Initialize the BCM2711's hardware SPI0 peripheral.
- Use the `spidev` kernel module or a library like `bcm2835` to set the clock speed (SPI_CLK) to the highest stable frequency (start with 10 MHz and test up to 30 MHz).
- Replace all Pico `NAND_Read_Byte()` and `NAND_Write_Byte()` calls with the Pi's DMA-backed hardware SPI transfer functions (e.g., `spi_transfer()`).

### 3.2. Timing and Control Signals (Replacing `pio_spi.c` Logic)

The control signals (RST and DBG_EN) require precise, non-interrupted toggling.

- **Clock Generation (For SPI):** Rely on the hardware SPI peripheral or, if bit-banging, the `pigpio` wave generator functions to produce the clock pulse without CPU involvement, minimizing jitter.
- **Control Signal Toggling:** For critical state changes, use `pigpio`'s "real-time" functions like `gpioSetMode()` and `gpioWrite()` combined with high process priority.

### 3.3. J-Runner Communication Protocol Port

The PicoFlasher acts as a standard `serprog` device. The Pi 4 application must behave the same way.

- **Daemon/Application:** Create a persistent C/C++ application on the Pi 4 that runs in the background.
- **Interface:** Use the Pi's built-in USB-Serial port or a separate UART interface to talk to J-Runner.
- **Command Handling:** Implement a loop that listens for J-Runner commands (0xAA for read, 0xBB for write, etc.) over the serial line and calls the ported NAND I/O functions accordingly.

---

## 4. 📝 Documentation and Reference Links

Your team must use these specific, low-level resources for successful implementation:

| Reference | Purpose | Link for Cursor IDE Indexing |
|-----------|---------|------------------------------|
| **BCM2711 Peripheral Datasheet** | Essential for BCM2711 register addresses and SPI/GPIO control. | [BCM2711 ARM Peripherals (PDF)](https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf) |
| **pigpio Library Documentation** | Low-latency Python/C library for precise GPIO waveform generation. | http://abyz.me.uk/rpi/pigpio/ |
| **Falcon Motherboard Pinout** | Visual guide to the required solder points (for reference). | https://xbox360hub.com/guides/rgh-3-guide/ |
| **PicoFlasher Source Code** | The reference logic for command sequencing and NAND protocol. | https://github.com/X360Tools/PicoFlasher |
| **RPi Real-Time Kernel Guide** | Guide to reducing OS jitter for time-critical applications. | Real Time Linux Kernel on Raspberry Pi (Google Search) |

---

*This document serves as the technical blueprint for porting the PicoFlasher functionality to the Raspberry Pi 4 B+ platform.*
