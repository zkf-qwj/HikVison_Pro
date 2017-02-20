#include "Threads.h"
#include "CardManagement.h"
#include "ReadCard.h"
short          http_port = 8081;
char          *http_addr = "192.168.1.108";
struct event_base *base;
struct evhttp *http_server;
/*
http服务器回调函数
*/
void generic_handler(struct evhttp_request *req, void *arg)
{
	char md5buf[64] = {0};
	time_t  timestamp = 0;
	timestamp = time(0);
	Json::Value root,rRoot;
	Json::FastWriter fw;
	Json::Reader reader;
	pthread_t  *Thread_Params = (pthread_t *)arg;
	LONG *lUserID = Thread_Params->lUserID;
	char body[4096] = {0};
	int iRet = 0;
	char httpstatus[1024] = {0};
	string Str;
	printf("HTTP Request Enter\n");
	strcpy_s(httpstatus,sizeof(httpstatus),"success");
	struct evbuffer *buf = evbuffer_new();
	if(!buf)
	{  
		puts("failed to create response buffer \n");
		return;
	}
	const char *uri = evhttp_request_get_uri(req);
	if ((iRet = GetHttpBody(req,body,sizeof(body))) < 0){
		printf("Get Http Body error :%d\n",iRet);
		strcpy_s(httpstatus,sizeof(httpstatus),"Get http body faild");
		goto SendHttp;
	}
	evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json; charset=utf-8");
	if (reader.parse(body, rRoot) == false){
		printf("parse http body faild\n");
		strcpy_s(httpstatus,sizeof(httpstatus),"parse http body faild");
		goto SendHttp;
	}
	Str = rRoot["timestamp"].asString();
	if (rRoot["timestamp"].asString().c_str() == NULL){
		strcpy_s(httpstatus,sizeof(httpstatus),"get timestamp faild");
		goto SendHttp;
	}
	if ((timestamp - atol(rRoot["timestamp"].asString().c_str())) > 300){
		strcpy_s(httpstatus,sizeof(httpstatus),"Request timed out faild");
		goto SendHttp;
	}
	strcpy(md5buf,rRoot["timestamp"].asString().c_str());
	strcat(md5buf+strlen(md5buf),salt);
	if (strcmp(rRoot["token"].asString().c_str(),md5(md5buf).c_str()) != 0){
		strcpy_s(httpstatus,sizeof(httpstatus),"Authentication failed");
		goto SendHttp;
	}
	if (strstr(uri,"door")){ //门禁控制主机接口
		if (strstr(uri,"control")){ 
			printf("ip:%s,number:%s,commond:%s\n",rRoot["ip"].asString().c_str(),
				rRoot["channel"].asString().c_str(),
				rRoot["commond"].asString().c_str()
				);
			int i = 0;
			for (i = 0; i < Thread_Params->wLen; i++){
				if (strcmp(rRoot["ip"].asString().c_str(),Thread_Params->ip[i]) == 0)
					break;
			}
			if (NET_DVR_ControlGateway(Thread_Params->lUserID[i],atoi(rRoot["channel"].asString().c_str()),atoi(rRoot["commond"].asString().c_str())) == FALSE){
				strcpy_s(httpstatus,sizeof(httpstatus),"door control faild");
				goto SendHttp;
			}
		}else if (strstr(uri,"setconfig")){

		}else if (strstr(uri,"getconfig")){

		}
	}else if (strstr(uri,"card")){//卡控制
		if (strstr(uri,"setlist")){//卡设置
			printf("Enter Card SetList--------------\n");
			NET_DVR_CARD_CFG_V50 struCond = {0};
			struCond.dwSize = sizeof(struCond);
			if (CM_HandleCardSet(rRoot,&struCond) == false){
				strcpy_s(httpstatus,sizeof(httpstatus),"get CM_HandleCardSet faild");
				goto SendHttp;
			}
			/*
			printf("ModfyType:%d,channel:%s,CardNo:%s,CardVild:%d,CardType:%d,CardPassWd:%s,leaderDoor:%d,VaildEnable:%d,ValidBegentime:%d,%d,%d,%d,%d,%d,Vaildendtime:%d,%d,%d,%d,%d,%d,DoorRigtht:%d,CardRightPlan:%d\n",
			struCond.dwModifyParamType,rRoot["channel"].asString().c_str(),struCond.byCardNo,struCond.byCardValid,struCond.byCardType,struCond.byCardPassword,struCond.byLeaderCard,struCond.struValid.byEnable,
			struCond.struValid.struBeginTime.wYear,struCond.struValid.struBeginTime.byMonth,struCond.struValid.struBeginTime.byDay,struCond.struValid.struBeginTime.byHour,struCond.struValid.struBeginTime.byMinute,struCond.struValid.struBeginTime.byMinute,
			struCond.struValid.struEndTime.wYear,struCond.struValid.struEndTime.byMonth,struCond.struValid.struEndTime.byDay,struCond.struValid.struEndTime.byHour,struCond.struValid.struEndTime.byMinute,struCond.struValid.struEndTime.bySecond,
			struCond.byDoorRight[1],struCond.wCardRightPlan[0][0]);
			*/	
			int i = 0;
			for (i = 0; i < Thread_Params->wLen; i++){
				if (strcmp(rRoot["ip"].asString().c_str(),Thread_Params->ip[i]) == 0){
					break;
				}
			}
			LONG lHandle = 0;
			printf("ip:%s\n",Thread_Params->ip[i]);
			lHandle = CM_SetOrGetCardNoFun(Thread_Params->lUserID[i],CM_RemoteCallBack,NET_DVR_SET_CARD_CFG_V50);
			if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&struCond,sizeof(struCond)) == FALSE){
				strcpy_s(httpstatus,sizeof(httpstatus),"set cardparam faild");
				goto SendHttp;
			}
			Sleep(100);
			printf("Exit Card SetList--------------\n");
		}else if (strstr(uri,"getlist")){

		}else if (strstr(uri,"find")){

		}
	}
