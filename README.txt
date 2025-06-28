# CSOPESY CPU Scheduler Simulation

This is a command-line simulation of a CPU scheduler for the CSOPESY course. It supports **Round-Robin (RR)** and **First-Come, First-Served (FCFS)** scheduling algorithms.

## Features

- Dynamic process generation
- Round-Robin and FCFS scheduling support
- Console-based UI
- CPU utilization tracking
- Logging of process activities

## Compilation Instructions

Use the following command to compile the project (Visual Studio Code):
(Note: must be in the directory or within the folder that contains main.cpp and Implementation.cpp files:
g++ main.cpp Implementation.cpp -o scheduler_sim -pthread


Make sure all related header files (Process.h, Scheduler.h) are in the same directory.


Run the Simulator
After successful compilation, run the program with:

directory:
./scheduler_sim or scheduler_sim

For Visual Studio 2022:
Make sure that the files properties language is set to standard 20 c++
Then press "Start Without Debugging"