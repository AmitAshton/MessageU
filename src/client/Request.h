#pragma once
#include <string>
#include <vector>
#include <array>
#include "Protocol.h" // We need the enums and constants

// Use pragma pack to ensure the struct is packed to 23 bytes
// exactly as the protocol requires, with no compiler padding.
#pragma pack(push, 1)
struct RequestHeader {
	std::array<char, UUID_SIZE> clientID;
	uint8_t  version;
	uint16_t code;
	uint32_t payloadSize;
};
#pragma pack(pop)

/**
 * @class Request
 * @brief Abstract base class for all client requests.
 *
 * Provides a common interface to pack a request (header + payload)
 * into a single binary string ready to be sent over the network.
 */
class Request
{
protected:
	RequestHeader _header;

	/**
	* @brief Packs the internal _header struct into a binary string.
	* @param payloadSize The size of the payload that follows this header.
	* @return A std::string containing the 23-byte raw header.
	*/
	std::string packHeader(uint32_t payloadSize);

public:
	// Virtual destructor to ensure proper cleanup of derived classes
	virtual ~Request() = default;

	/**
	 * @brief Pure virtual function to get the full packed request.
	 *
	 * Derived classes must implement this to return the
	 * packed header concatenated with their specific packed payload.
	 * @return A std::string containing the full binary request.
	 */
	virtual std::string getPackedRequest() = 0;
};


// --- Concrete Request Classes ---

/**
 * @class RegisterRequest
 * @brief A request to register a new user. Code 600.
 */
class RegisterRequest : public Request
{
private:
	std::string _username;
	std::string _publicKey; // Raw 160 bytes

public:
	RegisterRequest(const std::string& username, const std::string& publicKey);
	virtual std::string getPackedRequest() override;
};

/**
 * @class ClientListRequest
 * @brief A request for the list of registered clients. Code 601.
 */
class ClientListRequest : public Request
{
public:
	ClientListRequest(const std::array<char, UUID_SIZE>& clientID);
	virtual std::string getPackedRequest() override;
};

/**
 * @class PublicKeyRequest
 * @brief A request for another client's public key. Code 602.
 */
class PublicKeyRequest : public Request
{
private:
	std::array<char, UUID_SIZE> _targetClientID;
public:
	PublicKeyRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID);
	virtual std::string getPackedRequest() override;
};

/**
 * @class SendMessageRequest
 * @brief A request to send a message (any type) to a client. Code 603.
 */
class SendMessageRequest : public Request
{
private:
	std::array<char, UUID_SIZE> _targetClientID;
	MessageType _messageType;
	std::string _content; // Encrypted message, sym key, etc.

public:
	SendMessageRequest(const std::array<char, UUID_SIZE>& clientID, const std::array<char, UUID_SIZE>& targetID, MessageType type, const std::string& content);
	virtual std::string getPackedRequest() override;
};

/**
 * @class PullMessagesRequest
 * @brief A request to pull waiting messages. Code 604.
 */
class PullMessagesRequest : public Request
{
public:
	PullMessagesRequest(const std::array<char, UUID_SIZE>& clientID);
	virtual std::string getPackedRequest() override;
};