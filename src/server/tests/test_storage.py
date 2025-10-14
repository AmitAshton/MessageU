from storage.db_manager import DatabaseManager
from models.client import ClientRecord
from models.message import MessageRecord
import os


def reset_db(path="defensive.db"):
    """Deletes the existing DB for a clean test run"""
    if os.path.exists(path):
        os.remove(path)


def test_database_operations():
    reset_db()
    db = DatabaseManager("defensive.db")

    # --- Create and add clients ---
    alice = ClientRecord("Alice", b"alice_public_key")
    bob = ClientRecord("Bob", b"bob_public_key")

    db.add_client(alice)
    db.add_client(bob)

    print("=== Clients added ===")
    for client in db.list_clients():
        print(client.to_dict())

    # --- Retrieve clients ---
    retrieved = db.get_client_by_username("Alice")
    print("\nRetrieved Alice:", retrieved.to_dict())

    retrieved_by_id = db.get_client_by_id(bob.id)
    print("\nRetrieved Bob by ID:", retrieved_by_id.to_dict())

    # --- Send a message ---
    msg = MessageRecord(
        to_client=bob.id,
        from_client=alice.id,
        msg_type=3,
        content=b"Encrypted message from Alice"
    )

    db.save_message(msg)
    print("\nMessage saved:", msg.to_dict())

    # --- Fetch pending messages for Bob ---
    pending = db.get_pending_messages(bob.id)
    print("\nPending messages for Bob:")
    for p in pending:
        print(p.to_dict())

    # --- Delete the message ---
    db.delete_message(msg.id)
    print("\nAfter deletion, Bob’s pending messages:", db.get_pending_messages(bob.id))


if __name__ == "__main__":
    test_database_operations()
