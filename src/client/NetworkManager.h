#pragma once

// Required definitions for Winsock
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

// Link the Winsock library
#pragma comment(lib, "Ws2_32.lib")

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
	 * @param buffer The data to send.
	 * @param size The size of the data in bytes.
	 * @throws std::runtime_error if the send operation fails.
	 */
	void send_data(const char* buffer, size_t size);

	/**
	 * @brief Reads the full response from the server.
	 *
	 * This function implements the protocol's receive logic:
	 * 1. Reads the 7-byte response header.
	 * 2. Parses the payload size from the header.
	 * 3. Reads the exact payload size.
	 * @return A std::string containing the raw payload.
	 * @throws std::runtime_error if the receive operation fails.
	 */
	std::string receive_response();
};