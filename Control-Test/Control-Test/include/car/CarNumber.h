#ifndef __CARNUMBER__
#define __CARNUMBER__

#include <string.h>
#include <windows.h>
#include <iostream>

#include "HCNetSDK.h"
//#include "plaympeg4.h"
#include <map>
#include <deque>
#pragma comment(lib, "calib.lib")
#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "HCCore.lib")
#pragma comment(lib, "HCNetSDK.lib")
#pragma comment(lib, "iconv.lib")
#pragma comment(lib, "libxml2.lib")
#pragma comment(lib, "PlayCtrl.lib")
#pragma comment(lib, "Sadp.lib")
#pragma comment(lib, "CarNumber.lib")



typedef struct CallInfo
{
	SOCKADDR_IN addr;
	int iSocket;
}CALL_INFO_ST;

typedef struct UsedChannel
{
	int iSocket;
	long lRealId;
	HANDLE hHandle;
	CALL_INFO_ST *pCall;
}USED_CHANNEL_ST;


typedef struct CarInfo
{
	char id[20];		//车牌号
	char time[64];		//出入时间：  年:月:日-时:分:秒
	int  flags;			//标识： 1：入   2：出

}CarInfo_ST;

typedef std::map<long, USED_CHANNEL_ST>USED_CHAN_MAP; 



class CarNumber
{
public:
	CarNumber();
	~CarNumber();
public:
	int InitSvc();
	//调用返回操作句柄，供上级调用
	long InitCarNunber();
	//开始轮询等待车牌识别事件
	void StartCar(int (*pCallBack)(char * id,char * time , int flags,char * ip));
	//退出程序
	void StopCar();

public:
	char * GetTime();
	//BOOL MessageCallbackNo2(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser);

private:
	char time_n[64];
	char m_szIp[32];
	char m_szUserName[32];
	char m_szPasswd[32];
	long m_lHandle;
	long m_Listen;
	int  m_port;
public:
	int (*Callback1)(char * id,char * time , int flags,char * ip);
};


#endif