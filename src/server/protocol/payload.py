import struct
import uuid
from typing import Tuple

class PayloadParser:
    @staticmethod
    def parse_register_payload(data: bytes) -> Tuple[str, bytes]:
        NAME_FIELD_SIZE = 255
        KEY_FIELD_SIZE = 160
        EXPECTED_SIZE = NAME_FIELD_SIZE + KEY_FIELD_SIZE

        if len(data) != EXPECTED_SIZE:
            raise ValueError(f"Malformed registration payload: expected {EXPECTED_SIZE} bytes, got {len(data)}")

        name_field = data[:NAME_FIELD_SIZE]
        
        public_key = data[NAME_FIELD_SIZE:]

        try:
            null_index = name_field.index(b'\x00')
            name = name_field[:null_index].decode("ascii")
        except ValueError:
            name = name_field.rstrip(b'\x00').decode("ascii")
        
        if not name or len(public_key) != KEY_FIELD_SIZE:
             raise ValueError("Invalid registration payload components")
             
        return name, public_key

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
