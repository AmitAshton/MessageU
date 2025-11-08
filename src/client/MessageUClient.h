#pragma once

#include "NetworkManager.h"
#include "ClientConfig.h"
#include "ClientRegistry.h"
#include "RSAWrapper.h"
#include "Protocol.h"
#include <string>
#include <array>

class MessageUClient
{
public:
	MessageUClient();
	~MessageUClient();

	void run();

private:
	NetworkManager _netManager;
	ClientRegistry _registry;
	RSAPrivateWrapper* _myPrivateKey;

	MyInfo _myInfo;
	std::array<char, UUID_SIZE> _myUUID;
	bool _isRegistered;

	void connect();

	void loadMyInfo();

	void displayMenu();

	int getUserSelection();

	std::string getStringFromUser(const std::string& prompt);

	void clearCinBuffer();

	std::string getHexFromUUID(const std::string& uuid_bytes);

	std::string getUUIDFromHex(const std::string& hex_string);

	void handleRegister();
	void handleClientList();
	void handlePublicKey();
	void handlePullMessages();
	void handleSendText();
	void handleRequestSymKey();
	void handleSendSymKey();
};