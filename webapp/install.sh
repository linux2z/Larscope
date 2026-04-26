#!/bin/bash
# Larscope WebApp 1-Click Installer

echo "------------------------------------------------"
echo "  Larscope Virtual Diagnostics Installer"
echo "------------------------------------------------"

echo "[1/3] Updating package list..."
sudo apt-get update -y

echo "[2/3] Installing Python dependencies..."
sudo apt-get install -y python3-pip python3-flask
# libgpiod python bindings
sudo apt-get install -y python3-libgpiod || echo "Warning: python3-libgpiod not found, using fallback."

echo "[3/3] Setting up permissions..."
sudo usermod -a -G gpio $USER
sudo usermod -a -G i2c $USER

echo "------------------------------------------------"
echo "Installation Complete!"
echo "Run './start_webapp.sh' to start the dashboard."
echo "------------------------------------------------"
