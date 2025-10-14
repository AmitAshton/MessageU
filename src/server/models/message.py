import uuid
from typing import Dict, Any
from .base_record import BaseRecord

class MessageRecord(BaseRecord):
    __slots__ = ("_id", "_to_client", "_from_client", "_msg_type", "_content")

    def __init__(
        self,
        to_client: uuid.UUID,
        from_client: uuid.UUID,
        msg_type: int,
        content: bytes,
        message_id: uuid.UUID = None
    ):
        self._id = message_id or uuid.uuid4()
        self._to_client = to_client
        self._from_client = from_client
        self._msg_type = msg_type
        self._content = content

    @property
    def id(self) -> uuid.UUID:
        return self._id

    @property
    def to_client(self) -> uuid.UUID:
        return self._to_client

    @property
    def from_client(self) -> uuid.UUID:
        return self._from_client

    @property
    def msg_type(self) -> int:
        return self._msg_type

    @property
    def content(self) -> bytes:
        return self._content

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": str(self._id),
            "to_client": str(self._to_client),
            "from_client": str(self._from_client),
            "msg_type": self._msg_type,
            "content": self._content.hex()
        }

    def from_dict(self, data: Dict[str, Any]):
        self._id = uuid.UUID(data["id"])
        self._to_client = uuid.UUID(data["to_client"])
        self._from_client = uuid.UUID(data["from_client"])
        self._msg_type = int(data["msg_type"])
        self._content = bytes.fromhex(data["content"])
