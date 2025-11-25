/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Ported from PicoFlasher by Balázs Triszka <balika011@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __SPIEX_H__
#define __SPIEX_H__

#include <stdint.h>

/**
 * Initialize the SPI Extended interface for Xbox NAND communication
 */
void spiex_init(void);

/**
 * Deinitialize the SPI Extended interface
 */
void spiex_deinit(void);

/**
 * Read a 32-bit register from the Xbox NAND controller
 * @param reg Register address (0x00-0x1F)
 * @return 32-bit register value
 */
uint32_t spiex_read_reg(uint8_t reg);

/**
 * Write a 32-bit value to Xbox NAND controller register
 * @param reg Register address (0x00-0x1F)
 * @param val 32-bit value to write
 */
void spiex_write_reg(uint8_t reg, uint32_t val);

#endif /* __SPIEX_H__ */

