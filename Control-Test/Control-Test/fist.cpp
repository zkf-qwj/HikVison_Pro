/*
#include "includes.h"
#include "CarNumber.h"
int iNum = 0;
int g_count = 0;
LONG g_DlUserID = 0;
LONG g_ClUserID = 0;
short          http_port = 8081;
char          *http_addr = "192.168.1.106";
struct event_base *base;
struct evhttp *http_server;

void generic_handler(struct evhttp_request *req, void *arg)
{

pthread_t  *Thread_Params = (pthread_t *)arg;

struct evbuffer *buf = evbuffer_new();
if(!buf)
{  
puts("failed to create response buffer \n");
return;
}

LONG *lUserID = Thread_Params->lUserID;

//printf("---recv: %s\n",evhttp_request_get_uri(req));
//char *recv_string = evhttp_request_get_uri(req);
char pOutBuf[10240] = {0};

if (NET_DVR_GetDeviceAbility(lUserID[0],DEVICE_SOFTHARDWARE_ABILITY,NULL,0,pOutBuf,sizeof(pOutBuf)) < 0){
printf("Login error, %d\n", NET_DVR_GetLastError());
return ;
}
evbuffer_add_printf(buf, "Server Responsed. Requested: %s\n", pOutBuf);
evhttp_send_reply(req, HTTP_OK, "OK", buf);
evbuffer_free(buf);
}

unsigned int __stdcall DoorThreadFun(PVOID pM)
{	
g_count++;
printf(" child thread beginthread ID:%d,g_count:%d\n",GetCurrentThreadId(),g_count);
return 0;
}
unsigned int __stdcall CameraThreadFun(PVOID pM)
{		
int iRet = -1;
long m_login = 0;
CarNumber *pThis = new CarNumber();
iRet = pThis->InitSvc();
m_login = pThis->InitCarNunber();
pThis->StartCar(test);
while(1);
return 0;
}
unsigned int __stdcall evThreadFun(PVOID pM)
{
NET_DVR_Init();
NET_DVR_SetConnectTime(2000, 1);
pthread_t *Thread_Params = (pthread_t *)malloc(sizeof(pthread_t));
if (NULL == Thread_Params){
printf("malloc error in evThreadFun\n");
return -1;
}
NET_DVR_DEVICEINFO_V30 struDeviceInfo;
LONG lUserID = NET_DVR_Login_V30("192.168.1.64", 8000, "admin", "@vsea.tv", &struDeviceInfo);
if (lUserID < 0)
{
printf("Login error, %d\n", NET_DVR_GetLastError());
NET_DVR_Cleanup();
return -1;
}
Thread_Params->lUserID[0] = lUserID;
Thread_Params->ip[0] = "192.168.1.64";

lUserID = NET_DVR_Login_V30("192.168.1.66", 8000, "admin", "@vsea.tv", &struDeviceInfo);
if (lUserID < 0)
{
printf("Login error, %d\n", NET_DVR_GetLastError());
NET_DVR_Cleanup();
return -1;
}
Thread_Params->lUserID[1] = lUserID;
Thread_Params->ip[1] = "192.168.1.66";
//http server
event_base * base = event_base_new();
evhttp * http_server = evhttp_new(base);
if(!http_server)
{
return -1;
}
int ret = evhttp_bind_socket(http_server,http_addr,http_port);
if(ret!=0)
{
return -1;
}
evhttp_set_gencb(http_server, generic_handler, (void *)Thread_Params);
printf("http server start OK! \n");
event_base_dispatch(base);
evhttp_free(http_server);
WSACleanup();
NET_DVR_Logout(g_DlUserID);
NET_DVR_Logout(g_ClUserID);
NET_DVR_Cleanup();
return 0;
}

void main()
{

init_win_socket();
NET_DVR_Init();
NET_DVR_SetConnectTime(2000, 1);
LONG lUserID;
short          http_port = 8081;
char          *http_addr = "192.168.1.106";

NET_DVR_DEVICEINFO_V30 struDeviceInfo;

//创建线程
const int THREAD_NUM = 3;
HANDLE handle[THREAD_NUM]; 
handle[0] =  (HANDLE)_beginthreadex(NULL, 0, CameraThreadFun, NULL, 0, NULL);
handle[1] =  (HANDLE)_beginthreadex(NULL, 0, DoorThreadFun, NULL, 0, NULL);
handle[2] =  (HANDLE)_beginthreadex(NULL, 0, evThreadFun,NULL, 0, NULL);
WaitForMultipleObjects(THREAD_NUM, handle, TRUE ,INFINITE); 
return;
}
int init_win_socket()
{
WSADATA wsaData;
if(WSAStartup(MAKEWORD(2,2) , &wsaData) != 0) 
{
return -1;
}
return 0;
}
int Login_Init(Login_params *LP_Params,WORD num,LONG *lOutUserID)
{
NET_DVR_Init();
NET_DVR_SetConnectTime(2000, 1);
for (int i = 0; i < num; i++){
lOutUserID[i] = NET_DVR_Login_V30(LP_Params->sDVRIP[i], LP_Params->wDVRPort[i],LP_Params->sUserName[i], LP_Params->sPassword[i], LP_Params->lpDeviceInfo[i]);
if (lOutUserID[i] < 0)
{
printf("Login error ip:%s, %d\n",LP_Params->sDVRIP[i], NET_DVR_GetLastError());
NET_DVR_Cleanup();
return -1;
}
}	
return 0;
}
int test(char *id, char *time, int flags, char *ip)
{
printf("id;%s,time:%s,flags:%d,ip:%s\n",id,time,flags,ip);
return 1;
}
*/