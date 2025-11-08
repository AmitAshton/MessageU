#include "ClientConfig.h"
#include <fstream>     
#include <sstream>     
#include <stdexcept>
#include <sys/stat.h>  


std::pair<std::string, int> ClientConfig::loadServerInfo()
{
	std::ifstream file(SERVER_INFO_FILE);
	if (!file.is_open()) {
		throw std::runtime_error("Error: server.info file not found or could not be opened.");
	}

	std::string line;
	if (std::getline(file, line))
	{
		std::size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			file.close();
			throw std::runtime_error("Error: Invalid format in server.info. Expected 'host:port'.");
		}

		try {
			std::string host = line.substr(0, colonPos);
			int port = std::stoi(line.substr(colonPos + 1));
			file.close();
			return { host, port };
		}
		catch (const std::exception&) {
			file.close();
			throw std::runtime_error("Error: Failed to parse port in server.info. Port must be a number.");
		}
	}

	file.close();
	throw std::runtime_error("Error: server.info file is empty.");
}



bool ClientConfig::myInfoExists()
{
	struct stat buffer;
	return (stat(MY_INFO_FILE, &buffer) == 0);
}

MyInfo ClientConfig::loadMyInfo()
{
	if (!myInfoExists()) {
		throw std::runtime_error("Error: my.info file not found.");
	}

	std::ifstream file(MY_INFO_FILE);
	if (!file.is_open()) {
		throw std::runtime_error("Error: Could not open my.info for reading.");
	}

	MyInfo info;

	if (!std::getline(file, info.username)) {
		file.close();
		throw std::runtime_error("Error: my.info file is corrupt (cannot read username).");
	}

	if (!std::getline(file, info.uuid)) {
		file.close();
		throw std::runtime_error("Error: my.info file is corrupt (cannot read uuid).");
	}

	std::stringstream keyStream;
	keyStream << file.rdbuf();
	info.privateKeyBase64 = keyStream.str();

	file.close();

	if (info.privateKeyBase64.empty()) {
		throw std::runtime_error("Error: my.info file is corrupt (private key is empty).");
	}

	size_t first = info.privateKeyBase64.find_first_not_of(" \t\n\r");
	if (std::string::npos == first)
	{
		throw std::runtime_error("Error: my.info file is corrupt (private key is empty).");
	}
	size_t last = info.privateKeyBase64.find_last_not_of(" \t\n\r");
	info.privateKeyBase64 = info.privateKeyBase64.substr(first, (last - first + 1));


	return info;
}

void ClientConfig::saveMyInfo(const std::string& username, const std::string& uuid, const std::string& privateKeyBase64)
{
	std::ofstream file(MY_INFO_FILE);
	if (!file.is_open()) {
		throw std::runtime_error("Error: Could not open my.info for writing.");
	}

	file << username << std::endl;
	file << uuid << std::endl;
	file << privateKeyBase64;

	file.close();
}