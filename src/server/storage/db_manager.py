import sqlite3
from typing import List, Optional
from models.client import ClientRecord
from models.message import MessageRecord
from datetime import datetime
import uuid
import os
import threading


class DatabaseManager:
    """Thread-safe SQLite database manager with BLOB username + key storage."""

    def __init__(self, db_path: str = "defensive.db"):
        self.db_path = db_path
        self._lock = threading.Lock()
        self.conn = sqlite3.connect(self.db_path, check_same_thread=False)
        self.conn.execute("PRAGMA foreign_keys = ON;")
        self._ensure_database()

    # ---------- Initialization ----------
    def _ensure_database(self) -> None:
        """Create database tables if they do not exist."""
        if os.path.dirname(self.db_path):
            os.makedirs(os.path.dirname(self.db_path), exist_ok=True)
        with self._lock:
            self.conn.execute("""
                CREATE TABLE IF NOT EXISTS clients (
                    id TEXT PRIMARY KEY,
                    username BLOB UNIQUE,
                    public_key BLOB,
                    last_seen TEXT
                )
            """)
            self.conn.execute("""
                CREATE TABLE IF NOT EXISTS messages (
                    id TEXT PRIMARY KEY,
                    to_client TEXT,
                    from_client TEXT,
                    msg_type INTEGER,
                    content BLOB
                )
            """)
            self.conn.commit()

    # ---------- Client Management ----------
    def add_client(self, client: ClientRecord) -> None:
        with self._lock:
            try:
                self.conn.execute(
                    "INSERT INTO clients (id, username, public_key, last_seen) VALUES (?, ?, ?, ?)",
                    (
                        str(client.id),
                        client.username_raw,  # store as raw BLOB including null
                        client.public_key,
                        client.last_seen.isoformat(),
                    ),
                )
                self.conn.commit()
            except sqlite3.IntegrityError as e:
                raise ValueError(f"Client with username '{client.username}' already exists") from e

    def get_client_by_username(self, username: str) -> Optional[ClientRecord]:
        # include null termination when searching
        name_bytes = username.encode("ascii") + b"\x00"
        with self._lock:
            row = self.conn.execute(
                "SELECT id, username, public_key, last_seen FROM clients WHERE username = ?",
                (name_bytes,),
            ).fetchone()
            if not row:
                return None
            username_clean = row[1].decode("ascii")
            c = ClientRecord(username_clean, row[2], uuid.UUID(row[0]))
            c._last_seen = datetime.fromisoformat(row[3])
            return c

    def get_client_by_id(self, client_id: uuid.UUID) -> Optional[ClientRecord]:
        with self._lock:
            row = self.conn.execute(
                "SELECT id, username, public_key, last_seen FROM clients WHERE id = ?",
                (str(client_id),),
            ).fetchone()
            if not row:
                return None
            username_clean = row[1].decode("ascii")
            c = ClientRecord(username_clean, row[2], uuid.UUID(row[0]))
            c._last_seen = datetime.fromisoformat(row[3])
            return c

    def list_clients(self) -> List[ClientRecord]:
        with self._lock:
            rows = self.conn.execute("SELECT id, username, public_key, last_seen FROM clients").fetchall()
            clients = []
            for row in rows:
                username_clean = row[1].decode("ascii")
                c = ClientRecord(username_clean, row[2], uuid.UUID(row[0]))
                c._last_seen = datetime.fromisoformat(row[3])
                clients.append(c)
            return clients

    # ---------- Message Management ----------
    def save_message(self, message: MessageRecord) -> None:
        with self._lock:
            self.conn.execute(
                "INSERT INTO messages (id, to_client, from_client, msg_type, content) VALUES (?, ?, ?, ?, ?)",
                (
                    str(message.id),
                    str(message.to_client),
                    str(message.from_client),
                    message.msg_type,
                    message.content,
                ),
            )
            self.conn.commit()

    def get_pending_messages(self, client_id: uuid.UUID) -> List[MessageRecord]:
        with self._lock:
            rows = self.conn.execute(
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
                        row[4],
                        uuid.UUID(row[0]),
                    )
                )
            return messages

    def delete_message(self, message_id: uuid.UUID) -> None:
        with self._lock:
            self.conn.execute("DELETE FROM messages WHERE id = ?", (str(message_id),))
            self.conn.commit()

    # ---------- Utilities ----------
    def clear_all(self) -> None:
        with self._lock:
            self.conn.execute("DELETE FROM clients")
            self.conn.execute("DELETE FROM messages")
            self.conn.commit()

    def close(self):
        with self._lock:
            try:
                self.conn.commit()
                self.conn.close()
            except Exception:
                pass
