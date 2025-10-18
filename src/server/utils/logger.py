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
        dir_name = os.path.dirname(log_file)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)

        self.logger = logging.getLogger("MessageUServer")
        if not self.logger.handlers:  # prevent duplication
            self.logger.setLevel(logging.DEBUG)
            fmt = logging.Formatter("%(asctime)s [%(levelname)s] [%(threadName)s] %(message)s")

            file_handler = logging.FileHandler(log_file, mode="a", encoding="utf-8", delay=False)
            stream_handler = logging.StreamHandler()

            file_handler.setFormatter(fmt)
            stream_handler.setFormatter(fmt)

            self.logger.addHandler(file_handler)
            self.logger.addHandler(stream_handler)

    def _flush(self):
        for h in self.logger.handlers:
            h.flush()

    def info(self, message):
        self.logger.info(message)
        self._flush()

    def debug(self, message):
        self.logger.debug(message)
        self._flush()

    def warning(self, message):
        self.logger.warning(message)
        self._flush()

    def error(self, message):
        self.logger.error(message)
        self._flush()

    def exception(self, message):
        self.logger.exception(message)
        self._flush()

    def separator(self):
        self.logger.info("=" * 80)
        self._flush()

    def session_log(self, client_id: str, event: str, extra: str = ""):
        self.logger.info(f"[Client: {client_id}] {event} {extra}")
        self._flush()
