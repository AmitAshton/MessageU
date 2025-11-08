#include "Utils.h"
#include <cctype>
#include <iomanip>

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    if (std::string::npos == start || std::string::npos == end) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Utils::split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        out.push_back(item);
    }
    return out;
}

std::vector<uint8_t> Utils::hexToBytes(const std::string& hex) {
    std::string cleanedHex;
    for (char c : hex) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            cleanedHex += c;
        }
    }
    if (cleanedHex.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even length");
    }
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < cleanedHex.length(); i += 2) {
        std::string byteString = cleanedHex.substr(i, 2);
        bytes.push_back(static_cast<uint8_t>(std::stoul(byteString, nullptr, 16)));
    }
    return bytes;
}

std::string Utils::bytesToHexString(const uint8_t* bytes, size_t size, bool uppercase) {
    std::stringstream ss;
    ss << std::hex << (uppercase ? std::uppercase : std::nouppercase) << std::setfill('0');
    for (size_t i = 0; i < size; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

std::string Utils::bytesToHexString(const std::vector<uint8_t>& bytes, bool uppercase) {
    return bytesToHexString(bytes.data(), bytes.size(), uppercase);
}

std::vector<uint8_t> Utils::stringToBytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string Utils::bytesToString(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

uint32_t Utils::readUint32LittleEndian(const uint8_t* data) {
    uint32_t result = 0;
    // Reads 4 bytes as little-endian
    for (int i = 0; i < 4; ++i) {
        result |= static_cast<uint32_t>(data[i]) << (8 * i);
    }
    return result;
}

uint16_t Utils::readUint16LittleEndian(const uint8_t* data) {
    uint16_t result = 0;
    // Reads 2 bytes as little-endian
    for (int i = 0; i < 2; ++i) {
        result |= static_cast<uint16_t>(data[i]) << (8 * i);
    }
    return result;
}