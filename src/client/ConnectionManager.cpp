#include "ConnectionManager.h"
#include "Response.h"

ConnectionManager::ConnectionManager(const std::string& fullAddress) {
    std::regex addrRe(R"(^\s*\d{1,3}(\.\d{1,3}){3}:\d{1,5}\s*$)");
    if (!std::regex_match(fullAddress, addrRe))
        throw std::invalid_argument("Invalid address format");

    size_t colonPos = fullAddress.find(':');
    IPAddress = fullAddress.substr(0, colonPos);
    port = fullAddress.substr(colonPos + 1);

    if (!isValid())
        throw std::invalid_argument("Invalid IP or port");
}

bool ConnectionManager::isValid() const {
    if (port.empty() || IPAddress.empty())
        return false;

    if (std::stoi(port) < 1 || std::stoi(port) > MAX_PORT)
        return false;

    std::vector<std::string> ipParts = Utils::split(IPAddress, '.');
    for (const std::string& part : ipParts) {
        if (part.empty() || std::stoi(part) < 0 || std::stoi(part) > MAX_IP_PART)
            return false;
    }

    return true;
}

void ConnectionManager::connect(boost::asio::ip::tcp::socket& socket) {
    try {
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address(IPAddress),
            static_cast<unsigned short>(std::stoi(port))
        );
        socket.connect(endpoint);
    }
    catch (const std::exception& e) {
        // Log connection errors. connect() is a thin helper used by
        // sendAndReceiveOnce which will handle failures and timeouts.
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
    }
}


std::vector<uint8_t> ConnectionManager::sendAndReceiveOnce(const std::vector<uint8_t>& data, int timeoutMs) {
    // Use a temporary io_context and socket so the connection is per-request
    boost::asio::io_context req_io;
    boost::asio::ip::tcp::socket req_socket(req_io);

    std::vector<uint8_t> received;
    try {
        // Resolve and connect (use numeric IP or port string)
        connect(req_socket);

        // send all data
        boost::asio::write(req_socket, boost::asio::buffer(data));

        // First, try to read the fixed-size response header so we can determine
        // the payload length. This avoids returning partial responses when the
        // server sends header and payload in separate bursts.
        // THIS IS THE CORRECT CODE
        const size_t headerLen = VERSION_SIZE + RESPONSE_CODE_SIZE + CONTENT_SIZE_FIELD;
        const int pollIntervalMs = ConnectionManager::POLL_INTERVAL_MS;
        int elapsed = 0;

        // Helper to read up to `want` bytes into `out` (appends). Returns true
        // when we read exactly `want` bytes before timeout, false otherwise.
        auto readUntil = [&](std::vector<uint8_t>& out, size_t want, int& elapsedMs) -> bool {
            while (elapsedMs < timeoutMs && out.size() < want) {
                boost::system::error_code ec;
                std::size_t avail = req_socket.available(ec);
                if (!ec && avail > 0) {
                    // read at most what's available or what's still needed
                    std::size_t toRead = std::min(avail, want - out.size());
                    std::vector<uint8_t> buf(toRead);
                    std::size_t n = req_socket.read_some(boost::asio::buffer(buf), ec);
                    if (!ec && n > 0) {
                        buf.resize(n);
                        out.insert(out.end(), buf.begin(), buf.end());
                        continue; // try to get more until `want` reached
                    }
                    if (ec == boost::asio::error::eof) {
                        // remote closed connection — stop reading
                        return out.size() >= want;
                    }
                    if (ec && ec != boost::asio::error::would_block)
                        throw boost::system::system_error(ec);
                }
                // nothing right now
                if (!out.empty() && req_socket.available() == 0) {
                    // we've read something but there's currently nothing more;
                    // wait a bit for the rest to arrive
                    std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
                    elapsedMs += pollIntervalMs;
                    continue;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
                elapsedMs += pollIntervalMs;
            }
            return out.size() >= want;
            };

        std::vector<uint8_t> headerBuf;
        if (!readUntil(headerBuf, headerLen, elapsed)) {
            // fallback: no header fully read within timeout — try to return
            // whatever we have (empty or partial)
            // Try to read any remaining available bytes once
            boost::system::error_code ec;
            std::size_t avail = req_socket.available(ec);
            if (!ec && avail > 0) {
                std::vector<uint8_t> buf(avail);
                std::size_t n = req_socket.read_some(boost::asio::buffer(buf), ec);
                if (!ec && n > 0) {
                    buf.resize(n);
                    received.insert(received.end(), buf.begin(), buf.end());
                }
            }
            // return whatever was gathered
        }
        else {
            // We have a header; parse payload length trying both endiannesses.
            // Compose a temporary buffer containing header first
            received.insert(received.end(), headerBuf.begin(), headerBuf.end());

            // extract payload-size bytes from header as little-endian (LSB first)
            size_t psOffset = VERSION_SIZE + RESPONSE_CODE_SIZE;
            uint32_t payloadLen = 0;
            for (int i = 0; i < CONTENT_SIZE_FIELD; ++i) { // Also fix PAYLOAD_SIZE_FIELD
                uint8_t b = headerBuf[psOffset + i];
                payloadLen |= static_cast<uint32_t>(b) << (8 * i);
            }

            // Attempt to read exactly `payloadLen` bytes (after header)
            auto tryReadPayload = [&](uint32_t pl, int& elapsedMs) -> bool {
                size_t before = received.size();
                size_t want = before + pl;
                return readUntil(received, want, elapsedMs);
                };

            int elapsedForRead = elapsed;
            if (!tryReadPayload(payloadLen, elapsedForRead)) {
                // Couldn't read full payload within timeout — try to read any
                // remaining available bytes once and return partial data.
                boost::system::error_code ec;
                std::size_t avail = req_socket.available(ec);
                if (!ec && avail > 0) {
                    std::vector<uint8_t> buf(avail);
                    std::size_t n = req_socket.read_some(boost::asio::buffer(buf), ec);
                    if (!ec && n > 0) {
                        buf.resize(n);
                        received.insert(received.end(), buf.begin(), buf.end());
                    }
                }
            }
            else {
                elapsed = elapsedForRead;
            }
        }

        // shutdown and close
        boost::system::error_code shut_ec;
        req_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, shut_ec);
        req_socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        // ensure socket closed
        boost::system::error_code close_ec;
        if (req_socket.is_open()) req_socket.close(close_ec);
    }
    return received;
}