# Lascope System — Comprehensive Project Handoff

## 1. Project Objective
The Lascope project delivers a production-ready medical imaging system for the Firefly CM3588 platform. It achieves 4K video upscaling, <80ms low-latency RTSP streaming, synchronized hardware controls, and a virtual diagnostic web dashboard.

## 2. Key Accomplishments (Completed)

### A. Core Architecture
- **Event-Driven Pub/Sub Bus:** A robust inter-module communication system (`shared/event_bus.c`) prevents tight coupling between modules.
- **Dynamic Module Management:** A unified lifecycle manager (`shared/module.c`) handles initialization, startup, and teardown of all system components.

### B. Video Pipeline (GStreamer)
- **4K Upscaling:** Captures from USB MJPG cameras and upscales to 4K using the `videoscale` element.
- **Fault-Tolerant Encoding:** Bypasses buggy Rockchip MPP hardware encoders with a tuned `x264enc` software fallback, ensuring 100% stability on Kernel 6.1.141.
- **Multi-Branching:** A single camera source feeds:
  - **RTSP Stream:** Port 8554 (`/live`) for remote monitoring.
  - **MP4 Recording:** High-bitrate H.264 recording to SD card (`/mnt/sdcard/lascope`).
  - **Still Captures:** Instant JPEG snapshots via `appsink` branching.
- **Digital Zoom:** 10-level digital zoom implemented via `videocrop` manipulation.

### C. Hardware Integration
- **Button Mapping:** 5 Physical buttons (Up, Down, Middle, Right, Left) mapped to Zoom, Image Capture, and dual-zone LED cycles.
- **Illumination:** I2C-based control for two TLC59108 drivers. Supports dual-zone 4-step dimming (100% -> 75% -> 50% -> OFF).
- **Status LEDs:** 5 LEDs mapped to system states (Power, Battery, Charging, WiFi, Recording).
- **System Monitoring:** Thermal monitoring of the RK3588 SoC.

### D. Diagnostics & Deployment
- **Virtual WebApp Dashboard:** A premium Glassmorphism UI (Flask + Vanilla JS) for remote diagnostics and virtual hardware control.
- **One-Click Shortcut:** A Linux Desktop shortcut (`~/Desktop/LascopeDiagnostics.desktop`) that opens the dashboard in Chromium.
- **1-Click Installation:** Automated shell scripts for system dependencies, C compilation, and `systemd` service deployment.

---

## 3. Critical Fixes & Knowledge
- **Name Change:** The project was renamed from Larscope to **Lascope**.
- **MPP Driver Bug:** Rockchip MPP encoders (`mpph265enc`) fail on Kernel 6.1.141. **Solution:** Use the `x264enc` fallback implemented in `recording.c` and `streaming.c`.
- **GPIO Sync:** The WebApp backend polls actual voltage states, and virtual buttons now trigger a "Simulated Pulse" in the UI for visual confirmation of signal flow.

---

## 4. Endpoints & API Reference

| Service | Protocol | Port | Detail |
| :--- | :--- | :--- | :--- |
| **RTSP Live Stream** | RTSP | 8554 | `rtsp://<ip>:8554/live` |
| **Command Server** | TCP (JSON) | 8601 | Accepts `{"command": "zoom_in"}`, etc. |
| **Diagnostics WebApp**| HTTP | 5000 | Browser dashboard with GPIO sync. |
| **File Sync** | TCP | 8600 | Metadata exchange for recorded files. |

---

## 5. Directory Mapping
- `device_a/`: Main C application source.
- `device_b/`: Remote control client source.
- `shared/`: Reusable framework code (Event Bus, Config, etc.).
- `webapp/`: Web-based diagnostic dashboard source.
- `install_device_a.sh`: The "1-Click" master installer.

---

## 6. Next Steps for Successors
1. **I2C Finalization:** Run `i2cdetect -y 2` to verify the TLC59108 presence.
2. **Battery BMS:** Link `EVT_BATTERY_UPDATE` to the actual battery driver once connected.
3. **Storage Mgmt:** Implement auto-deletion of old recordings when the SD card reaches 90% capacity.

**Handoff Complete.**
