#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include "my402list.h"
#include "warmup2.h"
#include "arrivalt.h"
#include "tokbucket.h"
#include "server.h"
#include "queue.h"
#include "token.h"

#define BUFLENGTH 1024
pthread_mutex_t mutex;
int processedPacket = 0;
int packetCount = 0;
int sigHit = 0;
long int emulationTime = 0;
pthread_cond_t processQueue;
Parameters Param;
pthread_t arrivalt, tokbucket, server;

Statistics Stat;

My402List Queue1;
My402List Queue2;
My402List tokList;

/*
 * Sets the default parameters
 */
void defaultParameters()
{
    double temp;

    /*
     * Default values
     */

    temp = 1000000/0.5;
    Param.lambda = (long int)temp;

    temp = 1000000/1.5;
    Param.r = (long int)temp;
    
    temp = 1000000/0.35;
    Param.mu = (long int)temp;
    
    Param.B = 10;
    Param.P = 3;

    Param.packet_num = 20;

}

/*
 * gets the file name
 */
int tParameter(char *optarg)
{
    int len;

    len = strlen(optarg) + 1;

    Param.tfile = malloc(sizeof( (strlen(optarg) + 1) * sizeof(char)));
    if(!Param.tfile){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }

    strncpy(Param.tfile, optarg, len);
    return 1;
}

/*
 * gets lambda value and stores its invers
 */
void lambdaParameter(char *optarg)
{
    double lambda;

    lambda = strtod(optarg, NULL);
    lambda = 1000000/lambda;
    Param.lambda = (long int)lambda;
}


void muParameter(char *optarg)
{
    double mu;
    
    mu = strtod(optarg, NULL);
    mu = 1000000/mu;
    Param.mu = (long int)mu;
}

void rParameter(char *optarg)
{
    double r;
    
    r = strtod(optarg, NULL);
    r = 1000000/r;
    Param.r = (long int)r;
}

void PParameter(char *optarg)
{
    Param.P = atoi(optarg);
}

void BParameter(char *optarg)
{
    Param.B = atoi(optarg);
}

void nParameter(char *optarg)
{
    Param.packet_num = atoi(optarg);
}



/*
 * Read input from a file
 * and setting the parameters to a 
 * Packet structure
 */

Packet * getPacket(FILE *fp, int *result)
{
    int num_fields = 0;
    Packet *packet;
    char line[BUFLENGTH];
    char *tok;

    packet = malloc(sizeof(Packet));
    if(!packet){
        fprintf(stderr, "Error in allocating memory for the packet: %s\n", strerror(errno));
        return NULL;
    }
    memset(packet, 0, sizeof(Packet));

    if(!fp){
        packet->lambda = Param.lambda;
        packet->mu = Param.mu;
        packet->r = Param.r;
        packet->P = Param.P;
        packet->Bucket = Param.B;
        return packet;
    }
    if(fgets(line, BUFLENGTH, fp) != NULL) {
        tok = strtok(line," \t");
        if(!tok) {
            fprintf(stdout,"The file provided does not have the right input format\n");
            free(packet);
            *result = 0;
            return NULL;
        }
        while (tok != NULL){
            if(num_fields == 0){
                packet->lambda = strtol(tok, NULL, 10) * 1000;
                num_fields++;
                tok = strtok(NULL, " \t");
                continue;
            }

            if(num_fields == 1){
                packet->P = atoi(tok);
                num_fields++;
                tok = strtok(NULL, " \t");
                continue;
            }

            if(num_fields == 2){
                packet->mu = strtol(tok, NULL, 10) * 1000;
                num_fields++;
                tok = strtok(NULL, " \t");
                continue;
            }
            
            if(num_fields == 3){
                *result = 0;
                return NULL;
            }
        }
        *result = 1;
        packet->r = Param.r;
        packet->Bucket = Param.B;
        return packet;
    }
    *result = 1;
    free(packet);
    return NULL;
}



/*
 * The getopt_long_only has been adapted from the following code 
 * http://www.gnu.org/software/libc/manual/html_node/Getopt.html
 * vvvvvvvvvvvvvvvv   Code Begins   vvvvvvvvvvvvvvvvvvvvvvvvvvv
 */

