import socket

# Configura o computador para escutar em todas as suas placas de rede na porta 4242
UDP_IP = "0.0.0.0"  
UDP_PORT = 4242

# Cria o socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print("=== Escutando o Arduino na porta 4242... Mexa o sensor! ===")

try:
    while True:
        # Fica esperando um pacote chegar
        data, addr = sock.recvfrom(1024) 
        # Decodifica e exibe na tela
        print(f"Vindo de {addr[0]} -> Ângulos: {data.decode('utf-8')}")
except KeyboardInterrupt:
    print("\nParando escuta...")
finally:
    sock.close()