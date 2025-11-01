import os
from storage.db_manager import DatabaseManager
from models.client import ClientRecord
from models.message import MessageRecord
from protocol.enums import MessageType
import uuid

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

def test_list_clients(db: DatabaseManager):
    db.add_client(ClientRecord("U1", b"k1"))
    db.add_client(ClientRecord("U2", b"k2"))
    clients = db.list_clients()
    assert len(clients) == 2
    assert "U1" in [c.username for c in clients]

def test_get_client_not_found(db: DatabaseManager):
    assert db.get_client_by_username("Missing") is None
    assert db.get_client_by_id(uuid.uuid4()) is None

def test_message_delete_on_pull(db: DatabaseManager):
    c = ClientRecord("Sender", b"k")
    db.add_client(c)
    msg1 = MessageRecord(c.id, c.id, MessageType.TEXT_MESSAGE, b"1")
    msg2 = MessageRecord(c.id, c.id, MessageType.TEXT_MESSAGE, b"2")
    db.save_message(msg1)
    db.save_message(msg2)
    
    db.delete_message(msg1.id)
    found = db.get_pending_messages(c.id)
    assert len(found) == 1
    assert found[0].id == msg2.id