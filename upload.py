import paramiko
import os
import sys

host = '192.168.137.59'
user = 'pi'
password = 'pi'

client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(host, username=user, password=password, timeout=15)

sftp = client.open_sftp()

def upload_dir(local_dir, remote_dir):
    try:
        sftp.mkdir(remote_dir)
    except:
        pass
    for item in os.listdir(local_dir):
        local_path = os.path.join(local_dir, item)
        remote_path = f"{remote_dir}/{item}"
        if os.path.isfile(local_path):
            print(f"Uploading {local_path} to {remote_path}")
            sftp.put(local_path, remote_path)
        elif os.path.isdir(local_path):
            upload_dir(local_path, remote_path)

try:
    sftp.mkdir('larscope_build')
except:
    pass

upload_dir('shared', 'larscope_build/shared')
upload_dir('device_a', 'larscope_build/device_a')
upload_dir('device_b', 'larscope_build/device_b')
sftp.put('config.json', 'larscope_build/config.json')

sftp.close()
client.close()
print("Upload complete")
