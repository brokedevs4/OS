import os
import csv
import matplotlib.pyplot as plt

def main():
    csv_file = "results.csv"
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} not found. Please run ./scheduler first.")
        return

    algorithms = []
    avg_wt = []
    avg_tat = []
    avg_rt = []
    ctx_switches = []

    # Map long names to concise labels for graphs
    label_map = {
        "FCFS (First Come First Served)": "FCFS",
        "SJF (Shortest Job First, Non-Preemptive)": "SJF (NP)",
        "SRTF (Shortest Remaining Time First, Preemptive)": "SRTF (P)",
        "Round Robin (q=1 ms)": "RR (q=1)",
        "Round Robin (q=4 ms)": "RR (q=4)",
        "HRRN (Highest Response Ratio Next, Non-Preemptive)": "HRRN"
    }

    with open(csv_file, mode='r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            raw_name = row["Algorithm"]
            algorithms.append(label_map.get(raw_name, raw_name))
            avg_wt.append(float(row["Avg_WT"]))
            avg_tat.append(float(row["Avg_TAT"]))
            avg_rt.append(float(row["Avg_RT"]))
            ctx_switches.append(int(row["Context_Switches"]))

    os.makedirs("graphs", exist_ok=True)
    colors = ['#2b5c8f', '#3b82f6', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6']

    # 1. Combined 2x2 Dashboard (Perfect for 2-Page Report)
    fig, axes = plt.subplots(2, 2, figsize=(13, 10))
    fig.suptitle("CPU Scheduling Algorithms Performance Comparison", fontsize=16, fontweight='bold')

    # Chart 1: Average Waiting Time
    ax1 = axes[0, 0]
    bars1 = ax1.bar(algorithms, avg_wt, color='#3b82f6', edgecolor='black', alpha=0.85)
    ax1.set_title("Average Waiting Time (ms) - Lower is Better", fontweight='bold')
    ax1.set_ylabel("Time (ms)")
    ax1.bar_label(bars1, fmt='%.2f', padding=3, fontweight='bold')
    ax1.grid(axis='y', linestyle='--', alpha=0.5)

    # Chart 2: Average Turnaround Time
    ax2 = axes[0, 1]
    bars2 = ax2.bar(algorithms, avg_tat, color='#10b981', edgecolor='black', alpha=0.85)
    ax2.set_title("Average Turnaround Time (ms) - Lower is Better", fontweight='bold')
    ax2.set_ylabel("Time (ms)")
    ax2.bar_label(bars2, fmt='%.2f', padding=3, fontweight='bold')
    ax2.grid(axis='y', linestyle='--', alpha=0.5)

    # Chart 3: Average Response Time
    ax3 = axes[1, 0]
    bars3 = ax3.bar(algorithms, avg_rt, color='#f59e0b', edgecolor='black', alpha=0.85)
    ax3.set_title("Average Response Time (ms) - Lower is Better", fontweight='bold')
    ax3.set_ylabel("Time (ms)")
    ax3.bar_label(bars3, fmt='%.2f', padding=3, fontweight='bold')
    ax3.grid(axis='y', linestyle='--', alpha=0.5)

    # Chart 4: Context Switches
    ax4 = axes[1, 1]
    bars4 = ax4.bar(algorithms, ctx_switches, color='#ef4444', edgecolor='black', alpha=0.85)
    ax4.set_title("Context Switches (Count) - Lower is Better", fontweight='bold')
    ax4.set_ylabel("Number of Switches")
    ax4.bar_label(bars4, fmt='%d', padding=3, fontweight='bold')
    ax4.grid(axis='y', linestyle='--', alpha=0.5)

    for ax in axes.flat:
        ax.set_ylim(0, max([b.get_height() for b in ax.patches]) * 1.18)

    plt.tight_layout()
    summary_path = os.path.join("graphs", "comparison_dashboard.png")
    plt.savefig(summary_path, dpi=300)
    plt.close()
    print(f"Generated comparison dashboard: {summary_path}")

    # Generate individual charts
    metrics_info = [
        ("Average Waiting Time (ms)", avg_wt, '#3b82f6', "avg_waiting_time.png", "Lower is Better"),
        ("Average Turnaround Time (ms)", avg_tat, '#10b981', "avg_turnaround_time.png", "Lower is Better"),
        ("Average Response Time (ms)", avg_rt, '#f59e0b', "avg_response_time.png", "Lower is Better"),
        ("Number of Context Switches", ctx_switches, '#ef4444', "context_switches.png", "Lower is Better")
    ]

    for title, vals, color, filename, subtitle in metrics_info:
        plt.figure(figsize=(8, 5))
        bars = plt.bar(algorithms, vals, color=color, edgecolor='black', alpha=0.85)
        plt.title(f"{title} ({subtitle})", fontsize=13, fontweight='bold')
        plt.ylabel(title)
        plt.grid(axis='y', linestyle='--', alpha=0.5)
        plt.ylim(0, max(vals) * 1.2)
        plt.bar_label(bars, fmt='%.2f' if isinstance(vals[0], float) else '%d', padding=3, fontweight='bold')
        out_file = os.path.join("graphs", filename)
        plt.tight_layout()
        plt.savefig(out_file, dpi=300)
        plt.close()
        print(f"Generated chart: {out_file}")

if __name__ == "__main__":
    main()
