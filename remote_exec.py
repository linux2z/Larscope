import paramiko
import sys
import os

if len(sys.argv) < 2:
    sys.stdout.buffer.write(b"Usage: python remote_exec.py <command>\n")
    sys.exit(1)

host = '192.168.137.59'
user = 'pi'
password = 'pi'
command = sys.argv[1]

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
try:
    client.connect(host, username=user, password=password, timeout=15)
    stdin, stdout, stderr = client.exec_command(command, timeout=120)
    out = stdout.read().decode('utf-8', errors='replace')
    err = stderr.read().decode('utf-8', errors='replace')
    if out:
        sys.stdout.buffer.write(out.encode('utf-8', errors='replace'))
        sys.stdout.buffer.write(b'\n')
    if err:
        sys.stderr.buffer.write(err.encode('utf-8', errors='replace'))
        sys.stderr.buffer.write(b'\n')
    sys.exit(stdout.channel.recv_exit_status())
except Exception as e:
    sys.stderr.buffer.write(f"SSH Error: {e}\n".encode('utf-8'))
    sys.exit(1)
finally:
    client.close()
