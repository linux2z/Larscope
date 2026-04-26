# Larscope Medical Imaging System

Larscope is a modular, event-driven medical imaging and recording system built for the Firefly CM3588 (RK3588 SoC). It features a hardware-accelerated 4K video pipeline, multi-device networking, and low-latency RTSP streaming.

## Features
- **Event-Driven Architecture**: Fully asynchronous C-based modules utilizing a central Pub/Sub Event Bus.
- **Hardware-Accelerated Video Pipeline**: Captures from V4L2, upscales to 4K, and encodes via Rockchip MPP (`mpph265enc` / `mpph264enc`).
- **Low-Latency RTSP**: Serves an optimized 4K RTSP stream on port 8554.
- **Hardware Integrations**: Direct I2C manipulation for TLC59108 LED drivers, `libgpiod` button handling, and SoC thermal monitoring.
- **Client-Server Control**: Headless "Device A" controlled remotely via TCP JSON commands from "Device B".

## Directory Structure
- `/shared/`: Core framework (Event Bus, Config, Logger, Module Manager).
- `/device_a/`: The primary headless capture system (`larscope-a`).
- `/device_b/`: The client control system (`larscope-b`).

## Prerequisites
- Firefly CM3588 (or equivalent RK3588 board)
- Ubuntu 20.04+ (Kernel 6.1.x)
- Dependencies: `libgstreamer1.0-dev`, `libgstrtspserver-1.0-dev`, `libjson-c-dev`, `libgpiod-dev`

## Building
```bash
# Build Device A (Capture System)
cd device_a
make

# Build Device B (Client)
cd ../device_b
make
```

## Running
On Device A:
```bash
./larscope-a
```
On Device B (ensure IP matches config):
```bash
./larscope-b
```

## Production
A systemd service (`larscope-a.service`) is provided in `device_a/` for headless execution, auto-start, and crash recovery.

---
*Developed as a high-performance, edge-compute medical imaging solution.*
