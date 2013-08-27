#include "token.h"

int pushToken()
{
    My402ListAppend(&tokList, (void *)0);
    return 1;
}


int checkToken(int num)
{
    if(tokList.num_members >= num){
        return 1;
    }
    return 0;
}

int removeTokens(int num)
{
    int i;

    if(!num) {
        My402ListUnlinkAll(&tokList);
        return 1;
    }

    My402ListElem *tempTok, *tempTok1;
    tempTok = My402ListFirst(&tokList);
    for(i = 0; i<num ; i++){
        tempTok1 = tempTok;
        tempTok = My402ListNext(&tokList, tempTok);
        My402ListUnlink(&tokList, tempTok1);
    }
    return 1;
}
