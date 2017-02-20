#ifndef _CARDMANAGEMENT_H_
#define _CARDMANAGEMENT_H_
#include "includes.h"



LONG CM_SetOrGetCardNoFun(LONG lUserID,void *CallBack,int type);
void GetDateTime(char *aTime, NET_DVR_TIME_EX *Date);
BOOL CM_HandleCardSet(Json::Value rRoot,NET_DVR_CARD_CFG_V50 *struCond);
BOOL CALLBACK CM_AlarmCallBack(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser);
void CALLBACK CM_RemoteCallBack(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData);
#endif