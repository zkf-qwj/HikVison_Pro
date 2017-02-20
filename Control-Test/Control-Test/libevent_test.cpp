/*
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#define WIN32
#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/event.h"
#include "event2/http.h""

#include <WinSock2.h>
#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libevent.lib")

int init_win_socket()
{
WSADATA wsaData;
if(WSAStartup(MAKEWORD(2,2) , &wsaData) != 0) 
{
return -1;
}
return 0;
}

void generic_handler(struct evhttp_request *req, void *arg)
{
printf("开始回调\n");
struct evbuffer *buf = evbuffer_new();
if(!buf)
{
puts("failed to create response buffer \n");
return;
}
Sleep(5000);
printf("---recv: %s\n",evhttp_request_get_uri(req));

evbuffer_add_printf(buf, "Server Responsed. Requested: %s\n", "hello word");
evhttp_send_reply(req, HTTP_OK, "OK", buf);
evbuffer_free(buf);
}

int main(int argc, char* argv[])
{
#ifdef WIN32
init_win_socket();
#endif

short          http_port = 8081;
char          *http_addr = "192.168.1.108";

struct event_base * base = event_base_new();

struct evhttp * http_server = evhttp_new(base);
if(!http_server)
{
return -1;
}

int ret = evhttp_bind_socket(http_server,http_addr,http_port);
if(ret!=0)
{
return -1;
}

evhttp_set_gencb(http_server, generic_handler, NULL);
//evhttp_set_cb(http_server, "abc", generic_handler,NULL);

printf("http server start OK! \n");

event_base_dispatch(base);

evhttp_free(http_server);

WSACleanup();
return 0;
}
*/







/*

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#define WIN32
#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif

#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/event.h"

#include <WinSock2.h>
#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libevent.lib")
static const char MESSAGE[] = "Hello, World!\n";

static const int PORT = 6699;


static void conn_writecb(struct bufferevent *bev, void *user_data)
{
	printf("信号写事件回调\n");
	Sleep(5000);
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) 
	{
		printf("flushed answer\n");
		bufferevent_free(bev);
	}
}

static void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	printf("信号事件回调\n");
	if (events & BEV_EVENT_EOF) 
	{
		printf("Connection closed.\n");
	} 
	else if (events & BEV_EVENT_ERROR) 
	{
		printf("Got an error on the connection: %s\n",
			strerror(errno));
	}

	bufferevent_free(bev);
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{	
	printf("信号事件回调\n");
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 2, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,struct sockaddr *sa, int socklen, void *user_data)
{
	printf("监听事件回调\n");
	struct event_base *base = (struct event_base *)user_data;
	struct bufferevent *bev;

	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) 
	{
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_disable(bev, EV_READ);

	bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}

int main(int argc, char **argv)
{
	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;

#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0202, &wsa_data);
#endif

	base = event_base_new();
	if (!base) 
	{
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) 
	{
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);

	if (!signal_event || event_add(signal_event, NULL)<0) 
	{
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}
*/



/*
#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
#pragma comment(lib,"HCNetSDk.lib") 
using namespace std;
typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindows;
int iNum = 0;

void main()
{
//---------------------------------------
//初始化
NET_DVR_Init();
//获取控制台窗口句柄
HMODULE hKernel32 = GetModuleHandle("kernel32");
GetConsoleWindows = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");
//设置连接时间与重连时间
NET_DVR_SetConnectTime(2000, 1);
//NET_DVR_SetReconnect(10000, true);
//---------------------------------------
//注册设备
LONG lUserID;
BOOL BRet;
NET_DVR_DEVICEINFO_V30 struDeviceInfo;
lUserID = NET_DVR_Login_V30("192.168.1.64", 8000, "admin", "@vsea.tv", &struDeviceInfo);
if (lUserID < 0)
{
printf("Login error, %d\n", NET_DVR_GetLastError());
NET_DVR_Cleanup();
return;
}
printf("----------------------login success---------------------\n");
char OutBuf[10240] = {0};
BRet = NET_DVR_GetDeviceAbility(lUserID,DEVICE_NETWORK_ABILITY,NULL,0,OutBuf,sizeof(OutBuf));
if (BRet == FALSE){
printf("GetDeviceAbility error, %d\n", NET_DVR_GetLastError());
return;
}
//printf("recv DeviceAbility:%s\n",OutBuf);
//关门
BRet = NET_DVR_ControlGateway(lUserID, -1, 0);
if(FALSE == BRet)
{
printf("Login error, %d\n", NET_DVR_GetLastError());
return;
}
Sleep(3000);
//开门
BRet = NET_DVR_ControlGateway(lUserID, -1, 1);
if(FALSE == BRet)
{
printf("Login error, %d\n", NET_DVR_GetLastError());
return;
}

//注销用户
NET_DVR_Logout(lUserID);
//释放 SDK 资源
NET_DVR_Cleanup();
printf("----------------------logout success---------------------\n");
while(1);

//注销用户
NET_DVR_Logout(lUserID);
//释放 SDK 资源
NET_DVR_Cleanup();
return;
}
*/