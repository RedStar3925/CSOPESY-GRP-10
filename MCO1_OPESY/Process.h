#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

enum class ProcessState {
    READY,
    RUNNING,
    WAITING,
    FINISHED
};

enum class SchedulerType {
    FCFS,
    RR
};

struct Instruction {
    std::string type;
    std::vector<std::string> params;

    Instruction(const std::string& t, const std::vector<std::string>& p) : type(t), params(p) {}
};

class Process {
public:
    std::string name;
    int pid;
    std::vector<Instruction> instructions;
    std::map<std::string, int> variables;
    int pc; 
    ProcessState state;
    int core_id;
    int creation_time;
    int finish_time;
    std::vector<std::string> logs;
    int quantum_remaining;
    int sleep_until;

    Process(const std::string& n, int p);
    bool isFinished() const;
};

class Config {
public:
    int num_cpu;
    SchedulerType scheduler;
    int quantum_cycles;
    int batch_process_freq;
    int min_ins;
    int max_ins;
    int delays_per_exec;

    Config();
    void loadFromFile(const std::string& filename = "config.txt");

private:
    void createDefaultConfig();
};

#endif 