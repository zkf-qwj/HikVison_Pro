#include "includes.h"
#include "CarNumber.h"
int iNum = 0;
int g_count = 0;
LONG g_DlUserID = 0;
LONG g_ClUserID = 0;
short          http_port = 8081;
char          *http_addr = "192.168.1.106";
const char *salt= "woareateam!!!";
struct event_base *base;
struct evhttp *http_server;
NET_DVR_CARD_CFG_V50* g_pCardCfg;//获取数据
map<char*, char*>   my_Map;

void generic_handler(struct evhttp_request *req, void *arg)
{
	char md5buf[64] = {0};
	int  timestamp = 0;
	timestamp = time(0);
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

	evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json; charset=utf-8");
	if (reader.parse(body, rRoot) == false){
		printf("parse http body faild\n");
		strcpy(httpstatus,"parse http body faild");
		goto SendHttp;
	}
	printf("---timestamp:%d\n",rRoot["timestamp"].asString().c_str());
	if (rRoot["timestamp"].asString().c_str() == NULL){
		strcpy(httpstatus,"get timestamp faild");
		goto SendHttp;
	}
	if ((timestamp - atoi(rRoot["timestamp"].asString().c_str())) > 300){
		strcpy(httpstatus,"Request timed out faild");
		goto SendHttp;
	}
	strcpy(md5buf,rRoot["timestamp"].asString().c_str());
	strcat(md5buf+strlen(md5buf),salt);
	if (strcmp(rRoot["token"].asString().c_str(),md5(md5buf).c_str()) != 0){
		strcpy(httpstatus,"Authentication failed");
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
				strcpy(httpstatus,"door control faild");
				goto SendHttp;
			}
		}else if (strstr(uri,"setconfig")){
			
		}else if (strstr(uri,"getconfig")){
			
		}
	}else if (strstr(uri,"card")){//卡控制
		if (strstr(uri,"setlist")){//卡设置
			//OperatCardNoFun()
			NET_DVR_CARD_CFG_V50 struCond = {0};
			struCond.dwSize = sizeof(struCond);
			if (HandleCardSet(rRoot,&struCond) == false){
				strcpy(httpstatus,"get HandleCardSet faild");
				goto SendHttp;
			}
			int i = 0;
			for (i = 0; i < Thread_Params->wLen; i++){
				if (strcmp(rRoot["ip"].asString().c_str(),Thread_Params->ip[i]) == 0){
					break;
				}
			}
			LONG lHandle = 0;
			lHandle = OperatCardNoFun(Thread_Params->lUserID[i],CallBack,NET_DVR_SET_CARD_CFG_V50);
			if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&struCond,sizeof(struCond)) == FALSE){
				strcpy(httpstatus,"set cardparam faild");
				goto SendHttp;
			}
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
	evbuffer_free(buf);
}

