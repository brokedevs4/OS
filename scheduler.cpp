#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <cmath>

using namespace std;

// Structure to hold input process data
struct Process {
    string id;
    int arrival_time;
    int burst_time;
    int original_index;
};

// Structure to track per-process execution metrics
struct ProcessMetric {
    string id;
    int arrival_time;
    int burst_time;
    int completion_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int response_time = -1;
};

// Gantt chart execution interval
struct GanttBlock {
    string id;
    int start_time;
    int end_time;
};

// Result bundle for each algorithm
struct ScheduleResult {
    string algorithm_name;
    vector<ProcessMetric> metrics;
    vector<GanttBlock> gantt_chart;
    int total_time = 0;
    int busy_time = 0;
    int context_switches = 0;
    double avg_wt = 0.0;
    double avg_tat = 0.0;
    double avg_rt = 0.0;
    double cpu_utilization = 0.0;
    double throughput = 0.0;
};

// Function to read data from data.txt
vector<Process> read_processes(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << endl;
        return processes;
    }

    string line;
    int index = 0;
    while (getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        Process p;
        if (ss >> p.id >> p.arrival_time >> p.burst_time) {
            p.original_index = index++;
            processes.push_back(p);
        }
    }
    return processes;
}

// Consolidate continuous time slots for the same process into single Gantt blocks
vector<GanttBlock> compress_gantt(const vector<GanttBlock>& raw_gantt) {
    vector<GanttBlock> compressed;
    for (const auto& block : raw_gantt) {
        if (!compressed.empty() && compressed.back().id == block.id && compressed.back().end_time == block.start_time) {
            compressed.back().end_time = block.end_time;
        } else {
            compressed.push_back(block);
        }
    }
    return compressed;
}

// Compute aggregate metrics and context switches
void finalize_metrics(ScheduleResult& res) {
    res.gantt_chart = compress_gantt(res.gantt_chart);

    // Count context switches strictly when CPU switches between DIFFERENT non-idle processes
    int switches = 0;
    string prev_proc = "";
    for (const auto& b : res.gantt_chart) {
        if (b.id != "IDLE") {
            if (!prev_proc.empty() && prev_proc != b.id) {
                switches++;
            }
            prev_proc = b.id;
        }
    }
    res.context_switches = switches;

    double sum_wt = 0, sum_tat = 0, sum_rt = 0;
    int total_busy = 0;
    for (auto& m : res.metrics) {
        m.turnaround_time = m.completion_time - m.arrival_time;
        m.waiting_time = m.turnaround_time - m.burst_time;
        sum_wt += m.waiting_time;
        sum_tat += m.turnaround_time;
        sum_rt += m.response_time;
        total_busy += m.burst_time;
    }

    res.busy_time = total_busy;
    int n = res.metrics.size();
    if (n > 0) {
        res.avg_wt = sum_wt / n;
        res.avg_tat = sum_tat / n;
        res.avg_rt = sum_rt / n;
    }
    if (res.total_time > 0) {
        res.cpu_utilization = (double(res.busy_time) / res.total_time) * 100.0;
        res.throughput = double(n) / res.total_time;
    }
}

// 1. FCFS (First Come First Served)
ScheduleResult run_fcfs(vector<Process> procs) {
    ScheduleResult res;
    res.algorithm_name = "FCFS (First Come First Served)";
    sort(procs.begin(), procs.end(), [](const Process& a, const Process& b) {
        if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
        return a.original_index < b.original_index;
    });

    int current_time = 0;
    res.metrics.resize(procs.size());

    for (size_t i = 0; i < procs.size(); ++i) {
        const auto& p = procs[i];
        if (current_time < p.arrival_time) {
            res.gantt_chart.push_back({"IDLE", current_time, p.arrival_time});
            current_time = p.arrival_time;
        }

        int start = current_time;
        int finish = start + p.burst_time;
        res.gantt_chart.push_back({p.id, start, finish});

        res.metrics[i].id = p.id;
        res.metrics[i].arrival_time = p.arrival_time;
        res.metrics[i].burst_time = p.burst_time;
        res.metrics[i].response_time = start - p.arrival_time;
        res.metrics[i].completion_time = finish;

        current_time = finish;
    }

    res.total_time = current_time;
    // Sort metrics back to original process order
    sort(res.metrics.begin(), res.metrics.end(), [](const ProcessMetric& a, const ProcessMetric& b) {
        return a.id < b.id;
    });

    finalize_metrics(res);
    return res;
}

