#!/usr/bin/env python3

from time import sleep
import socket

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

RECV_DATA_FROM_SERVER = False

def recv(sock):
    data = b''
    while True:
        char = s.recv(1)
        if char == b"\x00": break
        data += char
    text = data.decode()
    print("Recv:", text)
    return text

def send(socket, text):
    socket.sendall(text.encode() + b'\x00')
    print("Send:", text)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    sleep(3) # Wait for QEMU boot

    recv(s)
    send(s, '$GPGGA,121314.56,4124.8934,N,08151.6849,W')

    sleep(1000)

    print('Connection Closed')

print('Client Ended!')
