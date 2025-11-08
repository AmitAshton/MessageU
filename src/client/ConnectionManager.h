#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <regex>
#include <thread>
#include <chrono>

#include "Utils.h"

class ConnectionManager {
public:
    static constexpr int MAX_PORT = 65535;
    static constexpr int MAX_IP_PART = 255;
    // Default read timeout used by sendAndReceiveOnce (milliseconds)
    static constexpr int DEFAULT_TIMEOUT_MS = 5000;
    // Polling interval while waiting for socket data (milliseconds)
    static constexpr int POLL_INTERVAL_MS = 10;

    ConnectionManager(const std::string& fullAddress);
    // Connect, send data, wait for a response or timeout, then disconnect.
    // This implements a per-request connection lifecycle.
    std::vector<uint8_t> sendAndReceiveOnce(const std::vector<uint8_t>& data, int timeoutMs = DEFAULT_TIMEOUT_MS);


private:
    std::string IPAddress;
    std::string port;

    bool isValid() const;
    void connect(boost::asio::ip::tcp::socket& socket);
};

// ConnectionManager: small helper that performs a single request/response
// interaction with the server. It opens a socket to the configured address,
// writes the serialized request bytes, then reads and returns the raw
// response bytes. The implementation uses a polling read loop to collect a
// header first and then the payload (respecting a timeout), which prevents
// returning partial responses when the server sends header and payload in
// separate TCP segments.