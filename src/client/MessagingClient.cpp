#include "MessagingClient.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// --- Constructor & Initialization ---

MessagingClient::MessagingClient(const std::string& serverAddress)
    : m_connManager(serverAddress), m_isRegistered(false) {
    try {
        loadClientInfo();
        if (m_isRegistered) {
            std::cout << "Welcome back, " << m_username << "!" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Warning: Could not load client info file. " << e.what() << std::endl;
        m_isRegistered = false;
    }
}

bool MessagingClient::isRegistered() const {
    if (!m_isRegistered) {
        std::cout << "Client is not registered. Please register first." << std::endl;
    }
    return m_isRegistered;
}

void MessagingClient::loadClientInfo() {
    std::ifstream file(CLIENT_INFO_FILENAME);
    if (!file.is_open()) {
        m_isRegistered = false;
        return;
    }

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) {
        lines.push_back(Utils::trim(line));
    }

    if (lines.size() < 3) {
        throw std::runtime_error("Invalid " + CLIENT_INFO_FILENAME + " format.");
    }

    m_username = lines[0];

    std::vector<uint8_t> idVec = Utils::hexToBytes(lines[1]);
    if (idVec.size() != CLIENT_ID_SIZE) {
        throw std::runtime_error("Invalid client ID size in " + CLIENT_INFO_FILENAME);
    }
    std::copy(idVec.begin(), idVec.end(), m_clientID.begin());

    std::string decodedPrivKey = Base64Wrapper::decode(lines[2]);
    m_rsaPrivate.loadFromString(decodedPrivKey);

    m_isRegistered = true;
}

void MessagingClient::saveClientInfo(const uint8_t newClientID[CLIENT_ID_SIZE]) {
    std::copy(newClientID, newClientID + CLIENT_ID_SIZE, m_clientID.begin());

    std::ofstream outFile(CLIENT_INFO_FILENAME);
    if (!outFile.is_open()) {
        throw std::runtime_error("Could not write to " + CLIENT_INFO_FILENAME);
    }

    std::string idHex = Utils::bytesToHexString(m_clientID.data(), CLIENT_ID_SIZE);
    std::string privKeyStr = m_rsaPrivate.getPrivateKey();
    std::string privKeyBase64 = Base64Wrapper::encode(privKeyStr);

    outFile << m_username << std::endl;
    outFile << idHex << std::endl;
    outFile << privKeyBase64 << std::endl;
}

// --- Network I/O ---

Response MessagingClient::sendAndReceive(Request& request, uint16_t expectedResponseCode) {
    std::vector<uint8_t> requestBuffer = request.serialize();
    std::vector<uint8_t> responseBuffer = m_connManager.sendAndReceiveOnce(requestBuffer);

    Response res(responseBuffer);

    if (res.getResponseCode() == RESPONSE_CODE_GENERAL_ERROR) {
        throw ServerError();
    }

    if (res.getResponseCode() != expectedResponseCode) {
        throw ServerError("Unexpected server response code.");
    }

    return res;
}

// --- Handler Dispatcher ---

void MessagingClient::handleOption(int code) {
    switch (code) {
    case MENU_REGISTER:
        return registerClient();
    case MENU_REQUEST_CLIENTS:
        if (isRegistered()) requestClientList();
        break;
    case MENU_REQUEST_PUBLIC_KEY:
        if (isRegistered()) requestPublicKey();
        break;
    case MENU_REQUEST_WAITING:
        if (isRegistered()) requestIncomingMessages();
        break;
    case MENU_SEND_TEXT:
        if (isRegistered()) sendMessage();
        break;
    case MENU_SEND_REQUEST_SYMM_KEY:
        if (isRegistered()) requestSymmetricKey();
        break;
    case MENU_SEND_SYMM_KEY:
        if (isRegistered()) sendSymmetricKey();
        break;
    case MENU_SEND_FILE:
        if (isRegistered()) sendFile();
        break;
    default:
        std::cout << "Invalid option." << std::endl;
        return;
    }
}

