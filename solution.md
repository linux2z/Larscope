# Larscope System — AI Context & Handoff Prompt

**To the AI Assistant taking over this project:**
Please read this document carefully. It contains the complete context, architectural decisions, and current technical debt for the "Larscope Medical Imaging System".

---

## 1. Project Overview & Architecture
*   **Project Name:** Larscope
*   **Hardware Platform:** Firefly CM3588 (RK3588 SoC, aarch64, Ubuntu 20.04, Kernel 6.1.141).
*   **Language & Build:** Pure `C`, built via `Makefile`, utilizing `pthread`, `libgpiod`, `json-c`, and `GStreamer 1.18.5`.
*   **WebApp:** Python/Flask backend with a Vanilla JS/CSS Glassmorphism Frontend for virtual hardware diagnostics.
*   **System Layout:**
    *   **Device A (Headless Capture):** Runs `larscope-a`. Connects to a USB camera (`/dev/video21`), reads physical buttons/LEDs via GPIO, records to an SD card, and serves an RTSP stream.
    *   **Device B (Control/Monitor):** Runs `larscope-b`. Receives the RTSP stream, and sends commands via TCP to Device A.
    *   **WebApp (Virtual Hardware):** Runs on Device A. Allows simulating hardware buttons and viewing virtual LEDs via a browser.
*   **Core Architecture (Device A):**
    *   Strictly **Event-Driven**. Modules communicate solely through an internal publish/subscribe Event Bus (`shared/event_bus.c`).
    *   **Digital Zoom:** Implemented via `videocrop` in the GStreamer pipeline, controlled by `EVT_ZOOM_IN/OUT`.
    *   **Illumination:** Dual-zone (Part 1: LEDs 0-7, Part 2: LEDs 8-15) with a 4-step dimming cycle (100% -> 75% -> 50% -> OFF).

---

## 2. Directory Structure
*   **`shared/`**: Event Bus, Module Manager, Config, Logger.
*   **`device_a/`**:
    *   `camera.c`: Includes `videocrop` for 10-level digital zoom.
    *   `recording.c`: Toggles video recording and captures still JPEGs via `appsink`.
    *   `illum_led.c`: Controls two TLC59108 drivers for dual-zone lighting.
    *   `status_led.c`: Maps 5 LEDs (Power, Battery, Charging, WiFi, Recording).
    *   `cmd_server.c`: Listens on Port 8601 for JSON commands.
*   **`webapp/`**:
    *   `backend.py`: Flask bridge that communicates with the C-server.
    *   `index.html / style.css`: Professional Glassmorphism UI for virtual diagnosis.

---

## 3. Issues Resolved So Far
1.  **MPP Encoder Fix:** Bypassed the buggy Rockchip MPP hardware encoders by switching to `x264enc` (software) with `zerolatency` tuning to prevent application crashes.
2.  **Hardware Button Mapping:** Fully mapped the 5 physical buttons (Up, Down, Middle, Right, Left) to specific system features (Zoom, Still Image, LED Cycles).
3.  **Virtual Hardware:** Built a web-based dashboard to diagnose the system without needing a physical PCB attached to the GPIOs.
4.  **Still Image Capture:** Implemented a non-blocking snapshot mechanism using GStreamer appsink.

---

## 4. Current Blockers & Next Steps
1.  **I2C Bus Verification:** The TLC59108 address and bus need final physical verification on the production board.
2.  **Battery/Charging Integration:** Currently, these LEDs are triggered via virtual events. Need to link them to the actual battery driver/sysfs if hardware is present.
3.  **Device B UI:** The C-based Device B is a CLI stub. The WebApp provides a better experience, but a native Device B dashboard might still be desired.

---

## 5. Endpoints Summary
*   **Device A IP:** `192.168.137.59` (SSH: `pi` / `pi`)
*   **RTSP Stream:** `rtsp://192.168.137.59:8554/live`
*   **Command Server:** TCP `8601`
*   **Diagnostics WebApp:** `http://192.168.137.59:5000` (Run `start_webapp.sh`)

*End of Prompt.*
