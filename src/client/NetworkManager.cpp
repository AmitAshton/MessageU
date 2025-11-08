#include "NetworkManager.h"
#include <stdexcept>
#include <vector>

#pragma pack(push, 1)
struct ResponseHeader {
	uint8_t  version;
	uint16_t code;
	uint32_t payloadSize;
};
#pragma pack(pop)


NetworkManager::NetworkManager() : _clientSocket(INVALID_SOCKET), _connected(false)
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		throw std::runtime_error("WSAStartup failed with error: " + std::to_string(result));
	}
}

NetworkManager::~NetworkManager()
{
	if (_connected) {
		disconnect_server();
	}
	WSACleanup();
}

void NetworkManager::connect_to_server(const std::string& host, int port)
{
	if (_connected) {
		disconnect_server();
	}

	addrinfo* result = nullptr;
	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::string portStr = std::to_string(port);
	int iResult = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
	if (iResult != 0) {
		throw std::runtime_error("getaddrinfo failed: " + std::to_string(iResult));
	}

	for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		_clientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (_clientSocket == INVALID_SOCKET) {
			freeaddrinfo(result);
			throw std::runtime_error("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
		}

		iResult = connect(_clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(_clientSocket);
			_clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (_clientSocket == INVALID_SOCKET) {
		throw std::runtime_error("Unable to connect to server.");
	}

	_connected = true;
}

void NetworkManager::disconnect_server()
{
	if (_connected) {
		shutdown(_clientSocket, SD_SEND);
		closesocket(_clientSocket);
		_clientSocket = INVALID_SOCKET;
		_connected = false;
	}
}

void NetworkManager::send_data(const std::string& data)
{
	if (!_connected) {
		throw std::runtime_error("Not connected to server.");
	}

	int bytesSent = send(_clientSocket, data.c_str(), (int)data.length(), 0);
	if (bytesSent == SOCKET_ERROR) {
		throw std::runtime_error("Send failed with error: " + std::to_string(WSAGetLastError()));
	}

	if (bytesSent < data.length()) {
		throw std::runtime_error("Failed to send all data.");
	}
}

void NetworkManager::receive_exact(char* buffer, size_t size)
{
	size_t totalBytesReceived = 0;
	while (totalBytesReceived < size)
	{
		int bytesReceived = recv(_clientSocket, buffer + totalBytesReceived, (int)(size - totalBytesReceived), 0);

		if (bytesReceived > 0) {
			totalBytesReceived += bytesReceived;
		}
		else if (bytesReceived == 0) {
			_connected = false;
			throw std::runtime_error("Connection closed by server.");
		}
		else {
			_connected = false;
			throw std::runtime_error("Recv failed with error: " + std::to_string(WSAGetLastError()));
		}
	}
}

ServerResponse NetworkManager::receive_response()
{
	if (!_connected) {
		throw std::runtime_error("Not connected to server.");
	}

	char headerBuffer[sizeof(ResponseHeader)];
	receive_exact(headerBuffer, sizeof(ResponseHeader));

	ResponseHeader* header = reinterpret_cast<ResponseHeader*>(headerBuffer);

	if (header->payloadSize == 0) {
		return { header->code, "" };
	}

	if (header->payloadSize > 10 * 1024 * 1024) {
		throw std::runtime_error("Server response payload too large.");
	}

	std::vector<char> payloadBuffer(header->payloadSize);
	receive_exact(payloadBuffer.data(), header->payloadSize);

	return { header->code, std::string(payloadBuffer.data(), payloadBuffer.size()) };
}