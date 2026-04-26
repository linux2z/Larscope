<div align="center">
  <h1>Larscope Medical Imaging System</h1>
  <p><b>High-Performance, Hardware-Accelerated Endoscopic Imaging & Control</b></p>
  
  [![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
  [![Platform](https://img.shields.io/badge/platform-RK3588%20%7C%20Ubuntu-blue.svg)]()
  [![Language](https://img.shields.io/badge/language-C-orange.svg)]()
  [![GStreamer](https://img.shields.io/badge/framework-GStreamer%201.18-lightgrey.svg)]()
</div>

<br>

Larscope is a production-grade, modular, and event-driven medical imaging system designed for the Firefly CM3588 (Rockchip RK3588 SoC). 

## 🚀 New Features
- **Virtual Diagnostics WebApp:** A stunning glassmorphism dashboard to control and diagnose the system remotely.
- **10-Level Digital Zoom:** Precision cropping via GStreamer `videocrop`.
- **Still Image Capture:** Instant JPEG snapshots saved directly to the storage.
- **Dual-Zone Illumination:** 4-step dimming cycles for separate LED banks via I2C.
- **5-LED Status Panel:** Visual indicators for Power, Battery, Charging, WiFi, and Recording.

## ✨ Core Technology
- **Event-Driven Architecture:** Decoupled Pub/Sub event bus ensures non-blocking operation.
- **Software Encoder Fallback:** Bypasses buggy Rockchip MPP drivers using optimized `x264enc` for maximum stability.
- **Remote Command Server:** Full control via a TCP JSON API (Port 8601).

---

## 🏗️ System Architecture

1. **`shared/`**: Core framework (Event Bus, Config, Logger, Module Manager).
2. **`device_a/`**: The primary capture system (`larscope-a`).
3. **`device_b/`**: The monitoring/control client (`larscope-b`).
4. **`webapp/`**: Virtual hardware dashboard (Flask + Vanilla JS).

---

## 🛠️ 1-Click Installation

### 🎥 Device A (Headless Capture Unit)
```bash
git clone https://github.com/linux2z/Larscope.git
cd Larscope
chmod +x install_device_a.sh
./install_device_a.sh
```

### 🌐 Diagnostics WebApp
```bash
cd webapp
chmod +x start_webapp.sh
./start_webapp.sh
```
*Access the dashboard at `http://<device_a_ip>:5000`*

---

## 🎛️ Hardware Button Mapping
- **Upper Button:** Digital Zoom In
- **Lower Button:** Digital Zoom Out
- **Middle Button:** Still Image Capture (JPEG)
- **Right Button:** Illumination Zone 1 Cycle (100% -> 75% -> 50% -> OFF)
- **Left Button:** Illumination Zone 2 Cycle (100% -> 75% -> 50% -> OFF)

---

## 🛡️ License
Proprietary & Confidential. Unauthorized copying is prohibited.