// 2. SJF (Shortest Job First - Non-preemptive)
ScheduleResult run_sjf(vector<Process> procs) {
    ScheduleResult res;
    res.algorithm_name = "SJF (Shortest Job First, Non-Preemptive)";
    int n = procs.size();
    vector<bool> done(n, false);
    res.metrics.resize(n);
    for (int i = 0; i < n; ++i) {
        res.metrics[i].id = procs[i].id;
        res.metrics[i].arrival_time = procs[i].arrival_time;
        res.metrics[i].burst_time = procs[i].burst_time;
    }

    int completed = 0;
    int current_time = 0;

    while (completed < n) {
        int best_idx = -1;
        int min_bt = 1e9;

        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival_time <= current_time) {
                if (procs[i].burst_time < min_bt) {
                    min_bt = procs[i].burst_time;
                    best_idx = i;
                } else if (procs[i].burst_time == min_bt) {
                    if (procs[i].arrival_time < procs[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx == -1) {
            // CPU is idle, advance to next arriving process
            int next_arrival = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!done[i]) next_arrival = min(next_arrival, procs[i].arrival_time);
            }
            res.gantt_chart.push_back({"IDLE", current_time, next_arrival});
            current_time = next_arrival;
            continue;
        }

        int start = current_time;
        int finish = start + procs[best_idx].burst_time;
        res.gantt_chart.push_back({procs[best_idx].id, start, finish});

        res.metrics[best_idx].response_time = start - procs[best_idx].arrival_time;
        res.metrics[best_idx].completion_time = finish;
        done[best_idx] = true;
        completed++;
        current_time = finish;
    }

    res.total_time = current_time;
    finalize_metrics(res);
    return res;
}

