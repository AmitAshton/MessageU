#pragma once

// Required definitions for Winsock
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstdint> // For uint16_t, uint32_t

// Link the Winsock library
#pragma comment(lib, "Ws2_32.lib")

/**
 * @struct ServerResponse
 * @brief A structure to hold the processed response from the server.
 */
struct ServerResponse {
	uint16_t code;        // The response code (e.g., 2100, 9000)
	std::string payload;  // The raw payload data
};

/**
 * @class NetworkManager
 * @brief Manages the raw TCP socket connection using Winsock.
 *
 * This class handles initializing Winsock, connecting to the server,
 * disconnecting, and providing reliable send/receive operations.
 */
class NetworkManager
{
private:
	SOCKET _clientSocket;
	bool _connected;

	/**
	 * @brief Receives an exact number of bytes from the socket.
	 *
	 * This is a blocking call that will read from the socket until
	 * exactly 'size' bytes have been received.
	 * @param buffer The buffer to write the received data into.
	 * @param size The exact number of bytes to read.
	 * @throws std::runtime_error if the connection is lost or an error occurs.
	 */
	void receive_exact(char* buffer, size_t size);

public:
	/**
	 * @brief Constructor. Initializes Winsock.
	 */
	NetworkManager();

	/**
	 * @brief Destructor. Cleans up Winsock.
	 */
	virtual ~NetworkManager();

	/**
	 * @brief Connects to the specified server.
	 * @param host The server's IP address or hostname.
	 * @param port The server's port number.
	 * @throws std::runtime_error on any connection failure.
	 */
	void connect_to_server(const std::string& host, int port);

	/**
	 * @brief Disconnects from the server.
	 */
	void disconnect_server();

	/**
	 * @brief Sends a block of data to the server.
	 *
	 * Ensures all data in the buffer is sent.
	 * @param data The data to send.
	 * @throws std::runtime_error if the send operation fails.
	 */
	void send_data(const std::string& data);

	/**
	 * @brief Reads the full response from the server.
	 *
	 * This function implements the protocol's receive logic:
	 * 1. Reads the 7-byte response header.
	 * 2. Parses the code and payload size.
	 * 3. Reads the exact payload size.
	 * @return A ServerResponse struct containing the code and payload.
	 * @throws std::runtime_error if the receive operation fails.
	 */
	ServerResponse receive_response();
};