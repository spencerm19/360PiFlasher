/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __PI4_SPI_H__
#define __PI4_SPI_H__

#include <stdint.h>
#include <stddef.h>

/**
 * Initialize SPI0 on BCM2711
 * @param freq_hz SPI clock frequency in Hz (e.g., 28000000 for 28 MHz)
 * @return 0 on success, -1 on failure
 */
int pi4_spi_init(uint32_t freq_hz);

/**
 * Deinitialize SPI0
 */
void pi4_spi_deinit(void);

/**
 * Write data to SPI (blocking)
 * @param src Source buffer
 * @param len Number of bytes to write
 */
void pi4_spi_write_blocking(const uint8_t *src, size_t len);

/**
 * Write and read data simultaneously (full-duplex SPI transfer)
 * @param src Source buffer (data to write)
 * @param dst Destination buffer (data to read)
 * @param len Number of bytes to transfer
 */
void pi4_spi_write_read_blocking(const uint8_t *src, uint8_t *dst, size_t len);

#endif /* __PI4_SPI_H__ */

