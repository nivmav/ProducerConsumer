Objective of the program:
To emualate a traffic shaper who transmits packets controlled by a token bucket filter

Files submitted:
1>my402list.h
2>my402list.c
3>cs402.h
4>warmup2.h
5>warmup2.c
6>arrivalt.h
7>arrivalt.c
8>tokbucket.h
9>tokbucket.c
10>server.c
11>server.h
12>queue.h
13>queue.c
14>token.h
16>token.c
17>Makefile

Commandline execution:
>./warmup2 [-lambda ][..] [-tfile]

Arrival thread: the arrival thread executes the function arrivalFunc. It creates packets. Incase of a file, reads each line during every loop to create a new packet.
The thread fowards it
accordingly to Q1 else Q2(if Q1 empty);
It takes care of deleting both the queue's during  "Ctrl+c".

The token thread, generates the token and regulates the traffic using the function tokbucketFunc.
It takes care of removing itself and Q1 during normal execution.

The server thread goes to sleep to simulate the processing of each packet.

Working:
usleep: is used to simulate: inter-arrival times between packets, inter-arrival times between tokens, and the processing of a packet by the server.

the mutex: protects queue (q1, q2), token bucket, and guarded variable operations

guarded variable with pthread_cond_wait/broadcast : is used to wake up the server to serve a process.

The main thread handles cntrl+c , and the remaining three threads block SIGINT. Main thread sets a flag sigHit =1, which when checked by each thread causes its exit.
Each thread do the required cleanup.


Situation:
Came across one situatuion when r is really high and hence inter-arrival time is low ~ 1microsecond or less and this causes the time given to usleep in negative.
as the bookeeping and printf statements take more time than the inter arrival time, in such cases I have avoided usleep rather ignore to sleep.

