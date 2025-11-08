#pragma once
#include <string>
#include <map>
#include <array>
#include <vector>
// We no longer include <optional>

#include "Protocol.h" // For UUID_SIZE

/**
 * @struct ClientData
 * @brief Holds all known information about another client.
 *
 * This struct stores the essential data for interacting with
 * other users, including their keys.
 * An empty string ("") indicates the key is not yet known.
 */
struct ClientData {
	std::array<char, UUID_SIZE> uuid;
	std::string username;
	std::string publicKey;    // The raw 160-byte public key (or "" if unknown)
	std::string symmetricKey; // The raw 16-byte AES key (or "" if unknown)
};

/**
 * @class ClientRegistry
 * @brief Manages the in-memory store of other clients.
 *
 * This class uses a map to store and retrieve client data
 * efficiently, both by UUID and by username.
 * It stores pointers and is responsible for their memory.
 */
class ClientRegistry
{
private:
	// The primary data store, mapping a UUID to its data
	std::map<std::array<char, UUID_SIZE>, ClientData*> _clientMap;

	// An index to quickly find a client's UUID by their username
	std::map<std::string, std::array<char, UUID_SIZE>> _nameIndex;

public:
	ClientRegistry() = default;

	/**
	 * @brief Destructor. Cleans up all allocated ClientData objects.
	 */
	~ClientRegistry();

	/**
	 * @brief Adds a new client or updates an existing one's name.
	 * @param uuid The client's unique 16-byte UUID.
	 * @param name The client's username.
	 */
	void registerClient(const std::array<char, UUID_SIZE>& uuid, const std::string& name);

	/**
	 * @brief Gets a list of all known clients.
	 * @return A vector of ClientData structs (by value).
	 */
	std::vector<ClientData> getAllClients() const;

	/**
	 * @brief Finds a client by their username.
	 * @param name The username to search for.
	 * @return A pointer to ClientData, or nullptr if not found.
	 */
	ClientData* findByName(const std::string& name);

	/**
	 * @brief Finds a client by their UUID.
	 * @param uuid The UUID to search for.
	 * @return A pointer to ClientData, or nullptr if not found.
	 */
	ClientData* findByUUID(const std::array<char, UUID_SIZE>& uuid);

	/**
	 * @brief Updates a client's public key.
	 * @param uuid The UUID of the client to update.
	 * @param pubKey The raw 160-byte public key.
	 * @return true on success, false if client UUID was not found.
	 */
	bool setPublicKey(const std::array<char, UUID_SIZE>& uuid, const std::string& pubKey);

	/**
	 * @brief Updates a client's symmetric key.
	 * @param uuid The UUID of the client to update.
	 * @param symKey The raw 16-byte symmetric key.
	 * @return true on success, false if client UUID was not found.
	 */
	bool setSymmetricKey(const std::array<char, UUID_SIZE>& uuid, const std::string& symKey);
};