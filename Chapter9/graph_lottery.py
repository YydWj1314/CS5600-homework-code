import subprocess
import re
from pathlib import Path
import matplotlib.pyplot as plt


# Different job lengths to test
job_lengths = [10, 20, 50, 100, 200, 500]

# Use multiple random seeds for each job length
seeds = range(1, 21)

# Store the average unfairness for each job length
avg_unfairness = []


def run_lottery(job_length, seed):
    """
    Run lottery.py with two jobs of equal length and equal ticket counts.
    Return the completion times of Job 0 and Job 1.
    """

    # Get the directory containing this script
    script_dir = Path(__file__).resolve().parent

    # lottery.py is expected to be in the same directory
    lottery_path = script_dir / "lottery.py"

    command = [
        "python3",
        str(lottery_path),
        "-l",
        f"{job_length}:100,{job_length}:100",
        "-s",
        str(seed),
        "-c",
    ]

    result = subprocess.run(
        command,
        capture_output=True,
        text=True
    )

    # Check whether lottery.py ran successfully
    if result.returncode != 0:
        print("Error running lottery.py:")
        print(result.stderr)
        raise RuntimeError("lottery.py failed")

    output = result.stdout

    # Match lines such as:
    # --> JOB 0 DONE at time 185
    pattern = r"JOB\s+(\d+)\s+DONE at time\s+(\d+)"

    matches = re.findall(pattern, output)

    completion_times = {}

    for job_id, finish_time in matches:
        completion_times[int(job_id)] = int(finish_time)

    # Make sure both completion times were found
    if 0 not in completion_times or 1 not in completion_times:
        print("Could not find completion times.")
        print("Command:")
        print(" ".join(command))

        print("\nOutput:")
        print(output)

        raise RuntimeError(
            f"Missing completion times: {completion_times}"
        )

    return completion_times[0], completion_times[1]


# Run the experiment for each job length
for length in job_lengths:

    unfairness_values = []

    for seed in seeds:

        finish_0, finish_1 = run_lottery(length, seed)

        # Unfairness is the absolute difference in completion times
        unfairness = abs(finish_0 - finish_1)

        unfairness_values.append(unfairness)

    # Compute the average unfairness across all seeds
    average = sum(unfairness_values) / len(unfairness_values)

    avg_unfairness.append(average)

    print(
        f"Job length = {length}, "
        f"Average unfairness = {average:.2f}"
    )


# Create the graph
plt.figure(figsize=(8, 5))

plt.plot(
    job_lengths,
    avg_unfairness,
    marker="o"
)

plt.xlabel("Job Length")
plt.ylabel("Average Unfairness (Completion Time Difference)")
plt.title("Lottery Scheduling Fairness")

plt.grid(True)
plt.tight_layout()

# Save the graph in the same directory as this script
output_path = Path(__file__).resolve().parent / "lottery_unfairness.png"

plt.savefig(output_path)

print(f"\nGraph saved to: {output_path}")

plt.show()