int readCommandline(int argc, char *argv[], double *lambda, double *mu, double *r)
{
    int c;

    while(1){
        int option_index = 0;
        static struct option long_options[] =
        {
            {"lambda", required_argument, 0, 'a'},
            {"mu", required_argument, 0, 'm'},
            {"r",  required_argument, 0, 'r'},
            {"B",  required_argument, 0, 'b'},
            {"P",  required_argument, 0, 'p'},
            {"n",  required_argument, 0, 'n'},
            {"t",  required_argument, 0, 't'},
            {0, 0, 0, 0}
        };
    
        c = getopt_long_only (argc, argv, "a:m:r:b:p:n:t:", long_options, &option_index);

        if(c == -1){
            break;
        }

        switch (c){

            case 'a':
                lambdaParameter(optarg);
                *lambda = strtod(optarg, NULL);
                continue;
            case 'm':
                muParameter(optarg);
                *mu = strtod(optarg, NULL);
                continue;
            case 'r':
                rParameter(optarg);
                *r = strtod(optarg, NULL);
                continue;
            case 'b':
                BParameter(optarg);
                continue;
            case 'p':
                PParameter(optarg);
                continue;
            case 'n':
                nParameter(optarg);
                continue;
            case 't':
                if(!tParameter(optarg)){
                    return 0;
                }
                continue;
            case '?':
                return 0;
            default:
                fprintf(stdout,"erraneous comandline option\n");
                return 0;
        }
    }

    return 1;
}

/*End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*
 * To check the validity of the file given
 */

int checkFile(char *file)
{
    FILE* fid;
    struct stat stat_buf;

    if(stat(file, &stat_buf) != 0){
        fprintf(stderr, "Error, in file status: %s\n", strerror(errno));
        return 0;
    }

    if (S_ISDIR (stat_buf.st_mode)) {
        fprintf(stderr, "Error, %s is a directory\n", file);
        return 0;
    }
    fid = fopen(file, "r");
    if(!fid) {
        fprintf(stderr, "warmup2: Could not open the file %s: %s\n",
                file, strerror(errno));
        return 0;
    }
    fclose(fid);
    return 1;

}

/*
 * This function prints the statistics of the emulation done
 */

void statistics(double totalTime)
{
    double avg_interval;
    double avg_service;
    double avg_Q1;
    double avg_Q2;
    double avg_S;
    double avg_sys, avg_sqsys;
    double var, std;
    double tok_prob, pack_prob;
    char s = ' ';

    if(!Stat.act_pack){
        avg_interval = 0;
    }else {
        avg_interval = Stat.arrival/Stat.act_pack;
        avg_interval = avg_interval/1000.0;
    } 
    if(!Stat.pack_S){
        avg_service = 0;
    }else{
        avg_service = Stat.service/Stat.pack_S;
        avg_service = avg_service/1000.0;
    }

    if(!totalTime){
        avg_Q1 = 0;
        avg_Q2 = 0;
        avg_S = 0;
    }else {
        avg_Q1 = Stat.Q1/totalTime;
        avg_Q2 = Stat.Q2/totalTime;
        avg_S = Stat.service/totalTime;
    }
    
    if(!Stat.pack_S){
        avg_sys = 0;
        avg_sqsys = 0;
    }else {
        avg_sys = Stat.tot/Stat.pack_S;
        avg_sys = avg_sys/1000.0;
        avg_sqsys = Stat.sqtot/Stat.pack_S;
        avg_sqsys = avg_sqsys/1000000.0;
    }
    var = avg_sqsys - (avg_sys*avg_sys);
    std = sqrt(var);

    if(!Stat.tok){
        tok_prob = 0;
    } else{
        tok_prob = Stat.tok_drop/Stat.tok;
    }
    if(!Stat.act_pack){
        pack_prob = 0;
    }else {
        pack_prob = Stat.pack_drop/Stat.total_pack;
    }

    fprintf(stdout, "\n\nStatistics:\n\n");

    fprintf(stdout, "%3c average packet inter-arrival time = %.6gsec\n", s, avg_interval);
    fprintf(stdout, "%3c average packet service time = %.6gsec\n\n", s, avg_service);


    fprintf(stdout, "%3c average number of packets in Q1 = %.6g\n", s, avg_Q1);
    fprintf(stdout, "%3c average number of packets in Q2 = %.6g\n", s, avg_Q2);
    fprintf(stdout, "%3c average number of packets at S = %.6g\n\n", s, avg_S);

    fprintf(stdout, "%3c average time a packet spent in system = %.6gsec\n", s, avg_sys);
    fprintf(stdout, "%3c standard deviation for time spent in system = %.6gsec\n\n", s, std);
    
    fprintf(stdout, "%3c token drop probability = %.6g\n", s, tok_prob);
    fprintf(stdout, "%3c packet drop probability = %.6g\n", s, pack_prob);
}


