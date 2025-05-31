#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
    #define CLEAR_COMMAND "cls"
#else
    #define CLEAR_COMMAND "clear"
#endif

class Screen {
public:
    std::string name;
    int id;
    int current_instruction = 0;
    int total_instructions = 50;
    std::string timestamp;

    Screen(const std::string& screenName, int screenId) {
        name = screenName;
        id = screenId;
        setTimestamp();
    }

    void display() const {
        clearScreen();
        std::cout << "Process: " << name << "\n"
                  << "ID: " << id << "\n"
                  << "Current instruction line: " << current_instruction << "/" << total_instructions << "\n"
                  << "Screen created on: " << timestamp << "\n";
    }

private:
    void setTimestamp() {
        std::time_t t = std::time(nullptr);
        char buffer[50];
        std::strftime(buffer, sizeof(buffer), "%m/%d/%Y, %I:%M:%S %p", std::localtime(&t));
        timestamp = buffer;
    }

    static void clearScreen() {
        std::system(CLEAR_COMMAND);
    }
};

class ScreenManager {
private:
    std::vector<Screen> screens;
    bool inMainMenu = true;
    const int MAX_SCREENS = 10;

public:
    void run() {
        displayMainMenu();
        std::string command;
        while (true) {
            std::cout << (inMainMenu ? "Enter a command: " : "root:\\> ");
            std::getline(std::cin, command);
            handleCommand(command);
        }
    }

private:
    void clearScreen() {
        std::system(CLEAR_COMMAND);
    }

    void displayMainMenu() {
        clearScreen();
        std::cout <<
        "  _____    _____    ____    _____   ______   _____   __     __\n"
        " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n"
        "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n"
        "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n"
        "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n"
        " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";

        std::cout << "Hello, Welcome to CSOPESY commandline!\n";
        std::cout << "Type 'exit' to return to the main menu, 'clear' to clear the screen\n";
        inMainMenu = true;
    }

    void createScreen(const std::string& name) {
        if (screens.size() >= MAX_SCREENS) {
            std::cout << "Maximum screen limit reached.\n";
            return;
        }

        screens.emplace_back(name, screens.size() + 1);
        screens.back().display();
        inMainMenu = false;
      
    }

    void resumeScreen(const std::string& name) {
        for (auto& screen : screens) {
            if (screen.name == name) {
                screen.display();
                inMainMenu = false;
                return;
            }
        }
        std::cout << "Screen '" << name << "' not found.\n";
        std::cout << (inMainMenu ? "Enter a command: " : "root:\\> ");
    }

    void handleMainMenuCommand(const std::string& command) {
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
            displayMainMenu();
        } else if (command == "exit") {
            std::cout << "Exiting program...\n";
            std::exit(0);
        } else {
            std::cout << "Command not recognized.\n";
        }
    }

    void handleCommand(const std::string& command) {
        if (inMainMenu) {
            if (command.rfind("screen -s ", 0) == 0) {
                createScreen(command.substr(10));
            } else if (command.rfind("screen -r ", 0) == 0) {
                resumeScreen(command.substr(10));
            } else {
                handleMainMenuCommand(command);
            }
        } else {
            if (command == "exit") {
                displayMainMenu();
            } else {
                std::cout << "Only the 'exit' command works in this mode.\n";
            }
        }
    }
};

int main() {
    ScreenManager manager;
    manager.run();
    return 0;
}