unsigned int __stdcall DoorThreadFun(PVOID pM)
{	

	//test();
	/**///初始化
	NET_DVR_Init();
	//设置连接时间和重连时间
	NET_DVR_SetConnectTime(2000,1);
	NET_DVR_SetReconnect(10000,true);
	//-------------------------------
	//注册设备
	LONG lUserID,lHandle;
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
	
	if (NET_DVR_SetDVRMessageCallBack_V31(msgCallBack,NULL) == FALSE){
		printf("------SetDVRMessageCallBack_V31:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	printf("-------------------------------------\n");
	getchar();
	printf(" child thread beginthread ID:%d,g_count:%d\n",GetCurrentThreadId(),g_count);
	return 0;
}
unsigned int __stdcall CameraThreadFun(PVOID pM)
{		
/**/
	int iRet = -1;
	long m_login = 0;
	CarNumber *pThis = new CarNumber();
	iRet = pThis->InitSvc();
	m_login = pThis->InitCarNunber();
	pThis->StartCar(HandleCarNumber);
getchar();
	if (pThis)
	{
		delete pThis;
		pThis = NULL;
	}

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
	Thread_Params->wLen = 1;
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
	if (connect_Hik(&sclient,"192.168.1.104",80) < 0){
		printf("HandleCarNumber connect error\n");
		return -1;
	}
	char ctx[1024] = {0};
	sprintf(ctx,"{\"id\":\"%s\",\"time\":\"%s\",\"flags\":%d,\"ip\":\"%s\"}",id,time,flags,ip);
	send_recv_http(sclient,ctx,recvbuf,"192.168.1.104","80");
	closesocket(sclient);
	WSACleanup();
	return 1;
}
//封装  send http recv
int send_recv_http(SOCKET sclient,char *jscontext,char *Out_Recv,const char *ip, const char *port)
{	
	char sendbuf[MAX_HTTP_LEN] = {0};
	char recvbuf[MAX_HTTP_LEN] = {0};
	sprintf(sendbuf,"POST /park/index.php/api/in HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s",ip,port,strlen(jscontext),jscontext);
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
	//初始化
	NET_DVR_Init();
	//设置连接时间和重连时间
	NET_DVR_SetConnectTime(2000,1);
	NET_DVR_SetReconnect(10000,true);
	//-------------------------------
	//注册设备
	LONG lUserID,lHandle;
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


	lHandle = OperatCardNoFun(lUserID,(void *)CallBack,NET_DVR_SET_CARD_CFG_V50);

	/**/
	NET_DVR_CARD_CFG_V50 SendCardCfg = {0};
	SendCardCfg.dwSize = sizeof(SendCardCfg);
	//SendCardCfg.dwModifyParamType = CARD_PARAM_CARD_VALID|CARD_PARAM_VALID|CARD_PARAM_CARD_TYPE|CARD_PARAM_DOOR_RIGHT|CARD_PARAM_RIGHT_PLAN;
	SendCardCfg.dwModifyParamType = CARD_PARAM_RIGHT_PLAN|CARD_PARAM_DOOR_RIGHT;
	BYTE CardNo[32] = {0};
	strcpy((char *)CardNo,"3069606104");
	memcpy((char *)SendCardCfg.byCardNo,CardNo,32);
	//memcpy(SendCardCfg.byCardPassword,"88888888",8);
	SendCardCfg.byCardValid = true;
	SendCardCfg.byCardType = 1;
	SendCardCfg.byLeaderCard = 0;
	SendCardCfg.byDoorRight[1] = 1;
	SendCardCfg.struValid.byEnable = 1;
	SendCardCfg.wCardRightPlan[0][1] = 1;
	SendCardCfg.wCardRightPlan[0][0] = 1;
	SendCardCfg.wCardRightPlan[0][2] = 1;
	SendCardCfg.wCardRightPlan[0][3] = 1;
	SendCardCfg.wCardRightPlan[0][4] = 1;
	SendCardCfg.struValid.struBeginTime.wYear = 2000;
	SendCardCfg.struValid.struBeginTime.byMonth = 1;
	SendCardCfg.struValid.struBeginTime.byDay = 1;
	SendCardCfg.struValid.struBeginTime.byHour = 0;
	SendCardCfg.struValid.struBeginTime.byMinute = 0;
	SendCardCfg.struValid.struBeginTime.bySecond = 0;
	SendCardCfg.struValid.struEndTime.wYear = 2027;
	SendCardCfg.struValid.struEndTime.byMonth = 02;
	SendCardCfg.struValid.struEndTime.byDay = 16;
	SendCardCfg.struValid.struEndTime.byHour = 0;
	SendCardCfg.struValid.struEndTime.byMinute = 0;
	SendCardCfg.struValid.struEndTime.bySecond = 0;
	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&SendCardCfg,sizeof(SendCardCfg)) == FALSE){
		printf("-------SendRemoteConfig faild:errno:%d\n",NET_DVR_GetLastError());
		return -1;
	}
}
void CALLBACK CallBack(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData)
{
	LONG UserData = 0;
	int dwStatus;
	struct Status_t *CardError;
	CardError = (struct Status_t *)lpBuffer;
	switch(dwType){
		case NET_SDK_CALLBACK_TYPE_STATUS:
			printf("NET_SDK_CALLBACK_TYPE_STATUS\n");
			dwStatus = *((int *)lpBuffer);//前四个字节
			switch(dwStatus){
				case NET_SDK_CALLBACK_STATUS_SUCCESS:
					printf("SUCCESS\n");
					break;
				case NET_SDK_CALLBACK_STATUS_PROCESSING:
					printf("NET_SDK_CALLBACK_STATUS_PROCESSING:%d\n",NET_DVR_GetLastError());
					break;
				case NET_SDK_CALLBACK_STATUS_FAILED:
					printf("NET_SDK_CALLBACK_STATUS_FAILED:%d\n",NET_DVR_GetLastError());
					printf("----------------status:%d,errno:%d,CarNo:%s\n",*((int *)lpBuffer),*((int *)(((char *)lpBuffer) + 4)),((char *)lpBuffer+8));
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
			g_pCardCfg = (NET_DVR_CARD_CFG_V50*)lpBuffer;
			printf("------CardNumber:%s,CardVaild:%d,byCardType   :%d\n",g_pCardCfg->byCardNo,g_pCardCfg->byCardValid,g_pCardCfg->byCardType );
			break;
		default:
			printf("default\n");
			break;
	}
}
LONG OperatCardNoFun(LONG lUserID,void *CallBack,int type)
{
	NET_DVR_CARD_CFG_COND struCond = {0};
	struCond.dwSize  = sizeof(struCond);
	if (type == NET_DVR_GET_CARD_CFG_V50 || type == NET_DVR_GET_CARD_CFG){
		struCond.dwCardNum = 0xffffffff;
	}else{
		struCond.dwCardNum = 1;
	}
	struCond.byCheckCardNo = 1; //校验
	//struCond.wLocalControllerID = 2;
	for (int i = 0; i < sizeof(struCond.byRes1)/sizeof(struCond.byRes1[0]); i++){
		struCond.byRes1[i] = 0;
	}
	LONG lHandle = NET_DVR_StartRemoteConfig(lUserID,type,&struCond,sizeof(struCond),(fRemoteConfigCallback)CallBack, NULL);
	if (lHandle == -1)
	{
		printf("StartRemote faild:errno:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	else
	{
		printf("StartRemote success\n");
		return lHandle;
	}
}
BOOL HandleCardSet(Json::Value rRoot,NET_DVR_CARD_CFG_V50 *struCond)
{
	char *tmp = NULL;
	string SJson;
	int Card_Vaild,Date_Vaild,Card_Type,Card_Passwd,LeaderCard,DoorRight,CardRightPlan;
	SJson = rRoot["ModifyParamType"].asString();
	tmp = (char *)(SJson.c_str());
	if (tmp == NULL){
		printf("get ModifyParamType faild\n");
		return false;
	}
	int ModifyParamType = atoi(tmp);
	Card_Vaild = CARD_PARAM_CARD_VALID & ModifyParamType;   //卡有效
	Date_Vaild = CARD_PARAM_VALID & ModifyParamType;	    //有效期参数
	Card_Type = CARD_PARAM_CARD_TYPE & ModifyParamType;		//卡类型
	Card_Passwd = CARD_PARAM_PASSWORD & ModifyParamType;	//卡密码
	LeaderCard = CARD_PARAM_LEADER_CARD & ModifyParamType;	//首卡
	DoorRight = CARD_PARAM_DOOR_RIGHT & ModifyParamType;	//门权限
	CardRightPlan = CARD_PARAM_RIGHT_PLAN & ModifyParamType;//卡权限计划参数
	SJson = rRoot["channel"].asString();
	if ((tmp = (char *)SJson.c_str()) == NULL){
		printf("get channel faild\n");
		return false;
	}
	int DoorNo = atoi(tmp);
	struCond->dwModifyParamType = ModifyParamType;
	SJson = rRoot["CardNo"].asString();
	if (SJson.c_str() == NULL){
		printf("get CardNo faild\n");
		return false;
	}
	strcpy((char *)struCond->byCardNo,SJson.c_str());//获取卡号
	SJson = rRoot["CardValid"].asString();
	if (Card_Vaild){
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get CardVaild faild\n");
			return false;
		}
		struCond->byCardValid = atoi(tmp);
	}else {
		struCond->byCardValid = 1;//默认有效
	}
	SJson = rRoot["CardType"].asString();
	if (Card_Type){//默认是空白卡
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get CardType faild\n");
			return false;
		}
		struCond->byCardType = atoi(tmp);
	}else {
		struCond->byCardType =0;
	}
	SJson = rRoot["LeaderCard"].asString();
	if (LeaderCard){ //默认是非首卡
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get LeaderCard faild\n");
			return false;
		}
		struCond->byLeaderCard = atoi(tmp);
	}else{
		struCond->byLeaderCard = 0;
	}
	SJson = rRoot["ValidEnable"].asString();
	if (Date_Vaild){//默认是不使能日期
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get ValidEnable faild\n");
			return false;
		}
		struCond->struValid.byEnable = atoi(tmp);
		if ((rRoot["ValidBeginTime"].asString().c_str() == NULL) || (rRoot["ValidEndTime"].asString().c_str() == NULL)){
			printf("get ValidBeginTime faild\n");
			return false;
		}
		GetDateTime((char *)rRoot["ValidBeginTime"].asString().c_str(),&(struCond->struValid.struBeginTime));
		GetDateTime((char *)rRoot["ValidEndTime"].asString().c_str(),&(struCond->struValid.struEndTime));
	}else {
		struCond->struValid.byEnable = 0;
	}
	SJson = rRoot["DoorRight"].asString();
	if (DoorRight){//门权限
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get DoorRight faild\n");
			return false;
		}
		struCond->byDoorRight[DoorNo] = atoi(tmp);
	}else{
		struCond->byDoorRight[DoorNo] = 0;
	}
	SJson = rRoot["CardRightPlan"].asString();
	if (CardRightPlan){ //门权限计划
		if ((tmp = (char *)SJson.c_str()) == NULL){
			printf("get CardRightPlan faild\n");
			return false;
		}
		(struCond->wCardRightPlan)[DoorNo-1][0] = atoi(tmp);
		(struCond->wCardRightPlan)[DoorNo-1][1] = atoi(tmp);
		(struCond->wCardRightPlan)[DoorNo-1][2] = atoi(tmp);
		(struCond->wCardRightPlan)[DoorNo-1][3] = atoi(tmp);
	}else{
		(struCond->wCardRightPlan)[DoorNo-1][0] = 0;
	}
	return true;
}
//2017-02-16/12:13:34
void GetDateTime(char *aTime, NET_DVR_TIME_EX *Date)
{
	char four[5];
	memset(four,0,sizeof(four));
	memcpy(four,aTime,4);
	Date->wYear = atoi(four);
	memset(four,0,sizeof(four));
	memcpy(four,aTime+5,2);
	Date->byMonth = atoi(four);
	memset(four,0,sizeof(four));
	memcpy(four,aTime+8,2);
	Date->byDay = atoi(four);
	memset(four,0,sizeof(four));
	memcpy(four,aTime+11,2);
	Date->byHour = atoi(four);
	memset(four,0,sizeof(four));
	memcpy(four,aTime+14,2);
	Date->byMinute = atoi(four);
	memset(four,0,sizeof(four));
	memcpy(four,aTime+17,2);
	Date->bySecond = atoi(four);
}