// 3. SRTF (Shortest Remaining Time First - Preemptive SJF)
ScheduleResult run_srtf(vector<Process> procs) {
    ScheduleResult res;
    res.algorithm_name = "SRTF (Shortest Remaining Time First, Preemptive)";
    int n = procs.size();
    vector<int> rem_bt(n);
    vector<int> first_cpu(n, -1);
    res.metrics.resize(n);

    for (int i = 0; i < n; ++i) {
        rem_bt[i] = procs[i].burst_time;
        res.metrics[i].id = procs[i].id;
        res.metrics[i].arrival_time = procs[i].arrival_time;
        res.metrics[i].burst_time = procs[i].burst_time;
    }

    int completed = 0;
    int current_time = 0;
    vector<GanttBlock> raw_gantt;

    while (completed < n) {
        int best_idx = -1;
        int min_rem = 1e9;

        for (int i = 0; i < n; ++i) {
            if (procs[i].arrival_time <= current_time && rem_bt[i] > 0) {
                if (rem_bt[i] < min_rem) {
                    min_rem = rem_bt[i];
                    best_idx = i;
                } else if (rem_bt[i] == min_rem) {
                    if (best_idx != -1 && procs[i].arrival_time < procs[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx == -1) {
            raw_gantt.push_back({"IDLE", current_time, current_time + 1});
            current_time++;
            continue;
        }

        if (first_cpu[best_idx] == -1) {
            first_cpu[best_idx] = current_time;
            res.metrics[best_idx].response_time = current_time - procs[best_idx].arrival_time;
        }

        raw_gantt.push_back({procs[best_idx].id, current_time, current_time + 1});
        rem_bt[best_idx]--;
        current_time++;

        if (rem_bt[best_idx] == 0) {
            res.metrics[best_idx].completion_time = current_time;
            completed++;
        }
    }

    res.gantt_chart = raw_gantt;
    res.total_time = current_time;
    finalize_metrics(res);
    return res;
}

// 4. Round Robin (Configurable Quantum)
ScheduleResult run_round_robin(vector<Process> procs, int quantum) {
    ScheduleResult res;
    res.algorithm_name = "Round Robin (q=" + to_string(quantum) + " ms)";
    int n = procs.size();
    vector<int> rem_bt(n);
    vector<int> first_cpu(n, -1);
    res.metrics.resize(n);

    for (int i = 0; i < n; ++i) {
        rem_bt[i] = procs[i].burst_time;
        res.metrics[i].id = procs[i].id;
        res.metrics[i].arrival_time = procs[i].arrival_time;
        res.metrics[i].burst_time = procs[i].burst_time;
    }

    queue<int> ready_q;
    vector<bool> in_queue(n, false);
    int current_time = 0;
    int completed = 0;

    // Check newly arrived processes at t=0
    for (int i = 0; i < n; ++i) {
        if (procs[i].arrival_time <= current_time && !in_queue[i]) {
            ready_q.push(i);
            in_queue[i] = true;
        }
    }

    while (completed < n) {
        if (ready_q.empty()) {
            // Find next arriving process
            int next_arr = 1e9;
            for (int i = 0; i < n; ++i) {
                if (rem_bt[i] > 0) next_arr = min(next_arr, procs[i].arrival_time);
            }
            res.gantt_chart.push_back({"IDLE", current_time, next_arr});
            current_time = next_arr;
            for (int i = 0; i < n; ++i) {
                if (procs[i].arrival_time <= current_time && !in_queue[i] && rem_bt[i] > 0) {
                    ready_q.push(i);
                    in_queue[i] = true;
                }
            }
            continue;
        }

        int curr = ready_q.front();
        ready_q.pop();

        if (first_cpu[curr] == -1) {
            first_cpu[curr] = current_time;
            res.metrics[curr].response_time = current_time - procs[curr].arrival_time;
        }

        int run_time = min(quantum, rem_bt[curr]);
        int start = current_time;
        int finish = start + run_time;
        res.gantt_chart.push_back({procs[curr].id, start, finish});

        current_time = finish;
        rem_bt[curr] -= run_time;

        // Enqueue newly arrived processes during this time interval
        for (int i = 0; i < n; ++i) {
            if (procs[i].arrival_time <= current_time && !in_queue[i] && rem_bt[i] > 0) {
                ready_q.push(i);
                in_queue[i] = true;
            }
        }

        if (rem_bt[curr] > 0) {
            ready_q.push(curr); // re-append to ready queue
        } else {
            res.metrics[curr].completion_time = current_time;
            completed++;
        }
    }

    res.total_time = current_time;
    finalize_metrics(res);
    return res;
}

// 5. HRRN (Highest Response Ratio Next - Non-preemptive)
ScheduleResult run_hrrn(vector<Process> procs) {
    ScheduleResult res;
    res.algorithm_name = "HRRN (Highest Response Ratio Next, Non-Preemptive)";
    int n = procs.size();
    vector<bool> done(n, false);
    res.metrics.resize(n);

    for (int i = 0; i < n; ++i) {
        res.metrics[i].id = procs[i].id;
        res.metrics[i].arrival_time = procs[i].arrival_time;
        res.metrics[i].burst_time = procs[i].burst_time;
    }

    int completed = 0;
    int current_time = 0;

    while (completed < n) {
        int best_idx = -1;
        double max_rr = -1.0;

        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival_time <= current_time) {
                int wait_time = current_time - procs[i].arrival_time;
                double rr = double(wait_time + procs[i].burst_time) / procs[i].burst_time;
                if (rr > max_rr) {
                    max_rr = rr;
                    best_idx = i;
                } else if (abs(rr - max_rr) < 1e-9) {
                    if (best_idx != -1 && procs[i].arrival_time < procs[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx == -1) {
            int next_arr = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!done[i]) next_arr = min(next_arr, procs[i].arrival_time);
            }
            res.gantt_chart.push_back({"IDLE", current_time, next_arr});
            current_time = next_arr;
            continue;
        }

        int start = current_time;
        int finish = start + procs[best_idx].burst_time;
        res.gantt_chart.push_back({procs[best_idx].id, start, finish});

        res.metrics[best_idx].response_time = start - procs[best_idx].arrival_time;
        res.metrics[best_idx].completion_time = finish;
        done[best_idx] = true;
        completed++;
        current_time = finish;
    }

    res.total_time = current_time;
    finalize_metrics(res);
    return res;
}

// Print detailed text outputs matching assignment format
void print_result(const ScheduleResult& res) {
    cout << "\n========================================================================\n";
    cout << " Algorithm: " << res.algorithm_name << "\n";
    cout << "========================================================================\n";

    // Text Gantt chart
    cout << "\nGantt Chart:\n";
    for (const auto& block : res.gantt_chart) {
        cout << "|" << block.id << " " << block.start_time << "-" << block.end_time;
    }
    cout << "|\n\n";

    // Per-process metrics table matching page 2 layout
    cout << "Process Table:\n";
    cout << left << setw(10) << "Process"
         << setw(8) << "AT"
         << setw(8) << "BT"
         << setw(8) << "CT"
         << setw(8) << "TAT"
         << setw(8) << "WT"
         << setw(8) << "RT" << "\n";
    cout << string(58, '-') << "\n";

    for (const auto& m : res.metrics) {
        cout << left << setw(10) << m.id
             << setw(8) << m.arrival_time
             << setw(8) << m.burst_time
             << setw(8) << m.completion_time
             << setw(8) << m.turnaround_time
             << setw(8) << m.waiting_time
             << setw(8) << m.response_time << "\n";
    }
    cout << string(58, '-') << "\n";

    // Summary performance metrics
    cout << fixed << setprecision(2);
    cout << "Summary Metrics:\n";
    cout << "  - Average Waiting Time (WT):      " << res.avg_wt << " ms\n";
    cout << "  - Average Turnaround Time (TAT):  " << res.avg_tat << " ms\n";
    cout << "  - Average Response Time (RT):     " << res.avg_rt << " ms\n";
    cout << "  - CPU Utilization:                " << res.cpu_utilization << " %\n";
    cout << "  - Throughput:                     " << setprecision(4) << res.throughput << " processes/unit time\n";
    cout << "  - Context Switches:               " << res.context_switches << "\n";
}

// Export metrics to CSV for plotting script
void export_csv(const vector<ScheduleResult>& all_results, const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) return;

    out << "Algorithm,Avg_WT,Avg_TAT,Avg_RT,CPU_Util,Throughput,Context_Switches\n";
    for (const auto& r : all_results) {
        out << "\"" << r.algorithm_name << "\","
            << r.avg_wt << ","
            << r.avg_tat << ","
            << r.avg_rt << ","
            << r.cpu_utilization << ","
            << r.throughput << ","
            << r.context_switches << "\n";
    }
}

int main(int argc, char* argv[]) {
    string filename = "data.txt";
    if (argc > 1) {
        filename = argv[1];
    }

    vector<Process> processes = read_processes(filename);
    if (processes.empty()) {
        cerr << "Error: No processes loaded from " << filename << endl;
        return 1;
    }

    cout << "CPU Scheduling Simulator initialized.\n";
    cout << "Loaded " << processes.size() << " processes from '" << filename << "':\n";
    for (const auto& p : processes) {
        cout << "  PID: " << p.id << ", Arrival: " << p.arrival_time << ", Burst: " << p.burst_time << "\n";
    }

    vector<ScheduleResult> results;
    results.push_back(run_fcfs(processes));
    results.push_back(run_sjf(processes));
    results.push_back(run_srtf(processes));
    results.push_back(run_round_robin(processes, 1));
    results.push_back(run_round_robin(processes, 4));
    results.push_back(run_hrrn(processes));

    for (const auto& res : results) {
        print_result(res);
    }

    export_csv(results, "results.csv");
    cout << "\n[Info] Performance results successfully exported to 'results.csv' for graph generation.\n";

    return 0;
}