SendHttp:
	root["HttpStatus"] = Json::Value(httpstatus);
	std::string strData = fw.write(root);
	char *pJsonData = (char *)strData.c_str();
	evbuffer_add_printf(buf, "%s\n",pJsonData);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	printf("HTTP Responed.....\n");
	evbuffer_free(buf);


}
/*
FunctionName:DoorThreadFun 门禁控制主机线程回调函数
*/
unsigned int __stdcall DoorThreadFun(PVOID pM)
{	

	//	test();
	/**///初始化
	NET_DVR_Init();
	//设置连接时间和重连时间
	NET_DVR_SetConnectTime(2000,1);
	NET_DVR_SetReconnect(10000,true);
	//-------------------------------
	//注册设备
	LONG lUserID = 0,lHandle = 0;
	NET_DVR_USER_LOGIN_INFO struLoginInfo;
	struLoginInfo.bUseAsynLogin = 0;//同步登录方式
	strcpy(struLoginInfo.sDeviceAddress,"192.168.1.68");
	struLoginInfo.wPort = 8000;
	strcpy(struLoginInfo.sUserName,"admin");
	strcpy(struLoginInfo.sPassword,"@vsea.tv");
	//设备信息，输出参数
	NET_DVR_DEVICEINFO_V40 struDeviceIndoV40 = {0};
	if ((lUserID = NET_DVR_Login_V40(&struLoginInfo,&struDeviceIndoV40)) < 0){
		printf("Login faild,error code:%d\n",NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return -1;
	}
	if (NET_DVR_SetupAlarmChan_V30(lUserID) < 0){
		printf("SetupAlarmChan_V30 faild,errno:%d\n",NET_DVR_GetLastError());
		return -1;
	}

	if (NET_DVR_SetDVRMessageCallBack_V31(CM_AlarmCallBack,NULL) == FALSE){
		printf("------SetDVRMessageCallBack_V31:%d\n",NET_DVR_GetLastError());
		return -1;
	}

	printf("-------------------------------------\n");
	getchar();

	return 0;
}
/*
FunctionName:CameraThreadFun 汽车出入口线程回调函数
*/
unsigned int __stdcall CameraThreadFun(PVOID pM)
{		
	/*
	int iRet = -1;
	long m_login = 0;
	CarNumber *pThis = new CarNumber();
	iRet = pThis->InitSvc();
	m_login = pThis->InitCarNunber();
	pThis->StartCar(HandleCarNumberCB);
	getchar();
	if (pThis)
	{
	delete pThis;
	pThis = NULL;
	}*/

	return 0;
}
/*
FunctionName:IDReaderFun 读卡器线程回调函数
*/
unsigned int __stdcall IDReaderFun(PVOID pM)
{
	IDReader *pThis = new IDReader();
	pThis->ReaderInit(RC_ID,RC_IC);
	printf("----IDReader Thread....\n");
	return 0;
}
/*
FunctionName:evThreadFun libeventhttp线程回调函数
*/
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

	/**/
	lUserID = NET_DVR_Login_V30("192.168.1.68", 8000, "admin", "@vsea.tv", &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return -1;
	}
	Thread_Params->lUserID[1] = lUserID;
	Thread_Params->ip[1] = "192.168.1.68";
	Thread_Params->wLen = 2;
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
	NET_DVR_Logout(Thread_Params->lUserID[0]);
	NET_DVR_Logout(Thread_Params->lUserID[1]);
	free(Thread_Params);
	NET_DVR_Cleanup();
	return 0;
}
