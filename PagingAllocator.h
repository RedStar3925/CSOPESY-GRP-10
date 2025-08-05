#pragma once
#include <memory>
#include <Windows.h>
#include <iostream>
#include "ScreenConsole.h"
#include "Process.h"
#include <unordered_map>
#include <queue>
class PagingAllocator
{
public:
	static PagingAllocator* getInstance();
	static void initialize();
	static void destroy();

	void setMaxMemory(int max_mem);
	void setMemoryPerFrame(int mem_per_frame);
	void setScheduler(std::string scheduler);
	void populateFreeFrameList();
	bool allocate(std::shared_ptr<Process> process);
	void allocateFrames(int numFramesNeeded, std::string processName);
	void deallocate(std::shared_ptr<Process> process);
	void deallocateFrames(std::vector<int> frames);
	void removeProcFromProcInMemList(std::shared_ptr<Process> process);
	void process_smi();
	void vmstat();
	bool isProcessInMemory(std::shared_ptr<Process> process);
	std::shared_ptr<Process> getOldestProcessInMemory() const;

private:
	PagingAllocator();
	~PagingAllocator() = default;
	PagingAllocator(PagingAllocator const&) {}; //copy operator
	PagingAllocator& operator=(PagingAllocator const&) {}; //assignment operator
	static PagingAllocator* sharedInstance;
	std::unordered_map<int, std::string> frameMap;
	std::queue<int> freeFrameList;
	std::vector<std::shared_ptr<Process>> processInMemoryList;
	int max_mem;
	int mem_per_frame;
	std::string scheduler;
	int num_paged_in = 0;
	int num_paged_out = 0;
};