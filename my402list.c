#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "my402list.h"


int My402ListLength(My402List *list)
{
    return list->num_members;
}

int My402ListEmpty(My402List *list)
{
    if(list->num_members == 0) {
        return 1;
    }
    return 0;
}

int My402ListAppend(My402List *list, void *obj)
{
    My402ListElem *listElem;

    listElem = malloc(sizeof(My402ListElem));
    if(!listElem){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }
    memset(listElem, 0, sizeof(My402ListElem));
    listElem->obj = obj;

    if(list->num_members == 0) {
        list->anchor.next = listElem;
        list->anchor.prev = listElem;
        listElem->next = &list->anchor;
        listElem->prev = &list->anchor;
        list->num_members = 1;
    } else {
        listElem->prev = list->anchor.prev;
        list->anchor.prev->next = listElem;
        list->anchor.prev = listElem;
        listElem->next = &list->anchor;
        ++(list->num_members);
    }

    return 1;
}

int  My402ListPrepend(My402List* list, void* obj)
{

    My402ListElem *listElem;

    listElem = malloc(sizeof(My402ListElem));
    if(!listElem){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }
    memset(listElem, 0, sizeof(My402ListElem));
    listElem->obj = obj;

    if(list->num_members == 0) {
        list->anchor.next = listElem;
        list->anchor.prev = listElem;
        listElem->next = &list->anchor;
        listElem->prev = &list->anchor;
        list->num_members = 1;
    } else {
        listElem->next = list->anchor.next;
        list->anchor.next->prev = listElem;
        list->anchor.next = listElem;
        listElem->prev = &list->anchor;
        ++(list->num_members);
    }
    return 1;
}



void My402ListUnlink(My402List *list, My402ListElem* listEle)
{
    My402ListElem *temp;

    temp = My402ListNext(list, &list->anchor);
    while(temp != listEle) {
        temp = My402ListNext(list, temp);
    }
    temp->next->prev = temp->prev;
    temp->prev->next = temp->next;

    free(temp);
    --(list->num_members);
}

void My402ListUnlinkAll(My402List *list)
{
    My402ListElem *temp, *temp1;

    temp = My402ListNext(list, &list->anchor);
    while(temp) {
        temp1 = temp;
        temp =  My402ListNext(list, temp);
        free(temp1);
    }
    list->anchor.next = &list->anchor;
    list->anchor.prev = &list->anchor;
    list->num_members = 0;
}

int My402ListInsertAfter(My402List* list, void *obj, My402ListElem* elem)
{

    My402ListElem *newelem;
    newelem = malloc(sizeof(My402ListElem));
    if(!newelem){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }
    memset(newelem, 0, sizeof(My402ListElem));

    if(!elem){
        My402ListAppend(list, obj);
    }

    newelem->obj = obj;
    if(My402ListNext(list, elem)){
        elem->next->prev = newelem;
        newelem->prev = elem;
        newelem->next = elem->next;
        elem->next = newelem;

    } else {
        elem->next = newelem;
        newelem->next = &list->anchor;
        newelem->prev = elem;
        list->anchor.prev = newelem;
    }
    ++(list->num_members);

    return 1;

}

int My402ListInsertBefore(My402List* list, void *obj, My402ListElem* elem)
{
    My402ListElem  *newelem;
    newelem = malloc(sizeof(My402ListElem));
    if(!newelem){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }
    memset(newelem, 0, sizeof(My402ListElem));
    newelem->obj = obj;

    if(!elem){
        My402ListPrepend(list, obj);
    }

    if(My402ListPrev(list, elem)) {
        elem->prev->next = newelem;
        newelem->prev = elem->prev;
        elem->prev = newelem;
        newelem->next = elem;
    } else{
        newelem->next = elem;
        elem->prev = newelem;
        newelem->prev = &list->anchor;
        list->anchor.next = newelem;
    }
   ++(list->num_members);

    return 1;

}

My402ListElem *My402ListFirst(My402List* list)
{
    if(list->anchor.next == &list->anchor){
        return NULL;
    }
    return list->anchor.next;

}

My402ListElem *My402ListLast(My402List* list)
{
    if(list->anchor.prev == &list->anchor){
        return NULL;
    }
    return list->anchor.prev;

}

My402ListElem *My402ListNext(My402List *list, My402ListElem* listElem)
{
    if(listElem->next == &list->anchor){
        return NULL;
    }
    return listElem->next;
}

My402ListElem *My402ListPrev(My402List *list, My402ListElem *listElem)
{
    if(listElem->prev == &list->anchor){
        return NULL;
    }
    return listElem->prev;
}

My402ListElem *My402ListFind(My402List *list, void* obj)
{
    My402ListElem *temp;

    temp = My402ListNext(list, &list->anchor);

    while(temp){
        if(temp->obj == obj) {
            return temp;
        }
        temp = My402ListNext(list, temp);
    }
    return NULL;
}

int My402ListInit(My402List *list)
{

    list->num_members = 0;
    list->anchor.next = &list->anchor;
    list->anchor.prev = &list->anchor;
    list->anchor.obj = NULL;

    return 1;
}


