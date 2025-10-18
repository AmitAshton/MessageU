#include "Protocol.h"

static std::vector<uint8_t> concat(const std::vector<std::vector<uint8_t>>& blocks) {
    size_t total = 0;
    for (auto& b : blocks) total += b.size();
    std::vector<uint8_t> out; out.reserve(total);
    for (auto& b : blocks) out.insert(out.end(), b.begin(), b.end());
    return out;
}

std::vector<uint8_t> Protocol::buildRegister(const std::string& username,
    const std::vector<uint8_t>& publicKey) {
    if (username.empty() || username.size() > 254)
        throw std::runtime_error("Invalid username length");
    if (publicKey.size() != 160)
        throw std::runtime_error("Public key must be 160 bytes");

    std::vector<uint8_t> payload(255 + 160, 0);
    std::memcpy(payload.data(), username.c_str(), username.size());
    payload[username.size()] = 0; // null terminator
    std::memcpy(payload.data() + 255, publicKey.data(), 160);

    RequestHeader hdr;
    hdr.code = static_cast<uint16_t>(RequestCode::REGISTER);
    hdr.payloadSize = static_cast<uint32_t>(payload.size());

    auto headerBytes = hdr.toBytes();
    return concat({ headerBytes, payload });
}

std::vector<uint8_t> Protocol::buildClientList(const std::array<uint8_t, 16>& id) {
    RequestHeader hdr;
    hdr.clientId = id;
    hdr.code = static_cast<uint16_t>(RequestCode::CLIENT_LIST);
    hdr.payloadSize = 0;
    return hdr.toBytes();
}

std::vector<uint8_t> Protocol::buildPublicKeyReq(const std::array<uint8_t, 16>& id,
    const std::array<uint8_t, 16>& target) {
    RequestHeader hdr;
    hdr.clientId = id;
    hdr.code = static_cast<uint16_t>(RequestCode::PUBLIC_KEY);
    hdr.payloadSize = 16;
    auto header = hdr.toBytes();
    std::vector<uint8_t> out(header);
    out.insert(out.end(), target.begin(), target.end());
    return out;
}

std::vector<uint8_t> Protocol::buildSendMessage(const std::array<uint8_t, 16>& id,
    const std::array<uint8_t, 16>& target,
    uint8_t msgType,
    const std::vector<uint8_t>& content) {
    RequestHeader hdr;
    hdr.clientId = id;
    hdr.code = static_cast<uint16_t>(RequestCode::SEND_MESSAGE);
    hdr.payloadSize = static_cast<uint32_t>(16 + 1 + 4 + content.size());

    auto header = hdr.toBytes();
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), target.begin(), target.end());
    payload.push_back(msgType);
    uint32_t contentSize = static_cast<uint32_t>(content.size());
    std::memcpy(std::back_inserter(payload).base(), &contentSize, 4);
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&contentSize),
        reinterpret_cast<uint8_t*>(&contentSize) + 4);
    payload.insert(payload.end(), content.begin(), content.end());

    return concat({ header, payload });
}

std::vector<uint8_t> Protocol::buildPullMessages(const std::array<uint8_t, 16>& id) {
    RequestHeader hdr;
    hdr.clientId = id;
    hdr.code = static_cast<uint16_t>(RequestCode::PULL_MESSAGES);
    hdr.payloadSize = 0;
    return hdr.toBytes();
}
