import socket
import threading
import time
import pytest
from server import Server


@pytest.mark.timeout(5)
def test_server_starts_and_accepts_connections(monkeypatch):
    s = Server()
    t = threading.Thread(target=s.run, daemon=True)
    t.start()
    time.sleep(0.5)
    c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    c.connect(("127.0.0.1", s.port))
    c.close()
