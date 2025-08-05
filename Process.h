#pragma once
#include <memory>
#include <string>
#include "ICommand.h"
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

class Process
{
public:
	Process(int pid, std::string name, int totalInstructions, int memoryRequired, int memPerFrame, int commandCounter, std::vector<std::string> customInstructions = {});
	~Process() = default;

	enum ProcessState
	{
		READY,
		RUNNING,
		WAITING,
		FINISHED
	};

	void addCommand(ICommand::CommandType command);
	void executeCurrentCommand();
	void moveToNextLine();
	void setCoreId(int coreID);
	bool isFinished() const;
	std::string getDateNow();
	std::string getTimeFinished();
	std::string getTimeStarted();
	int getProcessID() const;
	void setTimeStarted();
	int getCommandCounter() const;
	int getTotalInstructions() const;
	int getTotalMemoryRequired() const;
	int getNumberOfPages() const;
	int getMemPerFrame() const;
	void setInBackingStore(bool value);
	bool getInBackingStore() const;
	std::string getScreenOutput() const {
		return screenOutput.str();
	}
	std::string getName();
	std::string toHex(uint16_t num) const;
private:
	std::string name;
	int pid;
	typedef std::vector<std::shared_ptr<ICommand>> CommandList;
	CommandList commandList;
	std::vector<std::string> customInstructions;
	int commandCounter = 0;
	int totalInstructions;
	int cpuCoreId = -1;
	std::string timeFinished;
	std::string timeStarted;
	ProcessState currentState = ProcessState::READY;
	int memoryRequired;
	int memPerFrame;
	bool inBackingStore = false;
	std::unordered_map<std::string, uint16_t> variables;
	std::unordered_map<uint16_t, uint16_t> memory;
	std::ostringstream screenOutput;
};

