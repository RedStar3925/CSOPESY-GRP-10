#include "ConsoleManager.h"
#include "MainConsole.h"
#include "ScreenConsole.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include "PagingAllocator.h"
#include "MemoryManager.h"
#include <memory>
#include <thread>
#include <random>

ConsoleManager* ConsoleManager::sharedInstance = nullptr;
AConsole::String input;

ConsoleManager::ConsoleManager()
{
	this->running = true;

	this->consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	const std::shared_ptr<MainConsole> mainConsole = std::make_shared<MainConsole>("MAIN_CONSOLE");

	this->consoleTable[MAIN_CONSOLE] = mainConsole;

	this->switchConsole(MAIN_CONSOLE);

	std::cout << "constructed" << std::endl;
}

std::vector<int> ConsoleManager::getPowersOfTwo(int min, int max)
{
	std::vector<int> powers;
	for (int i = 0; ; ++i) {
		int power = 1 << i; // Equivalent to pow(2, i)
		if (power > max) break; // Stop when the power exceeds the max
		if (power >= min) powers.push_back(power); // Include if within range
	}
	return powers;
}

int ConsoleManager::getRandomPowerOfTwo(int min, int max)
{
	std::vector<int> powers = getPowersOfTwo(min, max);
	if (powers.empty()) {
		throw std::out_of_range("No powers of 2 in the specified range.");
	}

	// Random number generator
	std::random_device rd;  // Seed generator
	std::mt19937 gen(rd()); // Mersenne Twister engine
	std::uniform_int_distribution<> dis(0, powers.size() - 1);

	return powers[dis(gen)];
}


ConsoleManager* ConsoleManager::getInstance()
{
	if(sharedInstance == nullptr)
	{
		sharedInstance = new ConsoleManager();
	}

	

	return sharedInstance;
}

void ConsoleManager::initialize()
{
	getInstance();
}

void ConsoleManager::destroy()
{
	if (sharedInstance != nullptr)
	{
		delete sharedInstance;
		sharedInstance = nullptr;
	}
}

void ConsoleManager::drawConsole()
{
	this->startSchedulerTest = false;

	this->currentConsole->display();
	if (currentConsole == consoleTable.at(MAIN_CONSOLE)) {
		std::shared_ptr<MainConsole> mainConsole = std::dynamic_pointer_cast<MainConsole>(currentConsole);
		if (mainConsole->hasExited()) {
			this->exitApplication();
		}
		if(!mainConsole->getStringToRegister().empty() && !mainConsole->getMemoryStr().empty() && !mainConsole->getCustomInstructions()) {
			std::cout << "first goes in here" << std::endl;
			//std::cout << mainConsole->getStringToRegister() << mainConsole->getMemoryStr() << mainConsole->getCustomInstructions() << std::endl;
			const std::shared_ptr<ScreenConsole> screenConsole = std::make_shared<ScreenConsole>(mainConsole->getStringToRegister(), this->minInstructions, this->maxInstructions, mainConsole->getProcessMemory(), this->memPerFrame);
			this->registerConsole(screenConsole);
		}
		if (!mainConsole->getStringToRegister().empty() && !mainConsole->getMemoryStr().empty() && mainConsole->getCustomInstructions()) {
			//int processMemory = getRandomPowerOfTwo(this->minMemoryPerProcess, this->maxMemoryPerProcess);
			std::cout << "second goes in here" << std::endl;
			const std::shared_ptr<ScreenConsole> screenConsole = std::make_shared<ScreenConsole>(mainConsole->getStringToRegister(), this->minInstructions, this->maxInstructions, mainConsole->getProcessMemory(), this->memPerFrame, mainConsole->getInstructions());
			this->registerConsole(screenConsole);
		}
		if (!mainConsole->getStringToRead().empty()) {
			this->switchConsole(mainConsole->getStringToRead());
		}
		if (mainConsole->getShowListOfProcesses()) {
			if (this->scheduler == "fcfs")
				FCFSScheduler::getInstance()->showListOfProcesses();
			else 
				RRScheduler::getInstance()->showListOfProcesses();
		}
		if (mainConsole->getPrintListOfProcesses()) {
			if (this->scheduler == "fcfs")
				FCFSScheduler::getInstance()->printListOfProcesses();
			else
				RRScheduler::getInstance()->printListOfProcesses();
		}
		if (mainConsole->getStartSchedulerTest()) {
			this->startSchedulerTest = true;
		}
		if (mainConsole->getStopSchedulerTest()) {
			this->endSchedulerTest();
		}
		if (mainConsole->getProcessSMI()) {
			if (this->memory_allocator == "flat")
			{
				MemoryManager::getInstance()->process_smi();
			}
			else {
				PagingAllocator::getInstance()->process_smi();
			}
		}
		if (mainConsole->getVMStat()) {
			if (this->memory_allocator == "flat") {
				MemoryManager::getInstance()->vmstat();
			}
			else {
				PagingAllocator::getInstance()->vmstat();
			}
		}
	}
	else {
		if (this->currentConsole->hasExited()) {
			this->returnToPreviousConsole();
		}
	}
}

