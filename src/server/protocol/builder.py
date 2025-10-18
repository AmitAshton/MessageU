import struct
import uuid
from .header import ResponseHeader
from .enums import ResponseCode, ProtocolVersion

class ResponseBuilder:
    @staticmethod
    def make_header(code: int, payload_size: int) -> bytes:
        """Create only the header for a given response type"""
        header = ResponseHeader(ProtocolVersion.SERVER, code, payload_size)
        return header.to_bytes()

    @staticmethod
    def build_register_success(client_id: uuid.UUID) -> bytes:
        payload = client_id.bytes
        header = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.REGISTER_SUCCESS, len(payload))
        return header.to_bytes() + payload

    @staticmethod
    def build_client_list(client_records) -> bytes:
        payload = b"".join([
            c.id.bytes + c.username.encode('ascii').ljust(255, b'\x00') for c in client_records
        ])
        header = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.CLIENT_LIST, len(payload))
        return header.to_bytes() + payload

    @staticmethod
    def build_public_key(client_id: uuid.UUID, public_key: bytes) -> bytes:
        # Payload = 16 bytes client UUID + public key bytes
        payload = client_id.bytes + public_key
        payload_size = len(payload)

        # Correct header: version(1) + response_code(2) + payload_size(4)
        header = (
                ProtocolVersion.SERVER.to_bytes(1, "little") +
                ResponseCode.PUBLIC_KEY.to_bytes(2, "little") +
                payload_size.to_bytes(4, "little")
        )
        return header + payload

    @staticmethod
    def build_message_stored(to_client: uuid.UUID, message_id: int) -> bytes:
        payload = to_client.bytes + struct.pack("<I", message_id)
        header = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.MESSAGE_STORED, len(payload))
        return header.to_bytes() + payload

    @staticmethod
    def build_pending_messages(payload: bytes) -> bytes:
        """Used for 2104 - Pending messages"""
        header = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.PENDING_MESSAGES, len(payload))
        return header.to_bytes() + payload

    @staticmethod
    def build_error() -> bytes:
        header = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.GENERAL_ERROR, 0)
        return header.to_bytes()
