from flask import Flask, request, jsonify, send_from_directory
import socket
import json
import os

app = Flask(__name__)

C_SERVER_HOST = '127.0.0.1'
C_SERVER_PORT = 8601

def send_to_c_server(cmd):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(2)
            s.connect((C_SERVER_HOST, C_SERVER_PORT))
            payload = json.dumps({"command": cmd}) + "\n"
            s.sendall(payload.encode())
            data = s.recv(1024)
            return json.loads(data.decode())
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/<path:path>')
def static_files(path):
    return send_from_directory('.', path)

@app.route('/api/command', methods=['POST'])
def handle_command():
    data = request.json
    cmd = data.get('command')
    result = send_to_c_server(cmd)
    return jsonify(result)

@app.route('/api/status', methods=['GET'])
def get_status():
    # Attempt to read real GPIO states if on hardware
    gpios = []
    try:
        import gpiod
        chip = gpiod.Chip('gpiochip1') # Inputs
        for offset, name in [(30, "MIDDLE"), (11, "RIGHT"), (10, "LEFT"), (9, "UP"), (8, "DOWN")]:
            line = chip.get_line(offset)
            line.request(consumer="diag", type=gpiod.LINE_REQ_DIR_IN)
            gpios.append({"name": f"BTN_{name}", "value": line.get_value()})
            line.release()
            
        chip_out = gpiod.Chip('gpiochip3') # LEDs
        for offset, name in [(4, "POWER"), (3, "BATT"), (2, "CHRG"), (1, "WIFI"), (0, "REC")]:
            line = chip_out.get_line(offset)
            # Just peek at the value if it's already an output
            line.request(consumer="diag", type=gpiod.LINE_REQ_DIR_IN) 
            gpios.append({"name": f"LED_{name}", "value": line.get_value()})
            line.release()
    except Exception as e:
        # Fallback to mock for local testing
        gpios = [
            {"name": "BTN_MIDDLE", "value": 0},
            {"name": "LED_REC", "value": 0},
            {"name": "LED_POWER", "value": 1}
        ]

    return jsonify({
        "recording": False,
        "streaming": True,
        "gpios": gpios
    })

@app.route('/api/files', methods=['GET'])
def list_files():
    storage_path = '/mnt/sdcard/lascope'
    print(f"Scanning storage: {storage_path}")
    if not os.path.exists(storage_path):
        try:
            os.makedirs(storage_path, exist_ok=True)
            print(f"Created storage directory: {storage_path}")
        except Exception as e:
            print(f"Failed to create storage: {e}")
            return jsonify([])
    
    files = []
    try:
        for f in os.listdir(storage_path):
            path = os.path.join(storage_path, f)
            files.append({
                "name": f,
                "size": f"{os.path.getsize(path) // 1024} KB",
                "time": os.path.getmtime(path)
            })
        print(f"Found {len(files)} files")
    except Exception as e:
        print(f"Error listing files: {e}")
    
    return jsonify(sorted(files, key=lambda x: x['time'], reverse=True))

@app.route('/api/download/<filename>')
def download_file(filename):
    return send_from_directory('/mnt/sdcard/lascope', filename)

if __name__ == '__main__':
    print("Larscope Virtual Diagnostics WebApp running on http://0.0.0.0:5000")
    app.run(host='0.0.0.0', port=5000, debug=True)
