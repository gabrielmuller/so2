#!/usr/bin/env python3

import socket

HOST = "127.0.0.1"  # The server"s hostname or IP address
PORT = 65432        # The port used by the server

RECV_DATA_FROM_SERVER = False

def recv_data_from_server(sock):
    data = s.recv(1024)
    print("Received from Back-end: ", data.decode())


def serialize(sock):
    while True:
        mes = input("Message: ")
        if mes == "":
            break
        sock.sendall(mes.encode() + b"\x00")
        if RECV_DATA_FROM_SERVER:
            recv_data_from_server(sock)


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    serialize(s)
    print("Connection Closed")

print("Client Ended!")
