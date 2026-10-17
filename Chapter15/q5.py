import subprocess
import re
import sys
from pathlib import Path
import matplotlib.pyplot as plt

# Get the directory containing q5.py
script_dir = Path(__file__).resolve().parent
relocation_path = script_dir / "relocation.py"

# Experiment settings
address_space = 1024
limits = range(0, address_space + 1, 128)
seeds = [1, 2, 3, 4, 5]
n = 100

for seed in seeds:
    fractions = []

    for limit in limits:
        result = subprocess.run(
            [
                sys.executable,
                str(relocation_path),
                "-s", str(seed),
                "-n", str(n),
                "-a", "1k",
                "-l", str(limit),
                "-c"
            ],
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            raise RuntimeError(result.stderr)

        # Extract virtual addresses from relocation.py output
        addresses = re.findall(
            r"VA\s+\d+:\s+0x[0-9a-fA-F]+\s+\(decimal:\s*(\d+)\)",
            result.stdout
        )

        if len(addresses) != n:
            raise RuntimeError(
                f"Expected {n} addresses, found {len(addresses)}.\n"
                f"Output:\n{result.stdout}"
            )

        # Count valid addresses
        valid = sum(int(va) < limit for va in addresses)
        fractions.append(valid / n)

    plt.plot(
        limits,
        fractions,
        marker="o",
        label=f"Seed {seed}"
    )

# Theoretical expected fraction
expected = [limit / address_space for limit in limits]

plt.plot(
    limits,
    expected,
    "k--",
    label="Expected"
)

plt.xlabel("Bounds Register (Limit)")
plt.ylabel("Fraction of Valid Addresses")
plt.title("Valid Address Fraction vs. Bounds Register")
plt.legend()
plt.grid(True)

# Save the graph in the same folder as q5.py
plt.savefig(script_dir / "q5.png", dpi=300)
plt.show()