#pragma once
#include <string>

#define SERVER_INFO_FILE "server.info"
#define MY_INFO_FILE "my.info"

struct MyInfo {
	std::string username;
	std::string uuid;
	std::string privateKeyBase64;
};

class ClientConfig
{
public:

	static std::pair<std::string, int> loadServerInfo();

	static bool myInfoExists();

	static MyInfo loadMyInfo();

	static void saveMyInfo(const std::string& username, const std::string& uuid, const std::string& privateKeyBase64);
};