from protocol.header import RequestHeader, ResponseHeader
from protocol.enums import RequestCode, ResponseCode, ProtocolVersion
import uuid

cid = uuid.uuid4()
req = RequestHeader(cid, ProtocolVersion.CLIENT, RequestCode.REGISTER, 415)
data = req.to_bytes()
parsed = RequestHeader.from_bytes(data)
print(parsed.client_id, parsed.version, parsed.code, parsed.payload_size)

res = ResponseHeader(ProtocolVersion.SERVER, ResponseCode.GENERAL_ERROR, 0)
print(res.to_bytes())
