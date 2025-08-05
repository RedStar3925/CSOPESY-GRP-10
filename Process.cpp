#include "Process.h"
#include <memory>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <functional>
#include <sstream>

Process::Process(int pid, std::string name, int totalInstructions, int memoryRequired, int memPerFrame, int commandCounter, std::vector<std::string> customInstructions)
{
    this->pid = pid;
    this->name = name;
    this->totalInstructions = totalInstructions;
    this->memoryRequired = memoryRequired;
    this->memPerFrame = memPerFrame;
    this->commandCounter = commandCounter;
	this->customInstructions = customInstructions;
    if (!customInstructions.empty())
    {
        int idx = 0;
        for (const auto& instr : customInstructions) {
            try {
                this->commandList.push_back(createInstructionFromString(idx, instr));
                ++idx;
            }
            catch (const std::exception& e) {
                std::cerr << "Invalid instruction: " << instr << " (" << e.what() << ")\n";
            }
        }
    }
    else
    {
        std::vector<std::string> varPool = { "a", "b", "c", "d" };
        int idx = 0;

        auto randomVar = [&]() {
            return varPool[rand() % varPool.size()];
            };

        auto randomValue = [&]() {
            return std::to_string(rand() % 100); // uint16 safe
            };

        std::function<std::string(int)> createRandomInstruction;

        // fix the stuff in icommand
        createRandomInstruction = [&](int depth = 0) -> std::string {
            //int choice = rand() % (depth < 2 ? 6 : 5); // 0–5, but exclude FOR if nested too 
			int choice = rand() % 7; //i dont know how to do the for statement properly yet
            std::string v1 = randomVar(), v2 = randomVar(), v3 = randomVar();

            switch (choice) {
            case 0: return "DECLARE " + v1 + " " + randomValue();
            case 1: return "ADD " + v1 + " " + v2 + " " + randomValue();
            case 2: return "SUBTRACT " + v1 + " " + v2 + " " + v3;
            case 3: return "SLEEP " + std::to_string((rand() % 10) + 1);
            case 4: return "PRINT(\"Hello world from " + name + "\")";
            case 5: { // WRITE
                uint16_t addr = static_cast<uint16_t>((rand() % 4096) * 2); // even addresses in range
                return "WRITE 0x" + toHex(addr) + " " + randomValue();
            }
            case 6: { // READ
                uint16_t addr = static_cast<uint16_t>((rand() % 4096) * 2); // even addresses in range
                return "READ " + v1 + " 0x" + toHex(addr);
            }
            //case 7 : {
            //    // FOR([inner instructions], repeats)
            //    std::string inner;
            //    int repeat = (rand() % 3) + 1;
            //    for (int j = 0; j < repeat; ++j) {
            //        inner += createRandomInstruction(depth + 1) + "; ";
            //    }
            //    return "FOR([" + inner + "], " + std::to_string(repeat) + ")";
            //}
            default: return "PRINT(\"Hello world\")";
            }
            };

        for (int i = 0; i < totalInstructions; ++i) {
            std::string instr = createRandomInstruction(0);
			//std::cout << "Generated instruction: " << instr << std::endl;
            try {
                this->commandList.push_back(createInstructionFromString(i, instr));
            }
            catch (const std::exception& e) {
                std::cerr << "Error generating default instruction: " << instr << " (" << e.what() << ")\n";
            }
        }
    }
}

std::string Process::toHex(uint16_t num) const
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << num;
    return ss.str();
}


//void Process::addCommand(ICommand::CommandType command)
//{
//    if (command == ICommand::CommandType::PRINT) {
//        for (int i = 0; i < this->totalInstructions; i++) {
//            this->commandList.push_back(std::make_shared<ICommand>(i, ICommand::CommandType::PRINT));
//        }
//    }
//}


//change execute to proper usage with new ICommand class
void Process::executeCurrentCommand() {
    if (this->commandCounter < this->totalInstructions) {
        this->currentState = ProcessState::RUNNING;
        this->commandList.at(this->commandCounter)->execute(
            this->cpuCoreId,
            this->name,
            this->variables,
            this->screenOutput,
            this->memory
        );
    }
    else {
        this->currentState = ProcessState::FINISHED;
        this->timeFinished = getDateNow();
    }
}

void Process::moveToNextLine()
{
    this->commandCounter++;
    if (this->commandCounter >= this->totalInstructions) {
        this->currentState = ProcessState::FINISHED;
        this->timeFinished = getDateNow();
    }
}

void Process::setCoreId(int coreID)
{
    this->cpuCoreId = coreID;
}

bool Process::isFinished() const
{
    return (this->currentState == ProcessState::FINISHED);
}

std::string Process::getDateNow()
{
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to local time and format it
    std::tm local_tm;

    localtime_s(&local_tm, &now_c);

    // Use a string stream to format the date and time
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");

    // Store formatted time in creationTime
    return oss.str();
}

std::string Process::getTimeFinished()
{
    return this->timeFinished;
}

std::string Process::getTimeStarted()
{
    return this->timeStarted;
}

int Process::getProcessID() const
{
    return this->pid;
}

void Process::setTimeStarted()
{
    this->timeStarted = getDateNow();
}

int Process::getCommandCounter() const
{
    return this->commandCounter;
}

int Process::getTotalInstructions() const
{
    return this->totalInstructions;
}

int Process::getTotalMemoryRequired() const
{
    return this->memoryRequired;
}

int Process::getNumberOfPages() const
{
    if ((this->memoryRequired / this->memPerFrame) == 0)
        return 1;
    return this->memoryRequired/this->memPerFrame;
}

int Process::getMemPerFrame() const
{
    return this->memPerFrame;
}

void Process::setInBackingStore(bool value)
{
    this->inBackingStore = value;
}

bool Process::getInBackingStore() const
{
    return this->inBackingStore;
}

std::string Process::getName()
{
    return this->name;
}

