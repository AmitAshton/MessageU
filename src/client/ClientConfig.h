#pragma once
#include <string>

// Defines filenames used by the client
#define SERVER_INFO_FILE "server.info"
#define MY_INFO_FILE "my.info"

/**
 * @struct MyInfo
 * @brief Holds the client's personal details loaded from my.info
 */
struct MyInfo {
	std::string username;
	std::string uuid; // Stored as ASCII hex string
	std::string privateKeyBase64; // Stored as Base64
};

/**
 * @class ClientConfig
 * @brief Manages reading and writing configuration files.
 *
 * This class handles loading server details (server.info)
 * and managing the user's registration details (my.info).
 */
class ClientConfig
{
public:
	// --- server.info methods ---

	/**
	 * @brief Loads server IP and port from server.info
	 * @return A pair containing server host (string) and port (int)
	 * @throws std::runtime_error if file cannot be opened or parsed
	 */
	static std::pair<std::string, int> loadServerInfo();


	// --- my.info methods ---

	/**
	 * @brief Checks if the my.info file already exists.
	 * @return true if file exists, false otherwise.
	 */
	static bool myInfoExists();

	/**
	 * @brief Loads the client's details from my.info
	 * @return A MyInfo struct filled with data
	 * @throws std::runtime_error if file cannot be opened or is incomplete
	 */
	static MyInfo loadMyInfo();

	/**
	 * @brief Saves the client's details to my.info
	 * @param username The user's chosen name
	 * @param uuid The client UUID received from the server
	 * @param privateKeyBase64 The client's private key, Base64 encoded
	 * @throws std::runtime_error if file cannot be opened for writing
	 */
	static void saveMyInfo(const std::string& username, const std::string& uuid, const std::string& privateKeyBase64);
};