BOOL CALLBACK msgCallBack(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	NET_DVR_ACS_ALARM_INFO *alarminfo= (NET_DVR_ACS_ALARM_INFO *)pAlarmInfo;
	switch(lCommand)
	{
	case COMM_ALARM_ACS:
		DWORD Major = alarminfo->dwMajor;
		DWORD Minjor = alarminfo->dwMinor;
		switch(Major)
		{
		case MAJOR_EVENT:
			switch(Minjor){
				case MINOR_LEGAL_CARD_PASS:
					printf("合法卡认证通过\n");
					printf("-----:CardNo:%s,time:%d-%d-%d\/%d:%d:%d\n",alarminfo->struAcsEventInfo.byCardNo,alarminfo->struTime.dwYear,alarminfo->struTime.dwMonth,alarminfo->struTime.dwDay,alarminfo->struTime.dwHour,alarminfo->struTime.dwMinute,alarminfo->struTime.dwSecond);
					printf("-----:ReadCardtype:%d,DoorNo:%d,byDeviceNo:%d\n",alarminfo->struAcsEventInfo.byCardReaderKind,alarminfo->struAcsEventInfo.dwDoorNo,alarminfo->struAcsEventInfo.byDeviceNo);
					break;
				case MINOR_CARD_OUT_OF_DATE:
					printf("卡号过期\n");
					break;
				case MINOR_LOCK_OPEN:
					printf("门锁打开\n");
					
					break;
				case MINOR_LOCK_CLOSE:
					printf("门锁关闭\n");
					break;
				case MINOR_CARD_AND_PSW_PASS:
					printf("刷卡加密码成功\n");
					break;
				case MINOR_CARD_AND_PSW_FAIL:
					printf("刷卡加密码失败\n");
					break;
				case MINOR_INVALID_CARD:
					printf("无此卡号\n");
					break;
			}
		case MAJOR_ALARM:
			switch(Minjor){
				case MINOR_CASE_SENSOR_ALARM:
				printf("事件输入报警\n");
				break;
			}
			break;
		}
		break;
	}
	return true;
}