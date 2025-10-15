import pytest
from handler import RequestHandler
from models.client import ClientRecord
import uuid

class DummySocket:
    def __init__(self, data=b""):
        self.data = data
        self.sent = b""
    def recv(self, n): return b""
    def sendall(self, data): self.sent += data
    def close(self): pass

def test_register_duplicate(monkeypatch, tmp_path):
    handler = RequestHandler(None, ("127.0.0.1", 5555))
    handler.db.clear_all()
    handler.db.add_client(ClientRecord("UserA", b"k"))

    with pytest.raises(ValueError):
        handler.db.add_client(ClientRecord("UserA", b"k"))

def test_send_message_invalid_dest(monkeypatch):
    h = RequestHandler(DummySocket(), ("0.0.0.0", 1111))
    payload = (uuid.uuid4(), 3, b"msg")
    r = h._handle_send_message(uuid.uuid4(), payload)
    assert r is not None

def test_pull_messages_empty():
    h = RequestHandler(DummySocket(), ("0.0.0.0", 2222))
    client_id = uuid.uuid4()
    resp = h._handle_pull_messages(client_id)
    assert isinstance(resp, bytes)
