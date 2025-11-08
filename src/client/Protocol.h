#pragma once
#include <cstdint> // For uint8_t, uint16_t, uint32_t

// --- Client Configuration ---
const uint8_t CLIENT_VERSION = 1;

// --- Protocol Constants (from PDF) ---
const size_t CLIENT_NAME_SIZE = 255;
const size_t PUBLIC_KEY_SIZE = 160;
const size_t UUID_SIZE = 16;

/**
 * @enum RequestCode
 * @brief All request codes sent from Client to Server.
 * Values are taken from the project specification.
 */
enum class RequestCode : uint16_t
{
	REGISTER = 600,
	CLIENT_LIST = 601,
	PUBLIC_KEY = 602,
	SEND_MESSAGE = 603,
	PULL_MESSAGES = 604
};

/**
 * @enum MessageType
 * @brief Types of messages that can be sent (as payload).
 * Values are taken from the project specification.
 */
enum class MessageType : uint8_t
{
	REQUEST_SYM_KEY = 1,
	SEND_SYM_KEY = 2,
	TEXT_MESSAGE = 3,
	FILE_MESSAGE = 4
};