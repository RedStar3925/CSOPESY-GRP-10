#include "MainConsole.h"
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include "KeyboardHandler.h"
#include <sstream>
#include "FCFSScheduler.h"
#include <regex>

using namespace std;

MainConsole::MainConsole(String name) : AConsole(name)
{
	this->name = name;
}

void MainConsole::onEnabled()
{
    if (enabled == false)
    {
        clear();
    }
    enabled = true;

}

void MainConsole::offEnabled()
{
    this->enabled = false;
}

void MainConsole::display()
{
    this->stringToRead.clear();
    this->stringToRegister.clear();
	this->memoryStr.clear();
	this->processMemory = 0;
	this->instructions.clear();
    this->showListOfProcesses = false;
    this->printListOfProcesses = false;
    this->startSchedulerTest = false;
    this->stopSchedulerTest = false;
    this->process_smi = false;
    this->vmstat = false;
	this->customInstructions = false;
    string command, option, name;
    onEnabled();
    cout << "enter a command: ";
    getline(cin, command);
    if (command == "clear" || command == "cls") {
        clear();
    }
    else if (command == "exit") {
        exited = true;
    }
    else if (command == "screen -ls") {
        this->showListOfProcesses = true;
    }
    else if (command == "report-util") {
        this->printListOfProcesses = true;
    }
    else if (command.substr(0, 9) == "screen -r") {
        name = command.substr(10);

        if (!name.empty())
        {
            this->stringToRead = name;
        }
        else
        {
            cout << "Invalid command" << endl;
        }
    }
    else if (command.substr(0, 9) == "screen -s") {
		std::string args = command.substr(10);

        std::istringstream iss(args);

        iss >> name >> memoryStr;

        if (!name.empty() && !memoryStr.empty())
        {
            try {
                int memory = std::stoi(memoryStr);

                // Check if it's within range and a power of 2
                if (memory >= 64 && memory <= 65536 && (memory & (memory - 1)) == 0) {
                    this->stringToRegister = name;
                    this->processMemory = memory;
                }
                else {
                    std::cout << "Invalid memory allocation" << std::endl;
                }

            }
            catch (std::exception& e) {
                std::cout << "Invalid memory allocation" << std::endl;
            }
        }
        else
        {
            cout << "Invalid command" << endl;
        }
    }
    else if (command.substr(0, 9) == "screen -c")
    {

        // Parse screen -c <name> <memory> "<instructions>"
        size_t firstQuote = command.find('"');
        size_t lastQuote = command.rfind('"');

        if (firstQuote == std::string::npos || lastQuote == std::string::npos ||
            firstQuote == lastQuote) {
            std::cout << "Invalid command" << std::endl;
            return;
        }
		
        std::string beforeQuotes = command.substr(9, firstQuote - 9); // Skip "screen -c"
        std::istringstream iss(beforeQuotes);

        if (!(iss >> name >> memoryStr)) {
            std::cout << "Invalid command" << std::endl;
            return;
        }

        // Extract instructions from between quotes
        std::string instructionString = command.substr(firstQuote + 1, lastQuote - firstQuote - 1);


        int memory;
        try {
            memory = std::stoi(memoryStr);
        }
        catch (...) {
            std::cout << "Invalid memory allocation" << std::endl;
            return;
        }

        if (memory < 64 || memory > 65536 || (memory & (memory - 1)) != 0) {
            std::cout << "Invalid memory allocation" << std::endl;
            return;
        }

        // Parse instructions
        std::stringstream ss(instructionString);
        std::string instruction;
        this->instructions.clear();

        while (std::getline(ss, instruction, ';')) {
            // Trim whitespace
            instruction.erase(0, instruction.find_first_not_of(" \t\n\r"));
            instruction.erase(instruction.find_last_not_of(" \t\n\r") + 1);
            if (!instruction.empty()) {
                this->instructions.push_back(instruction);
            }
        }

        /*for (const auto& inst : instructions)
        {
            std::cout << inst << std::endl;
        }*/

        if (instructions.size() == 0 || instructions.size() > 50) {
            std::cout << "Invalid command" << std::endl;
            return;
        }


        this->stringToRegister = name;
        this->processMemory = memory;
        this->customInstructions = true;

        std::cout << processMemory << std::endl;

    }
    else if (command == "scheduler-start") {
        this->startSchedulerTest = true;
    }
    else if (command == "scheduler-stop") {
        this->stopSchedulerTest = true;
    }
    else if (command == "process-smi") {
        this->process_smi = true;
    }
    else if (command == "vmstat") {
        this->vmstat = true;
    }
    else {
        cout << "Command not found" << endl;
    }
}

void MainConsole::process()
{
	KeyboardHandler keyboardHandler;
    if (enabled) {
        if (_kbhit())
        {
            char key = _getch();
            char input[256];
           
            if (GetAsyncKeyState(key) & 0x8000)
            {
                keyboardHandler.OnKeyDown(key);
            }
            else
            {
                keyboardHandler.OnKeyUp(key);
            }

        }
    }
}

void MainConsole::clear()
{
    cout << "\033[2J\033[1;1H";
    headerPrint();
}

AConsole::String MainConsole::getStringToRegister()
{
    return this->stringToRegister;
}

AConsole::String MainConsole::getStringToRead()
{
    return this->stringToRead;
}

bool MainConsole::getShowListOfProcesses()
{
    return this->showListOfProcesses;
}

bool MainConsole::getPrintListOfProcesses()
{
    return this->printListOfProcesses;
}

bool MainConsole::getStartSchedulerTest()
{
    return this->startSchedulerTest;
}

bool MainConsole::getStopSchedulerTest()
{
    return this->stopSchedulerTest;
}

bool MainConsole::getProcessSMI()
{
    return this->process_smi;
}

bool MainConsole::getVMStat()
{
    return this->vmstat;
}

void MainConsole::headerPrint()
{
    std::cout << "  _____    _____    ____    _____   ______   _____   __     __\n";
    std::cout << " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n";
    std::cout << "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n";
    std::cout << "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n";
    std::cout << "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n";
    std::cout << " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";
    cout << "Hello, Welcome to CSOPESY commandline!\n";
    cout << "Type 'exit' to quit, 'clear' to clear the screen \n";

}

bool MainConsole::hasExited()
{
    return exited;
}
