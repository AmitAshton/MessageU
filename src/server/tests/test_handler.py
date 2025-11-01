import pytest
from handler import RequestHandler
from models.client import ClientRecord
from models.message import MessageRecord
from protocol.enums import RequestCode, ResponseCode, MessageType
from protocol.header import ResponseHeader
import uuid

class DummySocket:
    def __init__(self, data=b""):
        self.recv_data = data
        self.sent = b""
    
    def recv(self, n):
        if not self.recv_data:
            return b""
        chunk = self.recv_data[:n]
        self.recv_data = self.recv_data[n:]
        return chunk

    def sendall(self, data): self.sent += data
    def close(self): pass

def get_response_code(data: bytes) -> int:
    return int.from_bytes(data[1:3], "little")

@pytest.fixture
def handler_factory(db):
    def _handler_factory():
        h = RequestHandler(DummySocket(), ("0.0.0.0", 1111))
        h.db = db
        return h
    return _handler_factory

def test_register_duplicate(handler_factory):
    h = handler_factory()
    h.db.add_client(ClientRecord("UserA", b"k"))

    payload_data = b"UserA\x00" + b"\x00" * (255 - 6) + b"A" * 160
    resp = h._handle_register(payload_data)
    assert get_response_code(resp) == ResponseCode.GENERAL_ERROR

def test_pull_messages_empty(handler_factory):
    h = handler_factory()
    client_id = uuid.uuid4()
    resp = h._handle_pull_messages(client_id)
    assert get_response_code(resp) == ResponseCode.PENDING_MESSAGES

def test_handler_client_list_success(handler_factory):
    h = handler_factory()
    c1 = ClientRecord("Alice", b"k1")
    h.db.add_client(c1)
    
    resp = h._handle_client_list(uuid.uuid4())
    assert get_response_code(resp) == ResponseCode.CLIENT_LIST
    assert b"Alice" in resp

def test_handler_public_key_success(handler_factory):
    h = handler_factory()
    c1 = ClientRecord("Alice", b"k1k1k1k1k1k1k1k1k1k1")
    h.db.add_client(c1)
    
    payload = c1.id.bytes
    resp = h._handle_public_key(payload)
    
    assert get_response_code(resp) == ResponseCode.PUBLIC_KEY
    assert c1.id.bytes in resp
    assert b"k1k1k1k1k1k1k1k1k1k1" in resp

def test_handler_send_message_success(handler_factory):
    h = handler_factory()
    sender = ClientRecord("Sender", b"ks")
    dest = ClientRecord("Dest", b"kd")
    h.db.add_client(sender)
    h.db.add_client(dest)
    
    content = b"encrypted_message"
    payload = dest.id.bytes + MessageType.TEXT_MESSAGE.to_bytes(1, "little") + len(content).to_bytes(4, "little") + content
    
    resp = h._handle_send_message(sender.id, payload)
    
    assert get_response_code(resp) == ResponseCode.MESSAGE_STORED
    
    pending = h.db.get_pending_messages(dest.id)
    assert len(pending) == 1
    assert pending[0].content == content
    assert pending[0].from_client == sender.id

def test_handler_pull_messages_with_data(handler_factory):
    h = handler_factory()
    c1 = ClientRecord("Sender", b"ks")
    c2 = ClientRecord("Receiver", b"kr")
    h.db.add_client(c1)
    h.db.add_client(c2)
    
    msg1 = MessageRecord(c2.id, c1.id, MessageType.TEXT_MESSAGE, b"Msg1")
    msg2 = MessageRecord(c2.id, c1.id, MessageType.REQUEST_SYM_KEY, b"")
    h.db.save_message(msg1)
    h.db.save_message(msg2)
    
    resp = h._handle_pull_messages(c2.id)
    
    assert get_response_code(resp) == ResponseCode.PENDING_MESSAGES
    assert len(resp) > 7
    
    assert len(h.db.get_pending_messages(c2.id)) == 0