// --- UI / Logic Helpers ---

const uint8_t* MessagingClient::getTargetClientID(const std::string& prompt, bool requirePubKey, bool requireSymKey) {
    std::string targetUsername;
    while (true) {
        std::cout << prompt << " (press Enter to cancel)" << std::endl;
        std::getline(std::cin, targetUsername);
        targetUsername = Utils::trim(targetUsername);

        if (targetUsername.empty()) return nullptr;

        const uint8_t* targetID = m_clients.getOtherClientID(targetUsername);
        if (targetID == nullptr) {
            std::cout << "Username doesn't exist." << std::endl;
            continue;
        }

        if (requirePubKey && m_clients.getPublicKeyByID(targetID) == nullptr) {
            std::cout << "No public key found for this user. Please request it first (130)." << std::endl;
            continue;
        }

        if (requireSymKey && m_clients.getSymmetricKeyForID(targetID) == nullptr) {
            std::cout << "No symmetric key found for this user. Please exchange keys first (151/152)." << std::endl;
            continue;
        }

        return targetID;
    }
}

// --- Core Logic Methods ---

void MessagingClient::registerClient() {
    if (m_isRegistered) {
        std::cout << "Client is already registered as " << m_username << "." << std::endl;
        return;
    }

    std::cout << "Enter your username: ";
    std::getline(std::cin, m_username);
    m_username = Utils::trim(m_username);

    if (m_username.empty() || m_username.length() >= USERNAME_MAX_SIZE) {
        throw std::runtime_error("Invalid or empty username.");
    }

    m_rsaPrivate.generate();
    std::string publicKeyStr = m_rsaPrivate.getPublicKey();

    std::vector<uint8_t> payload = Payload::createRegisterPayload(m_username, publicKeyStr);
    Request request(DUMMY_CLIENT_ID, REQUEST_CODE_REGISTER, payload);
    Response response = sendAndReceive(request, RESPONSE_CODE_REGISTER_SUCCESS);

    const uint8_t* newClientID = response.getPayload().getClientID();
    saveClientInfo(newClientID);
    m_isRegistered = true;

    std::cout << "Registration successful." << std::endl;
}

void MessagingClient::requestClientList() {
    std::vector<uint8_t> emptyPayload;
    Request request(m_clientID.data(), REQUEST_CODE_CLIENT_LIST, emptyPayload);
    Response response = sendAndReceive(request, RESPONSE_CODE_CLIENT_LIST);

    m_clients.createClientsList(response.getPayload().getClientsList());
    m_clients.printClientsNames();
}

void MessagingClient::requestPublicKey() {
    const uint8_t* targetID = getTargetClientID("Enter username to get public key for:");
    if (targetID == nullptr) return;

    std::vector<uint8_t> payload = Payload::createClientKeyRequestPayload(targetID);
    Request request(m_clientID.data(), REQUEST_CODE_PUBLIC_KEY, payload);
    Response response = sendAndReceive(request, RESPONSE_CODE_PUBLIC_KEY);

    const Payload& resPayload = response.getPayload();
    std::array<uint8_t, CLIENT_ID_SIZE> targetIDArray;
    std::array<uint8_t, PUBLIC_KEY_SIZE> targetPubKeyArray;

    std::copy(resPayload.getClientID(), resPayload.getClientID() + CLIENT_ID_SIZE, targetIDArray.begin());
    std::copy(resPayload.getPublicKey(), resPayload.getPublicKey() + PUBLIC_KEY_SIZE, targetPubKeyArray.begin());

    m_clients.setPublicKeyForID(targetIDArray, targetPubKeyArray);
    std::cout << "Public key received and stored." << std::endl;
}

