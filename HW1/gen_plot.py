#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import re


timing_file = "timing.txt"
print("Timing file:", timing_file)

process_times = {}
thread_times = {}


def calc_stats(times):
    X = np.array(sorted(times.keys()))
    Y_mean = []
    Y_std = []
    for x in X:
        ts = np.array(times[x])
        Y_mean.append(np.mean(ts))
        Y_std.append(np.std(ts))
    Y_mean = np.array(Y_mean)
    Y_std = np.array(Y_std)
    return X, Y_mean, Y_std


with open(timing_file) as f:
    for line in f:
        line = line.strip()
        if len(line) == 0:
            continue

        if line.startswith('n'):
            num_data, num_processes, use_thread = re.findall(r'\d+', line)
            num_data = int(num_data)
            num_processes = int(num_processes)
            use_thread = int(use_thread) > 0

            # print(num_processes, use_thread)
            if use_thread:
                thread_times[num_processes] = []
            else:
                process_times[num_processes] = []
        else:
            time = float(line)

            if use_thread:
                thread_times[num_processes].append(time)
            else:
                process_times[num_processes].append(time)

fig = plt.figure(facecolor='white')
ax = fig.add_subplot(111)
x, y_mean, y_std = calc_stats(process_times)
ax.errorbar(x, y_mean, yerr=y_std, label='process')
x, y_mean, y_std = calc_stats(thread_times)
ax.errorbar(x, y_mean, yerr=y_std, label='thread')
ax.legend()
ax.set_ylabel('time [s]')
plt.xticks(x)
plt.grid(True)
plt.show()




