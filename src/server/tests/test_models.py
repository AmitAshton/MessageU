import uuid
from models.client import ClientRecord
from models.message import MessageRecord
from protocol.enums import MessageType


def test_client_record_serialization():
    c = ClientRecord("Alice", b"key")
    d = c.to_dict()
    assert d["username"] == "Alice"
    assert isinstance(uuid.UUID(d["id"]), uuid.UUID)

def test_message_record_structure():
    m = MessageRecord(uuid.uuid4(), uuid.uuid4(), MessageType.TEXT_MESSAGE, b"abc")
    d = m.to_dict()
    assert d["content"] in ["616263", "abc"]
