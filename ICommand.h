#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <iostream>

class ICommand
{
public:
	enum CommandType
	{
		PRINT,
		DECLARE,
		ADD,
		SUBTRACT,
		SLEEP,
		FOR,
		READ,
		WRITE
	};

	ICommand(int pid, CommandType commandType);
	virtual ~ICommand() = default;

	CommandType getCommandType() const;
	std::string extractKeyword(const std::string& input);
	std::string stripParens(const std::string& input);
	virtual void execute(int coreID, std::string processName,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) = 0;

	std::string getDateNow();

protected:
	int pid;
	CommandType commandType;
};

class PrintCommand : public ICommand {
	std::string message;
public:
	PrintCommand(int pid, const std::string& msg);
	void execute(int coreID, std::string processName,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class DeclareCommand : public ICommand {
	std::string var;
	uint16_t value;
public:
	DeclareCommand(int pid, const std::string& var, uint16_t value);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class AddCommand : public ICommand {
	std::string dest, op1, op2;
public:
	AddCommand(int pid, const std::string& dest, const std::string& op1, const std::string& op2);
	uint16_t resolveOperand(const std::string& operand, std::unordered_map<std::string, uint16_t>& vars);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class SubtractCommand : public ICommand {
	std::string dest, op1, op2;
public:
	SubtractCommand(int pid, const std::string& dest, const std::string& op1, const std::string& op2);
	uint16_t resolveOperand(const std::string& operand, std::unordered_map<std::string, uint16_t>& vars);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class SleepCommand : public ICommand {
	uint8_t ticks;
public:
	SleepCommand(int pid, uint8_t ticks);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class ForCommand : public ICommand {
	std::vector<std::shared_ptr<ICommand>> body;
	int repeats;
public:
	ForCommand(int pid, int repeats, const std::vector<std::shared_ptr<ICommand>>& body);
	void execute(int coreID, std::string processName,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class WriteCommand : public ICommand {
	uint16_t address;
	std::string source;
public:
	WriteCommand(int pid, uint16_t address, const std::string& source);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

class ReadCommand : public ICommand {
	std::string dest;
	uint16_t address;
public:
	ReadCommand(int pid, const std::string& dest, uint16_t address);
	void execute(int, std::string,
		std::unordered_map<std::string, uint16_t>& variables,
		std::ostream& screenOut,
		std::unordered_map<uint16_t, uint16_t>& memory) override;
};

std::shared_ptr<ICommand> createInstructionFromString(int pid, const std::string& raw);
