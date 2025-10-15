import os
from storage.db_manager import DatabaseManager
from models.client import ClientRecord
from models.message import MessageRecord
from protocol.enums import MessageType


def test_create_tables_and_insert(db: DatabaseManager):
    c = ClientRecord("TestUser", b"PubKey")
    db.add_client(c)
    result = db.get_client_by_username("TestUser")
    assert result.username == "TestUser"

def test_message_storage_and_retrieval(db: DatabaseManager):
    c1 = ClientRecord("Alice", b"k1")
    c2 = ClientRecord("Bob", b"k2")
    db.add_client(c1)
    db.add_client(c2)
    msg = MessageRecord(c2.id, c1.id, MessageType.TEXT_MESSAGE, b"payload")
    db.save_message(msg)
    found = db.get_pending_messages(c2.id)
    assert len(found) == 1
    db.delete_message(msg.id)
    assert len(db.get_pending_messages(c2.id)) == 0

def test_injection_protection(db: DatabaseManager):
    dangerous_name = "Evil'; DROP TABLE clients; --"
    c = ClientRecord(dangerous_name, b"k")
    db.add_client(c)
    assert db.get_client_by_username(dangerous_name).username == dangerous_name

def test_missing_db_file(tmp_path):
    db_path = tmp_path / "nonexistent.db"
    db = DatabaseManager(str(db_path))
    assert os.path.exists(db_path)
