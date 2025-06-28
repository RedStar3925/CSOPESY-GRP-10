#include "Process.h"
#include "Scheduler.h"
#include <algorithm>
#include <iomanip>
#include <cstdlib>

// Process Implementation
Process::Process(const std::string& n, int p) :
    name(n), pid(p), pc(0), state(ProcessState::READY),
    core_id(-1), creation_time(0), finish_time(-1),
    quantum_remaining(0), sleep_until(0) {
}

bool Process::isFinished() const {
    return pc >= static_cast<int>(instructions.size()) || state == ProcessState::FINISHED;
}

// Config Implementation
Config::Config() : num_cpu(4), scheduler(SchedulerType::FCFS), quantum_cycles(5),
batch_process_freq(1), min_ins(1000), max_ins(2000), delays_per_exec(0) {
}

void Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "config.txt not found, using default values\n";
        createDefaultConfig();
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key, value;
        if (iss >> key >> value) {
            if (key == "num-cpu") {
                num_cpu = std::max(1, std::min(128, std::stoi(value)));
            }
            else if (key == "scheduler") {
                scheduler = (value == "fcfs") ? SchedulerType::FCFS : SchedulerType::RR;
            }
            else if (key == "quantum-cycles") {
                quantum_cycles = std::max(1, std::stoi(value));
            }
            else if (key == "batch-process-freq") {
                batch_process_freq = std::max(1, std::stoi(value));
            }
            else if (key == "min-ins") {
                min_ins = std::max(1, std::stoi(value));
            }
            else if (key == "max-ins") {
                max_ins = std::max(1, std::stoi(value));
            }
            else if (key == "delays-per-exec") {
                delays_per_exec = std::max(0, std::stoi(value));
            }
        }
    }
    file.close();
}

void Config::createDefaultConfig() {
    std::ofstream file("config.txt");
    file << "num-cpu 4\n";
    file << "scheduler fcfs\n";
    file << "quantum-cycles 5\n";
    file << "batch-process-freq 1\n";
    file << "min-ins 1000\n";
    file << "max-ins 2000\n";
    file << "delays-per-exec 0\n";
    file.close();
}

CPU::CPU(int id) : core_id(id), current_process(nullptr), is_busy(false) {}

Scheduler::Scheduler(const Config& cfg) : config(cfg), cpu_ticks(0), running(false),
generating_processes(false), process_counter(1), gen(rd()) {
    for (int i = 0; i < config.num_cpu; ++i) {
        cpus.emplace_back(i);
    }
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    running = true;
    scheduler_thread = std::thread(&Scheduler::schedulerLoop, this);
}

void Scheduler::stop() {
    running = false;
    generating_processes = false;
    if (scheduler_thread.joinable()) {
        scheduler_thread.join();
    }
    if (generator_thread.joinable()) {
        generator_thread.join();
    }
}

void Scheduler::startProcessGeneration() {
    generating_processes = true;
    generator_thread = std::thread(&Scheduler::processGeneratorLoop, this);
}

void Scheduler::stopProcessGeneration() {
    generating_processes = false;
}

void Scheduler::addProcess(std::unique_ptr<Process> process) {
    process->creation_time = cpu_ticks.load();
    
    std::lock_guard<std::mutex> lock1(processes_mutex);  
    std::lock_guard<std::mutex> lock2(queue_mutex);      
    
    Process* proc_ptr = process.get();
    processes[process->name] = std::move(process);
    ready_queue.push(proc_ptr);
}

Process* Scheduler::getProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processes_mutex);
    auto it = processes.find(name);
    return (it != processes.end()) ? it->second.get() : nullptr;
}

std::vector<Process*> Scheduler::getRunningProcesses() {
    std::vector<Process*> running_procs;
    for (auto& cpu : cpus) {
        if (cpu.is_busy && cpu.current_process) {
            running_procs.push_back(cpu.current_process);
        }
    }
    return running_procs;
}

std::vector<Process*> Scheduler::getFinishedProcesses() {
    std::lock_guard<std::mutex> lock(finished_mutex);
    return finished_processes;
}

int Scheduler::getCpuTicks() const { return cpu_ticks.load(); }

int Scheduler::getUsedCores() const {
    return std::count_if(cpus.begin(), cpus.end(), [](const CPU& cpu) { return cpu.is_busy; });
}

