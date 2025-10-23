#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <map>
#include <memory>
#include <numeric>

struct Process {
    std::string id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;
    int deadline = 0;
    double vruntime = 0.0;
    bool finished = false;
};

void calculateMetrics(const std::vector<Process>& processes, int total_time, double& avg_wait, double& avg_turn, double& cpu_util, double& throughput) {
    avg_wait = 0;
    avg_turn = 0;
    int total_burst_time = 0;
    for (const auto& p : processes) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
        total_burst_time += p.burst_time;
    }
    int n = processes.size();
    if (n > 0) {
        avg_wait /= n;
        avg_turn /= n;
    }
    cpu_util = (total_time > 0) ? (double)total_burst_time / total_time * 100 : 0;
    throughput = (total_time > 0) ? (double)n / total_time : 0;
}
void printGantt(std::ostream& out, const std::vector<std::pair<std::string, int>>& gantt) {
    out << "Gantt Chart: ";
    for (const auto& entry : gantt) {
        out << entry.first << " (" << entry.second << ") ";
    }
    out << "\n";
}

void printResults(std::ostream& out, const std::vector<Process>& processes, int total_time, const std::vector<std::pair<std::string, int>>& gantt) {
    double avg_wait, avg_turn, cpu_util, throughput;
    calculateMetrics(processes, total_time, avg_wait, avg_turn, cpu_util, throughput);

    out << "Average Waiting Time: " << avg_wait << "\n";
    out << "Average Turnaround Time: " << avg_turn << "\n";
    out << "CPU Utilization: " << cpu_util << "%\n";
    out << "Throughput: " << throughput << " processes/unit time\n";
    printGantt(out, gantt);
}

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) = 0;
};

class FCFSScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b){
            return a.arrival_time < b.arrival_time;
        });

        int current_time = 0;
        for (auto& p : processes) {
            if (current_time < p.arrival_time) {
                current_time = p.arrival_time;
            }
            p.waiting_time = current_time - p.arrival_time;
            current_time += p.burst_time;
            p.turnaround_time = current_time - p.arrival_time;
            gantt.push_back({p.id, p.burst_time});
        }
        total_time = current_time;
    }
};

class SJFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
         for(auto& p : processes) p.remaining_time = p.burst_time;
        
        int current_time = 0;
        int completed = 0;
        while(completed < processes.size()){
            int shortest_idx = -1;
            int shortest_burst = 1e9;

            for(int i=0; i<processes.size(); ++i){
                if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && processes[i].burst_time < shortest_burst){
                    shortest_burst = processes[i].burst_time;
                    shortest_idx = i;
                }
            }

            if(shortest_idx == -1){
                current_time++;
                continue;
            }

            Process& p = processes[shortest_idx];
            current_time += p.burst_time;
            p.remaining_time = 0;
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            gantt.push_back({p.id, p.burst_time});
            completed++;
        }
        total_time = current_time;
    }
};

class SRTFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        for(auto& p : processes) p.remaining_time = p.burst_time;

        int current_time = 0;
        int completed = 0;
        std::string last_id = "";

        while(completed < processes.size()){
            int shortest_idx = -1;
            int shortest_rem = 1e9;

            for(int i=0; i<processes.size(); ++i){
                if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && processes[i].remaining_time < shortest_rem){
                    shortest_rem = processes[i].remaining_time;
                    shortest_idx = i;
                }
            }

            if(shortest_idx == -1){
                current_time++;
                continue;
            }
            
            Process& p = processes[shortest_idx];

            if(gantt.empty() || gantt.back().first != p.id) {
                gantt.push_back({p.id, 1});
            } else {
                gantt.back().second++;
            }

            p.remaining_time--;
            current_time++;

            if(p.remaining_time == 0){
                completed++;
                p.turnaround_time = current_time - p.arrival_time;
                p.waiting_time = p.turnaround_time - p.burst_time;
            }
        }
        total_time = current_time;
    }
};

class PriorityScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        for(auto& p : processes) p.remaining_time = p.burst_time;
        
        int current_time = 0;
        int completed = 0;
        while(completed < processes.size()){
            int highest_priority_idx = -1;
            int highest_priority = 1e9;

            for(int i=0; i<processes.size(); ++i){
                if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && processes[i].priority < highest_priority){
                    highest_priority = processes[i].priority;
                    highest_priority_idx = i;
                }
            }

            if(highest_priority_idx == -1){
                current_time++;
                continue;
            }

            Process& p = processes[highest_priority_idx];
            current_time += p.burst_time;
            p.remaining_time = 0;
            p.turnaround_time = current_time - p.arrival_time;
            p.waiting_time = p.turnaround_time - p.burst_time;
            gantt.push_back({p.id, p.burst_time});
            completed++;
        }
        total_time = current_time;
    }
};

