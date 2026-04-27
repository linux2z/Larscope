# remote_exec.py helper
import paramiko
import sys

def execute_remote(cmd):
    host = '192.168.137.59'
    user = 'pi'
    password = 'pi'
    
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(host, username=user, password=password)
    
    stdin, stdout, stderr = client.exec_command(cmd, get_pty=True)
    stdin.write(password + '\n')
    stdin.flush()
    
    output = stdout.read().decode()
    error = stderr.read().decode()
    exit_status = stdout.channel.recv_exit_status()
    
    client.close()
    return output, error, exit_status

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.stdout.buffer.write(b"Usage: python remote_exec.py <command>\n")
        sys.exit(1)
        
    out, err, status = execute_remote(sys.argv[1])
    if out:
        print(out)
    if err:
        sys.stderr.buffer.write(err.encode('utf-8', errors='replace'))
    sys.exit(1)
finally:
    client.close()
