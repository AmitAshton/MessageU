#pragma once

#include <string>
#include <vector>
#include <map>
#include <array>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <iostream>

#include "Constants.h"
#include "ConnectionManager.h"
#include "ClientInfo.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"
#include "Base64Wrapper.h"
#include "Response.h"
#include "Request.h"
#include "Payload.h"
#include "ServerError.h"

namespace fs = std::filesystem;

class MessagingClient {
public:
    MessagingClient(const std::string& serverAddress);

    void handleOption(int code);

    // --- Menu Actions ---
    void registerClient();
    void requestClientList();
    void requestPublicKey();
    void requestIncomingMessages();
    void sendMessage();
    void requestSymmetricKey();
    void sendSymmetricKey();
    void sendFile();

private:
    ConnectionManager m_connManager;
    ClientsDirectory m_clients;
    RSAPrivateWrapper m_rsaPrivate;
    std::string m_username;
    std::array<uint8_t, CLIENT_ID_SIZE> m_clientID;
    bool m_isRegistered;

    // --- Core Helpers ---
    bool isRegistered() const;
    void loadClientInfo();
    void saveClientInfo(const uint8_t newClientID[CLIENT_ID_SIZE]);
    Response sendAndReceive(Request& request, uint16_t expectedResponseCode);

    // --- UI Helpers ---
    const uint8_t* getTargetClientID(const std::string& prompt, bool requirePubKey = false, bool requireSymKey = false);

    // --- Crypto Helpers ---
    std::vector<uint8_t> encryptRSA(const std::vector<uint8_t>& plain, const uint8_t pubKey[PUBLIC_KEY_SIZE]);
    std::vector<uint8_t> decryptRSA(const std::vector<uint8_t>& cipher);
    std::vector<uint8_t> encryptAES(const std::vector<uint8_t>& plain, const std::vector<uint8_t>& key);
    std::vector<uint8_t> decryptAES(const std::vector<uint8_t>& cipher, const std::vector<uint8_t>& key);
};