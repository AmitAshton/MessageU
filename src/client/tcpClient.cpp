#include "tcpClient.h"
#include <iostream>
#include <ws2tcpip.h>

TcpClient::TcpClient() : sock(INVALID_SOCKET), connected(false) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
    }
}

TcpClient::~TcpClient() {
    disconnect();
    WSACleanup();
}

bool TcpClient::connectToServer(const std::string& ip, unsigned short port) {
    if (connected) return true;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed to " << ip << ":" << port << std::endl;
        closesocket(sock);
        return false;
    }

    connected = true;
    std::cout << "Connected to server " << ip << ":" << port << std::endl;
    return true;
}

bool TcpClient::sendAll(const std::vector<uint8_t>& data) {
    if (!connected) return false;
    size_t totalSent = 0;
    while (totalSent < data.size()) {
        int sent = send(sock, reinterpret_cast<const char*>(data.data() + totalSent),
            static_cast<int>(data.size() - totalSent), 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "Send failed" << std::endl;
            return false;
        }
        totalSent += sent;
    }
    return true;
}

bool TcpClient::recvAll(std::vector<uint8_t>& buffer, size_t size) {
    if (!connected) return false;
    buffer.resize(size);
    size_t totalRecv = 0;
    while (totalRecv < size) {
        int recvBytes = recv(sock, reinterpret_cast<char*>(buffer.data() + totalRecv),
            static_cast<int>(size - totalRecv), 0);
        if (recvBytes <= 0) {
            std::cerr << "Receive failed or connection closed" << std::endl;
            return false;
        }
        totalRecv += recvBytes;
    }
    return true;
}

void TcpClient::disconnect() {
    if (connected) {
        closesocket(sock);
        connected = false;
        std::cout << "Disconnected from server." << std::endl;
    }
}
