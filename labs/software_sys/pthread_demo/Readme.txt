COSC-560 Software Systems
Copyright Hongbiao yang
02/06/2014

How to use:
============================================
1. To run demo of thread lifecycle, use command:
make test1

This will demonstrate how to create, exit, join and cancel threads


2. To run demo of thread sychronization: semaphore
make test2

This will show an example on multiple application(threads) competing 
for limited resources


3. To run demo of thread sychronization: condition variable
make test3

This will show an example on how to use condition variable by both 
sending a signal to one thread (one lunch is ready), or broadcasting 
on all blocking threads (Enough pizzas for everybody are delivered).


To see a clean process / LWP tree for each demo program, just 
start another session and run the script runTest1.sh, like:
./runTest1.sh

This script is just one sentence to find the nice clean PID and LWP for
each thread while the multi-thread program is running. The results on 
OS other than Linux might expect different results


File List:
============================================
Source code: 
multi_thr.c
sem1.c 
cond1.c

Binary: 
============================================
multi_thr
sem1
cond1

Assisting files:
============================================
Makefile
runTest1.sh
runTest2.sh
runTest3.sh



