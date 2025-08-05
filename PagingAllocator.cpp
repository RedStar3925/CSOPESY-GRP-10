#include "PagingAllocator.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"

PagingAllocator* PagingAllocator::sharedInstance = nullptr;

PagingAllocator::PagingAllocator()
{

}

PagingAllocator* PagingAllocator::getInstance()
{
	if (sharedInstance == nullptr)
	{
		sharedInstance = new PagingAllocator();
	}

	return sharedInstance;
}

void PagingAllocator::initialize()
{
	getInstance();
}

void PagingAllocator::destroy()
{
	if (sharedInstance != nullptr)
	{
		delete sharedInstance;
		sharedInstance = nullptr;
	}
}

void PagingAllocator::setMaxMemory(int max_mem)
{
	this->max_mem = max_mem;
}

void PagingAllocator::setMemoryPerFrame(int mem_per_frame)
{
	this->mem_per_frame = mem_per_frame;
}

void PagingAllocator::setScheduler(std::string scheduler)
{
	this->scheduler = scheduler;
}

void PagingAllocator::populateFreeFrameList()
{
	for (int i = 0; i < (this->max_mem / this->mem_per_frame); i++) {
		this->freeFrameList.push(i);
	}
}

bool PagingAllocator::allocate(std::shared_ptr<Process> process)
{
	std::string processName = process->getName();
	int numFramesNeeded = process->getNumberOfPages();
	if (numFramesNeeded > this->freeFrameList.size()) {
		return false;
	}
	allocateFrames(numFramesNeeded, processName);
	this->processInMemoryList.push_back(process);
	return true;
}

void PagingAllocator::allocateFrames(int numFramesNeeded, std::string processName)
{
	for (int i = 0; i < numFramesNeeded; i++) {
		this->frameMap[this->freeFrameList.front()] = processName;
		this->freeFrameList.pop();
	}
	this->num_paged_in += numFramesNeeded;
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process)
{
	std::string processName = process->getName();
	std::vector<int> frameIndexes;

	for (const auto& pair : this->frameMap) {
		if (pair.second == processName) {
			frameIndexes.push_back(pair.first);
		}
	}
	deallocateFrames(frameIndexes);
	this->removeProcFromProcInMemList(process);
}

void PagingAllocator::deallocateFrames(std::vector<int> frames)
{
	for (int frame : frames) {
		auto it = this->frameMap.find(frame);
		if (it != frameMap.end()) {
			this->frameMap.erase(it); // Remove the frame from the map if it exists
			this->freeFrameList.push(frame);
			this->num_paged_out += 1;
		}
	}
}

void PagingAllocator::removeProcFromProcInMemList(std::shared_ptr<Process> process)
{
	processInMemoryList.erase(
		std::remove_if(
			processInMemoryList.begin(),
			processInMemoryList.end(),
			[process](std::shared_ptr<Process> processToCompare) {
				return processToCompare == process; // Compare the shared_ptr directly
			}
		),
		processInMemoryList.end()
	);
}

void PagingAllocator::process_smi()
{
	float cpuUtilization;
	int memoryUsed = this->mem_per_frame * this->frameMap.size();
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

void PagingAllocator::vmstat()
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
	int memoryUsed = this->mem_per_frame * this->frameMap.size();
	std::cout << this->max_mem << " MiB total memory" << std::endl;
	std::cout << memoryUsed << " MiB used memory" << std::endl;
	std::cout << this->max_mem - memoryUsed << " MiB free memory" << std::endl;
	std::cout << totalIdleCPUTicks << " Idle CPU ticks" << std::endl;
	std::cout << totalActiveCPUTicks << " Active CPU ticks" << std::endl;
	std::cout << totalIdleCPUTicks + totalActiveCPUTicks << " Total CPU ticks" << std::endl;
	std::cout << this->num_paged_in << " pages paged in" << std::endl;
	std::cout << this->num_paged_out << " pages paged out" << std::endl;
}

bool PagingAllocator::isProcessInMemory(std::shared_ptr<Process> process)
{
	std::string processName = process->getName();
	for (const auto& pair : this->frameMap) {
		if (pair.second == processName) {
			return true; // Found the string
		}
	}
	return false;
}

std::shared_ptr<Process> PagingAllocator::getOldestProcessInMemory() const
{
	if (processInMemoryList.empty()) {
		return nullptr; // Return nullptr if the list is empty
	}
	return processInMemoryList[0]; // Return the first process otherwise
}
