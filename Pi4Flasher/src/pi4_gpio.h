/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __PI4_GPIO_H__
#define __PI4_GPIO_H__

#include <stdint.h>
#include <stdbool.h>

/* GPIO direction */
#define GPIO_IN 0
#define GPIO_OUT 1

/* GPIO levels */
#define GPIO_LOW 0
#define GPIO_HIGH 1

/**
 * Initialize the BCM2835 GPIO library
 * Must be called before any other GPIO operations
 * @return 0 on success, -1 on failure
 */
int pi4_gpio_init(void);

/**
 * Cleanup GPIO library resources
 */
void pi4_gpio_deinit(void);

/**
 * Initialize a specific GPIO pin
 * @param pin GPIO pin number (BCM numbering)
 */
void pi4_gpio_pin_init(uint8_t pin);

/**
 * Set GPIO pin direction
 * @param pin GPIO pin number
 * @param dir GPIO_IN or GPIO_OUT
 */
void pi4_gpio_set_dir(uint8_t pin, uint8_t dir);

/**
 * Write to GPIO pin
 * @param pin GPIO pin number
 * @param value GPIO_LOW or GPIO_HIGH
 */
void pi4_gpio_put(uint8_t pin, uint8_t value);

/**
 * Read from GPIO pin
 * @param pin GPIO pin number
 * @return GPIO_LOW or GPIO_HIGH
 */
uint8_t pi4_gpio_get(uint8_t pin);

#endif /* __PI4_GPIO_H__ */

