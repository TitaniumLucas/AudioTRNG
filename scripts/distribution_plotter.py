#!/usr/bin/env python3
import sys
import numpy as np
import matplotlib.pyplot as plt

def byte_distribution(filepath):
    """Return a NumPy array of 256 normalized byte frequencies."""
    with open(filepath, "rb") as f:
        data = np.frombuffer(f.read(), dtype=np.uint8)

    if data.size == 0:
        return np.zeros(256, dtype=float)

    # Count occurrences of each byte (0..255)
    counts = np.bincount(data, minlength=256)

    # Normalize
    return counts / data.size

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_bytes.py <file1> [file2 ...]")
        sys.exit(1)

    x = np.arange(256)  # byte values

    for filepath in sys.argv[1:]:
        dist = byte_distribution(filepath)
        plt.scatter(x, dist, s=10, label=filepath)

    log = True

    plt.xlabel("Byte value (0 - 255)")
    plt.xticks([i * 32 for i in range(9)])
    plt.ylabel("Normalized frequency" + (" (log)" if log else ""))
    if log:
        plt.yscale("log")
    plt.title("Byte Distribution")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
