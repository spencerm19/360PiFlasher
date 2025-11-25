/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

/* Command protocol definitions from PicoFlasher */
#define GET_VERSION 0x00
#define GET_FLASH_CONFIG 0x01
#define READ_FLASH 0x02
#define WRITE_FLASH 0x03
#define READ_FLASH_STREAM 0x04

#define EMMC_DETECT 0x50
#define EMMC_INIT 0x51
#define EMMC_GET_CID 0x52
#define EMMC_GET_CSD 0x53
#define EMMC_GET_EXT_CSD 0x54
#define EMMC_READ 0x55
#define EMMC_READ_STREAM 0x56
#define EMMC_WRITE 0x57

#define ISD1200_INIT 0xA0
#define ISD1200_DEINIT 0xA1
#define ISD1200_READ_ID 0xA2
#define ISD1200_READ_FLASH 0xA3
#define ISD1200_ERASE_FLASH 0xA4
#define ISD1200_WRITE_FLASH 0xA5
#define ISD1200_PLAY_VOICE 0xA6
#define ISD1200_EXEC_MACRO 0xA7
#define ISD1200_RESET 0xA8

#define REBOOT_TO_BOOTLOADER 0xFE

/* Command structure */
#pragma pack(push, 1)
struct cmd {
    uint8_t cmd;
    uint32_t lba;
};
#pragma pack(pop)

/* Version number */
#define PI4FLASHER_VERSION 4

#endif /* __PROTOCOL_H__ */

