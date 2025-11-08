#include "Response.h"
#include "Utils.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

Response::Response(const std::vector<uint8_t>& data) {
    const size_t minHeaderSize = VERSION_SIZE + RESPONSE_CODE_SIZE + CONTENT_SIZE_FIELD;

    if (data.size() < minHeaderSize)
        throw std::invalid_argument("Data size too small for Response header");

    size_t offset = 0;
    const uint8_t* dataPtr = data.data();

    version = dataPtr[offset];
    offset += VERSION_SIZE;

    responseCode = Utils::readUint16LittleEndian(dataPtr + offset);
    offset += RESPONSE_CODE_SIZE;

    payloadSize = Utils::readUint32LittleEndian(dataPtr + offset);
    offset += CONTENT_SIZE_FIELD;

    if (data.size() != offset + payloadSize)
        throw std::invalid_argument("Invalid total response size (mismatch)");

    std::vector<uint8_t> payloadData(data.begin() + offset, data.end());

    if (responseCode == RESPONSE_CODE_MESSAGE_LIST) {
        size_t messageOffset = 0;

        while (messageOffset < payloadSize) {
            size_t minHeaderSizeForMessage = CLIENT_ID_SIZE + MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE + CONTENT_SIZE_FIELD;

            if (payloadSize - messageOffset < minHeaderSizeForMessage) {
                throw std::invalid_argument("Partial message entry in 2104 response.");
            }

            uint32_t currentContentSize = Utils::readUint32LittleEndian(payloadData.data() + messageOffset + CLIENT_ID_SIZE + MESSAGE_ID_SIZE + MESSAGE_TYPE_SIZE);
            size_t currentMessageTotalSize = minHeaderSizeForMessage + currentContentSize;

            if (payloadSize - messageOffset < currentMessageTotalSize) {
                throw std::invalid_argument("Truncated message content in 2104 response.");
            }

            std::vector<uint8_t> currentMessageBytes(
                payloadData.begin() + messageOffset,
                payloadData.begin() + messageOffset + currentMessageTotalSize
            );

            Payload tempPayload(currentMessageBytes, responseCode);
            payloads.push_back(tempPayload);

            messageOffset += currentMessageTotalSize;
        }

    }
    else {
        payload = Payload(payloadData, responseCode);
    }
}

uint8_t Response::getVersion() const {
    return version;
}
uint16_t Response::getResponseCode() const {
    return responseCode;
}
uint32_t Response::getPayloadSize() const {
    return payloadSize;
}
const Payload& Response::getPayload() const {
    return payload;
}
const std::vector<Payload>& Response::getPayloads() const {
    return payloads;
}