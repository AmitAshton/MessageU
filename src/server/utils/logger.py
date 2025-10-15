import logging
import threading
import os

class ServerLogger:
    _instance = None
    _lock = threading.Lock()

    def __new__(cls, log_file="server.log"):
        with cls._lock:
            instance = super(ServerLogger, cls).__new__(cls)
            instance._initialize(log_file)
            return instance

    def _initialize(self, log_file):
        self.log_file = log_file
        dir_name = os.path.dirname(log_file)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)

        # Always create and close immediately so the file exists
        with open(log_file, "a") as f:
            f.write("")
        os.utime(log_file, None)

        self.logger = logging.getLogger(f"MessageUServer_{id(self)}")
        self.logger.setLevel(logging.DEBUG)
        self.logger.handlers.clear()

        fh = logging.FileHandler(log_file, mode="a", encoding="utf-8", delay=False)
        sh = logging.StreamHandler()
        fmt = logging.Formatter("%(asctime)s [%(levelname)s] [%(threadName)s] %(message)s")
        fh.setFormatter(fmt)
        sh.setFormatter(fmt)
        self.logger.addHandler(fh)
        self.logger.addHandler(sh)

        self._force_flush_close_open()

    def _force_flush_close_open(self):
        """Force-create and release file handle for immediate visibility on Windows."""
        for h in list(self.logger.handlers):
            if isinstance(h, logging.FileHandler):
                h.flush()
                h.close()
        # Reopen a fresh handler so logging continues safely
        fh = logging.FileHandler(self.log_file, mode="a", encoding="utf-8", delay=False)
        fmt = logging.Formatter("%(asctime)s [%(levelname)s] [%(threadName)s] %(message)s")
        fh.setFormatter(fmt)
        self.logger.addHandler(fh)

    def _ensure_file(self):
        if not os.path.exists(self.log_file):
            with open(self.log_file, "a") as f:
                f.write("")
        os.utime(self.log_file, None)

    def info(self, message):
        self.logger.info(message)
        self._force_flush_close_open()
        self._ensure_file()

    def debug(self, message):
        self.logger.debug(message)
        self._force_flush_close_open()
        self._ensure_file()

    def warning(self, message):
        self.logger.warning(message)
        self._force_flush_close_open()
        self._ensure_file()

    def error(self, message):
        self.logger.error(message)
        self._force_flush_close_open()
        self._ensure_file()

    def exception(self, message):
        self.logger.exception(message)
        self._force_flush_close_open()
        self._ensure_file()

    def separator(self):
        self.logger.info("=" * 80)
        self._force_flush_close_open()
        self._ensure_file()

    def session_log(self, client_id: str, event: str, extra: str = ""):
        self.logger.info(f"[Client: {client_id}] {event} {extra}")
        self._force_flush_close_open()
        self._ensure_file()
