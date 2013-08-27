#include "queue.h"

int pushQueue(int queueNum, Packet *packet)
{
    if(queueNum == 1){
        My402ListAppend(&Queue1, packet);
    }else{
        My402ListAppend(&Queue2, packet);
    }
    return 1;
}

Packet *firstQueue(int queueNum)
{
    My402ListElem *temp;
    if(queueNum == 1){
        temp = My402ListFirst(&Queue1);
        return (Packet *)temp->obj;
    } else{
        temp = My402ListFirst(&Queue2);
        return (Packet *)temp->obj;
    }
}

Packet *popQueue(int queueNum)
{
    My402ListElem *Elem;
    Packet *packet;

    if(queueNum == 1){
        Elem = My402ListFirst(&Queue1);
    }else{
        Elem = My402ListFirst(&Queue2);
    }
    packet = (Packet *)Elem->obj;

    if(queueNum == 1){
        My402ListUnlink(&Queue1, Elem);
    }else {
        My402ListUnlink(&Queue2, Elem);
    }

    return packet;
}

int checkQueueEmpty(int queueNum)
{
    if(queueNum == 1) {
        if(Queue1.num_members == 0){
            return 1;
        }
    }else{
        if(Queue2.num_members == 0){
            return 1;
        }
    }
    return 0;
}

void unlinkQueue(int queueNum)
{
    My402ListElem *temp;
    Packet *packet;
    if(queueNum == 1){
        temp = My402ListNext(&Queue1, &(Queue1.anchor));
        while(temp) {
            packet = (Packet *) temp->obj;
            free(packet);
            temp = My402ListNext(&Queue1, temp);
        }
        My402ListUnlinkAll(&Queue1);
    }else {
        temp = My402ListNext(&Queue2, &(Queue2.anchor));
        while(temp) {
            packet = (Packet *) temp->obj;
            free(packet);
            temp = My402ListNext(&Queue2, temp);
        }
        My402ListUnlinkAll(&Queue2);
    }
}

void removeQueue(int queueNum)
{
    if(queueNum == 1){
        My402ListUnlinkAll(&Queue1);
    }else {
        My402ListUnlinkAll(&Queue2);
    }
}


