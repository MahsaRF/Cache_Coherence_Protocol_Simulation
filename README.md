# Cache Coherence Protocol Simulator in C++

Project Overview 

In this project, I have implemented a cache coherence protocol simulator in C++ to for MSI, MESI and Dragon 
Protocol (the C++ source code named "cache"). Analyzed the effect of changing the configuration such as the number of processors, cache size, block size 
and associativity on the different protocol’s performance reported in the project evaluation document. Observed 
different statistics such as the number of read access misses, cache to cache transfers and the number of memory 
transactions to compare these protocols.

Sample run command for a cache :

 ./smp_cache 8192 8 64 4 MSI canneal.04t.longTrace 

===== 506 SMP Simulator configuration =====

L1_SIZE:		8192

L1_ASSOC:		8

L1_BLOCKSIZE:		64

NUMBER OF PROCESSORS:	4

COHERENCE PROTOCOL:	MSI

TRACE FILE:		traces/canneal.04t.longTrace

 
This project was submitted as part of a graduate level course (ECE 506: Architecture of Parallel Computers)
taught by Professor Yan Solihin at North Carolina State University.