int Scheduler::getAvailableCores() const { return config.num_cpu - getUsedCores(); }

std::vector<Instruction> Scheduler::generateRandomInstructions(int count) {
    std::vector<Instruction> instructions;
    std::vector<std::string> instruction_types = { "PRINT", "DECLARE", "ADD", "SUBTRACT", "SLEEP" };
    std::uniform_int_distribution<> type_dist(0, instruction_types.size() - 1);
    std::uniform_int_distribution<> value_dist(1, 100);
    std::uniform_int_distribution<> var_dist(1, 10);
    std::uniform_int_distribution<> sleep_dist(1, 10);

    for (int i = 0; i < count; ++i) {
        std::string inst_type = instruction_types[type_dist(gen)];

        if (inst_type == "PRINT") {
            instructions.emplace_back("PRINT", std::vector<std::string>{"Hello world from process!"});
        }
        else if (inst_type == "DECLARE") {
            std::string var_name = "var" + std::to_string(var_dist(gen));
            std::string value = std::to_string(value_dist(gen));
            instructions.emplace_back("DECLARE", std::vector<std::string>{var_name, value});
        }
        else if (inst_type == "ADD") {
            std::string var1 = "var" + std::to_string(var_dist(gen));
            std::string var2 = "var" + std::to_string(var_dist(gen));
            std::string var3 = "var" + std::to_string(var_dist(gen));
            instructions.emplace_back("ADD", std::vector<std::string>{var1, var2, var3});
        }
        else if (inst_type == "SUBTRACT") {
            std::string var1 = "var" + std::to_string(var_dist(gen));
            std::string var2 = "var" + std::to_string(var_dist(gen));
            std::string var3 = "var" + std::to_string(var_dist(gen));
            instructions.emplace_back("SUBTRACT", std::vector<std::string>{var1, var2, var3});
        }
        else if (inst_type == "SLEEP") {
            std::string sleep_time = std::to_string(sleep_dist(gen));
            instructions.emplace_back("SLEEP", std::vector<std::string>{sleep_time});
        }
    }

    return instructions;
}

void Scheduler::processGeneratorLoop() {
    int last_generation = 0;
    while (generating_processes && running) {
        if (cpu_ticks.load() - last_generation >= config.batch_process_freq) {
            // FIX: Proper string formatting
            std::string counter_str = std::to_string(process_counter);
            while (counter_str.length() < 3) {
                counter_str = "0" + counter_str;
            }
            std::string process_name = "p" + counter_str;
            process_counter++;

            std::uniform_int_distribution<> inst_count_dist(config.min_ins, config.max_ins);
            int instruction_count = inst_count_dist(gen);

            auto process = std::make_unique<Process>(process_name, process_counter);
            process->instructions = generateRandomInstructions(instruction_count);

            addProcess(std::move(process));
            last_generation = cpu_ticks.load();
        }
    }
}

