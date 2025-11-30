import matplotlib.pyplot as plt
import numpy as np
import re
from pathlib import Path
import scipy.stats as stats
import csv
file_paths = []
tests = np.arange(1, 16)

#Data processing
reg = re.compile(r'^\s*(?:\d+\s+){10}([0-9]*\.?[0-9]+)\s+(\d+/\d+)\s+(?:\*\s*)?(.+?)\s*$')
tests = ['Frequency', 'BlockFrequency', 'CumulativeSums', 'Runs', 'LongestRun', 'Rank', 'DFT', 'NonOverlappingTemplate','OverlappingTemplate', 'Universal', 'ApproximateEntropy', 'RandomExcursions', 'RandomExcursionsVariant', 'Serial', 'LinearComplexity'] 

def summarize_multi_tests(results):
    '''
    Summarize tests of one type to one value, only keeping the lowest p-value entry.
    '''
    test_results = []
    for test in tests:
        indexes = [i for i, x in enumerate(results['test']) if x == test]
        p_values = [results['p_value'][i] for i in indexes]
        min_test = np.argmin(p_values)
        
        test_result = 'Passed'
        # Keep only the entry with the lowest p-value
        for i in sorted(indexes, reverse=True):
            #check if a test was failed based on p-value and proportion
            _, lower, _ = calculate_bounds(num_sequences=results['num_sequences'][i])
            if results['p_value'][i] < 0.0001 or results['pass_fraction'][i] < lower:
                test_result = 'Failed'
                
            if i != indexes[min_test]:
                results['test'].pop(i)
                results['p_value'].pop(i)
                results['proportion'].pop(i)
                results['pass_fraction'].pop(i)
                results['num_sequences'].pop(i)
        test_results.append(test_result)
    results['result'] = test_results
    return results
    
def parse_nist_results(filepath):
    """
    Parse NIST results text file and extract test name, p-value and proportion.
    
    output:
    - results: dicts with keys: tests, p_value, proportion, pass_fraction and lists of values as items.
    """
    filepath = Path(filepath)
    results = {
                'test': [],
                'p_value': [],
                'proportion': [],
                'pass_fraction': [], 
                'num_sequences': []}

    with filepath.open('r', encoding='utf-8', errors='ignore') as file:
        for line in file:
            relevant_line = reg.match(line)
            if not relevant_line:
                continue
            p_value = float(relevant_line.group(1))
            proportion = relevant_line.group(2)
            prop = proportion.split('/') 
            num_passed = int(prop[0])
            total_tests = int(prop[1]) 
            propotion_decimal = num_passed / total_tests   
            test_name = relevant_line.group(3).strip()

            results['test'].append(test_name)
            results['p_value'].append(p_value)
            results['proportion'].append(proportion)
            results['pass_fraction'].append(propotion_decimal)
            results['num_sequences'].append(total_tests)
    results =summarize_multi_tests(results)      
    return results

def calculate_bounds(num_sequences=80, alpha=0.01,):
    '''Calculate upper and lower bounds for proportion'''
    center_line = 1 - alpha
    margin_of_error = 3 * np.sqrt((center_line * (1 - center_line)) / num_sequences)
    upper_bound = center_line + margin_of_error
    lower_bound = center_line - margin_of_error
    return upper_bound, lower_bound, center_line

def proportion_plot(tests, proportions, num_sequences, result_type="Low"):
    '''Plots the proportion of p-values and the required threshold across all tests.'''
    plt.figure(figsize=(8, 5))
    plt.scatter(tests, proportions)
    upper_bound, lower_bound, center_line = calculate_bounds(num_sequences)
    if upper_bound > 1:
        upper_bound = 1
    plt.axhline(upper_bound, linestyle='--', color = 'gray')
    plt.axhline(center_line, linestyle='-', color = 'darkblue')
    plt.axhline(lower_bound, linestyle='--', color = 'gray')
    plt.xticks(tests)
    plt.xticks(rotation=45, ha='right')
    plt.ylim(min(min(proportions),lower_bound)-0.01, 1.003)
    plt.xlabel("Tests")
    plt.ylabel("Proportions")
    plt.title(f"P-value Proportion Plot {result_type} Ambient Noise")
    plt.tight_layout()
    plt.savefig(f"proportion_plot_{result_type}.png")


def results_table(results, result_type="Low", p_alpha=0.0001):
    """
    Create a visual table of parsed results and add a Pass/Fail column.
    Threshold (lower bound) is computed per-row using the number of sequences
    parsed from the proportion (e.g. "79/80" -> 80).
    """
    cols = ['Test', 'P-value', 'Proportion', 'Result']
    rows = []
    for name, p, prop, result in zip(results['test'], results['p_value'],
                                results['proportion'], results['result']):
        rows.append([name, f"{p:.6f}", prop, result])

    csv_path = f'results_table_{result_type}.csv'
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(cols)
            writer.writerows(rows)
    
if __name__ == "__main__":
    results_office = parse_nist_results('./Office/finalAnalysisReport.txt')
    proportion_plot(results_office['test'], results_office['pass_fraction'], 1000, result_type="Office")
    results_table(results_office, result_type="Office")