void ConsoleManager::process() const
{
	this->currentConsole->process();
}

void ConsoleManager::switchConsole(AConsole::String consoleName)
{
	if(this->consoleTable.contains(consoleName))
	{
		std::cout << "\033[2J\033[1;1H";
		this->previousConsole = this->currentConsole;
		if (this->previousConsole != nullptr)
		{
			this->previousConsole->offEnabled();
		}
		this->currentConsole = this->consoleTable[consoleName];
	}
	else
	{
		std::cout << "Console not found" << std::endl;
	}
}

void ConsoleManager::returnToPreviousConsole()
{
	if (this->previousConsole != nullptr)
	{
		std::cout << "\033[2J\033[1;1H";
		this->currentConsole = this->previousConsole;
		this->previousConsole = nullptr;
	}
	else
	{
		std::cout << "No previous console" << std::endl;
	}
}

void ConsoleManager::exitApplication()
{
	this->running = false;
}

void ConsoleManager::registerConsole(std::shared_ptr<ScreenConsole> screenRef)
{
	if (this->consoleTable.contains(screenRef->getName()))
	{
		std::cerr << "Screen name " << screenRef->getName() << " already exists. Please use a different name." << std::endl;
		return;
	}
	this->consoleTable[screenRef->getName()] = screenRef;
	this->switchConsole(screenRef->getName());
	if (this->scheduler == "fcfs")
		FCFSScheduler::getInstance()->addProcess(screenRef->getProcess());
	else 
		RRScheduler::getInstance()->addProcess(screenRef->getProcess());
}

void ConsoleManager::unregisterConsole(std::string name)
{
	auto it = this->consoleTable.find(name);
	if (it != this->consoleTable.end())
	{
		this->consoleTable.erase(it);
	}
	else
	{
		std::cerr << "Console with name " << name << " does not exist." << std::endl;
	}
}

void ConsoleManager::registerConsoleForSchedulerTest(std::shared_ptr<ScreenConsole> screenRef)
{
	if (this->consoleTable.contains(screenRef->getName()))
	{
		std::cerr << "Screen name " << screenRef->getName() << " already exists. Please use a different name." << std::endl;
		return;
	}
	this->consoleTable[screenRef->getName()] = screenRef;
	if (this->scheduler == "fcfs")
		FCFSScheduler::getInstance()->addProcess(screenRef->getProcess());
	else
		RRScheduler::getInstance()->addProcess(screenRef->getProcess());
}

void ConsoleManager::setNumRangeOfInstructions(int minInstructions, int maxInstructions)
{
	this->minInstructions = minInstructions;
	this->maxInstructions = maxInstructions;
}

void ConsoleManager::setMinMemoryPerProcess(int mem_per_proc)
{
	this->minMemoryPerProcess = mem_per_proc;
}

void ConsoleManager::setMaxMemoryPerProcess(int mem_per_proc)
{
	this->maxMemoryPerProcess = mem_per_proc;
}

void ConsoleManager::setMemPerFrame(int mem_per_frame)
{
	this->memPerFrame = mem_per_frame;
}

void ConsoleManager::setBatchProcessFrequency(int batchProcessFreq)
{
	this->batchProcessFreq = batchProcessFreq;
}

void ConsoleManager::setScheduler(std::string scheduler)
{
	this->scheduler = scheduler;
}

void ConsoleManager::setMemoryAllocator(std::string allocator)
{
	this->memory_allocator = allocator;
}

void ConsoleManager::runSchedulerTest()
{
	this->schedulerTest = true;
	int processNameCounter = 1;
	int cpuCycle = 1;
	while (this->schedulerTest) {
		if (cpuCycle % batchProcessFreq == 0) {
			const std::string processName = "p" + std::to_string(processNameCounter);
			int processMemory = getRandomPowerOfTwo(this->minMemoryPerProcess, this->maxMemoryPerProcess);
			const std::shared_ptr<ScreenConsole> screenConsole = std::make_shared<ScreenConsole>(processName, this->minInstructions, this->maxInstructions, processMemory, this->memPerFrame);
			this->registerConsoleForSchedulerTest(screenConsole);
			processNameCounter++;
		}
		cpuCycle++;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void ConsoleManager::endSchedulerTest()
{
	this->schedulerTest = false;
}

bool ConsoleManager::isRunning() const
{
	return running;
}


bool ConsoleManager::getStartSchedulerTest()
{
	return this->startSchedulerTest;
}


HANDLE ConsoleManager::getConsole() const
{
	return consoleHandle;
}

void ConsoleManager::setCursorPosition(int posX, int posY) const
{
}