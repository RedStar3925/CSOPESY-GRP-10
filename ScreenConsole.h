#pragma once
#include "AConsole.h"
#include "Process.h"
#include <memory>

class ScreenConsole : public AConsole
{
public:
	ScreenConsole(String name, int minInstructions, int maxInstructions, int memoryRequired, int memPerFrame);
	ScreenConsole(int pid, String name, int totalInstructions, int memoryRequired, int memPerFrame, int commandCounter);
	ScreenConsole(String name, int minInstructions, int maxInstructions, int memoryRequired, int memPerFrame, std::vector<std::string> customInstructions);

	void onEnabled() override;
	void offEnabled() override;
	void display() override;
	void process() override;
	bool hasExited() override;

	std::shared_ptr<Process> getProcess();

private:
	void printProcessData() const;
	bool enabled = false;
	bool exited = false;
	std::shared_ptr<Process> linkedProcess;
	String creationTime;       // String to hold the creation date and time

	void initializeCreationTime();
	int generateUniqueInt();
};

