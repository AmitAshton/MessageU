#include "NetworkManager.h"
#include <stdexcept>
#include <vector>

// Define response header structure (7 bytes total)
#pragma pack(push, 1)
struct ResponseHeader {
	uint8_t  version;
	uint16_t code;
	uint32_t payloadSize;
};
#pragma pack(pop)


NetworkManager::NetworkManager() : _clientSocket(INVALID_SOCKET), _connected(false)
{
	// Initialize Winsock
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
	// Cleanup Winsock
	WSACleanup();
}

void NetworkManager::connect_to_server(const std::string& host, int port)
{
	if (_connected) {
		disconnect_server();
	}

	addrinfo* result = nullptr;
	addrinfo hints = {};
	hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	std::string portStr = std::to_string(port);
	int iResult = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
	if (iResult != 0) {
		throw std::runtime_error("getaddrinfo failed: " + std::to_string(iResult));
	}

	// Attempt to connect to an address until one succeeds
	for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		_clientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (_clientSocket == INVALID_SOCKET) {
			freeaddrinfo(result);
			throw std::runtime_error("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
		}

		// Connect to server
		iResult = connect(_clientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(_clientSocket);
			_clientSocket = INVALID_SOCKET;
			continue; // Try the next address
		}
		break; // Successfully connected
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

void NetworkManager::send_data(const char* buffer, size_t size)
{
	if (!_connected) {
		throw std::runtime_error("Not connected to server.");
	}

	int bytesSent = send(_clientSocket, buffer, (int)size, 0);
	if (bytesSent == SOCKET_ERROR) {
		throw std::runtime_error("Send failed with error: " + std::to_string(WSAGetLastError()));
	}

	if (bytesSent < size) {
		// This is a simple implementation. A more robust one would
		// loop until all bytes are sent.
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
			// Connection closed gracefully by server
			_connected = false;
			throw std::runtime_error("Connection closed by server.");
		}
		else {
			// Socket error
			_connected = false;
			throw std::runtime_error("Recv failed with error: " + std::to_string(WSAGetLastError()));
		}
	}
}

std::string NetworkManager::receive_response()
{
	if (!_connected) {
		throw std::runtime_error("Not connected to server.");
	}

	// 1. Read the fixed-size header (7 bytes)
	char headerBuffer[sizeof(ResponseHeader)];
	receive_exact(headerBuffer, sizeof(ResponseHeader));

	ResponseHeader* header = reinterpret_cast<ResponseHeader*>(headerBuffer);

	// 2. Check payload size
	if (header->payloadSize == 0) {
		// No payload, just return the header info
		// We'll handle this properly in the protocol layer
		return "";
	}

	if (header->payloadSize > 10 * 1024 * 1024) { // 10MB limit
		throw std::runtime_error("Server response payload too large.");
	}

	// 3. Read the exact payload size
	// We use std::vector as a safe, resizable buffer
	std::vector<char> payloadBuffer(header->payloadSize);
	receive_exact(payloadBuffer.data(), header->payloadSize);

	// Convert the payload buffer to a std::string
	return std::string(payloadBuffer.data(), payloadBuffer.size());
}