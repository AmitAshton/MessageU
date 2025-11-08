#include "Request.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

Request::Request(
    const uint8_t(&clientID)[CLIENT_ID_SIZE],
    uint16_t requestCode,
    const std::vector<uint8_t>& payloadData)
    : payload(payloadData)
{
    if (payloadData.size() > MAX_CONTENT_SIZE_ALLOWED)
        throw std::invalid_argument("Payload size exceeds maximum allowed bytes");

    std::copy(std::begin(clientID), std::end(clientID), std::begin(this->clientID));
    this->requestCode = requestCode;
    this->payloadSize = static_cast<uint32_t>(payloadData.size());
}

std::vector<uint8_t> Request::serialize() const {
    std::vector<uint8_t> data;
    data.reserve(CLIENT_ID_SIZE + VERSION_SIZE + REQUEST_CODE_SIZE + CONTENT_SIZE_FIELD + payload.size());

    // 1. Client ID (16 bytes)
    data.insert(data.end(), std::begin(clientID), std::end(clientID));

    // 2. Version (1 byte)
    data.push_back(CLIENT_VERSION);

    // 3. Request Code (2 bytes, little-endian)
    data.push_back(static_cast<uint8_t>(requestCode & 0xFF));
    data.push_back(static_cast<uint8_t>((requestCode >> 8) & 0xFF));

    // 4. Payload Size (4 bytes, little-endian)
    for (int i = 0; i < CONTENT_SIZE_FIELD; ++i)
        data.push_back(static_cast<uint8_t>((payloadSize >> (8 * i)) & 0xFF));

    // 5. Payload Data
    data.insert(data.end(), payload.begin(), payload.end());

    return data;
}