#pragma once
#include <string>
#include <vector>
#include <array>
#include "Protocol.h"

#pragma pack(push, 1)
struct RequestHeader {
	std::array<char, UUID_SIZE> clientID;
	uint8_t  version;
	uint16_t code;
	uint32_t payloadSize;
};
#pragma pack(pop)

class Request
{
protected:
	RequestHeader _header;

	std::string packHeader(uint32_t payloadSize);

public:
	virtual ~Request() = default;

	virtual std::string getPackedRequest() = 0;
};


class RegisterRequest : public Request
{
private:
	std::string _username;
	std::string _publicKey;

public:
	RegisterRequest(const std::string& username, const std::string& publicKey);
	virtual std::string getPackedRequest() override;
};

class ClientListRequest : public Request
{
public:
	ClientListRequest(const std::array<char, UUID_SIZE>& clientID);
	virtual std::string getPackedRequest() override;
};

class PublicKeyRequest : public Request
{
private:
	std::array<char, UUID_SIZE> _targetClientID;
public:
	PublicKeyRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID);
	virtual std::string getPackedRequest() override;
};

class SendMessageRequest : public Request
{
private:
	std::array<char, UUID_SIZE> _targetClientID;
	MessageType _messageType;
	std::string _content;

public:
	SendMessageRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID, MessageType type, const std::string& content);
	virtual std::string getPackedRequest() override;
};

class PullMessagesRequest : public Request
{
public:
	PullMessagesRequest(const std::array<char, UUID_SIZE>& clientID);
	virtual std::string getPackedRequest() override;
};