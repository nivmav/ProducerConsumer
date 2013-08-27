#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "server.h"
#include "queue.h"
#include "my402list.h"
#include "warmup2.h"

/*
 * The server thread invokes this function
 */

void* serverFunc(void *arg)
{
    int count = 0;
    Packet *packet;
    long int timestamp;
    double time, time1;
    char msg[4] = "time";
    sigset_t sig;

    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sig, NULL);

    while(1){
        pthread_mutex_lock(&mutex);
        while(packetCount == 0 && !sigHit){
            pthread_cond_wait(&processQueue, &mutex);
        }
        if(sigHit) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
            return 0;
        }
        packetCount--;
        packet = popQueue(2);
        Stat.pack_S++;
        time = (double)(getTime() - packet->Q2)/1000.0;
        Stat.Q2 = Stat.Q2 + time;
        packet->server = getTime();
        fprintf(stdout,"%012.3lfms: p%d begin service as S, time in Q2 = %lfms\n", 
                relativeTimeinDouble(getTime()), packet->val, time);
        count++;
        timestamp = packet->mu;
        pthread_mutex_unlock(&mutex);
        timestamp = timestamp - (getTime() - packet->server);
        usleep(timestamp);
        time = (double)(getTime() - packet->server)/1000.0;
        time1 = (double)(getTime() - packet->start_time)/1000.0;
        Stat.service = Stat.service + time;
        Stat.tot = Stat.tot + time1;
        Stat.sqtot = Stat.sqtot + (time1*time1);
        fprintf(stdout,"%012.3lfms: p%d departs from S, service time = %lfms,\n %19s in system = %lfms\n",
                relativeTimeinDouble(getTime()), packet->val, time, msg, time1);

        if(count == Param.packet_num) {
            free(packet);
            removeQueue(2);
            pthread_exit((void*)0);
            return (void*)0;
        }
        free(packet);
    }
}

int wakeupServer()
{
    packetCount++;
    if(sigHit){
        packetCount = 0;
        pthread_cond_broadcast(&processQueue);
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    pthread_cond_broadcast(&processQueue);
    pthread_mutex_unlock(&mutex);
    return 1;
}

