#!/bin/bash
#
# Pi4Flasher Systemd Service Setup Script
# Installs Pi4Flasher as a system service
#

set -e

echo "================================"
echo "Pi4Flasher Service Setup"
echo "================================"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root (use sudo)"
    exit 1
fi

# Check if pi4flasher binary exists
if [ ! -f "/usr/local/bin/pi4flasher" ]; then
    echo "Error: pi4flasher binary not found in /usr/local/bin"
    echo "Please run 'sudo make install' first"
    exit 1
fi

echo "Installing systemd service..."
cp pi4flasher.service /etc/systemd/system/

echo "Reloading systemd daemon..."
systemctl daemon-reload

echo "Enabling pi4flasher service..."
systemctl enable pi4flasher.service

echo ""
echo "================================"
echo "Service installed successfully!"
echo "================================"
echo ""
echo "Available commands:"
echo "  Start service:   sudo systemctl start pi4flasher"
echo "  Stop service:    sudo systemctl stop pi4flasher"
echo "  Check status:    sudo systemctl status pi4flasher"
echo "  View logs:       sudo journalctl -u pi4flasher -f"
echo "  Disable service: sudo systemctl disable pi4flasher"
echo ""
echo "The service will start automatically on boot."
echo ""

