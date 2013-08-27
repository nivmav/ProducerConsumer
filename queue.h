#include <stdio.h>
#include <stdlib.h>
#include "my402list.h"
#include "warmup2.h"

#ifndef __QUEUE_H__
#define __QUEUE_H__

/*
 * Packet structure
 */

extern My402List Queue1;
extern My402List Queue2;

/*
 * Queue functions
 */
extern int pushQueue(int queueNum, Packet *);
extern Packet* popQueue(int queueNum);
extern int checkQueueEmpty(int queueNum);
extern Packet* firstQueue(int queueNum);
extern void unlinkQueue(int queueNum);
extern void removeQueue(int queueNum);

#endif
