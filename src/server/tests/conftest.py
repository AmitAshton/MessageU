import sys
import os

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
SERVER_DIR = os.path.join(BASE_DIR, "server")
sys.path.insert(0, BASE_DIR)
sys.path.insert(0, SERVER_DIR)

import pytest
from storage.db_manager import DatabaseManager
from utils.crypto_utils import CryptoUtils
from utils.logger import ServerLogger

@pytest.fixture(scope="session", autouse=True)
def setup_env(tmp_path_factory):
    os.environ["ENV"] = "test"
    log_path = tmp_path_factory.mktemp("logs") / "test.log"
    ServerLogger(str(log_path))
    yield

@pytest.fixture(scope="function")
def db(tmp_path):
    db_path = tmp_path / "defensive.db"
    db = DatabaseManager(str(db_path))
    yield db
    db.close()
    try:
        os.remove(db_path)
    except Exception:
        pass

    try:
        if hasattr(db, "conn"):
            db.conn.close()
        if os.path.exists("defensive.db"):
            os.remove("defensive.db")
    except PermissionError:
        pass

@pytest.fixture
def crypto():
    return CryptoUtils()
