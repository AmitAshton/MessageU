import pytest
import uuid
from protocol.builder import ResponseBuilder
from protocol.parser import RequestParser


def test_response_builder_register_success():
    uid = uuid.uuid4()
    data = ResponseBuilder.build_register_success(uid)
    assert uid.bytes in data
    assert isinstance(data, bytes)

def test_parse_invalid_header_length():
    bad = b"\x00" * 5
    with pytest.raises(Exception):
        RequestParser.parse(bad)

def test_invalid_request_code_parsing():
    uid = uuid.uuid4().bytes
    invalid_code = (9999).to_bytes(2, "little")
    header_bytes = uid + b"\x02" + invalid_code + b"\x10\x00\x00\x00"
    with pytest.raises(Exception):
        RequestParser.parse(header_bytes)
