import socket

HOST = '0.0.0.0'  
PORT = 5000      

def get_raw_from_kernel():
    try:
        with open("/dev/hcsr04", "r") as f:
            return f.read().strip()
    except:
        return "Error"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Serveur Ethernet actif sur le port {PORT}...")
    while True:
        conn, addr = s.accept()
        with conn:
            data = get_raw_from_kernel()
            conn.sendall(data.encode()) 