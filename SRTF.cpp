
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
    bool finished = false; 
};

void calculateMetrics(std::vector<Process>& processes, int total_time) {
    double avg_wait = 0, avg_turn = 0;
    for (auto& p : processes) {
        
        p.waiting_time = p.turnaround_time - p.burst_time;
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
    }

    if (!processes.empty()) {
        avg_wait /= processes.size();
        avg_turn /= processes.size();
    }

    int total_burst_time = 0;
    for(const auto& p : processes) {
        total_burst_time += p.burst_time;
    }
    double cpu_util = (total_time > 0) ? ((double)total_burst_time / total_time * 100) : 0;
    
    std::cout << "Avg Waiting Time: " << avg_wait << "\n";
    std::cout << "Avg Turnaround Time: " << avg_turn << "\n";
    std::cout << "CPU Utilization: " << cpu_util << "%\n";
}

void printGantt(const std::vector<std::pair<std::string, int>>& gantt) {
    std::cout << "Gantt Chart: ";
    std::string last_id = "";
    int duration = 0;
    for (const auto& entry : gantt) {
        if (entry.first == last_id) {
            duration++;
        } else {
            if (!last_id.empty()) {
                std::cout << last_id << " (" << duration << ") ";
            }
            last_id = entry.first;
            duration = 1;
        }
    }
    if (!last_id.empty()) {
        std::cout << last_id << " (" << duration << ") ";
    }
    std::cout << "\n";
}

int main() {
    std::vector<Process> processes = {
        {"P1", 0, 8, 2, 0, 0, 0},
        {"P2", 1, 4, 1, 0, 0, 0},
        {"P3", 2, 9, 3, 0, 0, 0},
        {"P4", 3, 5, 4, 0, 0, 0}
    };


    for (auto& p : processes) {
        p.remaining_time = p.burst_time;
    }

    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < processes.size()) {
        Process* shortest_process = nullptr;
        int min_remaining_time = -1;

        for (auto& p : processes) {
            if (p.arrival_time <= current_time && !p.finished) {
                if (shortest_process == nullptr || p.remaining_time < min_remaining_time) {
                    min_remaining_time = p.remaining_time;
                    shortest_process = &p;
                }
            }
        }
        
        
        if (shortest_process == nullptr) {
            current_time++;
            continue;
        }

        gantt.push_back({shortest_process->id, 1});

        shortest_process->remaining_time--;
        current_time++;

        if (shortest_process->remaining_time == 0) {
            shortest_process->finished = true;
            completed_processes++;
            shortest_process->turnaround_time = current_time - shortest_process->arrival_time;
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);

    return 0;
}
