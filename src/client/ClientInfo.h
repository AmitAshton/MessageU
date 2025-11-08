#pragma once

#include "Constants.h"
#include <array>
#include <string>
#include <vector>
#include <map>
#include <tuple>

class ClientInfo {
public:
    ClientInfo();
    ClientInfo(const std::string& name, const std::array<uint8_t, CLIENT_ID_SIZE>& id);

    const std::string& getName() const;
    const std::array<uint8_t, CLIENT_ID_SIZE>& getID() const;

    void setPublicKey(const std::array<uint8_t, PUBLIC_KEY_SIZE>& pk);
    const std::array<uint8_t, PUBLIC_KEY_SIZE>& getPublicKey() const;
    bool hasPublicKey() const;

    void setSymmetricKey(const std::vector<uint8_t>& key);
    const std::vector<uint8_t>& getSymmetricKey() const;
    bool hasSymmetricKey() const;

private:
    std::string name;
    std::array<uint8_t, CLIENT_ID_SIZE> id = {};
    std::array<uint8_t, PUBLIC_KEY_SIZE> publicKey = {};
    std::vector<uint8_t> symmetricKey = {};
    bool hasSymmetric = false;
    bool hasPublic = false;
};


class ClientsDirectory {
public:
    ClientsDirectory() = default;

    void createClientsList(const std::vector<std::tuple<std::string, std::array<uint8_t, CLIENT_ID_SIZE>>>& clients);
    void deleteClientsList();
    void printClientsNames() const;

    // --- Lookups ---
    const uint8_t* getOtherClientID(const std::string& username) const;
    std::string getOtherUsername(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const;
    std::string getOtherUsername(const uint8_t* clientID) const;

    // --- Public Key Management ---
    void setPublicKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& id, const std::array<uint8_t, PUBLIC_KEY_SIZE>& pub);
    const uint8_t* getPublicKeyByID(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const;
    const uint8_t* getPublicKeyByID(const uint8_t* clientID) const;

    // --- Symmetric Key Management ---
    void setSymmetricKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& id, const std::vector<uint8_t>& key);
    void setSymmetricKeyForID(const uint8_t* id, const std::vector<uint8_t>& key);
    const std::vector<uint8_t>* getSymmetricKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const;
    const std::vector<uint8_t>* getSymmetricKeyForID(const uint8_t* clientID) const;

private:
    std::map<std::string, std::array<uint8_t, CLIENT_ID_SIZE>> name_to_id;
    std::map<std::array<uint8_t, CLIENT_ID_SIZE>, ClientInfo> id_to_info;
};