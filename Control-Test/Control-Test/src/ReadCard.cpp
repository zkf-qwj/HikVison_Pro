#include "ReadCard.h"
/*
读身份证信息
Name：姓名
*/
int RC_ID(char * Name, char * Gender, char * Folk,char *BirthDay, char * Code, char * Address,char *Agency, char * ExpireStart,char* ExpireEnd)
{
	printf("Code:%s\n",Code);
	return 0;
}
/*
读IC卡信息
ID 卡号，需要将卡号逆置取十进制
*/
int RC_IC(char * ID)
{
	printf("-----------------------IC-->%s\n", ID);
	return 0;
}