void Scheduler::schedulerLoop() {
    while (running) {
        cpu_ticks++;

        std::vector<Process*> processes_to_wake;
        {
            std::lock_guard<std::mutex> lock(processes_mutex);
            for (auto& [name, process] : processes) {
                if (!process) continue;

                if (process->state == ProcessState::WAITING &&
                    process->sleep_until <= cpu_ticks.load()) {
                    process->state = ProcessState::READY;
                    processes_to_wake.push_back(process.get());
                }
            }
        }

        if (!processes_to_wake.empty()) {
            std::lock_guard<std::mutex> queue_lock(queue_mutex);
            for (Process* process : processes_to_wake) {
                if (process) {
                    ready_queue.push(process);
                }
            }
        }

        {
            std::lock_guard<std::mutex> cpu_lock(cpu_mutex);
            std::lock_guard<std::mutex> queue_lock(queue_mutex);

            for (auto& cpu : cpus) {
                if (!cpu.is_busy && !ready_queue.empty()) {
                    Process* process = ready_queue.front();
                    ready_queue.pop();

                    if (!process) continue;

                    cpu.current_process = process;
                    cpu.is_busy = true;
                    process->state = ProcessState::RUNNING;
                    process->core_id = cpu.core_id;
                    if (config.scheduler == SchedulerType::RR) {
                        process->quantum_remaining = config.quantum_cycles;
                    }
                }
            }
        }

        std::vector<Process*> processes_to_finish;
        std::vector<Process*> processes_to_preempt;
        std::vector<Process*> processes_to_sleep;

        {
            std::lock_guard<std::mutex> cpu_lock(cpu_mutex);
            for (auto& cpu : cpus) {
                if (cpu.is_busy && cpu.current_process) {
                    Process* process = cpu.current_process;

                    if (!process) {
                        cpu.is_busy = false;
                        cpu.current_process = nullptr;
                        continue;
                    }

                    ProcessState old_state = process->state;
                    executeProcessInstruction(process);

                    if (process->state == ProcessState::WAITING) {
                        processes_to_sleep.push_back(process);
                        cpu.current_process = nullptr;
                        cpu.is_busy = false;
                    }

                    else if (process->isFinished()) {
                        process->state = ProcessState::FINISHED;
                        process->finish_time = cpu_ticks.load();
                        processes_to_finish.push_back(process);
                        cpu.current_process = nullptr;
                        cpu.is_busy = false;
                    }

                    else if (config.scheduler == SchedulerType::RR &&
                        process->quantum_remaining <= 0) {
                        process->state = ProcessState::READY;
                        processes_to_preempt.push_back(process);
                        cpu.current_process = nullptr;
                        cpu.is_busy = false;
                    }
                }
            }
        }

        if (!processes_to_finish.empty()) {
            std::lock_guard<std::mutex> finished_lock(finished_mutex);
            for (Process* process : processes_to_finish) {
                if (process) {
                    finished_processes.push_back(process);
                }
            }
        }

        if (!processes_to_preempt.empty()) {
            std::lock_guard<std::mutex> queue_lock(queue_mutex);
            for (Process* process : processes_to_preempt) {
                if (process) {
                    ready_queue.push(process);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Scheduler::executeProcessInstruction(Process* process) {
    if (!process || process->pc >= static_cast<int>(process->instructions.size())) {
        return;
    }

    const Instruction& instruction = process->instructions[process->pc];

    if (instruction.type == "PRINT") {
        if (!instruction.params.empty()) {
            std::string msg = instruction.params[0];
            size_t pos = msg.find("process");
            if (pos != std::string::npos) {
                msg.replace(pos, 7, process->name);
            }
            process->logs.push_back("[" + std::to_string(cpu_ticks.load()) + "] " + msg);
        }
    }
    else if (instruction.type == "DECLARE") {
        if (instruction.params.size() >= 2) {
            std::string var_name = instruction.params[0];
            int value = std::stoi(instruction.params[1]);
            process->variables[var_name] = value;
        }
    }
    else if (instruction.type == "ADD") {
        if (instruction.params.size() >= 3) {
            std::string var1 = instruction.params[0];
            std::string var2 = instruction.params[1];
            std::string var3 = instruction.params[2];
            int val2 = (process->variables.find(var2) != process->variables.end()) ?
                process->variables[var2] : 0;
            int val3 = (process->variables.find(var3) != process->variables.end()) ?
                process->variables[var3] : 0;
            process->variables[var1] = std::min(65535, std::max(0, val2 + val3));
        }
    }
    else if (instruction.type == "SUBTRACT") {
        if (instruction.params.size() >= 3) {
            std::string var1 = instruction.params[0];
            std::string var2 = instruction.params[1];
            std::string var3 = instruction.params[2];
            int val2 = (process->variables.find(var2) != process->variables.end()) ?
                process->variables[var2] : 0;
            int val3 = (process->variables.find(var3) != process->variables.end()) ?
                process->variables[var3] : 0;
            process->variables[var1] = std::min(65535, std::max(0, val2 - val3));
        }
    }
    else if (instruction.type == "SLEEP") {
        if (!instruction.params.empty()) {
            int sleep_time = std::stoi(instruction.params[0]);
            process->sleep_until = cpu_ticks.load() + sleep_time;
            process->state = ProcessState::WAITING;

            return; 
        }
    }

    process->pc++;
    if (config.scheduler == SchedulerType::RR) {
        process->quantum_remaining--;
    }

    if (config.delays_per_exec > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config.delays_per_exec));
    }
}