#pragma once

#include <stdexcept>
#include <cstdint>
#include <tuple>
#include <array>
#include <vector>
#include <string>

#include "Utils.h"
#include "Constants.h"
#include "ServerError.h"

// Struct for the fixed-size client entry inside a 2101 response
#pragma pack(push, 1)
struct ClientListEntry {
    uint8_t clientID[CLIENT_ID_SIZE];
    char username[USERNAME_MAX_SIZE];
};

// Struct for the fixed-size message entry inside a 2104 response (excluding content)
struct MessageEntryHeader {
    uint8_t fromClientID[CLIENT_ID_SIZE];
    uint32_t messageID;
    uint8_t messageType;
    uint32_t messageSize;
};
#pragma pack(pop)


class Payload {
public:
    // Default constructor is required for internal use in other classes (like Response)
    Payload() = default;

    // --- Accessors for Payload Components ---
    const uint8_t* getClientID() const { return clientID.data(); }
    const std::string& getName() const { return name; }
    const uint8_t* getPublicKey() const { return publicKey.data(); }
    uint32_t getMessageID() const { return messageID; }
    uint8_t getMessageType() const { return messageType; }
    const std::vector<uint8_t>& getMessageContent() const { return messageContent; }
    const std::vector<std::tuple<std::string, std::array<uint8_t, CLIENT_ID_SIZE>>>& getClientsList() const { return clientsList; }

    // --- Constructor (Parse received data based on response code) ---
    // This is used by the Response class to parse the raw byte buffer it receives.
    Payload(const std::vector<uint8_t>& data, uint16_t responseCode);

    // --- Factory Methods for Request Payloads (used to build outgoing buffers) ---
    static std::vector<uint8_t> createRegisterPayload(const std::string& username, const std::string& publicKey);
    static std::vector<uint8_t> createClientKeyRequestPayload(const uint8_t clientID[CLIENT_ID_SIZE]);
    static std::vector<uint8_t> createSendMessagePayload(
        const uint8_t toClientID[CLIENT_ID_SIZE],
        uint8_t messageType,
        const std::vector<uint8_t>& content
    );

private:
    // Stored Data
    std::array<uint8_t, CLIENT_ID_SIZE> clientID = { 0 };
    std::string name = "";
    std::array<uint8_t, PUBLIC_KEY_SIZE> publicKey = { 0 };
    uint32_t messageID = 0;
    uint8_t messageType = 0;
    std::vector<uint8_t> messageContent = {};
    std::vector<std::tuple<std::string, std::array<uint8_t, CLIENT_ID_SIZE>>> clientsList = {};

    // Internal parsing helper for 2104 response which may contain multiple messages
    void parseIncomingMessages(const std::vector<uint8_t>& data);
};