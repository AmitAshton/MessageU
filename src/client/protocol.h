#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <cstring>
#include <stdexcept>

enum class RequestCode : uint16_t {
    REGISTER = 600,
    CLIENT_LIST = 601,
    PUBLIC_KEY = 602,
    SEND_MESSAGE = 603,
    PULL_MESSAGES = 604
};

enum class ResponseCode : uint16_t {
    REGISTER_OK = 2100,
    CLIENT_LIST_OK = 2101,
    PUBLIC_KEY_OK = 2102,
    MESSAGE_STORED = 2103,
    MESSAGES_WAITING = 2104,
    ERROR_GENERAL = 9000
};

#pragma pack(push, 1)
struct RequestHeader {
    std::array<uint8_t, 16> clientId;
    uint8_t version;
    uint16_t code;
    uint32_t payloadSize;

    RequestHeader() : version(2), code(0), payloadSize(0) {
        clientId.fill(0);
    }

    std::vector<uint8_t> toBytes() const {
        std::vector<uint8_t> out(16 + 1 + 2 + 4);
        size_t offset = 0;
        std::memcpy(out.data() + offset, clientId.data(), 16); offset += 16;
        out[offset++] = version;
        std::memcpy(out.data() + offset, &code, 2); offset += 2;
        std::memcpy(out.data() + offset, &payloadSize, 4);
        return out;
    }
};
#pragma pack(pop)

class Protocol {
public:
    static std::vector<uint8_t> buildRegister(const std::string& username,
        const std::vector<uint8_t>& publicKey);
    static std::vector<uint8_t> buildClientList(const std::array<uint8_t, 16>& id);
    static std::vector<uint8_t> buildPublicKeyReq(const std::array<uint8_t, 16>& id,
        const std::array<uint8_t, 16>& target);
    static std::vector<uint8_t> buildSendMessage(const std::array<uint8_t, 16>& id,
        const std::array<uint8_t, 16>& target,
        uint8_t msgType,
        const std::vector<uint8_t>& content);
    static std::vector<uint8_t> buildPullMessages(const std::array<uint8_t, 16>& id);
};
