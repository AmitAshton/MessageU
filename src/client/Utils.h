#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <sstream>

class Utils {
public:
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& s, char delim);

    // Conversion utilities
    static std::vector<uint8_t> hexToBytes(const std::string& hex);
    static std::string bytesToHexString(const uint8_t* bytes, size_t size, bool uppercase = true);
    static std::string bytesToHexString(const std::vector<uint8_t>& bytes, bool uppercase = true);
    static std::vector<uint8_t> stringToBytes(const std::string& str);
    static std::string bytesToString(const std::vector<uint8_t>& bytes);

    // Protocol Utilities
    static uint32_t readUint32LittleEndian(const uint8_t* data);
    static uint16_t readUint16LittleEndian(const uint8_t* data);
};