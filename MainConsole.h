#pragma once
#include "AConsole.h"
#include <vector>
#include <string>

class MainConsole : public AConsole
{
public:
	MainConsole(String name);

	void onEnabled() override;
	void offEnabled() override;
	void display() override;
	void process() override;
	bool hasExited() override;

	void clear();
	AConsole::String getStringToRegister();
	AConsole::String getStringToRead();
	bool getShowListOfProcesses();
	bool getPrintListOfProcesses();
	bool getStartSchedulerTest();
	bool getStopSchedulerTest();
	bool getProcessSMI();
	bool getVMStat();
	bool getCustomInstructions() { return this->customInstructions; }
	std::vector<std::string> getInstructions() const { return this->instructions; }
	int getProcessMemory() const { return this->processMemory; }
	std::string getMemoryStr() const { return this->memoryStr; }

	
private:
	void headerPrint();
	bool enabled = false;
	bool exited = false;
	bool showListOfProcesses = false;
	bool printListOfProcesses = false;
	bool startSchedulerTest = false;
	bool stopSchedulerTest = false;
	bool process_smi = false;
	bool vmstat = false;
	bool customInstructions = false;
	std::vector<std::string> instructions = {};
	AConsole::String stringToRegister = "";
	AConsole::String stringToRead = "";
	std::string memoryStr = "";
	int processMemory = 0;
};

