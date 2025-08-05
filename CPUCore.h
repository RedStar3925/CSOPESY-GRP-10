#pragma once
#include "Process.h"
#include "CPUCore.h"
#include <memory>
class CPUCore
{
public:
	CPUCore(int id, int delayPerExecution, int quantumSlice, std::string scheduler);
	~CPUCore() = default;

	void runCPU();
	void registerProcess(std::shared_ptr<Process> process);
	bool containsProcess() const;
	std::shared_ptr<Process> getProcess();
	int getId();
	bool getIsFinished();
	bool getIsPreEmpted();
	void deallocateCPU();
	void stop();
	int getIdleCPUTicks() const;
	int getActiveCPUTicks() const;
private:
	int id;
	std::shared_ptr<Process> process = nullptr;
	bool finishedProcess = false;
	bool running = true;
	int cpuCycle = 0;
	int delayPerExecution;
	std::string scheduler;
	int quantumSlice;
	bool preEmptedProcess = false;
	int quantumCycle = 0;
	int idleCPUTicks = 0;
	int activeCPUTicks = 0;
};

