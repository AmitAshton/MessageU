#include "ClientApp.h"
#include "TcpClient.h"
#include <iostream>
#include <limits>

#define NOMINMAX
#ifdef max
#undef max
#endif

void ClientApp::run() {
    int choice = -1;
    while (choice != 0) {
        printMenu();
        std::cout << "? ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        handleInput(choice);
    }
}

void ClientApp::printMenu() const {
    std::cout << "\nMessageU client at your service.\n";
    std::cout << "110) Register\n";
    std::cout << "120) Request for clients list\n";
    std::cout << "130) Request for public key\n";
    std::cout << "140) Request for waiting messages\n";
    std::cout << "150) Send a text message\n";
    std::cout << "151) Send a request for symmetric key\n";
    std::cout << "152) Send your symmetric key\n";
    std::cout << "153) Send a file (bonus)\n";
    std::cout << "999) Test connection\n";
    std::cout << "0) Exit client\n";
}

void ClientApp::handleInput(int choice) {
    switch (choice) {
    case 110: std::cout << "[Register user]\n"; break;
    case 120: std::cout << "[Request clients list]\n"; break;
    case 130: std::cout << "[Request public key]\n"; break;
    case 140: std::cout << "[Pull waiting messages]\n"; break;
    case 150: std::cout << "[Send text message]\n"; break;
    case 151: std::cout << "[Request symmetric key]\n"; break;
    case 152: std::cout << "[Send symmetric key]\n"; break;
    case 153: std::cout << "[Send file (bonus)]\n"; break;

    case 999: {
        std::cout << "[Testing connection...]\n";
        TcpClient tcp;
        if (tcp.connectToServer("127.0.0.1", 1234))
            tcp.disconnect();
        break;
    }

    case 0:
        std::cout << "Exiting client.\n";
        break;

    default:
        std::cout << "Invalid option.\n";
        break;
    }
}
