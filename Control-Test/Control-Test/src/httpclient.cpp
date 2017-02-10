#include "event2/event-config.h"  
#include "event2/event_compat.h"  
#include "event2/event.h"  
#include "event2/util.h"  
#include "event2/bufferevent.h"  
#include "event2/dns.h"  
#include "event2/buffer.h"  
#include "http_client.h"  
#ifndef WIN32  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#endif  
#include <string.h>  
  
void write_cb(evutil_socket_t sock, short flags, void * args)  
{  
    struct http_client *httpc = (struct http_client *)args;   
    struct c_string * string = httpc->request_string(httpc);  
    int len = string->len(string);  
    int sent = 0;  
    int ret = 0;  
  
    printf("connected, write headers: %s\n", string->data);  
  
    ret = send(sock, string->data, len, 0);  
    while(ret != -1)  
    {  
        sent += ret;  
        if(sent == len) break;  
        ret = send(sock, string->data + sent, len - sent, 0);  
    }  
  
    delete_c_string(string);  
  
    event_add((struct event*)httpc->user_data[1], 0);  
}  
  
void read_cb(evutil_socket_t sock, short flags, void * args)  
{  
    struct http_client *httpc = (struct http_client*)args;  
    int ret = recv(sock, httpc->parse_buffer, PARSE_BUFFER_SIZE, 0);  
      
    printf("read_cb, read %d bytes\n", ret);  
    if(ret > 0)  
    {  
        httpc->process_data(httpc, httpc->parse_buffer, ret);  
    }  
    else if(ret == 0)  
    {  
        printf("read_cb connection closed\n");  
        event_base_loopexit((struct event_base*)httpc->user_data[0], NULL);  
        return;  
    }  
    if(httpc->finished(httpc) != 0)  
    {  
        event_add((struct event*)httpc->user_data[1], 0);  
    }  
}  
  
static evutil_socket_t make_tcp_socket()  
{  
    int on = 1;  
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
  
    evutil_make_socket_nonblocking(sock);  
#ifdef WIN32  
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof(on));  
#else  
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));  
#endif  
  
    return sock;  
}  
  
static struct http_client * make_http_client(struct event_base * base, const char *url)  
{  
    struct http_client * httpc = new_http_client();  
    /* initialize http client */  
    httpc->user_data[0] = base;  
    if(0 == httpc->parse_url(httpc, url) )  
    {  
        httpc->add_request_header(httpc, "Accept", "*/*");  
        httpc->add_request_header(httpc, "User-Agent", "test http client");  
        return httpc;  
    }  
  
    delete_http_client(httpc);  
    printf("parse url failed\n");  
    return 0;  
}  
  
int download(struct event_base * base, const char *url)  
{  
    evutil_socket_t sock = make_tcp_socket();  
    struct sockaddr_in serverAddr;  
    struct http_client * httpc = make_http_client(base, url);  
    struct event * ev_write = 0;  
    struct event * ev_read = 0;  
    struct timeval tv={10, 0};  
  
    if(!httpc) return -1;  
  
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(httpc->port);  
#ifdef WIN32  
    serverAddr.sin_addr.S_un.S_addr = inet_addr(httpc->host);  
#else  
    serverAddr.sin_addr.s_addr = inet_addr(httpc->host);  
#endif  
    memset(serverAddr.sin_zero, 0x00, 8);  
  
    connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));  
  
    ev_write = event_new(base, sock, EV_WRITE, write_cb, (void*)httpc);  
    ev_read = event_new(base, sock, EV_READ , read_cb, (void*)httpc);  
    httpc->user_data[1] = ev_read;  
    event_add(ev_write, &tv);  
  
    return 0;  
}  
  
  
int main(int argc, char** argv)  
{  
    struct event_base * base = 0;  
#ifdef WIN32  
    WORD wVersionRequested;  
    WSADATA wsaData;  
  
    wVersionRequested = MAKEWORD(2, 2);  
  
    (void) WSAStartup(wVersionRequested, &wsaData);  
#endif  
  
    if(argc < 2)  
    {  
        printf("usage: %s http://111.222.333.44:8080/xxx.htm\n    now only support ip.\n", argv[0]);  
        return 1;  
    }  
  
    base = event_base_new();  
  
    if( 0 == download(base, argv[1]) )  
    {  
        event_base_dispatch(base);  
        event_base_free(base);  
    }  
    else  
    {  
        printf("prepare download failed for %s\n", argv[1]);  
    }  
  
    return 0;  
}  