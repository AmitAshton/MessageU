import sqlite3
from typing import List, Optional
from models.client import ClientRecord
from models.message import MessageRecord
from datetime import datetime
import uuid
import os

class DatabaseManager:
    def __init__(self, db_path: str = "defensive.db"):
        self.db_path = db_path
        self._ensure_database()

    def _ensure_database(self):
        """Create database and tables if they don't exist"""
        os.makedirs(os.path.dirname(self.db_path), exist_ok=True) if os.path.dirname(self.db_path) else None
        with sqlite3.connect(self.db_path) as conn:
            conn.execute("""
                CREATE TABLE IF NOT EXISTS clients (
                    id TEXT PRIMARY KEY,
                    username TEXT UNIQUE,
                    public_key TEXT,
                    last_seen TEXT
                )
            """)
            conn.execute("""
                CREATE TABLE IF NOT EXISTS messages (
                    id TEXT PRIMARY KEY,
                    to_client TEXT,
                    from_client TEXT,
                    msg_type INTEGER,
                    content TEXT
                )
            """)
            conn.commit()

    # ---------- CLIENTS ----------
    def add_client(self, client: ClientRecord) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "INSERT INTO clients (id, username, public_key, last_seen) VALUES (?, ?, ?, ?)",
                (str(client.id), client.username, client.public_key.hex(), client.last_seen.isoformat())
            )
            conn.commit()

    def get_client_by_username(self, username: str) -> Optional[ClientRecord]:
        with sqlite3.connect(self.db_path) as conn:
            row = conn.execute(
                "SELECT id, username, public_key, last_seen FROM clients WHERE username = ?", (username,)
            ).fetchone()
            if not row:
                return None
            c = ClientRecord(row[1], bytes.fromhex(row[2]), uuid.UUID(row[0]))
            c._last_seen = datetime.fromisoformat(row[3])
            return c

    def get_client_by_id(self, client_id: uuid.UUID) -> Optional[ClientRecord]:
        with sqlite3.connect(self.db_path) as conn:
            row = conn.execute(
                "SELECT id, username, public_key, last_seen FROM clients WHERE id = ?", (str(client_id),)
            ).fetchone()
            if not row:
                return None
            c = ClientRecord(row[1], bytes.fromhex(row[2]), uuid.UUID(row[0]))
            c._last_seen = datetime.fromisoformat(row[3])
            return c

    def list_clients(self) -> List[ClientRecord]:
        with sqlite3.connect(self.db_path) as conn:
            rows = conn.execute("SELECT id, username, public_key, last_seen FROM clients").fetchall()
            clients = []
            for row in rows:
                c = ClientRecord(row[1], bytes.fromhex(row[2]), uuid.UUID(row[0]))
                c._last_seen = datetime.fromisoformat(row[3])
                clients.append(c)
            return clients

    # ---------- MESSAGES ----------
    def save_message(self, message: MessageRecord) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "INSERT INTO messages (id, to_client, from_client, msg_type, content) VALUES (?, ?, ?, ?, ?)",
                (
                    str(message.id),
                    str(message.to_client),
                    str(message.from_client),
                    message.msg_type,
                    message.content.hex(),
                ),
            )
            conn.commit()

    def get_pending_messages(self, client_id: uuid.UUID) -> List[MessageRecord]:
        with sqlite3.connect(self.db_path) as conn:
            rows = conn.execute(
                "SELECT id, to_client, from_client, msg_type, content FROM messages WHERE to_client = ?",
                (str(client_id),),
            ).fetchall()
            messages = []
            for row in rows:
                messages.append(
                    MessageRecord(
                        uuid.UUID(row[1]),
                        uuid.UUID(row[2]),
                        int(row[3]),
                        bytes.fromhex(row[4]),
                        uuid.UUID(row[0]),
                    )
                )
            return messages

    def delete_message(self, message_id: uuid.UUID) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute("DELETE FROM messages WHERE id = ?", (str(message_id),))
            conn.commit()
