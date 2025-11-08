#include "Request.h"
#include <stdexcept>
#include <cstring> // For memcpy

// Helper function to pack a 32-bit unsigned int in little-endian
void pack_uint32_le(char* buffer, uint32_t value) {
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
	buffer[2] = (value >> 16) & 0xFF;
	buffer[3] = (value >> 24) & 0xFF;
}

// Helper function to pack a 16-bit unsigned int in little-endian
void pack_uint16_le(char* buffer, uint16_t value) {
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
}

// --- Request (Base Class) Implementation ---

std::string Request::packHeader(uint32_t payloadSize)
{
	// Set the payload size in the header struct
	_header.payloadSize = payloadSize;

	// Create a 23-byte buffer for the raw header
	std::string packed(sizeof(RequestHeader), '\0');

	// Manually pack the struct fields to ensure correct endianness

	// 1. Client ID (16 bytes)
	memcpy(&packed[0], _header.clientID.data(), UUID_SIZE);

	// 2. Version (1 byte)
	packed[16] = _header.version;

	// 3. Code (2 bytes, little-endian)
	pack_uint16_le(&packed[17], static_cast<uint16_t>(_header.code));

	// 4. Payload Size (4 bytes, little-endian)
	pack_uint32_le(&packed[19], _header.payloadSize);

	return packed;
}


// --- RegisterRequest (600) Implementation ---

RegisterRequest::RegisterRequest(const std::string& username, const std::string& publicKey)
{
	if (publicKey.length() != PUBLIC_KEY_SIZE) {
		throw std::runtime_error("Invalid public key size for registration.");
	}
	_header.code = static_cast<uint16_t>(RequestCode::REGISTER);
	_header.version = CLIENT_VERSION;
	// ClientID is ignored by server on registration, can be zero
	_header.clientID.fill(0);

	_username = username;
	_publicKey = publicKey;
}

std::string RegisterRequest::getPackedRequest()
{
	// 1. Build Payload
	std::string payload;

	// Pad username to 255 bytes
	payload.append(_username);
	payload.append(CLIENT_NAME_SIZE - _username.length(), '\0');

	// Append public key
	payload.append(_publicKey);

	// 2. Get Header
	std::string header = packHeader((uint32_t)payload.length());

	// 3. Return Header + Payload
	return header + payload;
}


// --- ClientListRequest (601) Implementation ---

ClientListRequest::ClientListRequest(const std::array<char, UUID_SIZE>& clientID)
{
	_header.code = static_cast<uint16_t>(RequestCode::CLIENT_LIST);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
}

std::string ClientListRequest::getPackedRequest()
{
	// No payload for this request
	return packHeader(0);
}


// --- PublicKeyRequest (602) Implementation ---

PublicKeyRequest::PublicKeyRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID)
{
	_header.code = static_cast<uint16_t>(RequestCode::PUBLIC_KEY);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
	_targetClientID = targetID;
}

std::string PublicKeyRequest::getPackedRequest()
{
	// Payload is just the target client ID
	std::string payload(_targetClientID.data(), UUID_SIZE);
	std::string header = packHeader((uint32_t)payload.length());
	return header + payload;
}


// --- SendMessageRequest (603) Implementation ---

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
	// 1. Build Payload
	// Payload: TargetID (16) + MsgType (1) + ContentSize (4) + Content (...)
	const size_t payloadHeaderSize = UUID_SIZE + sizeof(uint8_t) + sizeof(uint32_t);
	std::string payload(payloadHeaderSize, '\0');

	// Target ID
	memcpy(&payload[0], _targetClientID.data(), UUID_SIZE);

	// Message Type
	payload[16] = static_cast<uint8_t>(_messageType);

	// Content Size (little-endian)
	pack_uint32_le(&payload[17], (uint32_t)_content.length());

	// Append actual content
	payload.append(_content);

	// 2. Get Header
	std::string header = packHeader((uint32_t)payload.length());

	// 3. Return Header + Payload
	return header + payload;
}


// --- PullMessagesRequest (604) Implementation ---

PullMessagesRequest::PullMessagesRequest(const std::array<char, UUID_SIZE>& clientID)
{
	_header.code = static_cast<uint16_t>(RequestCode::PULL_MESSAGES);
	_header.version = CLIENT_VERSION;
	_header.clientID = clientID;
}

std::string PullMessagesRequest::getPackedRequest()
{
	// No payload for this request
	return packHeader(0);
}