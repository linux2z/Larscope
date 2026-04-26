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
        } else {
            log(`Error: ${data.message || 'Failed to execute'}`, 'error');
        }
    } catch (err) {
        log(`Network error: ${err.message}`, 'error');
    }
}

async function updateStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        const status = await response.json();

        // Update LEDs
        document.getElementById('led-recording').classList.toggle('on', status.recording);
        document.getElementById('led-wifi').classList.toggle('on', status.streaming);
        document.getElementById('led-power').classList.add('on');
        
        // Update GPIO Stats
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
log('Diagnostics interface ready.');
