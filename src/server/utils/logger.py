import logging
import threading
import os

class ServerLogger:
    _instance = None
    _lock = threading.Lock()

    def __new__(cls, log_file="server.log"):
        with cls._lock:
            if cls._instance is None:
                cls._instance = super(ServerLogger, cls).__new__(cls)
                cls._instance._initialize(log_file)
        return cls._instance

    def _initialize(self, log_file):
        self.log_file = log_file
        os.makedirs(os.path.dirname(log_file), exist_ok=True) if os.path.dirname(log_file) else None

        logging.basicConfig(
            level=logging.DEBUG,
            format="%(asctime)s [%(levelname)s] [%(threadName)s] %(message)s",
            handlers=[
                logging.FileHandler(log_file, mode="a", encoding="utf-8"),
                logging.StreamHandler()
            ]
        )
        self.logger = logging.getLogger("MessageUServer")

    def info(self, message):
        self.logger.info(message)

    def debug(self, message):
        self.logger.debug(message)

    def error(self, message):
        self.logger.error(message)

    def exception(self, message):
        self.logger.exception(message)

    def separator(self):
        self.logger.info("=" * 80)
