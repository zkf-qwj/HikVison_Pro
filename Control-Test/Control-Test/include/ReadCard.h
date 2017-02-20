#ifndef _READCARD_H_
#define _READCARD_H_
#include "includes.h"

int RC_IC(char * ID);
int RC_ID(char * Name, char * Gender, char * Folk,char *BirthDay, char * Code, char * Address,char *Agency, char * ExpireStart,char* ExpireEnd);
#endif