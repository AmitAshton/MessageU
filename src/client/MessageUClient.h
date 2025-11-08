#pragma once

#include "NetworkManager.h"
#include "ClientConfig.h"
#include "ClientRegistry.h"
#include "RSAWrapper.h" // For RSAPrivateWrapper
#include "Protocol.h"
#include <string>
#include <array>

/**
 * @class MessageUClient
 * @brief The main controller class for the client application.
 *
 * This class orchestrates all components:
 * - Manages the network connection.
 * - Holds the in-memory client registry.
 * - Stores the current user's info (from my.info).
 * - Runs the main menu loop.
 */
class MessageUClient
{
public:
	MessageUClient();
	~MessageUClient();

	/**
	 * @brief Runs the main application loop.
	 * Displays the menu and handles user input.
	 */
	void run();

private:
	// --- Core Components ---
	NetworkManager _netManager;
	ClientRegistry _registry;
	RSAPrivateWrapper* _myPrivateKey; // Our private key, loaded from my.info

	// --- Client State ---
	MyInfo _myInfo; // Struct with our username, uuid(hex), and key(base64)
	std::array<char, UUID_SIZE> _myUUID; // Our raw 16-byte UUID
	bool _isRegistered;

	// --- Private Helper Methods ---

	/**
	 * @brief Loads server.info and connects the NetworkManager.
	 * @throws std::runtime_error on failure
	 */
	void connect();

	/**
	 * @brief Loads my.info, sets _isRegistered, and loads private key.
	 */
	void loadMyInfo();

	/**
	 * @brief Displays the main menu to the user.
	 */
	void displayMenu();

	/**
	 * @brief Reads an integer selection from the user.
	 * @return The user's menu choice.
	 */
	int getUserSelection();

	/**
	 * @brief Reads a line of text from the user.
	 * @param prompt The message to display to the user.
	 * @return The line of text entered.
	 */
	std::string getStringFromUser(const std::string& prompt);

	/**
	 * @brief Clears any error flags on std::cin and discards the buffer.
	 */
	void clearCinBuffer();

	/**
	 * @brief Helper to convert a raw 16-byte UUID string to a 32-char hex string.
	 */
	std::string getHexFromUUID(const std::string& uuid_bytes);

	/**
	 * @brief Helper to convert a 32-char hex string to a raw 16-byte UUID string.
	 */
	std::string getUUIDFromHex(const std::string& hex_string);


	// --- Menu Action Handlers ---
	void handleRegister();
	void handleClientList();
	void handlePublicKey();
	void handlePullMessages();
	void handleSendText();
	void handleRequestSymKey();
	void handleSendSymKey();
};