/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "pi4_gpio.h"
#include <bcm2835.h>
#include <stdio.h>

int pi4_gpio_init(void)
{
    if (!bcm2835_init()) {
        fprintf(stderr, "Failed to initialize BCM2835 library\n");
        return -1;
    }
    return 0;
}

void pi4_gpio_deinit(void)
{
    bcm2835_close();
}

void pi4_gpio_pin_init(uint8_t pin)
{
    /* BCM2835 library handles initialization internally */
}

void pi4_gpio_set_dir(uint8_t pin, uint8_t dir)
{
    if (dir == GPIO_OUT) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    } else {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    }
}

void pi4_gpio_put(uint8_t pin, uint8_t value)
{
    bcm2835_gpio_write(pin, value);
}

uint8_t pi4_gpio_get(uint8_t pin)
{
    return bcm2835_gpio_lev(pin);
}

