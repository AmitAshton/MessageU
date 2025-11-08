#include "ClientRegistry.h"

ClientRegistry::~ClientRegistry()
{
	// Iterate over the map and delete each allocated ClientData pointer
	for (auto& pair : _clientMap) {
		delete pair.second;
	}
	_clientMap.clear();
	_nameIndex.clear();
}

void ClientRegistry::registerClient(const std::array<char, UUID_SIZE>& uuid, const std::string& name)
{
	// Check if this client (by UUID) is already known
	auto it = _clientMap.find(uuid);
	if (it != _clientMap.end()) {
		// Client exists, update their name if it changed
		ClientData* client = it->second;

		// Remove the old name from the index if it's different
		if (client->username != name) {
			_nameIndex.erase(client->username);
		}
		client->username = name;
		_nameIndex[name] = uuid; // Update index
	}
	else {
		// New client, create a new entry
		ClientData* newClient = new ClientData{ uuid, name, "", "" };
		_clientMap[uuid] = newClient;
		_nameIndex[name] = uuid;
	}
}

std::vector<ClientData> ClientRegistry::getAllClients() const
{
	std::vector<ClientData> clients;
	clients.reserve(_clientMap.size());

	// Iterate over the map and add each client's data (by value) to the vector
	for (const auto& pair : _clientMap) {
		clients.push_back(*(pair.second));
	}
	return clients;
}

ClientData* ClientRegistry::findByName(const std::string& name)
{
	// Use the name index to find the UUID
	auto it = _nameIndex.find(name);
	if (it != _nameIndex.end()) {
		// If name exists, use the found UUID to get the full data
		return findByUUID(it->second);
	}
	// No client with this name
	return nullptr;
}

ClientData* ClientRegistry::findByUUID(const std::array<char, UUID_SIZE>& uuid)
{
	// Find the client directly in the main map
	auto it = _clientMap.find(uuid);
	if (it != _clientMap.end()) {
		return it->second; // Return the pointer
	}
	// No client with this UUID
	return nullptr;
}

bool ClientRegistry::setPublicKey(const std::array<char, UUID_SIZE>& uuid, const std::string& pubKey)
{
	// Find the client
	ClientData* client = findByUUID(uuid);
	if (client) {
		// Client found, update their public key
		client->publicKey = pubKey;
		return true;
	}
	// Client not found
	return false;
}

bool ClientRegistry::setSymmetricKey(const std::array<char, UUID_SIZE>& uuid, const std::string& symKey)
{
	// Find the client
	ClientData* client = findByUUID(uuid);
	if (client) {
		// Client found, update their symmetric key
		client->symmetricKey = symKey;
		return true;
	}
	// Client not found
	return false;
}