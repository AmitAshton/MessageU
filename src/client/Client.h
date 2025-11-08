#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <conio.h>
#include <functional>

// Include the definitions for the logic and constants
#include "Constants.h"
#include "MessagingClient.h" 
#include "Utils.h"

// This file only defines the menu string for client.cpp
static const std::string OPTIONS_MENU =
"MessageU client at your service.\n\n"
"110) Register\n"
"120) Request for clients list\n"
"130) Request for public key\n"
"140) Request for waiting messages\n"
"150) Send text message\n"
"151) Send a request for symmetric key\n"
"152) Send your symmetric key\n"
"153) Send a file\n"
"0) Exit client\n"
"?";