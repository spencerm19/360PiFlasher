# Pi4Flasher Testing Guide

This guide provides comprehensive testing procedures for Pi4Flasher to ensure reliable operation before using with an actual Xbox 360 Falcon motherboard.

## Testing Phases

Testing should be performed in this order:
1. Software build verification
2. Hardware initialization tests
3. GPIO/SPI functional tests (without Xbox)
4. Loopback tests
5. Xbox NAND integration tests
6. J-Runner integration tests

## Phase 1: Build Verification

### Test 1.1: Successful Compilation

```bash
cd Pi4Flasher
mkdir build && cd build
cmake ..
make -j4
```

**Expected Result**: Clean build with no errors, `pi4flasher` binary created.

### Test 1.2: Library Dependencies

```bash
ldd ./pi4flasher
```

**Expected Result**: Should show `libbcm2835.so` linked successfully.

### Test 1.3: Binary Information

```bash
file ./pi4flasher
./pi4flasher --help 2>&1 || echo "No help flag (expected)"
```

**Expected Result**: ARM 64-bit executable identified.

## Phase 2: Hardware Initialization Tests

### Test 2.1: Root Permission Check

```bash
# Run without root (should fail)
./pi4flasher

# Run with root (should work)
sudo ./pi4flasher
```

**Expected Result**: 
- Without sudo: "Failed to initialize GPIO"
- With sudo: Program starts, waits for serial commands

### Test 2.2: GPIO Initialization

```bash
sudo ./pi4flasher /dev/ttyAMA0
```

Watch for these initialization messages:
- "Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4"
- "Serial port /dev/ttyAMA0 opened successfully"
- "Xbox NAND interface initialized"
- "Pi4Flasher ready. Waiting for commands..."

**Expected Result**: All initialization succeeds, program waits for commands.

### Test 2.3: Signal Handling

```bash
sudo ./pi4flasher /dev/ttyAMA0
# Press Ctrl+C
```

**Expected Result**: Graceful shutdown with "Shutting down Pi4Flasher..." message.

## Phase 3: GPIO Functional Tests (No Xbox Required)

### Test 3.1: GPIO Output Test with Multimeter

Connect a multimeter to the following BCM GPIO pins and GND:

```bash
sudo ./pi4flasher /dev/ttyAMA0
```

Measure voltages (should be ~3.3V when HIGH):

| BCM GPIO | Expected State | Function |
|----------|---------------|----------|
| GPIO 2 | HIGH | SMC_RST_XDK_N (reset inactive) |
| GPIO 3 | HIGH | SMC_DBG_EN (debug enabled) |
| GPIO 8 | HIGH | SPI_SS_N (chip select inactive) |

**Pass Criteria**: All pins measure 3.3V ±0.1V

### Test 3.2: GPIO Toggle Test with Oscilloscope/Logic Analyzer

If you have an oscilloscope or logic analyzer, monitor the control signals during operation.

**Expected Behavior**: Pins should be stable at 3.3V when idle.

## Phase 4: SPI Loopback Tests

### Test 4.1: Hardware Loopback

**Setup**: Connect MOSI to MISO (GPIO 10 to GPIO 9) with a jumper wire.

**Test Code** (add to main.c temporarily):

```c
// After xbox_init(), add:
printf("Testing SPI loopback...\n");
uint8_t test_data[] = {0xAA, 0x55, 0xF0, 0x0F};
uint8_t read_data[sizeof(test_data)];

pi4_gpio_put(SPI_SS_N, GPIO_LOW);
pi4_spi_write_read_blocking(test_data, read_data, sizeof(test_data));
pi4_gpio_put(SPI_SS_N, GPIO_HIGH);

if (memcmp(test_data, read_data, sizeof(test_data)) == 0) {
    printf("SPI loopback test PASSED\n");
} else {
    printf("SPI loopback test FAILED\n");
}
```

**Pass Criteria**: Test prints "PASSED"

### Test 4.2: Bit Reversal Verification

Verify the LSB-to-MSB lookup table:

```c
// Test bit reversal
uint8_t test = 0xAA; // 10101010
uint8_t expected = 0x55; // 01010101
uint8_t result = lsb2msb[test];
printf("Bit reversal: 0x%02X -> 0x%02X (expected 0x%02X) %s\n",
       test, result, expected, result == expected ? "PASS" : "FAIL");
```

**Pass Criteria**: Correct bit reversal

## Phase 5: Xbox NAND Integration Tests

**⚠️ WARNING**: Only proceed if you have:
- Proper 3.3V level shifter installed
- Verified all connections multiple times
- Backed up NAND elsewhere (if possible)

### Test 5.1: Flash Configuration Read

**Hardware Setup**: Connect Pi4Flasher to Xbox 360 Falcon NAND.

```bash
sudo ./pi4flasher /dev/ttyAMA0
```

From another terminal or using a serial terminal program, send:
- Command: `0x01` (GET_FLASH_CONFIG)
- LBA: `0x00000000`

**Expected Result**: Receive a non-zero 32-bit flash config value.

Example response: `0x00443203` (varies by NAND type)

**Pass Criteria**: 
- Non-zero response
- No timeout
- Value matches known Falcon configurations

### Test 5.2: Single Block Read

