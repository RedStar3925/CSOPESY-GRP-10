#include "CPUCore.h"
#include "MemoryManager.h"
#include <memory>
#include <thread>

CPUCore::CPUCore(int id, int delayPerExecution, int quantumSlice, std::string scheduler)
{
	this->id = id;
	this->delayPerExecution = delayPerExecution;
	this->quantumSlice = quantumSlice;
	this->scheduler = scheduler;
}

void CPUCore::runCPU()
{
	while (this->running)
	{
		if (this->process != nullptr && !this->finishedProcess && !this->preEmptedProcess) {
			if (this->scheduler == "rr" && this->cpuCycle >= this->quantumSlice) {
				this->preEmptedProcess = true;
				this->quantumCycle++;
				//MemoryManager::getInstance()->printMemory(this->id, this->quantumCycle);
			}
			else if (this->cpuCycle % (delayPerExecution + 1) == 0 && !this->process->isFinished() && !this->preEmptedProcess) { //check if the process is not yet finished and the delay is over
				this->process->executeCurrentCommand();
				this->process->moveToNextLine();
				this->activeCPUTicks++;
			}
			if (this->process != nullptr) {
				if (this->process->isFinished()) {
					this->finishedProcess = true;
				}
			}
			this->cpuCycle++;
		}
		else {
			this->idleCPUTicks++;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void CPUCore::registerProcess(std::shared_ptr<Process> process)
{
	this->process = process;
	process->setCoreId(this->id);
	process->setTimeStarted();
}

bool CPUCore::containsProcess() const
{
	return (this->process != nullptr);
}

std::shared_ptr<Process> CPUCore::getProcess()
{
	return this->process;
}

int CPUCore::getId()
{
	return this->id;
}

bool CPUCore::getIsFinished()
{
	return this->finishedProcess;
}

bool CPUCore::getIsPreEmpted()
{
	return this->preEmptedProcess;
}

void CPUCore::deallocateCPU()
{
	this->process = nullptr;
	this->finishedProcess = false;
	this->preEmptedProcess = false;
	this->cpuCycle = 0;
}

void CPUCore::stop()
{
	this->running = false;
}

int CPUCore::getIdleCPUTicks() const
{
	return this->idleCPUTicks;
}

int CPUCore::getActiveCPUTicks() const
{
	return this->activeCPUTicks;
}




