#include "Payload.h"
#include "Utils.h"
#include "ServerError.h"
#include <stdexcept>
#include <cstring>
#include <iostream>


// --- Factory Methods for Request Payloads (Serialization) ---

std::vector<uint8_t> Payload::createRegisterPayload(const std::string& username, const std::string& publicKey) {
    if (username.length() >= USERNAME_MAX_SIZE) {
        throw std::invalid_argument("Username too long for registration");
    }
    if (publicKey.length() != PUBLIC_KEY_SIZE) {
        throw std::invalid_argument("Public key size is incorrect");
    }

    // Fixed size: 255 bytes (name) + 160 bytes (key)
    std::vector<uint8_t> payload(USERNAME_MAX_SIZE + PUBLIC_KEY_SIZE);

    // 1. Username (255 bytes, NUL-padded)
    std::memcpy(payload.data(), username.c_str(), username.length());
    payload[username.length()] = '\x00';

    // 2. Public Key (160 bytes, raw DER)
    std::memcpy(payload.data() + USERNAME_MAX_SIZE, publicKey.data(), PUBLIC_KEY_SIZE);

    return payload;
}

std::vector<uint8_t> Payload::createClientKeyRequestPayload(const uint8_t clientID[CLIENT_ID_SIZE]) {
    // Payload is exactly 16 bytes: the target client ID
    std::vector<uint8_t> payload(CLIENT_ID_SIZE);
    std::memcpy(payload.data(), clientID, CLIENT_ID_SIZE);
    return payload;
}

std::vector<uint8_t> Payload::createSendMessagePayload(
    const uint8_t toClientID[CLIENT_ID_SIZE],
    uint8_t messageType,
    const std::vector<uint8_t>& content)
{
    // Fixed part: 16 (ID) + 1 (Type) + 4 (Size) = 21 bytes
    size_t fixedSize = CLIENT_ID_SIZE + MESSAGE_TYPE_SIZE + CONTENT_SIZE_FIELD;
    size_t contentSize = content.size();
    size_t totalSize = fixedSize + contentSize;

    if (contentSize > MAX_CONTENT_SIZE_ALLOWED) throw std::invalid_argument("Content size overflow.");

    std::vector<uint8_t> payload(totalSize);
    uint8_t* ptr = payload.data();

    // 1. To Client ID (16 bytes)
    std::memcpy(ptr, toClientID, CLIENT_ID_SIZE);
    ptr += CLIENT_ID_SIZE;

    // 2. Message Type (1 byte)
    *ptr = messageType;
    ptr += MESSAGE_TYPE_SIZE;

    // 3. Content Size (4 bytes, little-endian)
    uint32_t size = static_cast<uint32_t>(contentSize);
    std::memcpy(ptr, &size, sizeof(uint32_t));
    ptr += CONTENT_SIZE_FIELD;

    // 4. Content (variable size)
    std::memcpy(ptr, content.data(), content.size());

    return payload;
}

// --- Parse Received Data (Deserialization) ---

