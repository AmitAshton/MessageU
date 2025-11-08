#include "client.h"
#include "Utils.h"
#include "ServerError.h"
#include <conio.h> // for _getch()

std::string readAddressFromFile(const std::string& filename) {
    std::ifstream serverFile(filename);
    if (!serverFile.is_open()) {
        throw std::ios_base::failure("Unable to open " + filename);
    }

    std::string line;
    while (std::getline(serverFile, line)) {
        line = Utils::trim(line);
        if (!line.empty()) {
            serverFile.close();
            return line;
        }
    }

    serverFile.close();
    throw std::ios_base::failure("No valid address found in " + filename);
}

bool handleClientOption(MessagingClient& client, int code) {
    auto runAndReport = [&](const std::function<void()>& fn) {
        try {
            fn();
        }
        catch (const ServerError& se) {
            std::cerr << SERVER_ERROR_MESSAGE << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        }
        };

    switch (code) {
    case MENU_REGISTER:
        runAndReport([&] { client.registerClient(); });
        break;
    case MENU_REQUEST_CLIENTS:
        runAndReport([&] { client.requestClientList(); });
        break;
    case MENU_REQUEST_PUBLIC_KEY:
        runAndReport([&] { client.requestPublicKey(); });
        break;
    case MENU_REQUEST_WAITING:
        runAndReport([&] { client.requestIncomingMessages(); });
        break;
    case MENU_SEND_TEXT:
        runAndReport([&] { client.sendMessage(); });
        break;
    case MENU_SEND_REQUEST_SYMM_KEY:
        runAndReport([&] { client.requestSymmetricKey(); });
        break;
    case MENU_SEND_SYMM_KEY:
        runAndReport([&] { client.sendSymmetricKey(); });
        break;
    case MENU_SEND_FILE:
        runAndReport([&] { client.sendFile(); });
        break;
    case MENU_EXIT:
        return false;
    default:
        std::cout << "Invalid option code." << std::endl;
    }

    return true;
}

int main() {
    try {
        std::string serverAddress = readAddressFromFile(SERVER_INFO_FILENAME);
        MessagingClient client(serverAddress);

        std::string input;
        std::cout << OPTIONS_MENU << std::endl;
        while (true) {
            try {
                std::cout << "> ";
                std::getline(std::cin, input);
                input = Utils::trim(input);
                if (input.empty()) continue;

                int code = std::stoi(input);

                if (!handleClientOption(client, code))
                    break;

            }
            catch (const std::invalid_argument&) {
                std::cout << "Please enter a valid number.\n" << std::endl;
                continue;
            }

            std::cout << "Press any key to continue...";
            _getch();

            std::cout << "\n\n" << OPTIONS_MENU << std::endl;

        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        std::cout << "Press any key to exit...";
        _getch();
        return 1;
    }

    return 0;
}