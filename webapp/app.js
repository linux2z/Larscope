const API_BASE = '/api';
let simulatedGpios = {};

function log(message, type = 'system') {
    const console = document.getElementById('log-console');
    const entry = document.createElement('div');
    entry.className = `log-entry ${type}`;
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
    console.appendChild(entry);
    console.scrollTop = console.scrollHeight;
}

async function sendCommand(command) {
    log(`Sending command: ${command}...`);
    
    // Simulate GPIO pulse for diagnosis
    let gpioName = "";
    if (command === 'zoom_in') gpioName = 'BTN_UP';
    if (command === 'zoom_out') gpioName = 'BTN_DOWN';
    if (command === 'capture_image') gpioName = 'BTN_MIDDLE';
    if (command === 'cycle_zone1') gpioName = 'BTN_RIGHT';
    if (command === 'cycle_zone2') gpioName = 'BTN_LEFT';
    
    if (gpioName) {
        simulatedGpios[gpioName] = true;
        updateStatus(); // Immediate visual feedback
        setTimeout(() => { simulatedGpios[gpioName] = false; updateStatus(); }, 500);
    }

    try {
        const response = await fetch(`${API_BASE}/command`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command })
        });
        const data = await response.json();
        if (data.status === 'ok') {
            log(`ACK: ${command} executed`, 'success');
            setTimeout(refreshFiles, 1000); // Wait a bit for file write
        } else {
            log(`Error: ${data.message || 'Failed to execute'}`, 'error');
        }
    } catch (err) {
        log(`Network error: ${err.message}`, 'error');
    }
}

async function refreshFiles() {
    try {
        const response = await fetch(`${API_BASE}/files`);
        if (!response.ok) throw new Error("HTTP " + response.status);
        const files = await response.json();
        const explorer = document.getElementById('file-list');
        
        if (files.length === 0) {
            explorer.innerHTML = '<div class="file-item">Storage is empty (/mnt/sdcard/lascope).</div>';
            return;
        }

        explorer.innerHTML = files.map(f => `
            <div class="file-item">
                <div class="file-info">
                    <span class="file-name">${f.name}</span>
                    <span class="file-meta">${f.size} | ${new Date(f.time * 1000).toLocaleString()}</span>
                </div>
                <a href="${API_BASE}/download/${f.name}" class="download-link" download>DOWNLOAD</a>
            </div>
        `).join('');
    } catch (err) {
        document.getElementById('file-list').innerHTML = `<div class="file-item error">Failed to load: ${err.message}</div>`;
    }
}

async function updateStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        const status = await response.json();

        // Merge real and simulated GPIOs
        status.gpios.forEach(g => {
            if (simulatedGpios[g.name]) g.value = 1;
        });

        // Update LEDs
        const mapping = {
            'LED_REC': 'led-recording',
            'LED_WIFI': 'led-wifi',
            'LED_POWER': 'led-power',
            'LED_BATT': 'led-battery',
            'LED_CHRG': 'led-charging'
        };

        Object.entries(mapping).forEach(([key, id]) => {
            const gpio = status.gpios.find(g => g.name === key);
            if (gpio) document.getElementById(id).classList.toggle('on', gpio.value === 1);
        });
        
        // Update GPIO Stats List
        const statsHtml = status.gpios.map(g => `
            <div class="stat-row">
                <span>${g.name}</span>
                <span class="val ${g.value ? 'pulse' : ''}">${g.value ? 'HIGH' : 'LOW'}</span>
            </div>
        `).join('');
        document.getElementById('gpio-stats').innerHTML = statsHtml;

        document.getElementById('connection-status').textContent = 'CONNECTED';
        document.getElementById('connection-status').style.background = 'rgba(0, 255, 135, 0.2)';
    } catch (err) {
        document.getElementById('connection-status').textContent = 'DISCONNECTED';
        document.getElementById('connection-status').style.background = 'rgba(255, 75, 43, 0.2)';
    }
}

setInterval(updateStatus, 1000);
setInterval(refreshFiles, 5000);
refreshFiles();
log('Lascope Diagnostics interface ready.');
