#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <numeric>

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

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<std::pair<std::string, int>> gantt;
    int current_time = 0;
    int completed_processes = 0;
    
    while (completed_processes < processes.size()) {
        std::vector<Process*> ready_queue;
        int total_tickets = 0;

        for (auto& p : processes) {
            if (!p.finished && p.arrival_time <= current_time) {
                ready_queue.push_back(&p);
                total_tickets += (10 / p.priority);
            }
        }

        if (ready_queue.empty()) {
            current_time++;
            continue;
        }

        std::uniform_int_distribution<> distrib(1, total_tickets);
        int winning_ticket = distrib(gen);

        Process* winner = nullptr;
        int ticket_sum = 0;
        for (auto* p : ready_queue) {
            ticket_sum += (10 / p->priority);
            if (winning_ticket <= ticket_sum) {
                winner = p;
                break;
            }
        }

        if (winner) {
            int time_to_run = winner->burst_time;
            gantt.push_back({winner->id, time_to_run});
            current_time += time_to_run;
            winner->finished = true;
            winner->turnaround_time = current_time - winner->arrival_time;
            completed_processes++;
        }
    }

    calculateMetrics(processes, current_time);
    printGantt(gantt);
    return 0;
}