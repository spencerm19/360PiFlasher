/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Ported from PicoFlasher by Balázs Triszka <balika011@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "xbox.h"
#include "pi4_gpio.h"
#include "pins.h"
#include "spiex.h"
#include <unistd.h>
#include <stdio.h>

/* Sleep wrapper to match Pico SDK sleep_ms() */
static inline void sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

void xbox_init(void)
{
    /* Initialize debug enable pin */
    pi4_gpio_pin_init(SMC_DBG_EN);
    pi4_gpio_put(SMC_DBG_EN, GPIO_HIGH);
    pi4_gpio_set_dir(SMC_DBG_EN, GPIO_OUT);

    /* Initialize reset pin */
    pi4_gpio_pin_init(SMC_RST_XDK_N);
    pi4_gpio_put(SMC_RST_XDK_N, GPIO_HIGH);
    pi4_gpio_set_dir(SMC_RST_XDK_N, GPIO_OUT);

    /* Initialize chip select pin */
    pi4_gpio_pin_init(SPI_SS_N);
    pi4_gpio_put(SPI_SS_N, GPIO_HIGH);
    pi4_gpio_set_dir(SPI_SS_N, GPIO_OUT);
    
    printf("Xbox NAND interface initialized\n");
}

void xbox_start_smc(void)
{
    spiex_deinit();

    pi4_gpio_put(SMC_DBG_EN, GPIO_LOW);
    pi4_gpio_put(SMC_RST_XDK_N, GPIO_LOW);

    sleep_ms(50);

    pi4_gpio_put(SMC_RST_XDK_N, GPIO_HIGH);
}

void xbox_stop_smc(void)
{
    pi4_gpio_put(SMC_DBG_EN, GPIO_LOW);

    sleep_ms(50);

    pi4_gpio_put(SPI_SS_N, GPIO_LOW);
    pi4_gpio_put(SMC_RST_XDK_N, GPIO_LOW);

    sleep_ms(50);

    pi4_gpio_put(SMC_DBG_EN, GPIO_HIGH);
    pi4_gpio_put(SMC_RST_XDK_N, GPIO_HIGH);

    sleep_ms(50);

    pi4_gpio_put(SPI_SS_N, GPIO_HIGH);

    sleep_ms(50);

    spiex_init();
}

uint32_t xbox_get_flash_config(void)
{
    static uint32_t flash_config = 0;
    if (!flash_config) {
        flash_config = spiex_read_reg(0);
        printf("Flash config: 0x%08X\n", flash_config);
    }
    return flash_config;
}

uint16_t xbox_nand_get_status(void)
{
    return spiex_read_reg(0x04);
}

void xbox_nand_clear_status(void)
{
    spiex_write_reg(0x04, spiex_read_reg(0x04));
}

int xbox_nand_wait_ready(uint16_t timeout)
{
    do {
        if (!(xbox_nand_get_status() & 0x01))
            return 0;
    } while (timeout--);

    return 1;
}

int xbox_nand_read_block(uint32_t lba, uint8_t *buffer, uint8_t *spare)
{
    xbox_nand_clear_status();

    spiex_write_reg(0x0C, lba << 9);

    spiex_write_reg(0x08, 0x03);

    if (xbox_nand_wait_ready(0x1000))
        return 0x8000 | xbox_nand_get_status();

    spiex_write_reg(0x0C, 0);

    uint8_t *end = buffer + 0x200;
    while (buffer < end) {
        spiex_write_reg(0x08, 0x00);

        *(uint32_t *)buffer = spiex_read_reg(0x10);
        buffer += 4;
    }

    end = spare + 0x10;
    while (spare < end) {
        spiex_write_reg(0x08, 0x00);

        *(uint32_t *)spare = spiex_read_reg(0x10);
        spare += 4;
    }

    return 0;
}

int xbox_nand_erase_block(uint32_t lba)
{
    xbox_nand_clear_status();

    spiex_write_reg(0x00, spiex_read_reg(0x00) | 0x08);

    spiex_write_reg(0x0C, lba << 9);

    spiex_write_reg(0x08, 0xAA);
    spiex_write_reg(0x08, 0x55);
    spiex_write_reg(0x08, 0x05);

    if (xbox_nand_wait_ready(0x1000))
        return 0x8000 | xbox_nand_get_status();

    return 0;
}

int xbox_nand_write_block(uint32_t lba, uint8_t *buffer, uint8_t *spare)
{
    int flash_config = xbox_get_flash_config();

    int major = (flash_config >> 17) & 3;
    int minor = (flash_config >> 4) & 3;

    int blocksize = 0x4000;
    if (major >= 1) {
        if (minor == 2)
            blocksize = 0x20000;
        else if (minor == 3)
            blocksize = 0x40000;
    }

    int sectors_in_block = blocksize / 0x200;

    /* Erase block if this is the first sector */
    if (lba % sectors_in_block == 0) {
        int ret = xbox_nand_erase_block(lba);
        if (ret)
            return ret;
    }

    xbox_nand_clear_status();

    spiex_write_reg(0x0C, 0);

    uint8_t *end = buffer + 0x200;
    while (buffer < end) {
        spiex_write_reg(0x10, *(uint32_t *)buffer);

        spiex_write_reg(0x08, 0x01);

        buffer += 4;
    }

    end = spare + 0x10;
    while (spare < end) {
        spiex_write_reg(0x10, *(uint32_t *)spare);

        spiex_write_reg(0x08, 0x01);

        spare += 4;
    }

    if (xbox_nand_wait_ready(0x1000))
        return 0x8000 | xbox_nand_get_status();

    spiex_write_reg(0x0C, lba << 9);

    if (xbox_nand_wait_ready(0x1000))
        return 0x8000 | xbox_nand_get_status();

    spiex_write_reg(0x08, 0x55);
    spiex_write_reg(0x08, 0xAA);
    spiex_write_reg(0x08, 0x04);

    if (xbox_nand_wait_ready(0x1000))
        return 0x8000 | xbox_nand_get_status();

    return 0;
}

