import socket
import threading
import os
from handler import RequestHandler

class Server:
    def __init__(self):
        self.host = "0.0.0.0"
        self.port = self.load_port()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((self.host, self.port))
        self.socket.listen(5)
        print(f"Server started on port {self.port}")

    def load_port(self):
        try:
            with open("info.myport", "r") as f:
                return int(f.read().strip())
        except FileNotFoundError:
            print("Warning: info.myport not found. Using default port 1357.")
            return 1357

    def handle_client(self, conn, addr):
        handler = RequestHandler(conn, addr)
        handler.process()

    def run(self):
        while True:
            conn, addr = self.socket.accept()
            print(f"New connection from {addr}")
            threading.Thread(target=self.handle_client, args=(conn, addr)).start()
