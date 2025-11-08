#pragma once
#include <string>
#include <map>
#include <array>
#include <vector>

#include "Protocol.h"

struct ClientData {
	std::array<char, UUID_SIZE> uuid;
	std::string username;
	std::string publicKey;
	std::string symmetricKey;
};

class ClientRegistry
{
private:
	std::map<std::array<char, UUID_SIZE>, ClientData*> _clientMap;

	std::map<std::string, std::array<char, UUID_SIZE>> _nameIndex;

public:
	ClientRegistry() = default;

	~ClientRegistry();

	void registerClient(const std::array<char, UUID_SIZE>& uuid, const std::string& name);

	std::vector<ClientData> getAllClients() const;

	ClientData* findByName(const std::string& name);

	ClientData* findByUUID(const std::array<char, UUID_SIZE>& uuid);

	bool setPublicKey(const std::array<char, UUID_SIZE>& uuid, const std::string& pubKey);

	bool setSymmetricKey(const std::array<char, UUID_SIZE>& uuid, const std::string& symKey);
};