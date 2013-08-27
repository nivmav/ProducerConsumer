#include <pthread.h>
#include <stdio.h>


#ifndef __WARMUP2_H__
#define __WARMUP2_H__
/*
 * mutex to protect 
 * q1, q2, and the token
 */
extern pthread_mutex_t mutex;

extern int processedPacket;
extern int packetCount;

extern int sigHit;

typedef struct statistics{
    double arrival;
    double total_pack;
    double act_pack;
    int pack_drop;
    int tok_drop;
    int pack_Q1;
    double service;
    int pack_S;
    double Q1;
    double Q2;
    double tok;
    double tot;
    double sqtot;
}Statistics;

extern Statistics Stat;
/*
 * Condition variable
 */
extern pthread_cond_t processQueue;

extern long int getTime();

extern double relativeTimeinDouble(long int);

extern long int emulationTime;
/*
 * Structure to hold the 
 * input
 */
typedef struct packet {
    int val;
    long int lambda;
    long int r;
    int Bucket;
    int P;
    long int mu;
    int processed;
    long int start_time;
    long int Q1;
    long int Q2;
    long int server;
}Packet;

extern Packet *getPacket(FILE *fp, int *result);

typedef struct parameter{
    long int lambda;
    long int r;
    int B;
    int P;
    long int mu;
    int packet_num;
    char *tfile;
}Parameters;

extern Parameters Param;

#endif
