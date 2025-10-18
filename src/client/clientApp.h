#pragma once
#include <string>

class ClientApp {
public:
    void run();

private:
    void printMenu() const;
    void handleInput(int choice);
};
