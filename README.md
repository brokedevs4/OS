# CPU Scheduling Simulator (Linux / C++)

A standard C++17 CPU scheduling simulator implementing FCFS, SJF (non-preemptive), SRTF (preemptive), Round Robin ($q=1$ and $q=4$), and HRRN with text Gantt charts and performance metrics.

---

## File Structure

- `scheduler.cpp`: Core simulator source code (standard library only).
- `data.txt`: Process workload file (`PID`, `Arrival Time`, `Burst Time`).
- `plot_metrics.py`: Script to generate comparison charts.
- `FULL_MARKS_GUIDE.md`: Grading rubric, formula breakdown, and edge-case guide.
- `VIVA_AND_ASSIGNMENT_GUIDE.md`: Plain-English explanation of algorithms, WSL, compilers, and oral viva Q&A.
- `CHECKLIST.md`: Milestone and verification tracker.
- `REPORT.md`: 2-page academic performance report.
- `graphs/`: Generated high-resolution performance comparison charts.

---

## Prerequisites

- **Compiler**: GCC / G++ supporting C++17
- **Environment**: Linux / WSL (Ubuntu) / any standard C++ environment
- **Graph Plotting**: Python 3 with `matplotlib`

To install prerequisites on Ubuntu/WSL:
```bash
sudo apt update
sudo apt install -y g++ python3-matplotlib
```

---

## How to Compile and Run

### 1. Compile the Simulator
```bash
g++ -std=c++17 -O2 -Wall -Wextra scheduler.cpp -o scheduler
```

### 2. Run the Simulator
```bash
./scheduler data.txt
```
*(If no argument is given, it defaults to `data.txt` in the current directory).*

### 3. Generate Comparison Graphs
```bash
python3 plot_metrics.py
```
*(On Windows: `python plot_metrics.py`)*

Charts are saved into the `graphs/` folder:
- `graphs/comparison_dashboard.png` (2x2 comparison grid)
- `graphs/avg_waiting_time.png`
- `graphs/avg_turnaround_time.png`
- `graphs/avg_response_time.png`
- `graphs/context_switches.png`

---

## Input Data Format (`data.txt`)

Lines starting with `#` are ignored as comments.
```text
# pid arrival burst
P1 0 8
P2 1 4
P3 2 9
P4 3 5
```

---

## Output Metrics Computed

- **CT**: Completion Time
- **TAT**: Turnaround Time ($\text{CT} - \text{AT}$)
- **WT**: Waiting Time ($\text{TAT} - \text{BT}$)
- **RT**: Response Time ($\text{First CPU Time} - \text{AT}$)
- **Avg WT / Avg TAT / Avg RT**
- **CPU Utilization (%)**
- **Throughput** (processes / unit time)
- **Context Switches** (counted strictly when switching between distinct processes)
