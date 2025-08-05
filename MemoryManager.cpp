#include "MemoryManager.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <unordered_map>

MemoryManager* MemoryManager::sharedInstance = nullptr;

MemoryManager::MemoryManager()
{

}

MemoryManager* MemoryManager::getInstance()
{
	if (sharedInstance == nullptr)
	{
		sharedInstance = new MemoryManager();
	}

	return sharedInstance;
}

void MemoryManager::initialize()
{
	getInstance();
}

void MemoryManager::destroy()
{
	if (sharedInstance != nullptr)
	{
		delete sharedInstance;
		sharedInstance = nullptr;
	}
}

void MemoryManager::setMaxMemory(int max_mem)
{
	this->max_mem = max_mem;
}

void MemoryManager::setMemoryPerFrame(int mem_per_frame)
{
	this->mem_per_frame = mem_per_frame;
}

void MemoryManager::setScheduler(std::string scheduler)
{
	this->scheduler = scheduler;
}

int MemoryManager::findMemory(std::shared_ptr<Process> process)
{
	if (processInMemoryList.empty()) {
		if (this->max_mem >= process->getTotalMemoryRequired())
			return 0;
		else
			return -1;
	}
	else {
		if (processInMemoryList[0].memoryStart >= process->getTotalMemoryRequired())
			return 0;
		for (size_t i = 0; i < processInMemoryList.size(); ++i) {
			if (i + 1 == processInMemoryList.size()) {
				if (this->max_mem - processInMemoryList[i].memoryEnd >= process->getTotalMemoryRequired())
					return processInMemoryList[i].memoryEnd;
				else
					return -1;
			}
			if (processInMemoryList[i + 1].memoryStart - processInMemoryList[i].memoryEnd >= process->getTotalMemoryRequired())
				return processInMemoryList[i].memoryEnd;
		}
	}
}

void MemoryManager::addProcessToMemory(std::shared_ptr<Process> process, int startMemoryAddress)
{
	const int endMemoryAddress = startMemoryAddress + process->getTotalMemoryRequired();
	this->processInMemoryList.push_back({ process->getName(), startMemoryAddress, endMemoryAddress });
	this->processesInMemory.push_back(process);
	std::sort(processInMemoryList.begin(), processInMemoryList.end(), [](const ProcessWithMemory& a, const ProcessWithMemory& b) {
		return a.memoryStart < b.memoryStart;
		});
}

bool MemoryManager::isProcessInMemory(std::shared_ptr<Process> process)
{
	
	// Use std::find_if with a lambda function to search by name
	auto it = std::find_if(processInMemoryList.begin(), processInMemoryList.end(), [&process](const ProcessWithMemory& processWithMemory) {
		return processWithMemory.processName == process->getName();
		});

	// Return true if found, otherwise false
	return it != processInMemoryList.end();
}

void MemoryManager::deallocateProcessFromMemory(std::shared_ptr<Process> process)
{
	processInMemoryList.erase(std::remove_if(processInMemoryList.begin(), processInMemoryList.end(),
		[&process](const ProcessWithMemory& processWithMemory) {
			return processWithMemory.processName == process->getName();
		}),
		processInMemoryList.end());

	this->removeProcFromProcInMemList(process);
}

int MemoryManager::getTotalExternalFragmentation() const
{
	if (processInMemoryList.empty()) {
		return this->max_mem;
	}
	else {
		int totalFragmentation = processInMemoryList[0].memoryStart;
		for (size_t i = 0; i < processInMemoryList.size(); ++i) {
			if (i + 1 == processInMemoryList.size()) {
				totalFragmentation += (this->max_mem - processInMemoryList[i].memoryEnd);
				return totalFragmentation;
			}
			totalFragmentation += (processInMemoryList[i + 1].memoryStart - processInMemoryList[i].memoryEnd);
		}
	}
}

void MemoryManager::printMemory(int cpuNo, int quantumCycle)
{
	const std::string filename = "cpu" + std::to_string(cpuNo) + "_memory_stamp_" + std::to_string(quantumCycle) + ".txt";
	std::ofstream outfile(filename);
	if (outfile.is_open()) {
		outfile << "Timestamp: (" << this->getDateNow() << ")" << "\n";
		outfile << "Number of processes in memory: " << this->processInMemoryList.size() << "\n";
		outfile << "Total external fragmentation in KB: " << this->getTotalExternalFragmentation() << "\n";
		outfile << "\n";
		outfile << "----end---- = " << this->max_mem << "\n";
		outfile << "\n";
		for (int i = this->processInMemoryList.size() - 1; i >= 0; --i) {
			outfile << this->processInMemoryList[i].memoryEnd << "\n";
			outfile << this->processInMemoryList[i].processName << "\n";
			outfile << this->processInMemoryList[i].memoryStart << "\n";
			outfile << "\n";
		}
		outfile << "----start---- = 0" << "\n";
		outfile.close();
	}
}