class RoundRobinScheduler : public Scheduler {
private:
    int quantum;
public:
    RoundRobinScheduler(int q) : quantum(q) {}
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
         for (auto& p : processes) p.remaining_time = p.burst_time;
        std::queue<Process*> ready_queue;
        int current_time = 0;
        int idx = 0;
        int completed = 0;

        while (completed < processes.size()) {
            while (idx < processes.size() && processes[idx].arrival_time <= current_time) {
                ready_queue.push(&processes[idx++]);
            }

            if (ready_queue.empty()) {
                if (idx < processes.size()) {
                    current_time = processes[idx].arrival_time;
                } else {
                    break;
                }
                continue;
            }

            Process* current = ready_queue.front();
            ready_queue.pop();

            int run_time = std::min(quantum, current->remaining_time);
            
            if (gantt.empty() || gantt.back().first != current->id) {
                gantt.push_back({current->id, run_time});
            } else {
                gantt.back().second += run_time;
            }

            current->remaining_time -= run_time;
            current_time += run_time;

            while (idx < processes.size() && processes[idx].arrival_time <= current_time) {
                ready_queue.push(&processes[idx++]);
            }

            if (current->remaining_time > 0) {
                ready_queue.push(current);
            } else {
                current->turnaround_time = current_time - current->arrival_time;
                current->waiting_time = current->turnaround_time - current->burst_time;
                completed++;
            }
        }
        total_time = current_time;
    }
};

class MLQScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        // Not implemented in this version
    }
};
class MLFQScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        // Not implemented in this version
    }
};
class LotteryScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        // Not implemented in this version
    }
};
class CFSScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        // Not implemented in this version
    }
};
class EDFScheduler : public Scheduler {
public:
    void schedule(std::vector<Process>& processes, std::vector<std::pair<std::string, int>>& gantt, int& total_time) override {
        // Not implemented in this version
    }
};

std::vector<Process> loadProcesses(const std::string& filename) {
    std::vector<Process> processes;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return processes;
    }
    std::string id;
    int at, bt, pri;
    while (file >> id >> at >> bt >> pri) {
        processes.push_back({id, at, bt, pri});
    }
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });
    return processes;
}

std::vector<Process> generateRandomProcesses(int num) {
    std::vector<Process> processes;
    std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    for (int i = 0; i < num; ++i) {
        std::string id = "P" + std::to_string(i + 1);
        int at = gen() % 20;
        int bt = 1 + gen() % 10;
        int pri = 1 + gen() % 5;
        processes.push_back({id, at, bt, pri});
    }
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });
    return processes;
}

int main(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            args[argv[i]] = argv[i + 1];
        } else {
             args[argv[i]] = "";
        }
    }
    std::string scheduler_type = args["--scheduler"];
    std::string input_file = args["--input"];
    std::string output_file = args["--output"];
    int quantum = args.count("--quantum") ? std::stoi(args["--quantum"]) : 4;
    bool random = args.count("--random");
    int num_random = args.count("--num") ? std::stoi(args["--num"]) : 10;

    std::vector<Process> processes;

    if (random) {
        processes = generateRandomProcesses(num_random);
    } else if (!input_file.empty()) {
        processes = loadProcesses(input_file);
    } else {
        processes = {
            {"P1", 0, 8, 2},
            {"P2", 1, 4, 1},
            {"P3", 2, 9, 3},
            {"P4", 3, 5, 4}
        };
    }

    if (processes.empty()) {
        std::cerr << "No processes loaded.\n";
        return 1;
    }

    std::unique_ptr<Scheduler> scheduler;
    if (scheduler_type == "rr") {
        scheduler = std::make_unique<RoundRobinScheduler>(quantum);
    } else if (scheduler_type == "fcfs") {
        scheduler = std::make_unique<FCFSScheduler>();
    } else if (scheduler_type == "sjf") {
        scheduler = std::make_unique<SJFScheduler>();
    } else if (scheduler_type == "srtf") {
        scheduler = std::make_unique<SRTFScheduler>();
    } else if (scheduler_type == "priority") {
        scheduler = std::make_unique<PriorityScheduler>();
    } else {
        std::cerr << "Unknown scheduler: " << scheduler_type << "\n";
        return 1;
    }

    std::vector<std::pair<std::string, int>> gantt;
    int total_time = 0;
    scheduler->schedule(processes, gantt, total_time);
    
    if (!output_file.empty()) {
        std::ofstream log(output_file);
        if (log.is_open()) {
            printResults(log, processes, total_time, gantt);
            log.close();
        } else {
            std::cerr << "Error: Could not open output file " << output_file << "\n";
        }
    } else {
        printResults(std::cout, processes, total_time, gantt);
    }

    return 0;
}