long int getTime()
{
    struct timeval now;
    long int t;

    (void)gettimeofday(&now, NULL);
    t = (now.tv_sec)*1000000 + now.tv_usec;
    return t;
}

double relativeTimeinDouble(long int time)
{
    double t;
    
    time = time - emulationTime;
    t = (double)time/1000.0;

    return t;
}

/*
 * Code has been adapted from the warmup2 slides
 * Author Bill Cheng
 *vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv-Code Begin-vvvvvvvvvvvvvvvvvvvv
 */

void interrupt()
{
    fprintf(stdout, "mainCtrl + C has been hit\n");
    pthread_kill(arrivalt, SIGUSR1);
    pthread_kill(tokbucket, SIGUSR2);
    sigHit = 1;
    return;
}

void *handler()
{
    struct sigaction act;
    sigset_t sig;
    act.sa_handler = interrupt;
    sigaction(SIGINT, &act, NULL);
    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &sig, NULL);
    return 0;
}

/*^^^END^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

int main(int argc, char *argv[])
{
    My402List list;
    int th1 = 0, th2 = 0, th3 = 0;
    void * status;
    FILE *fp = NULL;
    char line[BUFLENGTH];
    double lambda = 0, mu = 0, r = 0;

    My402ListInit(&list);
    
    defaultParameters();

    if(!readCommandline(argc, argv, &lambda, &mu, &r)){
        return 0;
    }
    

    /*
     * When Ctr + C is hit
     */
    handler();

    fprintf(stdout, "\n\nEmulation Parameters:\n");

    if(Param.tfile){
        if(!checkFile(Param.tfile)){
            return 0;
        }
        fp = fopen(Param.tfile, "r");
    }


    if(fp){
        fgets(line, BUFLENGTH, fp);
        Param.packet_num = atoi(line);
    } else{
        if(!lambda){
            fprintf(stdout, "    lambda = 0.5\n");
        }else {
            fprintf(stdout, "    lambda = %lf\n", lambda);
        }
        if(!mu){
            fprintf(stdout, "    mu = 0.35\n");
        } else{
            fprintf(stdout, "    mu = %lf\n", mu);
        }

        fprintf(stdout, "    P = %d\n", Param.P);
        
        fprintf(stdout, "    number to arrive = %d\n", Param.packet_num);
    }
    if(r){
        fprintf(stdout, "    r = %f\n", r);
    } else{
        fprintf(stdout, "    r = 1.5\n");
    }

    fprintf(stdout, "    B = %d\n", Param.B);

    if(fp){

        fprintf(stdout, "    tsfile = %s\n", Param.tfile);
    }
    
    Stat.total_pack = Param.packet_num;

    /*
     * Lets create the mutex
     */

    pthread_mutex_init(&mutex, NULL);

    /*
     * Initialize the condition 
     * variable
     */
    pthread_cond_init(&processQueue, NULL);

    /*
     * Initialse the queues and the buckets
     */

    My402ListInit(&Queue1);
    My402ListInit(&Queue2);
    My402ListInit(&tokList);

    /*
     * Log the start time
     */
    fprintf(stdout,"\n\n00000000.000ms: emulation begins\n");
    emulationTime = getTime();

    /*
     * create three different threads
     */
    th1 = pthread_create(&arrivalt, 0, arrivalFunc, fp);
    th2 = pthread_create(&tokbucket, 0, tokbucketFunc, NULL);
    th3 = pthread_create(&server, 0, serverFunc, NULL);

    /*
     * Join with the threads
     */
    pthread_join(arrivalt, &status);
    pthread_join(tokbucket, &status);
    pthread_join(server, &status);

    if(fp){
        fclose(fp);
    }

    statistics(relativeTimeinDouble(getTime()));
    free(Param.tfile);


    return 0;
}

