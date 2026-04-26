const API_BASE = '/api';

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
    try {
        const response = await fetch(`${API_BASE}/command`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command })
        });
        const data = await response.json();
        if (data.status === 'ok') {
            log(`ACK: ${command} executed`, 'success');
            refreshFiles(); // Refresh files if we took a capture
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
        const files = await response.json();
        const explorer = document.getElementById('file-list');
        
        if (files.length === 0) {
            explorer.innerHTML = '<div class="file-item">Storage is empty.</div>';
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
        console.error("Failed to load files", err);
    }
}

async function updateStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        const status = await response.json();

        // Update LEDs from GPIO State
        const recGpio = status.gpios.find(g => g.name === 'LED_REC');
        const wifiGpio = status.gpios.find(g => g.name === 'LED_WIFI');
        const powerGpio = status.gpios.find(g => g.name === 'LED_POWER');
        const battGpio = status.gpios.find(g => g.name === 'LED_BATT');
        const chrgGpio = status.gpios.find(g => g.name === 'LED_CHRG');

        if (recGpio) document.getElementById('led-recording').classList.toggle('on', recGpio.value === 1);
        if (wifiGpio) document.getElementById('led-wifi').classList.toggle('on', wifiGpio.value === 1);
        if (powerGpio) document.getElementById('led-power').classList.toggle('on', powerGpio.value === 1);
        if (battGpio) document.getElementById('led-battery').classList.toggle('on', battGpio.value === 1);
        if (chrgGpio) document.getElementById('led-charging').classList.toggle('on', chrgGpio.value === 1);
        
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

// Initial status check and polling
setInterval(updateStatus, 1000);
setInterval(refreshFiles, 5000);
refreshFiles();
log('Diagnostics interface ready.');
