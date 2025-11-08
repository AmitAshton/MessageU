#include "MessageUClient.h"
#include "Request.h"       
#include "AESWrapper.h"   
#include "Base64Wrapper.h"  
#include <iostream>
#include <iomanip>      
#include <sstream>       
#include <stdexcept>
#include <cstring>   
#include <limits>

std::string MessageUClient::getHexFromUUID(const std::string& uuid_bytes) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (unsigned char c : uuid_bytes) {
		ss << std::setw(2) << static_cast<int>(c);
	}
	return ss.str();
}

std::string MessageUClient::getUUIDFromHex(const std::string& hex_string) {
	std::string bytes;
	if (hex_string.length() != 32) {
		throw std::runtime_error("Invalid hex UUID string length.");
	}
	for (unsigned int i = 0; i < hex_string.length(); i += 2) {
		std::string byteString = hex_string.substr(i, 2);
		char byte = static_cast<char>(std::strtol(byteString.c_str(), NULL, 16));
		bytes.push_back(byte);
	}
	return bytes;
}

void MessageUClient::clearCinBuffer()
{
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string MessageUClient::getStringFromUser(const std::string& prompt)
{
	std::string input;
	std::cout << prompt;
	std::getline(std::cin, input);
	return input;
}


MessageUClient::MessageUClient() : _myPrivateKey(nullptr), _isRegistered(false)
{
	_myUUID.fill(0);
	loadMyInfo();
	connect();
	std::cout << "Client is connected to server." << std::endl;
}

MessageUClient::~MessageUClient()
{
	delete _myPrivateKey;
	_netManager.disconnect_server();
}


void MessageUClient::loadMyInfo()
{
	if (ClientConfig::myInfoExists())
	{
		_myInfo = ClientConfig::loadMyInfo();
		std::string rawUUID = getUUIDFromHex(_myInfo.uuid);
		memcpy(_myUUID.data(), rawUUID.data(), UUID_SIZE);

		std::string rawPrivateKey = Base64Wrapper::decode(_myInfo.privateKeyBase64);
		_myPrivateKey = new RSAPrivateWrapper(rawPrivateKey);

		_isRegistered = true;
		std::cout << "Logged in as: " << _myInfo.username << std::endl;
	}
	else
	{
		_isRegistered = false;
	}
}

void MessageUClient::connect()
{
	std::pair<std::string, int> server = ClientConfig::loadServerInfo();
	_netManager.connect_to_server(server.first, server.second);
}

void MessageUClient::displayMenu()
{
	std::cout << "\nMessageU client at your service." << std::endl;
	std::cout << "110) Register" << std::endl;
	std::cout << "120) Request for clients list" << std::endl;
	std::cout << "130) Request for public key" << std::endl;
	std::cout << "140) Request for waiting messages" << std::endl;
	std::cout << "150) Send a text message" << std::endl;
	std::cout << "151) Send a request for symmetric key" << std::endl;
	std::cout << "152) Send your symmetric key" << std::endl;
	std::cout << "0) Exit client" << std::endl;
	std::cout << "? ";
}

int MessageUClient::getUserSelection()
{
	int selection = -1;
	if (!(std::cin >> selection))
	{
		std::cin.clear();
		clearCinBuffer();
		return -1;
	}
	clearCinBuffer();
	return selection;
}

void MessageUClient::run()
{
	while (true)
	{
		displayMenu();
		int selection = getUserSelection();

		try {
			switch (selection)
			{
			case 110: handleRegister(); break;
			case 120: handleClientList(); break;
			case 130: handlePublicKey(); break;
			case 140: handlePullMessages(); break;
			case 150: handleSendText(); break;
			case 151: handleRequestSymKey(); break;
			case 152: handleSendSymKey(); break;
			case 0:
				std::cout << "Exiting. Goodbye!" << std::endl;
				return;
			default:
				std::cout << "Error: Unknown option selected. Please try again." << std::endl;
				break;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "An error occurred: " << e.what() << std::endl;
			try { connect(); }
			catch (...) { std::cerr << "Failed to reconnect." << std::endl; }
		}
	}
}


void MessageUClient::handleRegister()
{
	if (_isRegistered) {
		std::cout << "Error: You are already registered." << std::endl;
		return;
	}

	std::string name = getStringFromUser("Enter username: ");
	if (name.length() >= CLIENT_NAME_SIZE) {
		std::cout << "Error: Username is too long." << std::endl;
		return;
	}

	RSAPrivateWrapper newKeys;
	std::string pubKey = newKeys.getPublicKey();

	RegisterRequest req(name, pubKey);

	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2100) {
		std::string uuid_bytes = res.payload;
		std::string uuid_hex = getHexFromUUID(uuid_bytes);
		std::string privKeyRaw = newKeys.getPrivateKey();
		std::string privKey64 = Base64Wrapper::encode(privKeyRaw);

		ClientConfig::saveMyInfo(name, uuid_hex, privKey64);
		loadMyInfo();
		std::cout << "Registered successfully. Your UUID is: " << uuid_hex << std::endl;
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handleClientList()
{
	if (!_isRegistered) {
		std::cout << "Error: You must be registered to perform this action." << std::endl;
		return;
	}

	ClientListRequest req(_myUUID);
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2101) {
		std::cout << "Client List:" << std::endl;
		std::string payload = res.payload;
		const size_t recordSize = UUID_SIZE + CLIENT_NAME_SIZE;

		for (size_t i = 0; i < payload.length(); i += recordSize) {
			std::string uuid_bytes = payload.substr(i, UUID_SIZE);
			std::string name_bytes = payload.substr(i + UUID_SIZE, CLIENT_NAME_SIZE);

			std::string name_str(name_bytes.c_str());

			std::array<char, UUID_SIZE> uuid_arr;
			memcpy(uuid_arr.data(), uuid_bytes.data(), UUID_SIZE);

			_registry.registerClient(uuid_arr, name_str);
			std::cout << "- " << name_str << std::endl;
		}
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handlePublicKey()
{
	if (!_isRegistered) {
		std::cout << "Error: You must be registered to perform this action." << std::endl;
		return;
	}

	std::string name = getStringFromUser("Enter client name: ");
	ClientData* target = _registry.findByName(name);
	if (!target) {
		std::cout << "Error: Client not found. Please request client list first." << std::endl;
		return;
	}

	PublicKeyRequest req(_myUUID, target->uuid);
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2102) {
		std::string pubKey = res.payload.substr(UUID_SIZE);
		_registry.setPublicKey(target->uuid, pubKey);
		std::cout << "Successfully received public key for " << name << std::endl;
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handleSendSymKey()
{
	if (!_isRegistered) {
		std::cout << "Error: You must be registered." << std::endl;
		return;
	}

	std::string name = getStringFromUser("Enter client name: ");
	ClientData* target = _registry.findByName(name);
	if (!target) {
		std::cout << "Error: Client not found. Please request client list first." << std::endl;
		return;
	}
	if (target->publicKey.empty()) {
		std::cout << "Error: Public key for " << name << " is unknown. Please request it first (130)." << std::endl;
		return;
	}

	unsigned char key_bytes[AESWrapper::DEFAULT_KEYLENGTH];
	AESWrapper::GenerateKey(key_bytes, AESWrapper::DEFAULT_KEYLENGTH);
	std::string symKey(reinterpret_cast<char*>(key_bytes), AESWrapper::DEFAULT_KEYLENGTH);

	RSAPublicWrapper rsaPub(target->publicKey);
	std::string encryptedKey = rsaPub.encrypt(symKey);

	SendMessageRequest req(_myUUID, target->uuid, MessageType::SEND_SYM_KEY, encryptedKey);
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2103) {
		_registry.setSymmetricKey(target->uuid, symKey);
		std::cout << "Symmetric key sent to " << name << std::endl;
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handleRequestSymKey()
{
	if (!_isRegistered) {
		std::cout << "Error: You must be registered." << std::endl;
		return;
	}

	std::string name = getStringFromUser("Enter client name: ");
	ClientData* target = _registry.findByName(name);
	if (!target) {
		std::cout << "Error: Client not found. Please request client list first." << std::endl;
		return;
	}

	SendMessageRequest req(_myUUID, target->uuid, MessageType::REQUEST_SYM_KEY, "");
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2103) {
		std::cout << "Request for symmetric key sent to " << name << std::endl;
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handleSendText()
{
	if (!_isRegistered) {
		std::cout << "Error: You must be registered." << std::endl;
		return;
	}

	std::string name = getStringFromUser("Enter client name: ");
	ClientData* target = _registry.findByName(name);
	if (!target) {
		std::cout << "Error: Client not found. Please request client list first." << std::endl;
		return;
	}
	if (target->symmetricKey.empty()) {
		std::cout << "Error: Symmetric key for " << name << " is unknown. Please send one first (152)." << std::endl;
		return;
	}

	std::string text = getStringFromUser("Enter message: ");

	AESWrapper aes(reinterpret_cast<const unsigned char*>(target->symmetricKey.c_str()), AESWrapper::DEFAULT_KEYLENGTH);
	std::string cipher = aes.encrypt(text.c_str(), (unsigned int)text.length());

	SendMessageRequest req(_myUUID, target->uuid, MessageType::TEXT_MESSAGE, cipher);
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code == 2103) {
		std::cout << "Message sent." << std::endl;
	}
	else {
		std::cout << "server responded with an error" << std::endl;
	}
}

void MessageUClient::handlePullMessages()
{
	if (!_isRegistered || !_myPrivateKey) {
		std::cout << "Error: You must be registered to perform this action." << std::endl;
		return;
	}

	PullMessagesRequest req(_myUUID);
	_netManager.send_data(req.getPackedRequest());
	ServerResponse res = _netManager.receive_response();

	if (res.code != 2104) {
		std::cout << "server responded with an error" << std::endl;
		return;
	}
	if (res.payload.empty()) {
		std::cout << "No new messages." << std::endl;
		return;
	}

	size_t offset = 0;
	std::string payload = res.payload;

	while (offset < payload.length())
	{
		std::array<char, UUID_SIZE> fromUUID;
		memcpy(fromUUID.data(), &payload[offset], UUID_SIZE);
		offset += UUID_SIZE;

		uint32_t msgID = *reinterpret_cast<uint32_t*>(&payload[offset]);
		offset += sizeof(uint32_t);

		MessageType msgType = static_cast<MessageType>(payload[offset]);
		offset += sizeof(uint8_t);

		uint32_t contentSize = *reinterpret_cast<uint32_t*>(&payload[offset]);
		offset += sizeof(uint32_t);

		std::string content = payload.substr(offset, contentSize);
		offset += contentSize;

		ClientData* sender = _registry.findByUUID(fromUUID);
		std::string senderName = sender ? sender->username : "Unknown";

		std::cout << "From: " << senderName << std::endl;
		std::cout << "Content:" << std::endl;

		switch (msgType)
		{
		case MessageType::REQUEST_SYM_KEY:
			std::cout << "Request for symmetric key" << std::endl;
			break;
		case MessageType::SEND_SYM_KEY:
			try {
				std::string decryptedKey = _myPrivateKey->decrypt(content);
				_registry.setSymmetricKey(fromUUID, decryptedKey);
				std::cout << "symmetric key received" << std::endl;
			}
			catch (const std::exception&) {
				std::cout << "can't decrypt message" << std::endl;
			}
			break;
		case MessageType::TEXT_MESSAGE:
			if (sender && !sender->symmetricKey.empty()) {
				try {
					AESWrapper aes(reinterpret_cast<const unsigned char*>(sender->symmetricKey.c_str()), AESWrapper::DEFAULT_KEYLENGTH);
					std::string plain = aes.decrypt(content.c_str(), (unsigned int)content.length());
					std::cout << plain << std::endl;
				}
				catch (const std::exception&) {
					std::cout << "can't decrypt message" << std::endl;
				}
			}
			else {
				std::cout << "can't decrypt message" << std::endl;
			}
			break;
		default:
			std::cout << "Unknown message type received." << std::endl;
		}
		std::cout << "-----<EOM>-----" << std::endl << std::endl;
	}
}