void MessagingClient::requestSymmetricKey() {
    const uint8_t* targetID = getTargetClientID("Enter username to request symmetric key from:", true);
    if (targetID == nullptr) return;

    const uint8_t* targetPubKey = m_clients.getPublicKeyByID(targetID);

    std::vector<uint8_t> emptyContent;
    std::vector<uint8_t> encryptedContent = encryptRSA(emptyContent, targetPubKey);

    std::vector<uint8_t> payload = Payload::createSendMessagePayload(targetID, MESSAGE_TYPE_REQUEST_SYM_KEY, encryptedContent);
    Request request(m_clientID.data(), REQUEST_CODE_SEND_MESSAGE, payload);
    sendAndReceive(request, RESPONSE_CODE_MESSAGE_STORED);

    std::cout << "Symmetric key request sent." << std::endl;
}

void MessagingClient::sendSymmetricKey() {
    const uint8_t* targetID = getTargetClientID("Enter username to send symmetric key to:", true);
    if (targetID == nullptr) return;

    const uint8_t* targetPubKey = m_clients.getPublicKeyByID(targetID);

    AESWrapper aes;
    std::vector<uint8_t> symKey(aes.getKey(), aes.getKey() + AESWrapper::DEFAULT_KEYLENGTH);

    std::vector<uint8_t> encryptedKey = encryptRSA(symKey, targetPubKey);

    std::vector<uint8_t> payload = Payload::createSendMessagePayload(targetID, MESSAGE_TYPE_SEND_SYM_KEY, encryptedKey);
    Request request(m_clientID.data(), REQUEST_CODE_SEND_MESSAGE, payload);
    sendAndReceive(request, RESPONSE_CODE_MESSAGE_STORED);

    m_clients.setSymmetricKeyForID(targetID, symKey);
    std::cout << "Symmetric key generated and sent." << std::endl;
}

void MessagingClient::sendMessage() {
    const uint8_t* targetID = getTargetClientID("Enter username to send a message to:", false, true);
    if (targetID == nullptr) return;

    const std::vector<uint8_t>* symKey = m_clients.getSymmetricKeyForID(targetID);

    std::cout << "Enter your message:" << std::endl;
    std::string message;
    std::getline(std::cin, message);

    std::vector<uint8_t> encryptedMessage = encryptAES(Utils::stringToBytes(message), *symKey);

    std::vector<uint8_t> payload = Payload::createSendMessagePayload(targetID, MESSAGE_TYPE_TEXT_MESSAGE, encryptedMessage);
    Request request(m_clientID.data(), REQUEST_CODE_SEND_MESSAGE, payload);
    sendAndReceive(request, RESPONSE_CODE_MESSAGE_STORED);

    std::cout << "Message sent." << std::endl;
}

void MessagingClient::sendFile() {
    const uint8_t* targetID = getTargetClientID("Enter username to send a file to:", false, true);
    if (targetID == nullptr) return;

    const std::vector<uint8_t>* symKey = m_clients.getSymmetricKeyForID(targetID);

    std::string filePathStr;
    std::cout << "Enter file path: " << std::endl;
    std::getline(std::cin, filePathStr);

    fs::path filePath(filePathStr);
    if (!fs::exists(filePath)) {
        std::cout << "file not found" << std::endl;
        return;
    }

    std::ifstream fileStream(filePath, std::ios::binary);
    std::vector<uint8_t> fileBytes(
        (std::istreambuf_iterator<char>(fileStream)),
        std::istreambuf_iterator<char>()
    );

    std::vector<uint8_t> encryptedFile = encryptAES(fileBytes, *symKey);

    std::vector<uint8_t> payload = Payload::createSendMessagePayload(targetID, MESSAGE_TYPE_FILE_MESSAGE, encryptedFile);
    Request request(m_clientID.data(), REQUEST_CODE_SEND_MESSAGE, payload);
    sendAndReceive(request, RESPONSE_CODE_MESSAGE_STORED);

    std::cout << "File sent." << std::endl;
}

