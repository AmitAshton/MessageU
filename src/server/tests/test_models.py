from models.client import ClientRecord
from models.message import MessageRecord
import uuid

def test_client():
    c = ClientRecord("Alice", b"key123")
    print(c.to_dict())

def test_message():
    m = MessageRecord(uuid.uuid4(), uuid.uuid4(), 3, b"hello")
    print(m.to_dict())

if __name__ == "__main__":
    test_client()
    test_message()
