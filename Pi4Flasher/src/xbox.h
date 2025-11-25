/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Ported from PicoFlasher by Balázs Triszka <balika011@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __XBOX_H__
#define __XBOX_H__

#include <stdint.h>

/**
 * Initialize Xbox NAND interface GPIO pins
 */
void xbox_init(void);

/**
 * Start the System Management Controller (SMC)
 * Releases reset and disables debug mode
 */
void xbox_start_smc(void);

/**
 * Stop the System Management Controller (SMC)
 * Puts SMC in reset and enables debug mode for NAND access
 */
void xbox_stop_smc(void);

/**
 * Get NAND flash configuration from register 0x00
 * @return 32-bit flash configuration value
 */
uint32_t xbox_get_flash_config(void);

/**
 * Get NAND status register
 * @return 16-bit status value
 */
uint16_t xbox_nand_get_status(void);

/**
 * Clear NAND status register
 */
void xbox_nand_clear_status(void);

/**
 * Wait for NAND to become ready
 * @param timeout Timeout counter
 * @return 0 if ready, non-zero if timeout
 */
int xbox_nand_wait_ready(uint16_t timeout);

/**
 * Read a block from NAND flash
 * @param lba Logical block address (512-byte sector)
 * @param buffer Buffer for 512 bytes of data
 * @param spare Buffer for 16 bytes of spare/ECC data
 * @return 0 on success, error code on failure
 */
int xbox_nand_read_block(uint32_t lba, uint8_t *buffer, uint8_t *spare);

/**
 * Erase a block in NAND flash
 * @param lba Logical block address
 * @return 0 on success, error code on failure
 */
int xbox_nand_erase_block(uint32_t lba);

/**
 * Write a block to NAND flash (automatically erases if needed)
 * @param lba Logical block address
 * @param buffer 512 bytes of data to write
 * @param spare 16 bytes of spare/ECC data to write
 * @return 0 on success, error code on failure
 */
int xbox_nand_write_block(uint32_t lba, uint8_t *buffer, uint8_t *spare);

#endif /* __XBOX_H__ */

