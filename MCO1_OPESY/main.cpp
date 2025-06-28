#include "Process.h"
#include "Scheduler.h"
#include <iomanip>
#include <cstdlib>


class CSOpesyCLI {
private:
    Config config;
    std::unique_ptr<Scheduler> scheduler;
    bool initialized;
    std::string current_screen;

public:
    CSOpesyCLI() : initialized(false) {}

    void run() {
            std::cout << "  _____    _____    ____    _____   ______   _____   __     __\n";
            std::cout << " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n";
            std::cout << "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n";
            std::cout << "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n";
            std::cout << "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n";
            std::cout << " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";
            std::cout << "Hello, Welcome to CSOPESY commandline!\n";
            std::cout << "Type 'help' to view available commands, 'exit' to quit, 'clear' to clear the screen\n";
        

        std::string command;
        while (true) {
            if (!current_screen.empty()) {
                handleScreenCommands();
            }
            else {
                handleMainMenu();
            }
        }
    }

private:
    void handleMainMenu() {
        std::cout << "root:\\> ";
        std::string line;
        std::getline(std::cin, line);

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "exit") {
            if (scheduler) {
                scheduler->stop();
            }
            std::cout << "Goodbye!\n";
            exit(0);
        }
        else if (command == "initialize") {
            initialize();
        }
        else if (command == "help") {
            std::cout << "\nAvailable commands:\n";
            std::cout << "  initialize       - Initialize the scheduler\n";
            std::cout << "  scheduler-stop   - Stop the scheduler\n";
            std::cout << "  scheduler-test   - Run a scheduler test\n";
            std::cout << "  screen -ls       - List running and finished processes\n";
            std::cout << "  screen -s <name> - Create a screen with the given name\n";
            std::cout << "  screen -r <name> - Resume an existing screen by name\n";
            std::cout << "  report-util      - Generate utilization report\n";
            std::cout << "  clear            - Clear the screen\n";
            std::cout << "  help             - Show this help message\n";
            std::cout << "  exit             - Exit the program\n";
     }
        else if (!initialized) {
            std::cout << "Please run 'initialize' first\n";
        }
        else if (command == "scheduler-start") {
            startScheduler();
        }
        else if (command == "scheduler-stop") {
            stopScheduler();
        }
        else if (command == "report-util") {
            generateReport();
        }
        else if (command == "screen") {
            handleScreenCommand(iss);
        }
        else if (command == "clear") {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            std::cout << "  _____    _____    ____    _____   ______   _____   __     __\n";
            std::cout << " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n";
            std::cout << "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n";
            std::cout << "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n";
            std::cout << "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n";
            std::cout << " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";
            std::cout << "Hello, Welcome to CSOPESY commandline!\n";
            std::cout << "Type 'help' to view available commands, 'exit' to quit, 'clear' to clear the screen\n";
        }
        else if (!command.empty()) {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    void handleScreenCommands() {
        std::cout << current_screen << ":\\> ";
        std::string line;
        std::getline(std::cin, line);

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "exit") {
            current_screen.clear();
            std::cout << "  _____    _____    ____    _____   ______   _____   __     __\n";
            std::cout << " / ____|  / ____|  / __ \\  |  __ \\ |  ____| / ____|  \\ \\   / /\n";
            std::cout << "| |      | (___   | |  | | | |__) || |__   | (___     \\ \\_/ / \n";
            std::cout << "| |       \\___ \\  | |  | | |  ___/ |  __|   \\___ \\     \\   /  \n";
            std::cout << "| |____   ____) | | |__| | | |     | |____  ____) |     | |   \n";
            std::cout << " \\_____| |_____/   \\____/  |_|     |______||_____/      |_|   \n";
            std::cout << "Hello, Welcome to CSOPESY commandline!\n";
            std::cout << "Type 'help' to view available commands, 'exit' to quit, 'clear' to clear the screen\n";
        }
        else if (command == "process-smi") {
            showProcessInfo();
        }
        else if (!command.empty()) {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    void initialize() {
        config.loadFromFile();
        scheduler = std::make_unique<Scheduler>(config);
        scheduler->start();
        initialized = true;
        std::cout << "Initialize command recognized. Processor config loaded.\n";
    }

    void startScheduler() {
        if (scheduler) {
            scheduler->startProcessGeneration();
            std::cout << "Scheduler started generating processes.\n";
        }
    }

    void stopScheduler() {
        if (scheduler) {
            scheduler->stopProcessGeneration();
            std::cout << "Scheduler stopped generating processes.\n";
        }
    }

    void handleScreenCommand(std::istringstream& iss) {
        std::string flag, process_name;
        iss >> flag;

        if (flag == "-s") {
            iss >> process_name;
            if (!process_name.empty()) {
                createProcessScreen(process_name);
            }
            else {
                std::cout << "Please specify a process name\n";
            }
        }
        else if (flag == "-r") {
            iss >> process_name;
            if (!process_name.empty()) {
                resumeProcessScreen(process_name);
            }
            else {
                std::cout << "Please specify a process name\n";
            }
        }
        else if (flag == "-ls") {
            listProcesses();
        }
        else {
            std::cout << "Invalid screen command. Usage:\n";
            std::cout << "  screen -s <process_name>  : Create new process screen\n";
            std::cout << "  screen -r <process_name>  : Resume existing process screen\n";
            std::cout << "  screen -ls                : List all processes\n";
        }
    }

    void createProcessScreen(const std::string& process_name) {
        if (scheduler->getProcess(process_name)) {
            std::cout << "Process " << process_name << " already exists\n";
            return;
        }

        auto process = std::make_unique<Process>(process_name, 0);

        process->instructions.emplace_back("PRINT", std::vector<std::string>{"Hello world from " + process_name + "!"});
        process->instructions.emplace_back("DECLARE", std::vector<std::string>{"x", "5"});
        process->instructions.emplace_back("ADD", std::vector<std::string>{"x", "x", "10"});
        process->instructions.emplace_back("PRINT", std::vector<std::string>{"Process " + process_name + " finished"});

        scheduler->addProcess(std::move(process));
        current_screen = process_name;

#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        std::cout << "Process " << process_name << " created and screen attached.\n";
    }

    void resumeProcessScreen(const std::string& process_name) {
        Process* process = scheduler->getProcess(process_name);
        if (!process) {
            std::cout << "Process " << process_name << " not found.\n";
            std::cout << "Use 'screen -ls' to see available processes or 'screen -s " << process_name << "' to create it.\n";
            return;
        }

        current_screen = process_name;


#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        std::cout << "Attached to process " << process_name << "\n";
    }

    void showProcessInfo() {
        if (current_screen.empty()) {
            return;
        }

        Process* process = scheduler->getProcess(current_screen);
        if (!process) {
            std::cout << "Process not found\n";
            return;
        }

        std::cout << "Process: " << process->name << "\n";
        std::cout << "ID: " << process->pid << "\n";

        if (process->isFinished()) {
            std::cout << "Finished!\n";
        }
        else {
            std::cout << "Current instruction line: " << process->pc << "\n";
            std::cout << "Lines of code: " << process->instructions.size() << "\n";
        }

        std::cout << "\nLogs:\n";
        int start = std::max(0, static_cast<int>(process->logs.size()) - 10);
        for (int i = start; i < static_cast<int>(process->logs.size()); ++i) {
            std::cout << process->logs[i] << "\n";
        }
    }

    void listProcesses() {
        if (!scheduler) {
            return;
        }

        int total_cores = config.num_cpu;
        int cores_used = scheduler->getUsedCores();
        int cores_available = scheduler->getAvailableCores();

        double cpu_utilization = (total_cores > 0) ?
            (static_cast<double>(cores_used) / total_cores) * 100 : 0;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "CPU utilization: " << cpu_utilization << "%\n";
        std::cout << "Cores used: " << cores_used << "\n";
        std::cout << "Cores available: " << cores_available << "\n\n";

        auto running_processes = scheduler->getRunningProcesses();
        auto finished_processes = scheduler->getFinishedProcesses();

        std::cout << "Running processes:\n";
        for (Process* process : running_processes) {
            std::string core_info = (process->core_id >= 0) ?
                " (Core: " + std::to_string(process->core_id) + ")" : "";
            std::cout << process->name << core_info << " " << process->creation_time << "\n";
        }

        std::cout << "\nFinished processes:\n";
        for (Process* process : finished_processes) {
            std::cout << process->name << " (" << process->creation_time << ") "
                << process->finish_time << "\n";
        }
    }

    void generateReport() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm;

        localtime_s(&local_tm, &now_c);

        int total_cores = config.num_cpu;
        int cores_used = scheduler->getUsedCores();
        int cores_available = scheduler->getAvailableCores();

        double cpu_utilization = (total_cores > 0) ?
            (static_cast<double>(cores_used) / total_cores) * 100 : 0;

        std::ofstream report_file("csopesy-log.txt");

        report_file << "Process Report\n";
        report_file << "Generated at: " << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p") << "\n\n";
        report_file << std::fixed << std::setprecision(2);
        report_file << "CPU utilization: " << cpu_utilization << "%\n";
        report_file << "Cores used: " << cores_used << "\n";
        report_file << "Cores available: " << cores_available << "\n\n";

        auto running_processes = scheduler->getRunningProcesses();
        auto finished_processes = scheduler->getFinishedProcesses();

        report_file << "Running processes:\n";
        for (Process* process : running_processes) {
            std::string core_info = (process->core_id >= 0) ?
                " (Core: " + std::to_string(process->core_id) + ")" : "";
            report_file << process->name << core_info << " " << process->creation_time << "\n";
        }

        report_file << "\nFinished processes:\n";
        for (Process* process : finished_processes) {
            report_file << process->name << " (" << process->creation_time << ") "
                << process->finish_time << "\n";
        }

        report_file.close();
        std::cout << "Report generated and saved to csopesy-log.txt\n";
    }
};

int main() {
    CSOpesyCLI cli;
    cli.run();
    return 0;
}