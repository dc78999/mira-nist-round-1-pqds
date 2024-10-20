import re
import os
import pandas as pd
import matplotlib.pyplot as plt

# Regular expressions to capture the relevant metrics
patterns = {
    "signature_size": re.compile(r"smlen = (\d+)"),
    "cycles": re.compile(r"Total Cycles: (\d+)"),
    "instructions": re.compile(r"Total Instructions: (\d+)"),
    "l1_misses": re.compile(r"L1 Data Cache Misses: (\d+)"),
    "l2_misses": re.compile(r"L2 Data Cache Misses: (\d+)")
}

# Function to extract metrics from a file
def extract_metrics(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    signature_sizes = [int(m.group(1)) for m in patterns["signature_size"].finditer(content)]
    total_cycles = [int(m.group(1)) for m in patterns["cycles"].finditer(content)]
    total_instructions = [int(m.group(1)) for m in patterns["instructions"].finditer(content)]
    l1_misses = [int(m.group(1)) for m in patterns["l1_misses"].finditer(content)]
    l2_misses = [int(m.group(1)) for m in patterns["l2_misses"].finditer(content)]

    return {
        "signature_size": signature_sizes,
        "total_cycles": total_cycles,
        "total_instructions": total_instructions,
        "l1_misses": l1_misses,
        "l2_misses": l2_misses
    }

# Function to calculate averages and display a summary
def calculate_averages(metrics):
    summary = {
        "average_signature_size": sum(metrics["signature_size"]) / len(metrics["signature_size"]),
        "average_total_cycles": sum(metrics["total_cycles"]) / len(metrics["total_cycles"]),
        "average_total_instructions": sum(metrics["total_instructions"]) / len(metrics["total_instructions"]),
        "average_l1_misses": sum(metrics["l1_misses"]) / len(metrics["l1_misses"]),
        "average_l2_misses": sum(metrics["l2_misses"]) / len(metrics["l2_misses"]),
    }
    return summary

# Function to analyze each MIRA configuration
def analyze_mira(config_files):
    all_metrics = {}

    for config, file_path in config_files.items():
        metrics = extract_metrics(file_path)
        averages = calculate_averages(metrics)
        all_metrics[config] = averages

    return all_metrics

# Plotting function for visualization
def plot_metrics(metrics, title):
    df = pd.DataFrame(metrics).T
    df.plot(kind='bar', figsize=(10, 6))
    plt.title(title)
    plt.ylabel('Metric Averages')
    plt.xlabel('Configurations')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

# Main function to process the KAT results
def main():
    config_files = {
        "MIRA-128F": "mira128f-kat.txt",
        "MIRA-128S": "mira128s-kat.txt",
        "MIRA-192F": "mira192f-kat.txt",
        "MIRA-192S": "mira192s-kat.txt",
        "MIRA-256F": "mira256f-kat.txt",
        "MIRA-256S": "mira256s-kat.txt"
    }

    # Analyze each configuration
    metrics = analyze_mira(config_files)

    # Print summary
    for config, summary in metrics.items():
        print(f"\n{config} Performance Summary:")
        print(f"  Average Signature Size: {summary['average_signature_size']:.2f} bytes")
        print(f"  Average Total Cycles: {summary['average_total_cycles']:.2f}")
        print(f"  Average Total Instructions: {summary['average_total_instructions']:.2f}")
        print(f"  Average L1 Data Cache Misses: {summary['average_l1_misses']:.2f}")
        print(f"  Average L2 Data Cache Misses: {summary['average_l2_misses']:.2f}")

    # Visualize the data
    plot_metrics(metrics, "Average Performance Metrics Across MIRA Configurations")

if __name__ == "__main__":
    main()

