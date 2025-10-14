from utils.config_loader import ConfigLoader
from utils.logger import ServerLogger
import socket
import threading

class Server:
    def __init__(self):
        self.config = ConfigLoader()
        self.logger = ServerLogger(self.config.log_path)
        self.host = "0.0.0.0"
        self.port = self.config.port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((self.host, self.port))
        self.socket.listen(5)
        self.logger.info(f"Server started on port {self.port}")

    def handle_client(self, conn, addr):
        self.logger.info(f"New connection from {addr}")
        conn.close()

    def run(self):
        self.logger.separator()
        self.logger.info("Server is running and waiting for connections.")
        while True:
            conn, addr = self.socket.accept()
            self.logger.info(f"New connection from {addr}")
            threading.Thread(target=self.handle_client, args=(conn, addr), daemon=True).start()
