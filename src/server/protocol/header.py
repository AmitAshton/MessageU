import struct
import uuid

# All numeric values are little endian
REQUEST_HEADER_FORMAT = "<16sBHI"  # UUID(16) + Version(1) + Code(2) + PayloadSize(4)
RESPONSE_HEADER_FORMAT = "<BHI"    # Version(1) + Code(2) + PayloadSize(4)

class RequestHeader:
    SIZE = struct.calcsize(REQUEST_HEADER_FORMAT)

    def __init__(self, client_id: uuid.UUID, version: int, code: int, payload_size: int):
        self.client_id = client_id
        self.version = version
        self.code = code
        self.payload_size = payload_size

    @classmethod
    def from_bytes(cls, data: bytes):
        client_id, version, code, payload_size = struct.unpack(REQUEST_HEADER_FORMAT, data)
        return cls(uuid.UUID(bytes=client_id), version, code, payload_size)

    def to_bytes(self) -> bytes:
        return struct.pack(REQUEST_HEADER_FORMAT, self.client_id.bytes, self.version, self.code, self.payload_size)


class ResponseHeader:
    SIZE = struct.calcsize(RESPONSE_HEADER_FORMAT)

    def __init__(self, version: int, code: int, payload_size: int):
        self.version = version
        self.code = code
        self.payload_size = payload_size

    def to_bytes(self) -> bytes:
        return struct.pack(RESPONSE_HEADER_FORMAT, self.version, self.code, self.payload_size)
