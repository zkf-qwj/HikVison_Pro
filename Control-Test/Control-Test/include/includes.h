#ifndef _INCLUDES_H_
#define _INCLUDES_H_
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <map>
using namespace std;

#include "json/autolink.h"
#include "json/config.h"
#include "json/features.h"
#include "json/forwards.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/value.h"
#include "json/writer.h"
#include "md5.h"

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
#pragma comment(lib,"json_vc71_libmtd.lib")

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
#define MAX_HTTP_LEN 1024 * 6
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
int HandleCarNumber(char *id, char *time, int flags, char *ip);//出入口回调函数
int connect_Hik(SOCKET *sclient, char *ip, int port);//连接http服务器
int send_recv_http(SOCKET sclient,char *jscontext,char *Out_Recv,const char *ip, const char *port, const char *path,const char *contenttype);//发送http请求，并接收请求结果
int GetHttpBody(struct evhttp_request *req, char *outBody,int outLen);//获取http body
LONG OperatCardNoFun(LONG lUserID,void *CallBack,int type);//获取所有卡id
void CALLBACK CallBack(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData);//长连接StartRemoteConfigint
void CALLBACK CallBack_v1(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData);
int test();
BOOL HandleCardSet(Json::Value rRoot,NET_DVR_CARD_CFG_V50 *struCond);
void GetDateTime(char *aTime, NET_DVR_TIME_EX *Date);
BOOL CALLBACK msgCallBack(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser);

#endif