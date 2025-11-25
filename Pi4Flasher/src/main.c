/*
 * Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4
 * Copyright (c) 2025
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>

#include "pi4_gpio.h"
#include "pi4_spi.h"
#include "xbox.h"
#include "protocol.h"

/* Serial port file descriptor */
static int serial_fd = -1;
static volatile int running = 1;

/* Stream mode state */
static int do_stream = 0;
static int stream_emmc = 0;
static uint32_t stream_offset = 0;
static uint32_t stream_end = 0;

/**
 * Initialize serial port for communication with J-Runner
 * @param device Serial device path (e.g., "/dev/ttyAMA0")
 * @param baud Baud rate (e.g., B115200)
 * @return 0 on success, -1 on failure
 */
static int serial_init(const char *device, speed_t baud)
{
    struct termios tty;

    serial_fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
        return -1;
    }

    /* Get current serial port settings */
    if (tcgetattr(serial_fd, &tty) != 0) {
        fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno));
        close(serial_fd);
        return -1;
    }

    /* Set baud rate */
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    /* 8N1 mode, no hardware flow control */
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  /* 8-bit chars */
    tty.c_iflag &= ~IGNBRK;                       /* disable break processing */
    tty.c_lflag = 0;                              /* no signaling chars, no echo */
    tty.c_oflag = 0;                              /* no remapping, no delays */
    tty.c_cc[VMIN] = 0;                           /* read doesn't block */
    tty.c_cc[VTIME] = 5;                          /* 0.5 seconds read timeout */

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);       /* shut off xon/xoff ctrl */
    tty.c_cflag |= (CLOCAL | CREAD);              /* ignore modem controls, enable reading */
    tty.c_cflag &= ~(PARENB | PARODD);            /* no parity */
    tty.c_cflag &= ~CSTOPB;                       /* 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;                      /* no hardware flow control */

    /* Apply settings */
    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr: %s\n", strerror(errno));
        close(serial_fd);
        return -1;
    }

    printf("Serial port %s opened successfully\n", device);
    return 0;
}

/**
 * Read exactly n bytes from serial port (blocking with timeout)
 */
static int serial_read_exact(uint8_t *buffer, size_t len)
{
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(serial_fd, buffer + total, len - total);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Serial read error: %s\n", strerror(errno));
            return -1;
        }
        if (n == 0) {
            /* Timeout or EOF */
            if (total == 0)
                return 0;  /* No data yet */
            continue;  /* Keep trying */
        }
        total += n;
    }
    return total;
}

/**
 * Write data to serial port
 */
static int serial_write(const uint8_t *buffer, size_t len)
{
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(serial_fd, buffer + total, len - total);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Serial write error: %s\n", strerror(errno));
            return -1;
        }
        total += n;
    }
    return total;
}

/**
 * Handle streaming mode for fast bulk reads
 */
static void handle_stream(void)
{
    if (!do_stream)
        return;

    if (stream_offset >= stream_end) {
        do_stream = 0;
        return;
    }

    if (!stream_emmc) {
        /* NAND streaming */
        uint8_t buffer[4 + 0x210];
        uint32_t ret = xbox_nand_read_block(stream_offset, &buffer[4], &buffer[4 + 0x200]);
        *(uint32_t *)buffer = ret;

        if (ret == 0) {
            serial_write(buffer, sizeof(buffer));
            stream_offset++;
        } else {
            serial_write((uint8_t *)&ret, 4);
            do_stream = 0;
        }
    } else {
        /* eMMC streaming - not implemented yet */
        fprintf(stderr, "eMMC streaming not yet implemented\n");
        do_stream = 0;
    }
}

/**
 * Process a command from J-Runner
 */
static void handle_command(struct cmd *cmd)
{
    switch (cmd->cmd) {
        case GET_VERSION: {
            uint32_t ver = PI4FLASHER_VERSION;
            serial_write((uint8_t *)&ver, 4);
            printf("Version request: %u\n", ver);
            break;
        }

        case GET_FLASH_CONFIG: {
            uint32_t fc = xbox_get_flash_config();
            serial_write((uint8_t *)&fc, 4);
            printf("Flash config: 0x%08X\n", fc);
            break;
        }

        case READ_FLASH: {
            uint8_t buffer[0x210];
            uint32_t ret = xbox_nand_read_block(cmd->lba, buffer, &buffer[0x200]);
            serial_write((uint8_t *)&ret, 4);
            if (ret == 0) {
                serial_write(buffer, sizeof(buffer));
                printf("Read block %u: OK\n", cmd->lba);
            } else {
                printf("Read block %u: ERROR 0x%X\n", cmd->lba, ret);
            }
            break;
        }

        case WRITE_FLASH: {
            uint8_t buffer[0x210];
            if (serial_read_exact(buffer, sizeof(buffer)) != sizeof(buffer)) {
                fprintf(stderr, "Failed to read write data\n");
                return;
            }
            uint32_t ret = xbox_nand_write_block(cmd->lba, buffer, &buffer[0x200]);
            serial_write((uint8_t *)&ret, 4);
            if (ret == 0) {
                printf("Write block %u: OK\n", cmd->lba);
            } else {
                printf("Write block %u: ERROR 0x%X\n", cmd->lba, ret);
            }
            break;
        }

        case READ_FLASH_STREAM: {
            stream_emmc = 0;
            do_stream = 1;
            stream_offset = 0;
            stream_end = cmd->lba;
            printf("Stream read: %u blocks\n", stream_end);
            break;
        }

        case REBOOT_TO_BOOTLOADER: {
            printf("Reboot command received (not implemented on Pi4)\n");
            break;
        }

        default:
            printf("Unknown command: 0x%02X\n", cmd->cmd);
            break;
    }
}

/**
 * Signal handler for graceful shutdown
 */
static void signal_handler(int signum)
{
    (void)signum;
    running = 0;
}

/**
 * Main application loop
 */
int main(int argc, char *argv[])
{
    const char *serial_device = "/dev/ttyAMA0";

    printf("Pi4Flasher - Xbox 360 NAND Flasher for Raspberry Pi 4\n");
    printf("Version: %d\n\n", PI4FLASHER_VERSION);

    /* Parse command line arguments */
    if (argc > 1) {
        serial_device = argv[1];
    }

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize BCM2835 GPIO library */
    if (pi4_gpio_init() != 0) {
        fprintf(stderr, "Failed to initialize GPIO (are you running as root?)\n");
        return 1;
    }

    /* Initialize Xbox NAND interface */
    xbox_init();

    /* Initialize serial communication */
    if (serial_init(serial_device, B115200) != 0) {
        fprintf(stderr, "Failed to initialize serial port\n");
        pi4_gpio_deinit();
        return 1;
    }

    printf("Pi4Flasher ready. Waiting for commands...\n\n");

    /* Main command loop */
    while (running) {
        /* Handle streaming if active */
        handle_stream();

        /* Check for incoming commands */
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(serial_fd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  /* 100ms timeout */

        int ret = select(serial_fd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Select error: %s\n", strerror(errno));
            break;
        }

        if (ret == 0)
            continue;  /* Timeout, check stream and loop again */

        /* Read command header */
        struct cmd cmd;
        if (serial_read_exact((uint8_t *)&cmd, sizeof(cmd)) != sizeof(cmd)) {
            continue;
        }

        /* Process command */
        handle_command(&cmd);
    }

    printf("\nShutting down Pi4Flasher...\n");

    /* Start SMC before exit */
    xbox_start_smc();

    /* Cleanup */
    if (serial_fd >= 0)
        close(serial_fd);
    pi4_gpio_deinit();

    return 0;
}

