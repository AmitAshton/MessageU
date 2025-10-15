from .header import RequestHeader
from .payload import PayloadParser
from .enums import RequestCode

class RequestParser:
    @staticmethod
    def parse(data: bytes):
        header = RequestHeader.from_bytes(data[:RequestHeader.SIZE])
        payload = data[RequestHeader.SIZE : RequestHeader.SIZE + header.payload_size]
        if header.code not in [e.value for e in RequestCode]:
            raise ValueError(f"Invalid request code: {header.code}")
        return header, RequestParser._parse_payload(header.code, payload)

    @staticmethod
    def _parse_payload(code: int, payload: bytes):
        if code == RequestCode.REGISTER:
            return PayloadParser.parse_register_payload(payload)
        elif code == RequestCode.SEND_MESSAGE:
            return PayloadParser.parse_send_message_payload(payload)
        elif code == RequestCode.PULL_MESSAGES:
            return PayloadParser.parse_pull_payload(payload)
        elif code == RequestCode.CLIENT_LIST:
            return None
        else:
            return payload
