#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include <queue>
#include <iostream>
#include <unordered_map>
#include "Process.h"
#include "CPUCore.h"
class RRScheduler
{
public:
	typedef std::queue<std::shared_ptr<Process>> ReadyQueue;
	typedef std::vector<std::shared_ptr<CPUCore>> coreList;
	typedef std::vector< std::shared_ptr<Process>> FinishedList;
	void runRR();
	void addCPUCore(std::shared_ptr<CPUCore> cpuCore);
	void addProcess(std::shared_ptr<Process> process);
	void showListOfProcesses();
	void printListOfProcesses();
	void stop();
	int getCoresUsed() const;
	int getNoOfCores() const;
	std::unordered_map<std::string,int> getRunningProcesses() const;
	int getTotalIdleCPUTicks() const;
	int getTotalActiveCPUTicks() const;
	void putProcessToBackingStore(std::shared_ptr<Process> process);
	void returnProcessFromBackingStore();
	int getProcMemFromBackingStore() const;
	void setMemoryAllocator(std::string mem_allocator);
	static RRScheduler* getInstance();
	static void initialize();
	static void destroy();
private:
	RRScheduler();
	~RRScheduler() = default;
	RRScheduler(RRScheduler const&) {}; //copy operator
	RRScheduler& operator=(RRScheduler const&) {}; //assignment operator
	static RRScheduler* sharedInstance;

	ReadyQueue readyQueue;
	coreList cores;
	FinishedList finishedList;
	bool running = true;
	int coresUsed = 0;
	std::string memory_allocator;
	bool isBackingStoreEmpty = true;
};