"""Upload and run a hardware inspection script on Device A via SSH."""
import paramiko
import sys

SCRIPT = r'''#!/bin/bash
echo "=== THERMAL ZONES ==="
for z in /sys/class/thermal/thermal_zone*; do
  tp=$(cat "$z/type" 2>/dev/null)
  tm=$(cat "$z/temp" 2>/dev/null)
  echo "$tp = $tm"
done

echo "=== GPIO CHIP MAP ==="
for d in /sys/bus/gpio/devices/gpiochip*; do
  chip=$(basename "$d")
  base=$(cat "$d/base" 2>/dev/null)
  ngpio=$(cat "$d/ngpio" 2>/dev/null)
  label=$(cat "$d/label" 2>/dev/null)
  echo "$chip: base=$base ngpio=$ngpio label=$label"
done

echo "=== LANGUAGES ==="
python3 --version 2>/dev/null
rustc --version 2>/dev/null
go version 2>/dev/null
gcc --version 2>/dev/null | head -1
g++ --version 2>/dev/null | head -1
cmake --version 2>/dev/null | head -1

echo "=== BLOCK DEVICES ==="
lsblk 2>/dev/null

echo "=== MPP ENCODER TEST ==="
ls -la /dev/mpp_service 2>/dev/null
dpkg -l | grep -i mpp 2>/dev/null

echo "=== GSTREAMER ROCKCHIP ==="
gst-inspect-1.0 | grep -i rk 2>/dev/null
gst-inspect-1.0 | grep -i mpp 2>/dev/null

echo "=== RTSP SERVER ==="
which gst-rtsp-server 2>/dev/null
dpkg -l | grep -i rtsp 2>/dev/null
gst-inspect-1.0 | grep rtsp 2>/dev/null

echo "=== LIBGPIOD ==="
dpkg -l | grep -i gpiod 2>/dev/null
which gpiodetect gpioinfo gpioset gpioget 2>/dev/null

echo "=== I2C TOOLS ==="
dpkg -l | grep -i i2c-tools 2>/dev/null
which i2cdetect i2cget i2cset 2>/dev/null

echo "=== SD CARD MOUNT ==="
mount | grep mmc 2>/dev/null
ls /dev/mmcblk* 2>/dev/null

echo "=== SYSTEMD ==="
systemctl --version 2>/dev/null | head -1
'''

host = '192.168.137.59'
user = 'pi'
password = 'pi'

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(host, username=user, password=password, timeout=15)

# Upload script
sftp = client.open_sftp()
with sftp.open('/tmp/inspect_hw.sh', 'w') as f:
    f.write(SCRIPT)
sftp.close()

# Run it
stdin, stdout, stderr = client.exec_command('bash /tmp/inspect_hw.sh', timeout=30)
import os
os.environ.setdefault('PYTHONIOENCODING', 'utf-8')
out = stdout.read().decode('utf-8', errors='replace')
sys.stdout.buffer.write(out.encode('utf-8', errors='replace'))
err = stderr.read().decode('utf-8', errors='replace')
if err:
    print("STDERR:", err, file=sys.stderr)
client.close()
