#!/bin/bash
# Larscope - Device B One-Click Installer
# Run this script on the client Ubuntu machine

set -e

echo "=========================================="
echo " Larscope Device B - Installation Script  "
echo "=========================================="

echo "[1/3] Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y build-essential gcc make git \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libgstrtspserver-1.0-dev

echo "[2/3] Compiling Device B software..."
cd device_b
make clean
make

echo "[3/3] Creating desktop shortcut..."
CURRENT_DIR=$(pwd)
USER=$(whoami)
SHORTCUT_FILE="/home/$USER/Desktop/Larscope.desktop"

# Create a desktop shortcut for easy launching
cat > "$SHORTCUT_FILE" << EOF
[Desktop Entry]
Version=1.0
Name=Larscope Monitor
Comment=Launch Larscope Device B Client
Exec=$CURRENT_DIR/larscope-b
Path=$CURRENT_DIR
Icon=utilities-terminal
Terminal=true
Type=Application
Categories=Application;
EOF

chmod +x "$SHORTCUT_FILE"

echo "=========================================="
echo " Installation Complete!"
echo " You can now launch Larscope Device B by running:"
echo " ./device_b/larscope-b"
echo " Or by clicking the shortcut on your Desktop."
echo "=========================================="
