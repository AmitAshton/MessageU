import pytest
import uuid
from protocol.builder import ResponseBuilder
from protocol.parser import RequestParser
from protocol.header import RequestHeader, RESPONSE_HEADER_FORMAT
from protocol.enums import RequestCode, ResponseCode
from models.client import ClientRecord


def test_response_builder_register_success():
    uid = uuid.uuid4()
    data = ResponseBuilder.build_register_success(uid)
    assert len(data) == 7 + 16
    assert ResponseCode.REGISTER_SUCCESS.to_bytes(2, "little") in data
    assert uid.bytes in data

def test_response_builder_client_list():
    c1 = ClientRecord("Alice", b"k1")
    c2 = ClientRecord("Bob", b"k2")
    
    records = [c1, c2]
    data = ResponseBuilder.build_client_list(records)
    
    expected_payload_size = 2 * (16 + 255)
    expected_total_size = 7 + expected_payload_size
    
    assert len(data) == expected_total_size
    assert ResponseCode.CLIENT_LIST.to_bytes(2, "little") in data
    assert b"Alice\x00" in data
    assert b"Bob\x00" in data

def test_response_builder_error():
    data = ResponseBuilder.build_error()
    assert len(data) == 7
    assert ResponseCode.GENERAL_ERROR.to_bytes(2, "little") in data



def test_parse_register_fixed_size_payload():
    NAME_FIELD_SIZE = 255
    KEY_FIELD_SIZE = 160
    
    username = b"TestUser"
    public_key = b"A" * KEY_FIELD_SIZE
    payload_data = username + b"\x00" + (b"\x00" * (NAME_FIELD_SIZE - len(username) - 1)) + public_key
    
    header = RequestHeader(uuid.uuid4(), 1, RequestCode.REGISTER, len(payload_data))
    request_data = header.to_bytes() + payload_data

    parsed_header, (parsed_name, parsed_key) = RequestParser.parse(request_data)
    
    assert parsed_name == "TestUser"
    assert parsed_key == public_key
    assert len(parsed_key) == 160


def test_parse_invalid_header_length():
    bad = b"\x00" * 5
    with pytest.raises(Exception):
        RequestParser.parse(bad)

def test_invalid_request_code_parsing():
    uid = uuid.uuid4().bytes
    invalid_code = (9999).to_bytes(2, "little")
    header_bytes = uid + b"\x01" + invalid_code + b"\x10\x00\x00\x00"
    
    with pytest.raises(ValueError, match="Invalid request code"):
        RequestParser.parse(header_bytes + b"\x00"*16)