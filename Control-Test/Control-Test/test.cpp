#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include<cassert>
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
 #include<ws2tcpip.h>
#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libevent.lib")
#define LISTEN_PORT 6699
#define LIATEN_BACKLOG 32


static const int PORT = 6699;
//accept 回调函数
void do_accept_cb(evutil_socket_t listener, short event, void *arg);
//read callback function
void read_cb(struct bufferevent *bev, void *arg);
//error callback function
void error_cb(struct bufferevent *bev, short event,void *arg);
void wirte_cb(struct bufferevent *bev,void *arg);

void do_accept_cb(evutil_socket_t listener, short event, void *arg)
{
	//传入的event_base指针
	struct event_base *base = (struct event_base*)arg;
	//socket描述符
	evutil_socket_t fd;
	//声明地址
	struct sockaddr_in sin;
	//地址长度声明
	socklen_t slen = sizeof(sin);
	//接收客户端
	fd = accept(listener, (struct sockaddr *)&sin, &slen);
	if (fd < 0)
	{
		perror("error accept");
		return;
	}
	printf("ACCEPT: fd = %u\n", fd);
	////注册一个bufferevent_socket_new事件
	struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	////设置回掉函数
	bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
	////设置该事件的属性
	bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}
void read_cb(struct bufferevent *bev, void *arg)
{
	printf("-----read\n");

	#define MAX_LINE 256
	char line[MAX_LINE + 1];
	int n;
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//
	while (n = bufferevent_read(bev, line, MAX_LINE))
	{
		line[n] = '\0';
		printf("fd=%u, read line: %s\n", fd, line);
		//将获取的数据返回给客户端
		bufferevent_write(bev, line, n);
	}
	printf("server->client\n");
	char MESSAGE[] = "hello client..\n";
	if (-1 == (::send(fd, MESSAGE, strlen(MESSAGE), 0)))
	{
		printf("the net has a error occured..");
		return;
	}

}
void error_cb(struct bufferevent *bev, short event, void *arg)
{
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cout << "fd = " << fd << endl;
	if (event & BEV_EVENT_TIMEOUT)
	{
		printf("Timed out\n"); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	bufferevent_free(bev);
}
////write 回调函数
void write_cb(struct bufferevent *bev, void *arg)
{
	printf("写回调函数\n");
	char str[50];
	//通过传入参数bev找到socket fd
	evutil_socket_t fd = bufferevent_getfd(bev);
	//cin >> str;
	printf("输入数据！");
	scanf_s("%d", &str);
	bufferevent_write(bev, &str, sizeof(str));
}
void generic_handler(struct evhttp_request *req, void *arg)
{
	printf("开始回调http\n");
	struct evbuffer *buf = evbuffer_new();
	if(!buf)
	{
		puts("failed to create response buffer \n");
		return;
	}

	printf("---recv: %s\n",evhttp_request_get_uri(req));

	evbuffer_add_printf(buf, "Server Responsed. Requested: %s\n", "hello word");
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}
int main()
{
	int ret;
	evutil_socket_t listener;
	WSADATA Ws;
	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		return -1;
	}
	short          http_port = 8081;
	char          *http_addr = "192.168.1.106";
	listener = socket(AF_INET, SOCK_STREAM, 0);
	assert(listener > 0);
	evutil_make_listen_socket_reuseable(listener);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(LISTEN_PORT);
	if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		return 1;
	}
	if (listen(listener, 1000) < 0) {
		perror("listen");
		return 1;
	}
	printf("Listening...\n");
	evutil_make_socket_nonblocking(listener);
	struct event_base *base = event_base_new();
	struct event_base *httpbase = event_base_new();
	assert(base != NULL);
	assert(httpbase != NULL);
	struct evhttp * http_server = evhttp_new(httpbase);
	if(!http_server)
	{
		return -1;
	}
	struct event *listen_event;
	listen_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept_cb, (void*)base);
	ret = evhttp_bind_socket(http_server,http_addr,http_port);
	if(ret!=0)
	{
		return -1;
	}
	event_add(listen_event, NULL);
	evhttp_set_gencb(http_server, generic_handler, NULL);
	//event_base_dispatch(base);

	event_base_dispatch(httpbase);
	evhttp_free(http_server);
	WSACleanup();
	printf("The End.");
	return 0;
}

