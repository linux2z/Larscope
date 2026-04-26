# Larscope System — AI Context & Handoff Prompt

**To the AI Assistant taking over this project:**
Please read this document carefully. It contains the complete context, architectural decisions, and current technical debt for the "Larscope Medical Imaging System". 

---

## 1. Project Overview & Architecture
*   **Project Name:** Larscope
*   **Hardware Platform:** Firefly CM3588 (RK3588 SoC, aarch64, Ubuntu 20.04, Kernel 6.1.141).
*   **Language & Build:** Pure `C`, built via `Makefile`, utilizing `pthread`, `libgpiod`, `json-c`, and `GStreamer 1.18.5`.
*   **System Layout:**
    *   **Device A (Headless Capture):** Runs `larscope-a`. Connects to a USB camera (`/dev/video21`), reads physical buttons/LEDs via GPIO, records to an SD card (`/mnt/sdcard/larscope`), and serves an RTSP stream.
    *   **Device B (Control/Monitor):** Runs `larscope-b`. Receives the RTSP stream, and sends commands via TCP to Device A.
*   **Core Architecture (Device A):**
    *   Strictly **Event-Driven**. Modules communicate solely through an internal publish/subscribe Event Bus (`shared/event_bus.c`). No tight coupling between modules.
    *   **Main Loop:** Modules are initialized, started, and stopped via a central Module Manager (`shared/module.c`).

---

## 2. Directory Structure (Located at `~/larscope_build/` on the device)
*   **`shared/`**: Contains `event_bus.c`, `module.c`, `config.c` (JSON parser), and `logger.c`.
*   **`device_a/`**: 
    *   `gpio_input.c`: Polling `gpiochip1` for 5 hardware buttons (debounced).
    *   `status_led.c`: Toggling 5 status LEDs on `gpiochip3`.
    *   `illum_led.c`: I2C control for TLC59108 PWM LED drivers.
    *   `thermal_mon.c`: Polls `/sys/class/thermal/thermal_zone0/temp`.
    *   `camera.c`: GStreamer pipeline capturing from V4L2 and upscaling to 4K via `videoscale`. Exposes a `tee` element.
    *   `recording.c`: Taps the camera `tee` to encode video using Rockchip's MPP hardware encoder (`mpph265enc` / `mpph264enc`) to an MP4 file.
    *   **`streaming.c`**: Taps the camera `tee` to serve RTSP via `GstRTSPServer` on **Port 8554** (`/live`).
    *   `storage.c`: Basic TCP server on **Port 8600** to serve file lists.
    *   **`cmd_server.c`**: TCP server on **Port 8601** listening for JSON commands (`{"command":"capture_image"}`).
*   **`device_b/`**:
    *   `stream_client.c`: GStreamer `playbin` tuned for <80ms latency.
    *   `cmd_client.c`: TCP client to send commands to Device A.

---

## 3. Issues Resolved So Far
1.  **Remote Execution:** Established a Python-based SSH wrapper (`remote_exec.py`) to bypass PowerShell quoting issues and execute commands directly on Device A (IP: `192.168.137.59`, user: `pi`, pass: `pi`).
2.  **Hardware Discovery:** Confirmed the USB camera maxes out at 3264x2448@15fps (MJPG). The pipeline natively scales this to 4K to meet the 4K stream requirement while waiting for a hardware camera upgrade.
3.  **Storage:** Mounted the 15GB SD card partition to `/mnt/sdcard`.
4.  **Compilation:** Both `larscope-a` and `larscope-b` compile completely warning-free on the CM3588.

---

## 4. Current Blockers & Leftover Tasks (START HERE)

### A. Rockchip MPP Encoder Initialization Failure (CRITICAL)
When running `./larscope-a`, the `recording.c` module fails during the GStreamer pipeline link phase. 
*   **Error Logs:** 
    `mpp_platform: client 18 driver is not ready!`
    `hal_vp8e_init Failed to init due to unsupported hard mode, hw_flag = 269038106`
*   **Context:** The GStreamer pipeline attempts to use `mpph265enc` (or `mpph264enc`). We injected a `videoconvert ! video/x-raw,format=NV12` before the encoder, but the Rockchip MPP library is still failing to initialize the hardware.
*   **Action Required:** Debug the Rockchip MPP plugin. You may need to run the application via `sudo`, change the `mpp` plugin properties, or fall back to a software encoder (`x264enc`) temporarily to unblock development.

### B. I2C Bus Detection for TLC59108
*   **Context:** `illum_led.c` defaults to using `/dev/i2c-2`.
*   **Action Required:** You need to run `sudo i2cdetect -y [bus]` across the 9 available I2C buses to find the exact bus the TLC59108 LED drivers are connected to, and update the `#define I2C_BUS` macro in `illum_led.c`.

### C. Expand Device B (Client)
*   **Context:** Currently, Device B consists of simple C stubs that verify stream ingestion and send a hardcoded command. 
*   **Action Required:** Build out the UI/Dashboard for Device B (as requested in Phase 5 of the design brief) to show the stream, temperatures, and dynamic controls.

---

## 5. Endpoints Summary
*   **Device A IP:** `192.168.137.59` (SSH: `pi` / `pi`)
*   **RTSP Stream:** `rtsp://192.168.137.59:8554/live` (Served by `streaming.c`)
*   **Command Server:** TCP `192.168.137.59:8601` (Listens for `{"command": "..."}`)
*   **Storage Sync Server:** TCP `192.168.137.59:8600` (Listens for `LIST` or `GET` commands)

*End of Prompt. Please acknowledge this context and ask the user which blocker they would like to tackle first.*
