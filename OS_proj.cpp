#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <vector>

#include "ConsoleManager.h"
#include "MemoryManager.h"
#include "PagingAllocator.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include "CPUCore.h"


using namespace std;

struct functionHolder
{
    function<void()> func;
};

void headerPrint()
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


void initialize()
{
    cout << "initialize command recognized. Doing something.\n";
}

void screen()
{
    cout << "screen command recognized. Doing something.\n";
}

void schedulerTest()
{
    cout << "scheduler-test command recognized. Doing something.\n";
}

void schedulerStop()
{
    cout << "scheduler-stop command recognized. Doing something.\n";
}

void reportUtil()
{
    cout << "report-util command recognized. Doing something.\n";
}

void clear()
{
    cout << "\033[2J\033[1;1H";
    headerPrint();
}


int main()
{
    string input;
    int numCpu;
    string scheduler;
    int quantumCycles;
    int batchProcessFreq;
    int minInstructions;
    int maxInstructions;
    int delayPerExecution;
    int maxMemory;
    int memoryPerFrame;
    int minMemoryPerProcess;
    int maxMemoryPerProcess;

    // Vector to hold shared pointers to CPUCore objects
    std::vector<std::shared_ptr<CPUCore>> cpuCores;

    // Vector to hold CPUCore threads
    std::vector<std::thread> coreThreads;

    headerPrint();

    while (input != "initialize")
    {
        cout << "enter a command: ";
        cin >> input;
        if (input != "initialize")
            cout << "Unknown command.\n";
    }
    system("cls");

    // Open the config text file
    std::ifstream infile("config.txt");

    // Check if the file was successfully opened
    if (!infile) {
        std::cerr << "Unable to open config text file";
        return 1; // Return with an error code
    }
 
    std::string line;

    // Read the file line by line
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string key;

        // Extract the key (before the space) and then process based on the key
        if (line.find("num-cpu") != std::string::npos) {
            iss >> key >> numCpu;
        }
        else if (line.find("scheduler") != std::string::npos) {
            iss >> key >> scheduler;
            scheduler = scheduler.substr(1, scheduler.size() - 2); // Remove the quotes around the string
        }
        else if (line.find("quantum-cycles") != std::string::npos) {
            iss >> key >> quantumCycles;
        }
        else if (line.find("batch-process-freq") != std::string::npos) {
            iss >> key >> batchProcessFreq;
        }
        else if (line.find("min-ins") != std::string::npos) {
            iss >> key >> minInstructions;
        }
        else if (line.find("max-ins") != std::string::npos) {
            iss >> key >> maxInstructions;
        }
        else if (line.find("delay-per-exec") != std::string::npos) {
            iss >> key >> delayPerExecution;
        }
        else if (line.find("max-overall-mem") != std::string::npos) {
            iss >> key >> maxMemory;
        }
        else if (line.find("mem-per-frame") != std::string::npos) {
            iss >> key >> memoryPerFrame;
        }
        else if (line.find("min-mem-per-proc") != std::string::npos) {
            iss >> key >> minMemoryPerProcess;
        }
        else if (line.find("max-mem-per-proc") != std::string::npos) {
            iss >> key >> maxMemoryPerProcess;
        }
    }

    // Close the file
    infile.close();
    
    ConsoleManager::initialize();
    ConsoleManager::getInstance()->setNumRangeOfInstructions(minInstructions, maxInstructions);
    ConsoleManager::getInstance()->setBatchProcessFrequency(batchProcessFreq);
    ConsoleManager::getInstance()->setScheduler(scheduler);
    ConsoleManager::getInstance()->setMinMemoryPerProcess(minMemoryPerProcess);
    ConsoleManager::getInstance()->setMaxMemoryPerProcess(maxMemoryPerProcess);
    ConsoleManager::getInstance()->setMemPerFrame(memoryPerFrame);

    if (maxMemory == memoryPerFrame) {
        MemoryManager::initialize();
        MemoryManager::getInstance()->setMaxMemory(maxMemory);
        MemoryManager::getInstance()->setMemoryPerFrame(memoryPerFrame);
        ConsoleManager::getInstance()->setMemoryAllocator("flat");
        if (scheduler == "fcfs")
            MemoryManager::getInstance()->setScheduler("fcfs");
        else
            MemoryManager::getInstance()->setScheduler("rr");
    }
    else {
        PagingAllocator::initialize();
        PagingAllocator::getInstance()->setMaxMemory(maxMemory);
        PagingAllocator::getInstance()->setMemoryPerFrame(memoryPerFrame);
        PagingAllocator::getInstance()->populateFreeFrameList();
        ConsoleManager::getInstance()->setMemoryAllocator("paging");
        if (scheduler == "fcfs")
            PagingAllocator::getInstance()->setScheduler("fcfs");
        else
            PagingAllocator::getInstance()->setScheduler("rr");
    }
    if (scheduler == "fcfs") {
        FCFSScheduler::initialize();
        if (maxMemory == memoryPerFrame)
            FCFSScheduler::getInstance()->setMemoryAllocator("flat");
        else
            FCFSScheduler::getInstance()->setMemoryAllocator("paging");
    }
    else {
        RRScheduler::initialize();
        if (maxMemory == memoryPerFrame)
            RRScheduler::getInstance()->setMemoryAllocator("flat");
        else
            RRScheduler::getInstance()->setMemoryAllocator("paging");
    }

    // Create CPU cores dynamically based on numCPU and start threads
    for (int i = 0; i < numCpu; ++i) {
        // Create a new CPUCore and store it in the vector
        std::shared_ptr<CPUCore> core = std::make_shared<CPUCore>(i+1,delayPerExecution, quantumCycles, scheduler);
        cpuCores.push_back(core);

        // Create a new thread for each CPUCore::runCPU and store it in the vector
        coreThreads.emplace_back(&CPUCore::runCPU, core);
    }

    for (const auto& core : cpuCores) {
        if (scheduler == "fcfs")
            FCFSScheduler::getInstance()->addCPUCore(core);
        else 
            RRScheduler::getInstance()->addCPUCore(core);
    }
    if (scheduler == "fcfs") {
        std::thread fcfsThread(&FCFSScheduler::runFCFS, FCFSScheduler::getInstance());
        fcfsThread.detach();
    }
    else {
        std::thread rrThread(&RRScheduler::runRR, RRScheduler::getInstance());
        rrThread.detach();
    }
    
    bool running = true;
    while (running) 
    {
        ConsoleManager::getInstance()->process();
        ConsoleManager::getInstance()->drawConsole();
        if (ConsoleManager::getInstance()->getStartSchedulerTest()) {
            std::thread schedulerTestThread(&ConsoleManager::runSchedulerTest, ConsoleManager::getInstance());
            schedulerTestThread.detach();
        }
        running = ConsoleManager::getInstance()->isRunning();
    }
    if(scheduler == "fcfs")
        FCFSScheduler::getInstance()->stop();
    else 
        RRScheduler::getInstance()->stop();


    for (const auto& core : cpuCores) {
        core->stop();
    }

    for (std::thread& t : coreThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    ConsoleManager::destroy();
    if (maxMemory == memoryPerFrame)
        MemoryManager::destroy();
    else
        PagingAllocator::destroy();
    if (scheduler == "fcfs")
        FCFSScheduler::destroy();
    else
        RRScheduler::destroy();
    
    return 0;
}

