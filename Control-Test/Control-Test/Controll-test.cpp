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
	Json::Value root,rRoot;
	Json::FastWriter fw;
	Json::Reader reader;
	pthread_t  *Thread_Params = (pthread_t *)arg;
	LONG *lUserID = Thread_Params->lUserID;
	char body[4096] = {0};
	int iRet = 0;
	char httpstatus[1024] = {0};
	strcpy(httpstatus,"success");
	struct evbuffer *buf = evbuffer_new();
	if(!buf)
	{  
		puts("failed to create response buffer \n");
		return;
	}
	const char *uri = evhttp_request_get_uri(req);
	if ((iRet = GetHttpBody(req,body,sizeof(body))) < 0){
		printf("Get Http Body error :%d\n",iRet);
		strcpy(httpstatus,"Get http body faild");
		goto SendHttp;
	}
	if (reader.parse(body, rRoot) == false){
		printf("parse http body faild\n");
		strcpy(httpstatus,"parse http body faild");
		goto SendHttp;
	}
	if (strstr(uri,"door")){
		if (strstr(uri,"control")){
			printf("ip:%s,channel:%s,cardnum:%s,status:%s\n",rRoot["ip"].asString().c_str(),
				rRoot["channel"].asString().c_str(),
				rRoot["cardnum"].asString().c_str(),
				rRoot["status"].asString().c_str()
				);
		}else if (strstr(uri,"setconfig")){

		}else if (strstr(uri,"getconfig")){

		}
	}else if (strstr(evhttp_request_get_uri(req),"card")){

	}
SendHttp:
	root["HttpStatus"] = Json::Value(httpstatus);
	std::string strData = fw.write(root);
	char *pJsonData = (char *)strData.c_str();

	evbuffer_add_printf(buf, "%s\n",pJsonData);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

