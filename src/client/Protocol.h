#pragma once
#include <cstdint>

const uint8_t CLIENT_VERSION = 1;

const size_t CLIENT_NAME_SIZE = 255;
const size_t PUBLIC_KEY_SIZE = 160;
const size_t UUID_SIZE = 16;

enum class RequestCode : uint16_t
{
	REGISTER = 600,
	CLIENT_LIST = 601,
	PUBLIC_KEY = 602,
	SEND_MESSAGE = 603,
	PULL_MESSAGES = 604
};

enum class MessageType : uint8_t
{
	REQUEST_SYM_KEY = 1,
	SEND_SYM_KEY = 2,
	TEXT_MESSAGE = 3,
	FILE_MESSAGE = 4
};