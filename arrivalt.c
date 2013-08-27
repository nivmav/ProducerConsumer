#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "my402list.h"
#include "warmup2.h"
#include "arrivalt.h"
#include "server.h"
#include "queue.h"
#include "token.h"





void interrupt1()
{
    fprintf(stdout, "Arrival thread exiting\n");
    return;
}

void *handler1()
{
    struct sigaction act;
    sigset_t sig;
    act.sa_handler = interrupt1;
    sigaction(SIGUSR1, &act, NULL);
    sigemptyset(&sig);
    sigaddset(&sig, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &sig, NULL);
    return 0;
}


/*
 * The arrival thread starts executing this
 */
void* arrivalFunc(void * arg)
{
    Packet *packet;
    int result;
    int count = 0, pack_count = 0;
    double inter_arrival;
    long int timestamp, start_time, laststart_time;
    FILE *fp = NULL;
    sigset_t sig;

    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sig, NULL);

    handler1();

    fp = (FILE *)arg;

    laststart_time  = emulationTime;

    /*
     * Lets convert this into microsecond
     */

    start_time = emulationTime;
    while(1){
        if(fp){
            packet = getPacket(fp, &result);
            if(!packet) {
                sigHit = 1;
                fprintf(stdout, "\nFile: number of inputs does not match the foresaid number of packets\n\n");
                goto thread_exit;
            }
        } else{
            packet = getPacket(NULL, &result);
        }
        count++;
        pack_count++;

        packet->val = pack_count;

        if(packet->P > packet->Bucket){
            /*
             * Drop this packet
             */
            fprintf(stdout,"%012.3lfms: packet p%d arrives, needs %d tokens, dropped\n",
                    relativeTimeinDouble(getTime()), packet->val, packet->P);
            free(packet);
            Stat.pack_drop++;
            count --;
            Param.packet_num --;
            if(count == Param.packet_num){
                sigHit = 1;
                goto thread_exit;
            }
            continue;
        }
        timestamp = packet->lambda;

        if(sigHit){
            goto thread_exit;
        }
        timestamp = timestamp - (getTime() - start_time);
        if(timestamp <= 0){
            fprintf(stdout, "timestamp value is going negative value, ignoring sleep\n");
        } else{
            usleep(timestamp);
        }
        if(sigHit){
            goto thread_exit;
        }

        start_time = getTime();
        packet->start_time = start_time;
        inter_arrival =(double)(start_time - laststart_time)/1000.00;
        Stat.arrival = Stat.arrival + inter_arrival;
        Stat.act_pack++;
        fprintf(stdout,"%012.3lfms: p%d arrives, needs %d tokens, inter arrival time = %lfms\n", 
                relativeTimeinDouble(getTime()), packet->val, packet->P, inter_arrival);

        /*
         * Hold mutex
         */
        pthread_mutex_lock(&mutex);
        if(checkQueueEmpty(1)){
            if(checkToken(packet->P)){
                removeTokens(packet->P);
                fprintf(stdout,"%012.3lfms: p%d enters Q2\n", relativeTimeinDouble(getTime()), packet->val); 
                pushQueue(2, packet);
                packet->Q2 = getTime();
                processedPacket++;
                if(!wakeupServer() && sigHit){
                    pthread_exit(0);
                    return 0;
                }
            }else {
                fprintf(stdout,"%012.3lfms: p%d enters Q1\n", relativeTimeinDouble(getTime()), packet->val);
                pushQueue(1, packet);
                packet->Q1 = getTime();
                Stat.pack_Q1++;
                pthread_mutex_unlock(&mutex);
            }
        }else{
            fprintf(stdout,"%012.3lfms: p%d enters Q1\n", relativeTimeinDouble(getTime()), packet->val); 
            pushQueue(1, packet);
            Stat.pack_Q1++;
            packet->Q1 = getTime();
            pthread_mutex_unlock(&mutex);
        }
        if(sigHit){
            goto thread_exit;

        }
        laststart_time = start_time;
        if(count == Param.packet_num){
            break;
        }
    }
    pthread_exit((void *) 0);
    return (void *)0;

thread_exit:
            pthread_mutex_lock(&mutex);
            packetCount = 0;
            printf("Iam here\n");
            unlinkQueue(1);
            unlinkQueue(2);
            pthread_cond_broadcast(&processQueue);
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
            return 0;
}