void Payload::parseIncomingMessages(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    while (offset < data.size()) {
        const uint8_t* currentPtr = data.data() + offset;

        // Minimum required size for header
        size_t minHeaderSize = CLIENT_ID_SIZE + MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE + CONTENT_SIZE_FIELD;

        if (data.size() - offset < minHeaderSize) {
            throw std::invalid_argument("2104 payload too small for a message header");
        }

        // We read from the current pointer position
        const uint8_t* headerStartPtr = currentPtr;

        // 1. Determine content size (4 bytes, little-endian)
        uint32_t currentContentSize = Utils::readUint32LittleEndian(headerStartPtr + CLIENT_ID_SIZE + MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE);

        size_t currentMessageTotalSize = minHeaderSize + currentContentSize;

        if (data.size() - offset < currentMessageTotalSize) {
            throw std::invalid_argument("2104 payload truncated message content");
        }

        // 2. Extract Header Info
        std::array<uint8_t, CLIENT_ID_SIZE> tempClientID;
        std::memcpy(tempClientID.data(), headerStartPtr, CLIENT_ID_SIZE);

        uint32_t tempMessageID = Utils::readUint32LittleEndian(headerStartPtr + CLIENT_ID_SIZE);
        uint8_t tempMessageType = headerStartPtr[CLIENT_ID_SIZE + MESSAGE_ID_SIZE];

        // 3. Extract Content (starts after the fixed header fields)
        std::vector<uint8_t> tempContent(headerStartPtr + minHeaderSize, headerStartPtr + currentMessageTotalSize);

        // 4. Store parsed message components 
        std::array<uint8_t, CLIENT_ID_SIZE> zeroArray = { 0 };

        // For 2104 response, we store all messages in the 'clientsList' vector (used for internal transport)
        // Note: The tuple format is repurposed here: (Name, ClientID)
        clientsList.emplace_back(
            "", // Name is empty
            tempClientID
        );

        // The first message's data is also stored in the public fields of this Payload object
        if (offset == 0) {
            std::copy(tempClientID.begin(), tempClientID.end(), clientID.begin());
            messageID = tempMessageID;
            messageType = tempMessageType;
            messageContent = tempContent;
        }

        offset += currentMessageTotalSize;
    }
}


Payload::Payload(const std::vector<uint8_t>& data, uint16_t responseCode) {

    switch (responseCode) {
    case RESPONSE_CODE_REGISTER_SUCCESS:
        // Payload: 16 bytes client ID
        if (data.size() != CLIENT_ID_SIZE)
            throw std::invalid_argument("Invalid 2100 payload size");
        std::copy(data.begin(), data.end(), clientID.begin());
        break;

    case RESPONSE_CODE_CLIENT_LIST:
        // Payload: sequence of 16 (ID) + 255 (Name) entries
        if (data.size() % (CLIENT_ID_SIZE + USERNAME_MAX_SIZE) != 0)
            throw std::invalid_argument("Invalid 2101 payload size");

        for (size_t offset = 0; offset < data.size(); offset += (CLIENT_ID_SIZE + USERNAME_MAX_SIZE)) {
            std::array<uint8_t, CLIENT_ID_SIZE> tempClientID;
            std::memcpy(tempClientID.data(), data.data() + offset, CLIENT_ID_SIZE);

            // Name is fixed-width (255) and NUL-padded; trim NULs to get actual name
            std::string tempName(reinterpret_cast<const char*>(data.data() + offset + CLIENT_ID_SIZE), USERNAME_MAX_SIZE);
            auto pos = tempName.find('\0');
            if (pos != std::string::npos) tempName.resize(pos);

            clientsList.emplace_back(tempName, tempClientID);
        }
        break;

    case RESPONSE_CODE_PUBLIC_KEY:
        // Payload: 16 (ID) + 160 (Key)
        if (data.size() != CLIENT_ID_SIZE + PUBLIC_KEY_SIZE)
            throw std::invalid_argument("Invalid 2102 payload size");
        std::memcpy(clientID.data(), data.data(), CLIENT_ID_SIZE);
        std::memcpy(publicKey.data(), data.data() + CLIENT_ID_SIZE, PUBLIC_KEY_SIZE);
        break;

    case RESPONSE_CODE_MESSAGE_STORED:
        // Payload: 16 (ID) + 4 (Message ID)
        if (data.size() != CLIENT_ID_SIZE + MESSAGE_ID_SIZE)
            throw std::invalid_argument("Invalid 2103 payload size");
        std::memcpy(clientID.data(), data.data(), CLIENT_ID_SIZE);
        messageID = Utils::readUint32LittleEndian(data.data() + CLIENT_ID_SIZE);
        break;

    case RESPONSE_CODE_MESSAGE_LIST:
        // Payload: sequence of Message Entries
        parseIncomingMessages(data);
        break;

    case RESPONSE_CODE_GENERAL_ERROR:
        // Payload: empty
        if (!data.empty())
            throw std::invalid_argument("Invalid 9000 payload size");
        break;

    default:
        throw std::invalid_argument("Invalid response code received during Payload parsing");
    }
}