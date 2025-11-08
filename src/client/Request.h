#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include "Constants.h"
#include "Payload.h"
#include "ServerError.h"

class Request {
public:

    Request(
        const uint8_t(&clientID)[CLIENT_ID_SIZE],
        uint16_t requestCode,
        const std::vector<uint8_t>& payloadData
    );

    std::vector<uint8_t> serialize() const;

private:
    uint8_t clientID[CLIENT_ID_SIZE];
    uint16_t requestCode;
    uint32_t payloadSize;
    std::vector<uint8_t> payload;
};