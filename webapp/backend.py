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
    # In a real scenario, we would read GPIOs here using libgpiod
    # For diagnosis, we can return some mocked/detected states
    # or even poll the C server for full state.
    
    # Mocking GPIO state for virtual diagnosis
    # If this was running on hardware, we'd use:
    # import gpiod
    # chip = gpiod.Chip('gpiochip1')
    # line = chip.get_line(30)
    
    status = {
        "recording": False, # Would be updated by actual state
        "streaming": True,
        "gpios": [
            {"name": "GPIO1_D6 (MIDDLE)", "value": 0},
            {"name": "GPIO1_B3 (RIGHT)", "value": 1},
            {"name": "GPIO1_B2 (LEFT)", "value": 0},
            {"name": "GPIO1_B1 (UP)", "value": 0},
            {"name": "GPIO1_B0 (DOWN)", "value": 0}
        ]
    }
    return jsonify(status)

if __name__ == '__main__':
    print("Larscope Virtual Diagnostics WebApp running on http://0.0.0.0:5000")
    app.run(host='0.0.0.0', port=5000, debug=True)
