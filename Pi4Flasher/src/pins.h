/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __PINS_H__
#define __PINS_H__

/*
 * BCM2711 GPIO Pin Mapping for Pi 4 B+
 * Mapped from Pico pins according to ProjectDescription.md
 *
 * Original Pico → Pi 4 BCM GPIO
 * GP16 (MISO) → GPIO 9
 * GP17 (SS_N) → GPIO 8
 * GP18 (CLK) → GPIO 11
 * GP19 (MOSI) → GPIO 10
 * GP20 (DBG_EN) → GPIO 3
 * GP21 (RST) → GPIO 2
 */

#define SPI_MISO 9
#define SPI_SS_N 8
#define SPI_CLK 11
#define SPI_MOSI 10
#define SMC_DBG_EN 3
#define SMC_RST_XDK_N 2

/* Future expansion: EMMC and ISD1200 support */
#define MMC_RST_PIN 9
#define MMC_CLK_PIN 8
#define MMC_CMD_PIN 7
#define MMC_DAT0_PIN 6

#define NUVOTON_SPI_RDY 11
#define NUVOTON_SPI_MISO 12
#define NUVOTON_SPI_SS_N 13
#define NUVOTON_SPI_CLK 14
#define NUVOTON_SPI_MOSI 15

#endif /* __PINS_H__ */

