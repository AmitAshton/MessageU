#pragma once
#include <cstdint>
#include <string>

// Global Configurations
const std::string SERVER_INFO_FILENAME = "server.info";
const std::string CLIENT_INFO_FILENAME = "my.info";

// Protocol Sizes
constexpr size_t CLIENT_ID_SIZE = 16;
constexpr size_t PUBLIC_KEY_SIZE = 160;
constexpr size_t USERNAME_MAX_SIZE = 255;
constexpr uint8_t CLIENT_VERSION = 1;

// Fixed-size fields for message framing
constexpr size_t VERSION_SIZE = 1;
constexpr size_t REQUEST_CODE_SIZE = 2;
constexpr size_t RESPONSE_CODE_SIZE = 2;
constexpr size_t MESSAGE_ID_SIZE = 4;
constexpr size_t MESSAGE_TYPE_SIZE = 1;
constexpr size_t CONTENT_SIZE_FIELD = 4;

// Maximum allowed content size
constexpr uint32_t MAX_CONTENT_SIZE_ALLOWED = 0xFFFFFFFF;

// Request Codes (6xx)
constexpr uint16_t REQUEST_CODE_REGISTER = 600;
constexpr uint16_t REQUEST_CODE_CLIENT_LIST = 601;
constexpr uint16_t REQUEST_CODE_PUBLIC_KEY = 602;
constexpr uint16_t REQUEST_CODE_SEND_MESSAGE = 603;
constexpr uint16_t REQUEST_CODE_PULL_MESSAGES = 604;

// Response Codes (2xxx/9xxx)
constexpr uint16_t RESPONSE_CODE_REGISTER_SUCCESS = 2100;
constexpr uint16_t RESPONSE_CODE_CLIENT_LIST = 2101;
constexpr uint16_t RESPONSE_CODE_PUBLIC_KEY = 2102;
constexpr uint16_t RESPONSE_CODE_MESSAGE_STORED = 2103;
constexpr uint16_t RESPONSE_CODE_MESSAGE_LIST = 2104;
constexpr uint16_t RESPONSE_CODE_GENERAL_ERROR = 9000;

// Message Types (1-4)
constexpr uint8_t MESSAGE_TYPE_REQUEST_SYM_KEY = 1;
constexpr uint8_t MESSAGE_TYPE_SEND_SYM_KEY = 2;
constexpr uint8_t MESSAGE_TYPE_TEXT_MESSAGE = 3;
constexpr uint8_t MESSAGE_TYPE_FILE_MESSAGE = 4;
constexpr uint8_t MAX_MESSAGE_TYPE = MESSAGE_TYPE_FILE_MESSAGE;

// Menu Options (110-153)
constexpr int MENU_REGISTER = 110;
constexpr int MENU_REQUEST_CLIENTS = 120;
constexpr int MENU_REQUEST_PUBLIC_KEY = 130;
constexpr int MENU_REQUEST_WAITING = 140;
constexpr int MENU_SEND_TEXT = 150;
constexpr int MENU_SEND_REQUEST_SYMM_KEY = 151;
constexpr int MENU_SEND_SYMM_KEY = 152;
constexpr int MENU_SEND_FILE = 153;
constexpr int MENU_EXIT = 0;

// Default Data
const uint8_t DUMMY_CLIENT_ID[CLIENT_ID_SIZE] = { 0 };
const std::string SERVER_ERROR_MESSAGE = "server responded with an error";