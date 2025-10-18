import struct
import uuid
from typing import Tuple

class PayloadParser:
    @staticmethod
    def parse_register_payload(data: bytes) -> Tuple[str, bytes]:
        """
        Correctly parse a registration payload of the form:
        username (null-terminated ASCII) + DER-encoded public key.
        """
        try:
            sep = data.index(0)
            name = data[:sep].decode("ascii")
            public_key = data[sep + 1:]
            if not name or not public_key:
                raise ValueError
            return name, public_key
        except Exception:
            raise ValueError("Malformed registration payload")

    @staticmethod
    def parse_send_message_payload(data: bytes):
        dest_id = uuid.UUID(bytes=data[:16])
        msg_type = data[16]
        size = struct.unpack("<I", data[17:21])[0]
        content = data[21:21 + size]
        return dest_id, msg_type, content

    @staticmethod
    def parse_pull_payload(data: bytes):
        return uuid.UUID(bytes=data[:16])
