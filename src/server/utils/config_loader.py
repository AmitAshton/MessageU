import os

class ConfigLoader:
    DEFAULT_PORT = 1357
    DEFAULT_DB_PATH = "defensive.db"
    DEFAULT_LOG_PATH = "server.log"
    SERVER_VERSION = 2

    def __init__(self, config_file="myport.info"):
        self.config_file = config_file
        self.port = self._load_port()
        self.db_path = self.DEFAULT_DB_PATH
        self.log_path = self.DEFAULT_LOG_PATH
        self.version = self.SERVER_VERSION

    def _load_port(self) -> int:
        if os.path.exists(self.config_file):
            try:
                with open(self.config_file, "r") as file:
                    raw_value = file.read().strip()
                    return int(raw_value)
            except Exception:
                print(f"Invalid or unreadable {self.config_file}. Using default port {self.DEFAULT_PORT}.")
        else:
            print(f"Warning: {self.config_file} not found. Using default port {self.DEFAULT_PORT}.")
        return self.DEFAULT_PORT

    def as_dict(self):
        return {
            "port": self.port,
            "db_path": self.db_path,
            "log_path": self.log_path,
            "version": self.version
        }
