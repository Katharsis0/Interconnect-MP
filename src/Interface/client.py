# backend.py
import socket
import json

def create_socket():
    return socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def connect_to_server(sock, host='127.0.0.1', port=8080):
    sock.connect((host, port))

def send_message(sock, message):
    sock.sendall(message.encode())

def receive_data(sock):
    data = sock.recv(4096).decode()
    return json.loads(data)  # Esperamos un JSON con los datos

def close_socket(sock):
    sock.close()

# Función llamada por los botones
def get_button_action(sock, action_type):
    if action_type == "step":
        send_message(sock, "STEP")
    elif action_type == "clk":
        send_message(sock, "CLK")
    elif action_type == "reiniciar":
        send_message(sock, "RESET")

    # Esperamos datos actualizados
    datos = receive_data(sock)
    return datos  # Debe ser un diccionario con 'metricas' y 'pe'
