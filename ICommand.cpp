#include "ICommand.h"
#include <chrono>
#include <stdexcept>
#include <algorithm>

// Base
ICommand::ICommand(int pid, CommandType commandType) {
	this->pid = pid;
	this->commandType = commandType;
}

ICommand::CommandType ICommand::getCommandType() const {
	return this->commandType;
}

std::string ICommand::getDateNow() {
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm;
	localtime_s(&local_tm, &now_c);
	std::ostringstream oss;
	oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
	return oss.str();
}

// --- PRINT
PrintCommand::PrintCommand(int pid, const std::string& msg)
	: ICommand(pid, PRINT), message(msg) {
}

void PrintCommand::execute(int coreID, std::string processName,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream& screenOut,
	std::unordered_map<uint16_t, uint16_t>&) {
	std::string output = message;
	// Optional: Remove surrounding quotes
	if (!output.empty() && output.front() == '"' && output.back() == '"') {
		output = output.substr(1, output.size() - 2);
	}

	// Replace both "+var" and "+ var" patterns
	for (const auto& [key, value] : vars) {
		std::string token1 = "+" + key;           // e.g., +varC
		std::string token2 = "+ " + key;          // e.g., + varC
		std::string replacement = std::to_string(value);

		// Replace "+ var"
		size_t pos = 0;
		while ((pos = output.find(token2, pos)) != std::string::npos) {
			output.replace(pos, token2.length(), replacement);
			pos += replacement.length();
		}

		// Replace "+var"
		pos = 0;
		while ((pos = output.find(token1, pos)) != std::string::npos) {
			output.replace(pos, token1.length(), replacement);
			pos += replacement.length();
		}
	}

	// Fallback message
	if (output.empty()) {
		output = "Hello world from " + processName + "!";
	}

	// Output with timestamp
	std::string timestamp = getDateNow();
	std::cout << output << std::endl;
	screenOut << timestamp << " Core:" << coreID << " " << output << std::endl;
}

// --- DECLARE
DeclareCommand::DeclareCommand(int pid, const std::string& var, uint16_t value)
	: ICommand(pid, DECLARE), var(var), value(value) {
}

void DeclareCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>&) {
	vars[var] = value;
}

// --- ADD
AddCommand::AddCommand(int pid, const std::string& dest, const std::string& op1, const std::string& op2)
	: ICommand(pid, ADD), dest(dest), op1(op1), op2(op2) {
}

uint16_t AddCommand::resolveOperand(const std::string& operand, std::unordered_map<std::string, uint16_t>& vars) {
	// Try to parse the operand as a number
	try {
		return static_cast<uint16_t>(std::stoi(operand));
	}
	catch (...) {
		// Not a number — treat as a variable
		if (vars.find(operand) == vars.end()) {
			vars[operand] = 0;  // Auto-declare with default 0
		}
		return vars[operand];
	}
}

void AddCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>&) {
	uint16_t v1 = resolveOperand(op1, vars);
	uint16_t v2 = resolveOperand(op2, vars);
	vars[dest] = std::min<uint32_t>(v1 + v2, 65535);
}

// --- SUBTRACT
SubtractCommand::SubtractCommand(int pid, const std::string& dest, const std::string& op1, const std::string& op2)
	: ICommand(pid, SUBTRACT), dest(dest), op1(op1), op2(op2) {
}

uint16_t SubtractCommand::resolveOperand(const std::string& operand, std::unordered_map<std::string, uint16_t>& vars) {
	// Try to parse the operand as a number
	try {
		return static_cast<uint16_t>(std::stoi(operand));
	}
	catch (...) {
		// Not a number — treat as a variable
		if (vars.find(operand) == vars.end()) {
			vars[operand] = 0;  // Auto-declare with default 0
		}
		return vars[operand];
	}
}

void SubtractCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>&) {
	uint16_t v1 = resolveOperand(op1, vars);
	uint16_t v2 = resolveOperand(op2, vars);
	vars[dest] = (v1 > v2) ? static_cast<uint16_t>(v1 - v2) : 0;
}

// --- SLEEP
SleepCommand::SleepCommand(int pid, uint8_t ticks)
	: ICommand(pid, SLEEP), ticks(ticks) {
}

void SleepCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>&,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>&) {
	// Placeholder: handled by the scheduler
}

// --- FOR
ForCommand::ForCommand(int pid, int repeats, const std::vector<std::shared_ptr<ICommand>>& body)
	: ICommand(pid, FOR), body(body), repeats(repeats) {
}

void ForCommand::execute(int coreID, std::string processName,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream& screenOut,
	std::unordered_map<uint16_t, uint16_t>& memory) {
	for (int i = 0; i < repeats; ++i) {
		for (const auto& cmd : body) {
			cmd->execute(coreID, processName, vars, screenOut, memory);
		}
	}
}

