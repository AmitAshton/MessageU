import struct
import uuid
from typing import Tuple

class PayloadParser:
    @staticmethod
    def parse_register_payload(data: bytes) -> Tuple[str, bytes]:
        name = data[:255].split(b'\x00', 1)[0].decode('ascii')
        public_key = data[255:255+160]
        return name, public_key

    @staticmethod
    def parse_send_message_payload(data: bytes):
        dest_id = uuid.UUID(bytes=data[:16])
        msg_type = data[16]
        size = struct.unpack("<I", data[17:21])[0]
        content = data[21:21+size]
        return dest_id, msg_type, content

    @staticmethod
    def parse_pull_payload(data: bytes):
        return uuid.UUID(bytes=data[:16])
