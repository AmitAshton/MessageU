#include "ClientInfo.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>

// --- ClientInfo ---

ClientInfo::ClientInfo() = default;

ClientInfo::ClientInfo(const std::string& name, const std::array<uint8_t, CLIENT_ID_SIZE>& id)
    : name(name), id(id) {
}

const std::string& ClientInfo::getName() const {
    return name;
}
const std::array<uint8_t, CLIENT_ID_SIZE>& ClientInfo::getID() const {
    return id;
}

void ClientInfo::setPublicKey(const std::array<uint8_t, PUBLIC_KEY_SIZE>& pk) {
    publicKey = pk;
    hasPublic = true;
}
const std::array<uint8_t, PUBLIC_KEY_SIZE>& ClientInfo::getPublicKey() const {
    return publicKey;
}
bool ClientInfo::hasPublicKey() const {
    return hasPublic;
}

void ClientInfo::setSymmetricKey(const std::vector<uint8_t>& key) {
    symmetricKey = key;
    hasSymmetric = true;
}
const std::vector<uint8_t>& ClientInfo::getSymmetricKey() const {
    return symmetricKey;
}
bool ClientInfo::hasSymmetricKey() const {
    return hasSymmetric;
}

// --- ClientsDirectory ---

void ClientsDirectory::createClientsList(const std::vector<std::tuple<std::string, std::array<uint8_t, CLIENT_ID_SIZE>>>& clients) {
    deleteClientsList();
    for (const auto& client : clients) {
        const std::string& name = std::get<0>(client);
        const std::array<uint8_t, CLIENT_ID_SIZE>& id = std::get<1>(client);
        name_to_id[name] = id;
        id_to_info[id] = ClientInfo(name, id);
    }
}

void ClientsDirectory::deleteClientsList() {
    name_to_id.clear();
    id_to_info.clear();
}

void ClientsDirectory::printClientsNames() const {
    if (name_to_id.empty()) {
        std::cout << "No clients available." << std::endl;
        return;
    }
    for (const auto& p : name_to_id) {
        std::cout << p.first << std::endl;
    }
}

const uint8_t* ClientsDirectory::getOtherClientID(const std::string& username) const {
    auto it = name_to_id.find(username);
    if (it != name_to_id.end()) {
        return it->second.data();
    }
    return nullptr;
}

std::string ClientsDirectory::getOtherUsername(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const {
    auto it = id_to_info.find(clientID);
    if (it != id_to_info.end()) return it->second.getName();
    return std::string();
}

std::string ClientsDirectory::getOtherUsername(const uint8_t* clientID) const {
    std::array<uint8_t, CLIENT_ID_SIZE> idArray;
    std::copy(clientID, clientID + CLIENT_ID_SIZE, idArray.begin());
    return getOtherUsername(idArray);
}

void ClientsDirectory::setPublicKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& id, const std::array<uint8_t, PUBLIC_KEY_SIZE>& pub) {
    auto it = id_to_info.find(id);
    if (it != id_to_info.end()) {
        it->second.setPublicKey(pub);
    }
    else {
        ClientInfo ci("", id);
        ci.setPublicKey(pub);
        id_to_info[id] = ci;
    }
}

const uint8_t* ClientsDirectory::getPublicKeyByID(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const {
    auto it = id_to_info.find(clientID);
    if (it != id_to_info.end() && it->second.hasPublicKey()) return it->second.getPublicKey().data();
    return nullptr;
}

const uint8_t* ClientsDirectory::getPublicKeyByID(const uint8_t* clientID) const {
    std::array<uint8_t, CLIENT_ID_SIZE> idArray;
    std::copy(clientID, clientID + CLIENT_ID_SIZE, idArray.begin());
    return getPublicKeyByID(idArray);
}

void ClientsDirectory::setSymmetricKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& id, const std::vector<uint8_t>& key) {
    auto it = id_to_info.find(id);
    if (it != id_to_info.end()) {
        it->second.setSymmetricKey(key);
    }
    else {
        ClientInfo ci("", id);
        ci.setSymmetricKey(key);
        id_to_info[id] = ci;
    }
}

void ClientsDirectory::setSymmetricKeyForID(const uint8_t* id, const std::vector<uint8_t>& key) {
    std::array<uint8_t, CLIENT_ID_SIZE> idArray;
    std::copy(id, id + CLIENT_ID_SIZE, idArray.begin());
    setSymmetricKeyForID(idArray, key);
}

const std::vector<uint8_t>* ClientsDirectory::getSymmetricKeyForID(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID) const {
    auto it = id_to_info.find(clientID);
    if (it != id_to_info.end() && it->second.hasSymmetricKey())
        return &it->second.getSymmetricKey();
    return nullptr;
}

const std::vector<uint8_t>* ClientsDirectory::getSymmetricKeyForID(const uint8_t* clientID) const {
    std::array<uint8_t, CLIENT_ID_SIZE> idArray;
    std::copy(clientID, clientID + CLIENT_ID_SIZE, idArray.begin());
    return getSymmetricKeyForID(idArray);
}