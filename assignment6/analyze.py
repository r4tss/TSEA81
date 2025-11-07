#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt

MAX_ITERATIONS=2048
DELAY_IN_USECS=1000

sample_times = np.loadtxt("sample_times.txt", dtype="float")[:,0]
output = np.loadtxt("output.txt", dtype="int")
raw_data = np.fromfile("data.raw", dtype='int8')
raw_data = np.concatenate((raw_data, raw_data, raw_data, raw_data, raw_data, raw_data, raw_data, raw_data))


ideal_output = raw_data[0:(DELAY_IN_USECS * MAX_ITERATIONS):DELAY_IN_USECS]

plt.figure(0)

plt.plot(np.abs(np.diff(sample_times)))
plt.title('Time difference between samples');
plt.ylabel('Time difference [s]');
plt.xlabel('Sample no');


plt.figure(2)
plt.plot(output)
plt.title('Sampled input data');
plt.xlabel('Sample no');
plt.ylabel('Sample value');

plt.figure(3)
plt.plot(ideal_output - output[:,0]);
plt.title('Difference between ideally sampled data and experimental results');

plt.show()

