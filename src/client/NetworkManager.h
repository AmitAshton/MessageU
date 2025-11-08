#pragma once

#define WIN32_LEAN_AND_MEAN

#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstdint>

#pragma comment(lib, "Ws2_32.lib")

struct ServerResponse {
	uint16_t code;
	std::string payload;
};

class NetworkManager
{
private:
	SOCKET _clientSocket;
	bool _connected;

	void receive_exact(char* buffer, size_t size);

public:
	NetworkManager();

	virtual ~NetworkManager();

	void connect_to_server(const std::string& host, int port);

	void disconnect_server();

	void send_data(const std::string& data);

	ServerResponse receive_response();
};