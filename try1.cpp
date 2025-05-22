#include <iostream>
#include <string>
#include <map>
#include <limits>
#include <algorithm> // For std::transform and std::tolower

void initializeCommand();
void screenCommand();
void schedulerTestCommand();
void schedulerStopCommand();
void reportUtilCommand();
void clearCommand();
void exitCommand();

std::map<std::string, void (*)()> commandHandlers;
std::map<std::string, void (*)(const std::string&)> parameterizedCommandHandlers;

void initializeCommandHandlers() {
    commandHandlers["initialize"] = initializeCommand;
    commandHandlers["screen"] = screenCommand;
    commandHandlers["scheduler-test"] = schedulerTestCommand;
    commandHandlers["scheduler-stop"] = schedulerStopCommand;
    commandHandlers["report-util"] = reportUtilCommand;
    commandHandlers["clear"] = clearCommand;
    commandHandlers["exit"] = exitCommand;
}

void initializeCommand() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
    exit(0);
}

void screenCommand() {
    std::cout << "Screen command recognized. Doing something." << std::endl;
    exit(0); 
}

void schedulerTestCommand() {
    std::cout << "Scheduler Test command recognized. Doing something." << std::endl;
    exit(0);
}

void schedulerStopCommand() {
    std::cout << "Scheduler Stop command recognized. Doing something." << std::endl;
    exit(0);
}

void reportUtilCommand() {
    std::cout << "Report Util command recognized. Doing something." << std::endl;
    exit(0);
}

void clearCommand() {
    std::cout << "Clear command recognized. Doing something." << std::endl;
    exit(0);
}

void exitCommand() {
    std::cout << "Exit command recognized. Now Quiting." << std::endl;
    exit(0);
}

void handleUnknownCommand(const std::string& command) {
    std::cout << "Unknown command: '" << command << "'. Type again." << std::endl;
}

void processCommand(const std::string& input) {
    if (input.empty()) {
        return; // Do nothing if input is empty
    }

    // Find the first space to separate command from arguments
    size_t firstSpace = input.find(' ');
    std::string command = input.substr(0, firstSpace);
    std::string arguments = "";

    if (firstSpace != std::string::npos) {
        arguments = input.substr(firstSpace + 1);
    }

    // Try to find in non-parameterized commands
    auto it = commandHandlers.find(command);
    if (it != commandHandlers.end()) {
        if (!arguments.empty()) {
            std::cout << "Warning: Command '" << command << "' does not take arguments. Arguments ignored." << std::endl;
        }
        it->second(); // Call the corresponding function
        return;
    }

    // Try to find in parameterized commands
    auto it_param = parameterizedCommandHandlers.find(command);
    if (it_param != parameterizedCommandHandlers.end()) {
        it_param->second(arguments); // Call the corresponding function with arguments
        return;
    }

    // If command not found in either map
    handleUnknownCommand(command);
}


int main() {
    initializeCommandHandlers();

    std::cout << R"(   ____ ____   ___  ____  _____ ______   __
  / ___/ ___| / _ \|  _ \| ____/ ___\ \ / /
 | |   \___ \| | | | |_) |  _| \___ \\ V / 
 | |___ ___) | |_| |  __/| |___ ___) || |  
  \____|____/ \___/|_|   |_____|____/ |_|  
                                           )" << std::endl;
    std::cout << "Hello, Welcome to CSOPESY commandline!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen." << std::endl;

    std::string userInput;
    while (true) {

        std::cout << "Enter a command: "; // Command prompt
        std::getline(std::cin, userInput); // Read the whole line of input

        processCommand(userInput);

    }

    return 0;
}
