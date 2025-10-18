#pragma once
#include <string>
#include <vector>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    bool connectToServer(const std::string& ip, unsigned short port);
    bool sendAll(const std::vector<uint8_t>& data);
    bool recvAll(std::vector<uint8_t>& buffer, size_t size);
    void disconnect();

    bool isConnected() const { return connected; }

private:
    SOCKET sock;
    bool connected;
};
