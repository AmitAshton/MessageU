#pragma once
#include <stdexcept>
#include <string>
#include "Constants.h"

class ServerError : public std::runtime_error {
public:
    ServerError() : std::runtime_error(SERVER_ERROR_MESSAGE) {}
    explicit ServerError(const std::string& msg) : std::runtime_error(msg) {}
};