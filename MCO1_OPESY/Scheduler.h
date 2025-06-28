#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Process.h"
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <memory>

class CPU {
public:
    int core_id;
    Process* current_process;
    bool is_busy;

    CPU(int id);
};

class Scheduler {
private:
    Config config;
    std::queue<Process*> ready_queue;
    std::vector<CPU> cpus;
    std::map<std::string, std::unique_ptr<Process>> processes;
    std::vector<Process*> finished_processes;
    std::atomic<int> cpu_ticks;
    std::atomic<bool> running;
    std::atomic<bool> generating_processes;
    int process_counter;
    std::mutex cpu_mutex;
    std::mutex queue_mutex;
    std::mutex processes_mutex;
    std::mutex finished_mutex;
    std::thread scheduler_thread;
    std::thread generator_thread;
    std::random_device rd;
    std::mt19937 gen;

public:
    Scheduler(const Config& cfg);
    ~Scheduler();

    void start();
    void stop();
    void startProcessGeneration();
    void stopProcessGeneration();
    void addProcess(std::unique_ptr<Process> process);
    Process* getProcess(const std::string& name);
    std::vector<Process*> getRunningProcesses();
    std::vector<Process*> getFinishedProcesses();
    int getCpuTicks() const;
    int getUsedCores() const;
    int getAvailableCores() const;

private:
    std::vector<Instruction> generateRandomInstructions(int count);
    void processGeneratorLoop();
    void schedulerLoop();
    void executeProcessInstruction(Process* process);
};

#endif 