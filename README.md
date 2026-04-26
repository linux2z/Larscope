<div align="center">
  <h1>Larscope Medical Imaging System</h1>
  <p><b>High-Performance, Hardware-Accelerated Endoscopic Imaging & Control</b></p>
  
  [![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
  [![Platform](https://img.shields.io/badge/platform-RK3588%20%7C%20Ubuntu-blue.svg)]()
  [![Language](https://img.shields.io/badge/language-C-orange.svg)]()
  [![GStreamer](https://img.shields.io/badge/framework-GStreamer%201.18-lightgrey.svg)]()
</div>

<br>

Larscope is a production-grade, modular, and event-driven medical imaging system designed specifically for the Firefly CM3588 (Rockchip RK3588 SoC). Built entirely in **C**, it leverages Rockchip's Media Process Platform (MPP) and GStreamer to provide ultra-low-latency (<80ms) 4K video streaming, direct hardware manipulation (GPIO, I2C), and remote command execution.

## ✨ Key Features

- 🏎️ **Ultra-Low Latency Streaming:** Hardware-accelerated 4K RTSP pipeline using Rockchip MPP (`mpph265enc` / `mpph264enc`).
- 🔄 **Event-Driven Architecture:** Decoupled Pub/Sub event bus ensures non-blocking, asynchronous operation across all system components.
- 🎛️ **Direct Hardware Control:** Native integration with `libgpiod` for button/LED management and I2C for TLC59108 illumination drivers.
- 🌡️ **Thermal Management:** Active SoC thermal monitoring preventing hardware degradation during intensive video encoding.
- 📡 **Remote Command Server:** Headless operation controlled via a robust TCP JSON API from the client monitoring device.

---

## 🏗️ System Architecture

The project is split into three main logical components:

1. **`shared/` (Core Framework):** The foundational engine providing the thread-safe Event Bus, Module Manager lifecycle hooks, JSON config parsing, and standard leveled logging.
2. **`device_a/` (Headless Capture System):** Runs on the Firefly CM3588. Captures V4L2 video, upscales, encodes to SD card storage, and serves the RTSP feed alongside the TCP Command Server.
3. **`device_b/` (Monitor Client):** Runs on the operator's machine (Ubuntu). Provides an optimized `playbin` client to receive the feed instantly and send JSON remote commands to Device A.

---

## 🚀 1-Click Installation

Deploying Larscope is designed to be frictionless. Simply clone the repository onto your target machine and run the provided installation scripts.

### 🎥 Device A (Headless Capture Unit)
Run this on your Firefly CM3588. This script installs all required video/hardware dependencies, compiles the C code, and creates a `systemd` service for automatic boot recovery.

```bash
git clone https://github.com/linux2z/Larscope.git
cd Larscope
chmod +x install_device_a.sh
./install_device_a.sh
```
*To view system logs in production, run:* `journalctl -u larscope-a.service -f`

### 💻 Device B (Monitor Client)
Run this on the Ubuntu machine you intend to use as a monitor. It installs playback dependencies, compiles the client, and generates a Desktop shortcut.

```bash
git clone https://github.com/linux2z/Larscope.git
cd Larscope
chmod +x install_device_b.sh
./install_device_b.sh
```

---

## 📂 Directory Structure

```text
Larscope/
├── shared/                 # Core Event Bus & Utilities
│   ├── event_bus.c/.h      # Thread-safe pub/sub engine
│   ├── module.c/.h         # Application lifecycle manager
│   ├── config.c/.h         # JSON configuration parser
│   └── logger.c/.h         # Leveled async logger
├── device_a/               # Capture Subsystem
│   ├── main.c              # Entry point & module registration
│   ├── camera.c            # GStreamer V4L2 capture pipeline
│   ├── recording.c         # MPP Hardware H.264/H.265 encoding
│   ├── streaming.c         # RTSP low-latency server
│   ├── gpio_input.c        # Button polling via libgpiod
│   ├── illum_led.c         # I2C TLC59108 lighting control
│   └── cmd_server.c        # TCP JSON listener (Port 8601)
├── device_b/               # Monitor Client
│   ├── main.c              # Client entry point
│   ├── stream_client.c     # Low-latency playbin viewer
│   └── cmd_client.c        # TCP command emitter
├── install_device_a.sh     # 1-Click Installer (A)
├── install_device_b.sh     # 1-Click Installer (B)
└── config.json             # Global System Configuration
```

---

## 🔧 Dependencies & Prerequisites

- **OS:** Ubuntu 20.04+ (Linux Kernel 6.1.x)
- **Compiler:** `gcc`, `make`, `build-essential`
- **Libraries:**
  - `libgstreamer1.0-dev`, `libgstreamer-plugins-base1.0-dev`
  - `libgstrtspserver-1.0-dev`
  - `libjson-c-dev`
  - `libgpiod-dev`
  - `i2c-tools`

## 🛡️ License
Proprietary & Confidential. Unauthorized copying of this repository, via any medium, is strictly prohibited.