unsigned int __stdcall DoorThreadFun(PVOID pM)
{	
	test();
	printf(" child thread beginthread ID:%d,g_count:%d\n",GetCurrentThreadId(),g_count);
	return 0;
}
unsigned int __stdcall CameraThreadFun(PVOID pM)
{		
/*
	int iRet = -1;
	long m_login = 0;
	CarNumber *pThis = new CarNumber();
	iRet = pThis->InitSvc();
	m_login = pThis->InitCarNunber();
	pThis->StartCar(HandleCarNumber);
while(1);
	if (pThis)
	{
		delete pThis;
		pThis = NULL;
	}
*/
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
		printf("Door Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return -1;
	}
	Thread_Params->lUserID[0] = lUserID;
	Thread_Params->ip[0] = "192.168.1.64";
	/*
	lUserID = NET_DVR_Login_V30("192.168.1.66", 8000, "admin", "@vsea.tv", &struDeviceInfo);
	if (lUserID < 0)
	{
	printf("Login error, %d\n", NET_DVR_GetLastError());
	NET_DVR_Cleanup();
	return -1;
	}
	Thread_Params->lUserID[1] = lUserID;
	Thread_Params->ip[1] = "192.168.1.66";
	*/
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
	free(Thread_Params);
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
//回调函数，处理出入口事件
int HandleCarNumber(char *id, char *time, int flags, char *ip)
{

	char recvbuf[MAX_HTTP_LEN] = {0};
	init_win_socket();
	SOCKET sclient;
	if (connect_Hik(&sclient,"192.168.1.106",8081) < 0){
		printf("HandleCarNumber connect error\n");
		return -1;
	}
	char ctx[1024] = {0};
	sprintf(ctx,"{\"id\":\"%s\",\"time\":\"%s\",\"flags\":%d,\"ip\":\"%s\"}",id,time,flags,ip);
	send_recv_http(sclient,ctx,recvbuf,"192.168.1.106","8081");
	closesocket(sclient);
	WSACleanup();
	return 1;
}
//封装  send http recv
int send_recv_http(SOCKET sclient,char *jscontext,char *Out_Recv,const char *ip, const char *port)
{	
	char sendbuf[MAX_HTTP_LEN] = {0};
	char recvbuf[MAX_HTTP_LEN] = {0};
	sprintf(sendbuf,"POST /jumbo/shihai/Home/api HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\nContent-type: application/json\r\nContent-Length: %d\r\n\r\n%s",ip,port,strlen(jscontext),jscontext);
	if (send(sclient,sendbuf,strlen(sendbuf),0) < 0){
		printf("send_recv_http send error\n");
		return -1;
	}
	//	memset(recvbuf,0,sizeof(recvbuf));
	memset(recvbuf, 0, 1024 * 4);
	int ret = recv(sclient,recvbuf,sizeof(recvbuf),0);
	if (ret > 0){
		recvbuf[ret] = '\0';
	}else{
		printf("send_recv_http recv error\n");
		return -1;
	}
	char *ptr = recvbuf;
	while(*ptr != '\0')
	{
		if (*ptr == '\r' &&  *(ptr+1) == '\n' && *(ptr+2) == '\r' && *(ptr+3) == '\n'){
			strncpy(Out_Recv,ptr+3,strlen(ptr));
			break;
		}
		ptr++;
	}
	return 0;
}
//封装 connect();
int connect_Hik(SOCKET *sclient, char *ip, int port)
{
	*sclient = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (*sclient == INVALID_SOCKET)
	{
		printf("carnumber invaild socket !\n");
		return -1;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	serAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	if (connect(*sclient,(sockaddr *)&serAddr,sizeof(serAddr)) == SOCKET_ERROR){
		printf("Car Number connect error!\n");
		return -1;
	}
	return 0;
}
bool JsonDecode(char *data, char *key, void *outValue){

		Json::Value root;
		Json::Reader reader;
		bool bRet = false;

		try{
			bRet = reader.parse(data, root);
		}catch (...){
			return false;
		}
		if (bRet == false){
			return false;
		}

		std::string str;
		try{
			str = root[key].asString();
		}catch(...){
			return false;
		}
		strcpy((char *)outValue,str.c_str());
		return true;
}
int JsonEncode(){

	return 0;
}
/*
* return 0 success,-1 input buffer faild, -2 outbuffer len small, -3 remove faild
*/
int GetHttpBody(struct evhttp_request *req, char *outBody,int outLen){
	evbuffer *recvbuffer=evhttp_request_get_input_buffer(req);
	if (NULL == recvbuffer){
		return -1;
	}
	size_t len = evbuffer_get_length(recvbuffer);
	memset(outBody,0,sizeof(outBody));
	if (outLen < len){
		return -2;
	}
	if (evbuffer_remove( recvbuffer , outBody , len ) == 0){
		return -3;
	}
	return 0;
}



int test()
{
	NET_DVR_Init();
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	LONG lUserID = NET_DVR_Login_V30("192.168.1.64", 8000, "admin", "@vsea.tv", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Door Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return -1;
	}

	NET_DVR_CARD_CFG_COND card_cfg;
	card_cfg.dwSize = sizeof(card_cfg);
	card_cfg.dwCardNum = 0;
	card_cfg.byCheckCardNo = 1;
	card_cfg.wLocalControllerID = 0;
	for(int i = 0; i < sizeof(card_cfg.byRes)/sizeof(card_cfg.byRes[0]); i++){
		card_cfg.byRes[i] = 0;
	}
	for(int i = 0; i < sizeof(card_cfg.byRes1)/sizeof(card_cfg.byRes1[0]); i++){
		card_cfg.byRes1[i] = 0;
	}
	char UserData[10240] = {0};
	LONG lHandle = 0;
	/**/
	if ( (lHandle = NET_DVR_StartRemoteConfig(lUserID,NET_DVR_SET_CARD_CFG,(LPVOID)&card_cfg,(DWORD)sizeof(card_cfg),(fRemoteConfigCallback)CallBack,&lHandle)) < 0){
		printf("start remoteConfig error:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	NET_DVR_CARD_CFG pSendBuf;
	pSendBuf.dwSize = sizeof(pSendBuf);
	pSendBuf.dwModifyParamType = CARD_PARAM_CARD_VALID;
	strcpy_s((char *)pSendBuf.byCardNo, ACS_CARD_NO_LEN, "120");
	pSendBuf.byCardValid = 1;
	pSendBuf.byCardType = 1;
	pSendBuf.byLeaderCard = 0;//非首卡
	pSendBuf.byRes1 = 0;
	pSendBuf.dwDoorRight = 1;
	pSendBuf.struValid.byEnable = 0;
	pSendBuf.dwMaxSwipeTime  =0;
	/**/
	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&pSendBuf,sizeof(pSendBuf)) == FALSE){
		printf("SendRemoteCfg error:%d\n",NET_DVR_GetLastError());
		return -1;
	}

	card_cfg.dwSize = sizeof(card_cfg);
	card_cfg.dwCardNum = 1;
	card_cfg.byCheckCardNo = 1;
	card_cfg.wLocalControllerID = 0;
	for(int i = 0; i < sizeof(card_cfg.byRes)/sizeof(card_cfg.byRes[0]); i++){
		card_cfg.byRes[i] = 0;
	}
	for(int i = 0; i < sizeof(card_cfg.byRes1)/sizeof(card_cfg.byRes1[0]); i++){
		card_cfg.byRes1[i] = 0;
	}

	if ( (lHandle = NET_DVR_StartRemoteConfig(lUserID,NET_DVR_GET_CARD_CFG,(LPVOID)&card_cfg,(DWORD)sizeof(card_cfg),(fRemoteConfigCallback)CallBack,&lHandle)) < 0){
		printf("start RemoteConfig error:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	
	NET_DVR_CARD_CFG_SEND_DATA Get_Card_Cfg;
	Get_Card_Cfg.dwSize = sizeof(Get_Card_Cfg);
	strcpy_s((char *)Get_Card_Cfg.byCardNo, ACS_CARD_NO_LEN, "123");
	for (int i = 0; i < sizeof(Get_Card_Cfg.byRes)/sizeof(Get_Card_Cfg.byRes[0]); i++)
		Get_Card_Cfg.byRes[i] = 0;

	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&Get_Card_Cfg,sizeof(Get_Card_Cfg)) == FALSE){
		printf("SendRemoteCfg error:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	printf("file end\n");
	//获取卡参数
	getchar();
	return -1;
}
void CALLBACK CallBack(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData)
{
	LONG UserData = 0;
	int dwStatus;
	UserData = *((LONG *)pUserData);

	switch(dwType){
		case NET_SDK_CALLBACK_TYPE_STATUS:
			printf("NET_SDK_CALLBACK_TYPE_STATUS\n");
			dwStatus = *(int *)lpBuffer;//前四个字节
			switch(dwStatus){
				case NET_SDK_CALLBACK_STATUS_SUCCESS:
					printf("SUCCESS\n");
					break;
				case NET_SDK_CALLBACK_STATUS_PROCESSING:
					printf("NET_SDK_CALLBACK_STATUS_PROCESSING:%d\n",NET_DVR_GetLastError());
					break;
				case NET_SDK_CALLBACK_STATUS_FAILED:
					printf("NET_SDK_CALLBACK_STATUS_FAILED:%d\n",NET_DVR_GetLastError());
					break;
				case NET_SDK_CALLBACK_STATUS_EXCEPTION:
					printf("NET_SDK_CALLBACK_STATUS_EXCEPTION:%d\n",NET_DVR_GetLastError());
					break;
				case NET_DVR_CALLBACK_STATUS_SEND_WAIT:
					printf("NET_DVR_CALLBACK_STATUS_SEND_WAIT:%d\n",NET_DVR_GetLastError());
					break;
				default:
					break;
			}
			break;
		case NET_SDK_CALLBACK_TYPE_PROGRESS:
			printf("NET_SDK_CALLBACK_TYPE_PROGRESS\n");
			break;
		case NET_SDK_CALLBACK_TYPE_DATA:
			printf("NET_SDK_CALLBACK_TYPE_DATA\n");
			break;
		default:
			printf("default\n");
			break;
	}
	/**/
	printf("CallBack lHandle:%d\n",UserData);
	if (NET_DVR_StopRemoteConfig(UserData) == FALSE){
	printf("NET_DVR_StopRemoteConfig error:%d\n",NET_DVR_GetLastError());
	return ;
	}
}