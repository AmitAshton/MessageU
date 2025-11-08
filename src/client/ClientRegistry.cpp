#include "ClientRegistry.h"

ClientRegistry::~ClientRegistry()
{
	for (auto& pair : _clientMap) {
		delete pair.second;
	}
	_clientMap.clear();
	_nameIndex.clear();
}

void ClientRegistry::registerClient(const std::array<char, UUID_SIZE>& uuid, const std::string& name)
{
	auto it = _clientMap.find(uuid);
	if (it != _clientMap.end()) {
		ClientData* client = it->second;

		if (client->username != name) {
			_nameIndex.erase(client->username);
		}
		client->username = name;
		_nameIndex[name] = uuid;
	}
	else {
		ClientData* newClient = new ClientData{ uuid, name, "", "" };
		_clientMap[uuid] = newClient;
		_nameIndex[name] = uuid;
	}
}

std::vector<ClientData> ClientRegistry::getAllClients() const
{
	std::vector<ClientData> clients;
	clients.reserve(_clientMap.size());

	for (const auto& pair : _clientMap) {
		clients.push_back(*(pair.second));
	}
	return clients;
}

ClientData* ClientRegistry::findByName(const std::string& name)
{
	auto it = _nameIndex.find(name);
	if (it != _nameIndex.end()) {
		return findByUUID(it->second);
	}
	return nullptr;
}

ClientData* ClientRegistry::findByUUID(const std::array<char, UUID_SIZE>& uuid)
{
	auto it = _clientMap.find(uuid);
	if (it != _clientMap.end()) {
		return it->second;
	}
	return nullptr;
}

bool ClientRegistry::setPublicKey(const std::array<char, UUID_SIZE>& uuid, const std::string& pubKey)
{
	ClientData* client = findByUUID(uuid);
	if (client) {
		client->publicKey = pubKey;
		return true;
	}
	return false;
}

bool ClientRegistry::setSymmetricKey(const std::array<char, UUID_SIZE>& uuid, const std::string& symKey)
{
	ClientData* client = findByUUID(uuid);
	if (client) {
		client->symmetricKey = symKey;
		return true;
	}
	return false;
}