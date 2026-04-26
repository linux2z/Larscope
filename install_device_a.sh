#!/bin/bash
# Larscope - Device A One-Click Installer
# Run this script on the Firefly CM3588 (or Ubuntu headless unit)

set -e

echo "=========================================="
echo " Larscope Device A - Installation Script  "
echo "=========================================="

echo "[1/4] Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y build-essential gcc make git \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libgstrtspserver-1.0-dev libjson-c-dev libgpiod-dev i2c-tools

echo "[2/4] Compiling Device A software..."
# Build the application
cd device_a
make clean
make

echo "[3/4] Installing systemd service for auto-start..."
# Update the service file to use the absolute path of the current directory
SERVICE_FILE="/etc/systemd/system/larscope-a.service"
CURRENT_DIR=$(pwd)
USER=$(whoami)

sudo bash -c "cat > $SERVICE_FILE" << EOF
[Unit]
Description=Larscope Medical Imaging System (Device A)
After=network.target local-fs.target
Wants=network-online.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$CURRENT_DIR
ExecStart=$CURRENT_DIR/larscope-a
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

echo "[4/4] Starting Larscope Device A service..."
sudo systemctl daemon-reload
sudo systemctl enable larscope-a.service
sudo systemctl restart larscope-a.service

echo "=========================================="
echo " Installation Complete!"
echo " Device A is now running in the background."
echo " To view logs, run: journalctl -u larscope-a.service -f"
echo "=========================================="