void MemoryManager::process_smi()
{
	float cpuUtilization;
	int memoryUsed = this->max_mem - this->getTotalExternalFragmentation();
	float memoryUtilization = (memoryUsed / static_cast<float>(this->max_mem)) * 100;
	std::unordered_map<std::string, int> runningProcesses;
	if (this->scheduler == "fcfs") {
		cpuUtilization = (FCFSScheduler::getInstance()->getCoresUsed() / static_cast<float>(FCFSScheduler::getInstance()->getNoOfCores())) * 100;
		runningProcesses = FCFSScheduler::getInstance()->getRunningProcesses();
	}
	else {
		cpuUtilization = (RRScheduler::getInstance()->getCoresUsed() / static_cast<float>(RRScheduler::getInstance()->getNoOfCores())) * 100;
		runningProcesses = RRScheduler::getInstance()->getRunningProcesses();
	}
	for (int i = 0; i < 15; ++i) {
		std::cout << "-";
	}
	std::cout << std::endl;
	std::cout << "CPU-Util: " << cpuUtilization << "%" << std::endl;
	std::cout << "Memory Usage: " << memoryUsed << "MiB / " << this->max_mem << "MiB" << std::endl;
	std::cout << "Memory Util: " << memoryUtilization << "%" << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < 15; ++i) {
		std::cout << "=";
	}
	std::cout << std::endl;
	std::cout << "Running processes and memory usage:" << std::endl;
	for (int i = 0; i < 15; ++i) {
		std::cout << "-";
	}
	std::cout << std::endl;
	for (const auto& pair : runningProcesses) {
		std::cout << pair.first << " " << pair.second << "MiB" << "\n";
	}
	for (int i = 0; i < 15; ++i) {
		std::cout << "-";
	}
	std::cout << std::endl;
}

void MemoryManager::vmstat()
{
	int totalIdleCPUTicks;
	int totalActiveCPUTicks;
	if (this->scheduler == "fcfs") {
		totalIdleCPUTicks = FCFSScheduler::getInstance()->getTotalIdleCPUTicks();
		totalActiveCPUTicks = FCFSScheduler::getInstance()->getTotalActiveCPUTicks();
	}
	else {
		totalIdleCPUTicks = RRScheduler::getInstance()->getTotalIdleCPUTicks();
		totalActiveCPUTicks = RRScheduler::getInstance()->getTotalActiveCPUTicks();
	}
	int memoryUsed = this->max_mem - this->getTotalExternalFragmentation();
	std::cout << this->max_mem << " MiB total memory" << std::endl;
	std::cout << memoryUsed << " MiB used memory" << std::endl;
	std::cout << this->max_mem - memoryUsed << " MiB free memory" << std::endl;
	std::cout << totalIdleCPUTicks << " Idle CPU ticks" << std::endl;
	std::cout << totalActiveCPUTicks << " Active CPU ticks" << std::endl;
	std::cout << totalIdleCPUTicks + totalActiveCPUTicks << " Total CPU ticks" << std::endl;
	std::cout << "0 pages paged in" << std::endl;
	std::cout << "0 pages paged out" << std::endl;
}

std::string MemoryManager::getDateNow()
{
	// Get current time
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);

	// Convert to local time and format it
	std::tm local_tm;

	localtime_s(&local_tm, &now_c);

	// Use a string stream to format the date and time
	std::ostringstream oss;
	oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");

	// Store formatted time in creationTime
	return oss.str();
}

void MemoryManager::removeProcFromProcInMemList(std::shared_ptr<Process> process)
{
	processesInMemory.erase(
		std::remove_if(
			processesInMemory.begin(),
			processesInMemory.end(),
			[process](std::shared_ptr<Process> processToCompare) {
				return processToCompare == process; // Compare the shared_ptr directly
			}
		),
		processesInMemory.end()
	);
}

std::shared_ptr<Process> MemoryManager::getOldestProcessInMemory() const
{
	return processesInMemory[0];
}
