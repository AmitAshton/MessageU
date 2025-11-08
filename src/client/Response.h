#pragma once
#include <vector>
#include <cstdint>
#include "Payload.h"
#include "ServerError.h"
#include "Constants.h" 
#include "Utils.h" 

class Response {
public:
    Response() = default;
    Response(const std::vector<uint8_t>& data);

    uint8_t getVersion() const;
    uint16_t getResponseCode() const;
    uint32_t getPayloadSize() const;
    const Payload& getPayload() const;
    const std::vector<Payload>& getPayloads() const;

private:
    uint8_t version;
    uint16_t responseCode;
    uint32_t payloadSize;

    Payload payload;
    std::vector<Payload> payloads;
};