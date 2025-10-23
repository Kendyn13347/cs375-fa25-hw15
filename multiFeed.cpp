#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

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
    int total_burst_time = 0;
    for (auto& p : processes) {
        p.waiting_time = p.turnaround_time - p.burst_time;
        avg_wait += p.waiting_time;
        avg_turn += p.turnaround_time;
        total_burst_time += p.burst_time;
    }

    if (!processes.empty()) {
        avg_wait /= processes.size();
        avg_turn /= processes.size();
    }

    double cpu_util = (total_time > 0) ? ((double)total_burst_time / total_time * 100) : 0;
    
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
        {"P1", 0, 8, 2, 0, 0, 0},
        {"P2", 1, 4, 1, 0, 0, 0},
        {"P3", 2, 9, 3, 0, 0, 0},
        {"P4", 3, 5, 4, 0, 0, 0}
    };

    for (auto& p : processes) {
        p.remaining_time = p.burst_time;
    }

    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });

    std::vector<std::queue<Process*>> queues(3);
    std::vector<int> quanta = {2, 4, 8};
    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int completed_processes = 0;
    int process_idx = 0;

    while (completed_processes < processes.size()) {
        while (process_idx < processes.size() && processes[process_idx].arrival_time <= current_time) {
            queues[0].push(&processes[process_idx]);
            process_idx++;
        }

        int current_queue_idx = -1;
        for (int i = 0; i < 3; ++i) {
            if (!queues[i].empty()) {
                current_queue_idx = i;
                break;
            }
        }

        if (current_queue_idx != -1) {
            Process* current_process = queues[current_queue_idx].front();
            queues[current_queue_idx].pop();

            int time_to_run = std::min(quanta[current_queue_idx], current_process->remaining_time);
            
            gantt.push_back({current_process->id, time_to_run});
            
            int time_after_run = current_time + time_to_run;
            current_process->remaining_time -= time_to_run;
            
            while (process_idx < processes.size() && processes[process_idx].arrival_time <= time_after_run) {
                queues[0].push(&processes[process_idx]);
                process_idx++;
            }
            
            current_time = time_after_run;

            if (current_process->remaining_time > 0) {
                if (time_to_run == quanta[current_queue_idx]) {
                    int next_queue = std::min(current_queue_idx + 1, 2);
                    queues[next_queue].push(current_process);
                } else {
                    queues[current_queue_idx].push(current_process);
                }
            } else {
                current_process->finished = true;
                completed_processes++;
                current_process->turnaround_time = current_time - current_process->arrival_time;
            }
        } else {
            current_time++;
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}