Send READ_FLASH command (0x02) for LBA 0:

**Expected Result**: 
- 4-byte status (0x00000000 for success)
- 512 bytes of NAND data
- 16 bytes of spare data

**Pass Criteria**:
- Status = 0
- Data appears reasonable (not all 0x00 or 0xFF)
- Consistent on repeated reads

### Test 5.3: Read Stability Test

Read the same block 100 times and verify consistency:

```python
# Python test script
import serial
import struct

ser = serial.Serial('/dev/ttyAMA0', 115200)

# Read block 10, 100 times
checksums = []
for i in range(100):
    cmd = struct.pack('<BI', 0x02, 10)  # READ_FLASH, LBA=10
    ser.write(cmd)
    
    status = struct.unpack('<I', ser.read(4))[0]
    if status != 0:
        print(f"Error on iteration {i}: {status:08X}")
        break
    
    data = ser.read(0x210)
    checksums.append(hash(data))

if len(set(checksums)) == 1:
    print("Read stability test PASSED (100/100 consistent)")
else:
    print(f"Read stability test FAILED ({len(set(checksums))} unique reads)")
```

**Pass Criteria**: All 100 reads produce identical data

### Test 5.4: Write and Verify Test

**⚠️ CRITICAL**: Only test on a sacrificial NAND or blocks known to be safe!

1. Read a block (save original data)
2. Write test pattern to the block
3. Read back and verify
4. Restore original data

**Pass Criteria**: 
- Write returns status 0
- Read-back matches written data
- Original data can be restored

## Phase 6: J-Runner Integration Tests

### Test 6.1: J-Runner Device Detection

1. Connect PC to Pi4's serial port
2. Start Pi4Flasher on Pi
3. Open J-Runner on PC
4. Select the serial port

**Expected Result**: J-Runner detects a valid programmer

### Test 6.2: Read NAND Dump

Using J-Runner:
1. Select "Read NAND"
2. Choose output file location
3. Start read operation

**Pass Criteria**:
- Progress bar advances smoothly
- No read errors reported
- Complete 16MB dump created
- Dump passes J-Runner's validation

### Test 6.3: Compare Dumps

Read the NAND twice and compare:

```bash
# Create two dumps
md5sum dump1.bin
md5sum dump2.bin
```

**Pass Criteria**: MD5 checksums match exactly

### Test 6.4: Write Operation Test

**⚠️ DANGER**: Only proceed if:
- You have a backup of the NAND
- You understand the risks
- You're using a test console

1. Read original NAND
2. Write NAND image using J-Runner
3. Read NAND again
4. Compare before/after dumps

**Pass Criteria**: 
- Write completes without errors
- Before/after dumps are identical

## Performance Benchmarks

### Expected Performance Metrics

| Operation | Target Time | Acceptable Range |
|-----------|-------------|------------------|
| Single block read | 1-2 ms | 0.5-5 ms |
| Single block write | 2-5 ms | 1-10 ms |
| Full 16MB dump | 3-5 min | 2-8 min |
| Stream read (1000 blocks) | 10-20 sec | 5-30 sec |

### Benchmark Script

```bash
# Read 1000 blocks and time it
time sudo ./test_script_1000_reads.sh
```

**Pass Criteria**: Within acceptable range for hardware

## Troubleshooting Failed Tests

### GPIO Initialization Fails
- Check you're running as root
- Verify BCM2835 library is installed
- Check Pi4 model (must be Pi 4)

### SPI Loopback Fails
- Verify jumper wire connection
- Check SPI initialization
- Try lower clock speed

### Read Errors from Xbox
- Check all wire connections
- Verify level shifter is working
- Ensure Xbox is powered correctly
- Check for cold solder joints

### Inconsistent Reads
- Check for loose connections
- Verify power supply stability
- Try reducing SPI clock speed
- Enable RT scheduling

### J-Runner Communication Fails
- Verify serial device path
- Check baud rate (115200)
- Disable serial console in raspi-config
- Test with a loopback (TX to RX)

## Test Checklist

Use this checklist before deploying to production:

- [ ] Build completes without errors
- [ ] All dependencies installed correctly
- [ ] GPIO initialization succeeds
- [ ] Control signals are correct (3.3V)
- [ ] SPI loopback test passes
- [ ] Bit reversal verified
- [ ] Flash config reads successfully
- [ ] Single block reads consistently
- [ ] 100-read stability test passes
- [ ] Write-verify-restore succeeds
- [ ] J-Runner detects device
- [ ] Full NAND dump completes
- [ ] Dumps are consistent (matching MD5)
- [ ] Performance meets targets
- [ ] No errors in system logs

## Safety Reminders

1. **Always use a 3.3V level shifter**
2. **Double-check connections before powering on**
3. **Test on sacrificial hardware first**
4. **Keep backups of original NAND**
5. **Use proper ESD protection**
6. **Monitor temperatures during extended use**
7. **Never hotplug connections**

## Reporting Issues

If tests fail, collect this information:

```bash
# System info
uname -a
cat /proc/device-tree/model
vcgencmd version

# Pi4Flasher logs
sudo journalctl -u pi4flasher -n 100

# Hardware details
gpio readall
```

Include this data when seeking help or reporting bugs.

