#ifndef _INCLUDES_H_
#define _INCLUDES_H_
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
using namespace std;

#define WIN32
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/event.h"
#include "event2/http.h"
#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libevent.lib")

#include "HCNetSDK.h"
#pragma comment(lib,"HCNetSDk.lib") 

#include <WinSock2.h>
#include "Windows.h"
#include <process.h>
//thread args
typedef struct {
	LONG lUserID[20];
	WORD wLen;
	WORD wCommand;
	void *arg;
	char *ip[16];
} pthread_t;
#define MAX_DEVICE_LEN 20

typedef struct 
{
	char *sDVRIP[MAX_DEVICE_LEN];
	WORD wDVRPort[MAX_DEVICE_LEN];
	char *sUserName[MAX_DEVICE_LEN];
	char *sPassword[MAX_DEVICE_LEN];
	LPNET_DVR_DEVICEINFO_V30 lpDeviceInfo[MAX_DEVICE_LEN];
}Login_params;
int init_win_socket();
int Login_Init(Login_params *LP_Params,WORD num,LONG *lOutUserID);
int test(char *id, char *time, int flags, char *ip);
#endif