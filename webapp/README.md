# Lascope Virtual Diagnostics WebApp

This web application provides a professional graphical interface to control and diagnose the **Lascope** system on the Firefly CM3588. It acts as a "Virtual Hardware" layer, allowing you to test the system even without physical buttons or LEDs attached.

## 🛠️ Features
- **Virtual Control Pad:** 5 buttons (Up, Down, Middle, Right, Left) mapped to Zoom, Image Capture, and Lighting cycles.
- **Hardware Sync:** Real-time polling of physical GPIO pins. If a physical button is pressed, or a physical LED is turned on by the C code, the WebApp reflects it instantly.
- **Virtual SD Card:** Integrated file explorer to browse, view metadata, and download recorded videos and still images from `/mnt/sdcard/lascope`.
- **Glassmorphism UI:** Premium, modern design with frosted glass effects and dynamic glowing indicators.

## 🚀 1-Click Installation & Start

Run the following command in this directory:
```bash
chmod +x install.sh
./install.sh
```

The app will be available at: `http://<your_device_ip>:5000`

## 📂 Project Structure
- `backend.py`: Python Flask server acting as a bridge between the Web UI and the C Command Server (Port 8601).
- `index.html / style.css / app.js`: The frontend dashboard.
- `lascope-webapp.service`: Systemd service for automatic startup.

## 🎛️ Diagnostic Guide
- **GREEN LED (POWER):** Shows the system is alive.
- **RED LED (REC):** Turns on when video recording is active.
- **BLUE LED (WIFI):** Shows transmitting status.
- **GPIO SYNC Table:** Shows the raw HIGH/LOW state of every mapped pin for deep debugging.
- **SIMULATED PULSE:** When clicking virtual buttons, the GPIO table will show a brief "HIGH" pulse to confirm the signal was sent.
