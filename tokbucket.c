#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "tokbucket.h"
#include "server.h"
#include "queue.h"
#include "warmup2.h"
#include "my402list.h"
#include "token.h"







void interrupt2()
{
    fprintf(stdout, "Token thread exiting\n");
    return;
}

void *handler2()
{
    struct sigaction act;
    sigset_t sig;
    act.sa_handler = interrupt2;
    sigaction(SIGUSR2, &act, NULL);
    sigemptyset(&sig);
    sigaddset(&sig, SIGUSR2);
    pthread_sigmask(SIG_UNBLOCK, &sig, NULL);
    return 0;
}


/*
 * The token thread invokes this function
 */

void * tokbucketFunc(void * arg)
{
    long int timestamp, start_time;
    double time;
    int cnt = 0;
    Packet *packet;
    sigset_t sig;
    char msg[5] = "token";

    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sig, NULL);

    handler2();

    start_time = emulationTime;
    while(1){
        cnt++;
        if(Param.r > 10000000){
            Param.r = 10000000;
        }
        if(sigHit){
            goto thread_exit;
        }
        timestamp = Param.r - (getTime() - start_time);
        if(timestamp <= 0){
            fprintf(stdout, "timestamp value is going negative, ignoring sleep\n"); 
        }else{ 
            usleep(timestamp);
        }
        start_time = getTime();
        if(sigHit){
            goto thread_exit;
        }

        pthread_mutex_lock(&mutex);
        Stat.tok++;
        if(tokList.num_members < Param.B){
            pushToken();
            fprintf(stdout,"%012.3lfms: token t%d arrives, token bucket now has %d tokens\n", 
                    relativeTimeinDouble(getTime()), cnt, tokList.num_members);
        } else{
            fprintf(stdout,"%012.3lfms: token t%d arrives, dropped\n",
                    relativeTimeinDouble(getTime()), cnt);
            Stat.tok_drop++;
        }


        if(!checkQueueEmpty(1)){
            packet = firstQueue(1);
            if(checkToken(packet->P)){
                removeTokens(packet->P);
                packet = popQueue(1);
                time = (double)(getTime() - packet->Q1)/1000.0;
                Stat.Q1 = Stat.Q1 + time;
                fprintf(stdout,"%012.3lfms: p%d leaves Q1, time in Q1 = %lfms,\n %20s bucket now has %d tokens\n", 
                        relativeTimeinDouble(getTime()), packet->val, time, msg, tokList.num_members); 
                pushQueue(2, packet);
                processedPacket++;
                packet->Q2 = getTime(); 
                fprintf(stdout,"%012.3lfms: p%d enters Q2\n", relativeTimeinDouble(getTime()), packet->val); 
                if(!wakeupServer() && sigHit){
                    pthread_exit(0);
                    return 0;
                }
            }else {
                pthread_mutex_unlock(&mutex);
            }
        }else{
            pthread_mutex_unlock(&mutex);
        }
        if(processedPacket == Param.packet_num){
            removeQueue(1);
            removeTokens(0);
            pthread_exit(0);
            return 0;
        }
    }

thread_exit:
            pthread_mutex_lock(&mutex);
            packetCount = 0;
            removeTokens(0);
            pthread_cond_broadcast(&processQueue);
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
            return 0;

}

