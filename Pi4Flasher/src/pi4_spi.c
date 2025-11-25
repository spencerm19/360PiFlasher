/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "pi4_spi.h"
#include "pi4_gpio.h"
#include "pins.h"
#include <bcm2835.h>
#include <stdio.h>
#include <string.h>

int pi4_spi_init(uint32_t freq_hz)
{
    /* Initialize SPI0 */
    if (!bcm2835_spi_begin()) {
        fprintf(stderr, "Failed to initialize SPI\n");
        return -1;
    }

    /* Set SPI bit order to MSB first (bcm2835 default) */
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    
    /* Set SPI mode: CPOL=0, CPHA=0 (Mode 0) */
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    
    /* Calculate clock divider for target frequency
     * BCM2835_CORE_CLK = 250 MHz on Pi 4
     * For 28 MHz: divider = 250,000,000 / 28,000,000 ≈ 9
     * We'll use closest power of 2 or available divider
     */
    uint16_t divider;
    if (freq_hz >= 31250000) {
        divider = BCM2835_SPI_CLOCK_DIVIDER_8;  /* 31.25 MHz */
    } else if (freq_hz >= 15625000) {
        divider = BCM2835_SPI_CLOCK_DIVIDER_16; /* 15.625 MHz */
    } else if (freq_hz >= 7812500) {
        divider = BCM2835_SPI_CLOCK_DIVIDER_32; /* 7.8125 MHz */
    } else {
        divider = BCM2835_SPI_CLOCK_DIVIDER_64; /* 3.90625 MHz */
    }
    
    bcm2835_spi_setClockDivider(divider);
    
    /* 
     * CRITICAL: We use manual chip select control via GPIO
     * for precise timing, not hardware CS
     */
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
    
    printf("SPI initialized: target=%u Hz, divider=%u\n", freq_hz, divider);
    
    return 0;
}

void pi4_spi_deinit(void)
{
    bcm2835_spi_end();
}

void pi4_spi_write_blocking(const uint8_t *src, size_t len)
{
    /* Use transfern for DMA-backed transfer */
    uint8_t dummy[len];
    memcpy(dummy, src, len);
    bcm2835_spi_transfern((char *)dummy, len);
}

void pi4_spi_write_read_blocking(const uint8_t *src, uint8_t *dst, size_t len)
{
    /* Copy source to destination buffer for in-place transfer */
    memcpy(dst, src, len);
    /* bcm2835_spi_transfern does full-duplex in-place transfer */
    bcm2835_spi_transfern((char *)dst, len);
}