void MessagingClient::requestIncomingMessages() {
    std::vector<uint8_t> emptyPayload;
    Request request(m_clientID.data(), REQUEST_CODE_PULL_MESSAGES, emptyPayload);
    Response response = sendAndReceive(request, RESPONSE_CODE_MESSAGE_LIST);

    const std::vector<Payload>& messages = response.getPayloads();
    if (messages.empty()) {
        std::cout << "No new messages." << std::endl;
        return;
    }

    for (const auto& msg : messages) {
        std::array<uint8_t, CLIENT_ID_SIZE> senderIDArray;
        std::copy(msg.getClientID(), msg.getClientID() + CLIENT_ID_SIZE, senderIDArray.begin());
        std::string senderName = m_clients.getOtherUsername(senderIDArray);

        uint8_t type = msg.getMessageType();
        std::vector<uint8_t> content = msg.getMessageContent();
        std::vector<uint8_t> decryptedBytes;

        try {
            if (type == MESSAGE_TYPE_REQUEST_SYM_KEY || type == MESSAGE_TYPE_SEND_SYM_KEY) {
                decryptedBytes = decryptRSA(content);
                if (type == MESSAGE_TYPE_SEND_SYM_KEY) {
                    m_clients.setSymmetricKeyForID(senderIDArray, decryptedBytes);
                    std::cout << "From: " << senderName << "\nContent:\nSymmetric key received.\n" << std::endl;
                }
                else {
                    std::cout << "From: " << senderName << "\nContent:\nRequest for symmetric key.\n" << std::endl;
                }
            }
            else if (type == MESSAGE_TYPE_TEXT_MESSAGE || type == MESSAGE_TYPE_FILE_MESSAGE) {
                const std::vector<uint8_t>* symKey = m_clients.getSymmetricKeyForID(senderIDArray);
                if (symKey == nullptr) throw std::runtime_error("No symmetric key for sender.");

                decryptedBytes = decryptAES(content, *symKey);

                if (type == MESSAGE_TYPE_TEXT_MESSAGE) {
                    std::cout << "From: " << senderName << "\nContent:\n" << Utils::bytesToString(decryptedBytes) << "\n" << std::endl;
                }
                else {
                    fs::path tempPath = fs::temp_directory_path() / ("received_" + senderName);
                    std::ofstream outFile(tempPath, std::ios::binary);
                    outFile.write(reinterpret_cast<const char*>(decryptedBytes.data()), decryptedBytes.size());
                    std::cout << "From: " << senderName << "\nContent:\nFile received and saved to: " << tempPath.string() << "\n" << std::endl;
                }
            }
        }
        catch (...) {
            std::cout << "From: " << senderName << "\nContent:\ncan't decrypt message\n" << std::endl;
        }
        std::cout << "-----<EOM>-----" << std::endl;
    }
}

// --- Crypto Helpers ---

std::vector<uint8_t> MessagingClient::encryptRSA(const std::vector<uint8_t>& plain, const uint8_t pubKey[PUBLIC_KEY_SIZE]) {
    std::string keyStr(reinterpret_cast<const char*>(pubKey), PUBLIC_KEY_SIZE);
    RSAPublicWrapper rsaPub(keyStr);
    std::string cipher = rsaPub.encrypt(reinterpret_cast<const char*>(plain.data()), plain.size());
    return Utils::stringToBytes(cipher);
}

std::vector<uint8_t> MessagingClient::decryptRSA(const std::vector<uint8_t>& cipher) {
    std::string cipherStr = Utils::bytesToString(cipher);
    std::string plain = m_rsaPrivate.decrypt(cipherStr);
    return Utils::stringToBytes(plain);
}

std::vector<uint8_t> MessagingClient::encryptAES(const std::vector<uint8_t>& plain, const std::vector<uint8_t>& key) {
    AESWrapper aes(key.data(), key.size());
    std::string cipher = aes.encrypt(reinterpret_cast<const char*>(plain.data()), plain.size());
    return Utils::stringToBytes(cipher);
}

std::vector<uint8_t> MessagingClient::decryptAES(const std::vector<uint8_t>& cipher, const std::vector<uint8_t>& key) {
    AESWrapper aes(key.data(), key.size());
    std::string plain = aes.decrypt(reinterpret_cast<const char*>(cipher.data()), cipher.size());
    return Utils::stringToBytes(plain);
}