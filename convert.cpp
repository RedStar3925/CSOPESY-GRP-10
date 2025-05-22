#include <iostream>
#include <string>
#include <cstdlib>

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    std::cout << "  _____    _____    ____    _____   ______   _____   __     __\n";
    std::cout << " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n";
    std::cout << "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n";
    std::cout << "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n";
    std::cout << "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n";
    std::cout << " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";
    std::cout << "Hello, Welcome to CSOPESY commandline!\n";
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen\n";
}

void handleCommand(const std::string& command) {
    if (command == "initialize") {
        std::cout << "Initialize command recognized. Doing something.\n";
    } else if (command == "screen") {
        std::cout << "Screen command recognized. Doing something.\n";
    } else if (command == "scheduler-test") {
        std::cout << "Scheduler-test command recognized. Doing something.\n";
    } else if (command == "scheduler-stop") {
        std::cout << "Scheduler-stop command recognized. Doing something.\n";
    } else if (command == "report-util") {
        std::cout << "Report-util command recognized. Doing something.\n";
    } else if (command == "clear") {
        clearScreen();
    } else if (command == "exit") {
        std::cout << "Exiting...\n";
        exit(0);
    } else {
        std::cout << "Command not recognized.\n";
    }
}

int main() {
    std::string command;

    clearScreen();

    while (true) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        handleCommand(command);
    }

    return 0;
}