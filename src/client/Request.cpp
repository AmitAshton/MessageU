#include "Request.h"
#include <stdexcept>
#include <cstring>

void pack_uint32_le(char* buffer, uint32_t value) {
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
	buffer[2] = (value >> 16) & 0xFF;
	buffer[3] = (value >> 24) & 0xFF;
}

void pack_uint16_le(char* buffer, uint16_t value) {
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
}


std::string Request::packHeader(uint32_t payloadSize)
{
	_header.payloadSize = payloadSize;

	std::string packed(sizeof(RequestHeader), '\0');


	memcpy(&packed[0], _header.clientID.data(), UUID_SIZE);

	packed[16] = _header.version;

	pack_uint16_le(&packed[17], static_cast<uint16_t>(_header.code));

	pack_uint32_le(&packed[19], _header.payloadSize);

	return packed;
}


RegisterRequest::RegisterRequest(const std::string& username, const std::string& publicKey)
{
	if (publicKey.length() != PUBLIC_KEY_SIZE) {
		throw std::runtime_error("Invalid public key size for registration.");
	}
	_header.code = static_cast<uint16_t>(RequestCode::REGISTER);
	_header.version = CLIENT_VERSION;
	_header.clientID.fill(0);

	_username = username;
	_publicKey = publicKey;
}

std::string RegisterRequest::getPackedRequest()
{
	std::string payload;

	payload.append(_username);
	payload.append(CLIENT_NAME_SIZE - _username.length(), '\0');

	payload.append(_publicKey);

	std::string header = packHeader((uint32_t)payload.length());

	return header + payload;
}



ClientListRequest::ClientListRequest(const std::array<char, UUID_SIZE>& clientID)
{
	_header.code = static_cast<uint16_t>(RequestCode::CLIENT_LIST);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
}

std::string ClientListRequest::getPackedRequest()
{
	return packHeader(0);
}



PublicKeyRequest::PublicKeyRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID)
{
	_header.code = static_cast<uint16_t>(RequestCode::PUBLIC_KEY);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
	_targetClientID = targetID;
}

std::string PublicKeyRequest::getPackedRequest()
{
	std::string payload(_targetClientID.data(), UUID_SIZE);
	std::string header = packHeader((uint32_t)payload.length());
	return header + payload;
}



SendMessageRequest::SendMessageRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID, MessageType type, const std::string& content)
{
	_header.code = static_cast<uint16_t>(RequestCode::SEND_MESSAGE);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;

	_targetClientID = targetID;
	_messageType = type;
	_content = content;
}

std::string SendMessageRequest::getPackedRequest()
{
	const size_t payloadHeaderSize = UUID_SIZE + sizeof(uint8_t) + sizeof(uint32_t);
	std::string payload(payloadHeaderSize, '\0');

	memcpy(&payload[0], _targetClientID.data(), UUID_SIZE);

	payload[16] = static_cast<uint8_t>(_messageType);

	pack_uint32_le(&payload[17], (uint32_t)_content.length());

	payload.append(_content);

	std::string header = packHeader((uint32_t)payload.length());

	return header + payload;
}


PullMessagesRequest::PullMessagesRequest(const std::array<char, UUID_SIZE>& clientID)
{
	_header.code = static_cast<uint16_t>(RequestCode::PULL_MESSAGES);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
}

std::string PullMessagesRequest::getPackedRequest()
{
	return packHeader(0);
}