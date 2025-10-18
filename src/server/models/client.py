import uuid
from datetime import datetime, timezone
from typing import Dict, Any
from .base_record import BaseRecord


class ClientRecord(BaseRecord):
    __slots__ = ("_id", "_username", "_public_key", "_last_seen")

    def __init__(self, username: str, public_key: bytes, client_id: uuid.UUID = None):
        # ensure username includes null terminator at storage level
        if not username.endswith("\x00"):
            username += "\x00"

        self._id = client_id or uuid.uuid4()
        self._username = username  # stored as str but encoded as BLOB on insert
        self._public_key = public_key
        self._last_seen = datetime.now(timezone.utc)

    @property
    def id(self) -> uuid.UUID:
        return self._id

    @property
    def username(self) -> str:
        # when reading from DB (BLOB), convert to string and strip the null
        return self._username.rstrip("\x00")

    @property
    def username_raw(self) -> bytes:
        # return the raw bytes including the null terminator
        return self._username.encode("ascii")

    @property
    def public_key(self) -> bytes:
        return self._public_key

    @property
    def last_seen(self) -> datetime:
        return self._last_seen

    def update_last_seen(self):
        self._last_seen = datetime.now(timezone.utc)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": str(self._id),
            "username": self._username,
            "public_key": self._public_key.hex(),
            "last_seen": self._last_seen.isoformat(),
        }

    def from_dict(self, data: Dict[str, Any]):
        self._id = uuid.UUID(data["id"])
        self._username = data["username"]
        self._public_key = bytes.fromhex(data["public_key"])
        self._last_seen = datetime.fromisoformat(data["last_seen"])
