#include "my402list.h"

#ifndef __TOKEN_H__
#define __TOKEN_H__

extern My402List tokList;
//extern Token* popToken();
extern int pushToken();
extern int checkToken(int num);
extern int removeTokens(int num);

#endif