// --- WRITE
WriteCommand::WriteCommand(int pid, uint16_t address, const std::string& source)
	: ICommand(pid, WRITE), address(address), source(source) {
}

void WriteCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>& memory) {
	uint16_t value;
	if (vars.count(source)) {
		value = vars[source];
	}
	else {
		try {
			value = static_cast<uint16_t>(std::stoi(source));
		}
		catch (...) {
			value = 0; // Default value if parsing fails
		}
	}
	memory[address] = value;
}

// --- READ
ReadCommand::ReadCommand(int pid, const std::string& dest, uint16_t address)
	: ICommand(pid, READ), dest(dest), address(address) {
}

void ReadCommand::execute(int, std::string,
	std::unordered_map<std::string, uint16_t>& vars,
	std::ostream&,
	std::unordered_map<uint16_t, uint16_t>& memory) {
	vars[dest] = memory[address];
}

std::string extractKeyword(const std::string& raw) {
	std::string keyword;
	for (char ch : raw) {
		if (std::isspace(ch) || ch == '(') break;
		keyword += ch;
	}
	return keyword;
}

std::string stripParens(const std::string& input) {
	size_t start = input.find('(');
	size_t end = input.rfind(')');
	if (start != std::string::npos && end != std::string::npos && end > start) {
		return input.substr(start + 1, end - start - 1);
	}
	return "";
}


std::shared_ptr<ICommand> createInstructionFromString(int pid, const std::string& raw) {
	std::string keyword = extractKeyword(raw);

	std::cout << "Creating command from raw: " << raw << std::endl;

	if (keyword == "PRINT") {
		size_t lparen = raw.find('(');
		size_t rparen = raw.rfind(')');
		if (lparen == std::string::npos || rparen == std::string::npos || rparen <= lparen)
			throw std::runtime_error("Malformed PRINT statement");

		std::string msg = raw.substr(lparen + 1, rparen - lparen - 1);

		// Strip quotes
		if (!msg.empty() && msg.front() == '"' && msg.back() == '"')
			msg = msg.substr(1, msg.size() - 2);

		return std::make_shared<PrintCommand>(pid, msg);
	}
	else if (keyword == "DECLARE") {
		std::istringstream iss(raw);
		std::string dummy, var;
		int val;
		iss >> dummy >> var >> val;
		return std::make_shared<DeclareCommand>(pid, var, val);
	}
	else if (keyword == "FOR") {
		std::string bodyStr = stripParens(raw);
		if (bodyStr.empty())
			throw std::runtime_error("Malformed FOR statement");
		std::istringstream iss(bodyStr);
		int repeats;
		iss >> repeats;
		if (repeats <= 0)
			throw std::runtime_error("Invalid repeat count in FOR statement");
		std::vector<std::shared_ptr<ICommand>> body;
		std::string cmdStr;
		while (std::getline(iss, cmdStr, ';')) {
			cmdStr.erase(std::remove_if(cmdStr.begin(), cmdStr.end(), ::isspace), cmdStr.end());
			if (!cmdStr.empty()) {
				body.push_back(createInstructionFromString(pid, cmdStr));
			}
		}
		return std::make_shared<ForCommand>(pid, repeats, body);
	}
	else if (keyword == "ADD") {
		std::istringstream iss(raw);
		std::string dummy, a, b, c;
		iss >> dummy >> a >> b >> c;
		return std::make_shared<AddCommand>(pid, a, b, c);
	}
	else if (keyword == "SUBTRACT") {
		std::istringstream iss(raw);
		std::string dummy, a, b, c;
		iss >> dummy >> a >> b >> c;
		return std::make_shared<SubtractCommand>(pid, a, b, c);
	}
	else if (keyword == "SLEEP") {
		std::istringstream iss(raw);
		std::string dummy;
		int ticks;
		iss >> dummy >> ticks;
		return std::make_shared<SleepCommand>(pid, static_cast<uint8_t>(ticks));
	}
	else if (keyword == "WRITE") {
		std::istringstream iss(raw);
		std::string dummy, addrStr, source;
		iss >> dummy >> addrStr >> source;
		uint16_t addr = static_cast<uint16_t>(std::stoi(addrStr, nullptr, 0));
		return std::make_shared<WriteCommand>(pid, addr, source);
	}
	else if (keyword == "READ") {
		std::istringstream iss(raw);
		std::string dummy, var, addrStr;
		iss >> dummy >> var >> addrStr;
		uint16_t addr = static_cast<uint16_t>(std::stoi(addrStr, nullptr, 0));
		return std::make_shared<ReadCommand>(pid, var, addr);
	}

	throw std::runtime_error("Unknown instruction: " + keyword);
}