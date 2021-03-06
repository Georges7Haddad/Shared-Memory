﻿
# Shared-Memory



This is a programming model called worker/reducer, which is a simplified version of MapReduce, implemented on Linux. This model is used to concurrently count the occurrences of CMPS, CCE, ECE in a large file: *input.txt*, which consists of 320000 lines.

It's a direct inter-process communication using shared memory and an optimization using multithreading

## Shm
1. The main process takes a command-line argument denoting the number of splits N. If the number of splits is equal to 4, the file has to be split into 4 parts, each consisting of 320000/4 lines. Then, the main process has to spawn a new process that does the split, and then it waits for the termination of its child. 

2. The main process creates N shared memories of size equal to 4 × sizeof(long).
3. The main process creates N worker processes and 3 reducer processes.
4. Worker process i counts the occurrences of CMPS, CCE, ECE in split i (i.e., outputi). When it is done, it writes 999 (which is a special flag denoting the termination of the worker) and the occurrence of CMPS, the occurrence of CCE, and the occurrence of ECE in shared memory.
5. First, (resp. second and third) reducer process sums the occurrence of CMPS (resp. CCE and ECE) generated by all the worker processes. For this, it has to keep checking the first flag to be equal to 999 before reading the corresponding occurrence of CMPS (resp. CCE and ECE).

## Optimization
To improve the performance of the above implementation, we'll use multithreading to eliminate:

 1. The creation of the shared memory and use instead global variable
 
2. The split-phase done by the first child process. 

For this, when the main process takes the number of splits **N**:

- We create **N** threads modeling mappers, and 3 threads modeling reducers.

- Each mapper opens the original file and uses lseek to
explicitly set the file's offset. That way, each mapper thread is responsible of a portion of the file.


## Bench 
This program evaluate the worker/reducer model by considering **1**, **2**, **4** and **8** splits for both scenarios: not optimized (processes with child split process) and optimized (multithreading without splitting). It stores their runtime in 2 different files *temp.txt* and *temp2.txt* then merge them in *bench.txt*.

