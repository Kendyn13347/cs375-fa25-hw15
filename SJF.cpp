#include <iostream>
#include <vector>
#include <algorithm>
#include <string>


struct Process {
    std::string id;
    int arrival_time;
    int burst_time;
    int priority; 
    int remaining_time; 
    int waiting_time;
    int turnaround_time;
};

void calculateMetrics(std::vector<Process>& processes, int total_time) {
    double avg_wait = 0, avg_turn = 0;
    for (auto& p : processes) {
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }

    if (!processes.empty()) {
        avg_wait /= processes.size();
        avg_turn /= processes.size();
    }

    double cpu_util = (total_time > 0) ? ((double)total_time / total_time * 100) : 0; 
    
    std::cout << "Avg Waiting Time: " << avg_wait << "\n";
    std::cout << "Avg Turnaround Time: " << avg_turn << "\n";
    std::cout << "CPU Utilization: " << cpu_util << "%\n";
}

void printGantt(const std::vector<std::pair<std::string, int>>& gantt) {
    std::cout << "Gantt Chart: ";
    for (const auto& entry : gantt) {
        std::cout << entry.first << " (" << entry.second << ") ";
    }
    std::cout << "\n";
}

int main() {
    std::vector<Process> processes = {
        {"P1", 0, 8, 2, 8, 0, 0},
        {"P2", 1, 4, 1, 4, 0, 0},
        {"P3", 2, 9, 3, 9, 0, 0},
        {"P4", 3, 5, 4, 5, 0, 0}
    };

    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    std::vector<Process> ready_queue;
    int completed_processes = 0;
    std::vector<bool> is_in_queue(processes.size(), false);

    while (completed_processes < processes.size()) {
        for (int i = 0; i < processes.size(); ++i) {
            if (processes[i].arrival_time <= current_time && !is_in_queue[i]) {
                ready_queue.push_back(processes[i]);
                is_in_queue[i] = true;
            }
        }
        
        if (ready_queue.empty()) {
            for (int i = 0; i < processes.size(); ++i) {
                if (!is_in_queue[i]) {
                    current_time = processes[i].arrival_time;
                    break;
                }
            }
            continue;
        }

        std::sort(ready_queue.begin(), ready_queue.end(), [](const Process& a, const Process& b) {
            return a.burst_time < b.burst_time;
        });

        Process current_process = ready_queue.front();
        ready_queue.erase(ready_queue.begin());

        for (auto& p : processes) {
            if (p.id == current_process.id) {
                p.waiting_time = current_time - p.arrival_time;
                current_time += p.burst_time;
                p.turnaround_time = current_time - p.arrival_time;
                gantt.push_back({p.id, p.burst_time});
                break;
            }
        }
        
        completed_processes++;
    }


    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}