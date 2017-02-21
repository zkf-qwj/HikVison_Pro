#include "includes.h"
#include "ReadCard.h"
#include "Threads.h"
#include "CardManagement.h"

//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

int iNum = 0;
int g_count = 0;
LONG g_DlUserID = 0;
LONG g_ClUserID = 0;

ConfigFile DoorParam[64] = {0};     //门禁控制主机
ConfigFile DoorNum = {0};     //门禁控制主机总数

void main()
{
	char *ConfigFileName = "IbuguCfgsysterm.ini"; //配置文件
	DoorNum.KeyName[0] = "total";
	DoorNum.len = 1;
	if (ReadConfig("IbuguCfgsysterm.ini","NUM",&DoorNum) < 0){
		printf("read config file faild\n");
		return ;
	}
	int i = 0;
	for (i = 0; i < atoi(DoorNum.Value[0]); i++){
		char Section[64] = "DOOR";
		DoorParam[i].KeyName[0] = "ip";
		DoorParam[i].KeyName[1] = "Username";
		DoorParam[i].KeyName[2] = "password";
		DoorParam[i].KeyName[3] = "port";
		DoorParam[i].len = 4;
		sprintf(Section+strlen(Section),"%d",i);
		if (ReadConfig("IbuguCfgsysterm.ini",Section,&(DoorParam[i])) < 0){
			printf("read config file faild\n");
			return ;
		}
	}
	DoorParam[i] = DoorNum;

	init_win_socket();
	NET_DVR_Init();
	NET_DVR_SetConnectTime(2000, 1);
	//创建线程
	const int THREAD_NUM = 4;
	HANDLE handle[THREAD_NUM]; 
	handle[0] =  (HANDLE)_beginthreadex(NULL, 0, CameraThreadFun, NULL, 0, NULL);
	handle[1] =  (HANDLE)_beginthreadex(NULL, 0, DoorThreadFun, NULL, 0, NULL);
	handle[2] =  (HANDLE)_beginthreadex(NULL, 0, evThreadFun,DoorParam, 0, NULL);
	handle[3] =  (HANDLE)_beginthreadex(NULL, 0, IDReaderFun,NULL, 0, NULL);
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE ,INFINITE);

	return;
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
int HandleCarNumberCB(char *id, char *time, int flags, char *ip)
{

	char recvbuf[MAX_HTTP_LEN] = {0};
	init_win_socket();
	SOCKET sclient;
	if (connect_Hik(&sclient,"192.168.1.104",80) < 0){
		printf("HandleCarNumber connect error\n");
		return -1;
	}
	char ctx[1024] = {0};
	if (flags == 1){//车子进入
		sprintf(ctx,"{\"car_plate\":\"%s\",\"ip\":\"%s\",\"port\":\"%s\"}",id,ip,"80");
		send_recv_http(sclient,ctx,recvbuf,"192.168.1.104","80","/park/index.php/api/out","Content-Type: application/x-www-form-urlencoded");
	}else if (flags == 0){//车子出去
		sprintf(ctx,"{\"car_plate\":\"%s\",\"ip\":\"%s\",\"port\":\"%s\"}",id,ip,"80");
		send_recv_http(sclient,ctx,recvbuf,"192.168.1.104","80","/park/index.php/api/in","Content-Type: application/x-www-form-urlencoded");
	}
	printf("-----Car RecvMsg:%s\n",recvbuf);
	/**/
	closesocket(sclient);
	WSACleanup();
	
	return 1;
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
//封装  send http recv
int send_recv_http(SOCKET sclient,char *jscontext,char *Out_Recv,const char *ip, const char *port, const char *path,const char *contenttype)
{	
	char sendbuf[MAX_HTTP_LEN] = {0};
	char recvbuf[MAX_HTTP_LEN] = {0};
	sprintf(sendbuf,"POST %s HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\n%s\r\nContent-Length: %d\r\n\r\n%s",path,ip,port,contenttype,strlen(jscontext),jscontext);
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
/*
读取配置文件
*/
int ReadConfig(char *ConfigFileName,char *Section,ConfigFile *OutParam)
{
	char *ch = NULL;
	char PathBuf[1024];
	memset(PathBuf, 0, 1024);
	GetModuleFileName(NULL, PathBuf, 1024);
	ch = strrchr(PathBuf, '\\');
	if (!ch){
		return -1;	
	}
	PathBuf[strlen(PathBuf) - strlen(ch)+1] = '\0';
	
	strcat_s(PathBuf, 1024, ConfigFileName);
	char buf[1024] = {0};
	for (int i = 0; i < OutParam->len; i++){

		::GetPrivateProfileString(Section, (LPCSTR)(OutParam->KeyName)[i], "",(OutParam->Value)[i] , sizeof((OutParam->Value)[i]),PathBuf);
		//::GetPrivateProfileString("DOOR", "ip", "",buf , sizeof(buf),PathBuf);
		if(strlen((OutParam->Value)[i]) == 0){
			printf("Config file read faild\n");
			return -1;
		}
		printf("read ip:%s\n",OutParam->Value[i]);
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


	lHandle = CM_SetOrGetCardNoFun(lUserID,(void *)CM_RemoteCallBack,NET_DVR_SET_CARD_CFG_V50);

	/**/
	NET_DVR_CARD_CFG_V50 SendCardCfg = {0};
	SendCardCfg.dwSize = sizeof(SendCardCfg);
	SendCardCfg.dwModifyParamType = CARD_PARAM_CARD_VALID|CARD_PARAM_VALID|CARD_PARAM_CARD_TYPE|CARD_PARAM_DOOR_RIGHT|CARD_PARAM_RIGHT_PLAN;
	//SendCardCfg.dwModifyParamType = CARD_PARAM_RIGHT_PLAN|CARD_PARAM_DOOR_RIGHT;
	BYTE CardNo[32] = {0};
	strcpy((char *)CardNo,"3069606104");
	memcpy((char *)SendCardCfg.byCardNo,CardNo,32);
	//memcpy(SendCardCfg.byCardPassword,"88888888",8);
	SendCardCfg.byCardValid = 1;
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


	printf("ModfyType:%d,CardNo:%s,CardVild:%d,CardType:%d,CardPassWd:%s,leaderDoor:%d,VaildEnable:%d,ValidBegentime:%d,%d,%d,%d,%d,%d,Vaildendtime:%d,%d,%d,%d,%d,%d,DoorRigtht:%d,CardRightPlan:%d\n",
		SendCardCfg.dwModifyParamType,SendCardCfg.byCardNo,SendCardCfg.byCardValid,SendCardCfg.byCardType,SendCardCfg.byCardPassword,SendCardCfg.byLeaderCard,SendCardCfg.struValid.byEnable,
		SendCardCfg.struValid.struBeginTime.wYear,SendCardCfg.struValid.struBeginTime.byMonth,SendCardCfg.struValid.struBeginTime.byDay,SendCardCfg.struValid.struBeginTime.byHour,SendCardCfg.struValid.struBeginTime.byMinute,SendCardCfg.struValid.struBeginTime.byMinute,
		SendCardCfg.struValid.struBeginTime.wYear,SendCardCfg.struValid.struBeginTime.byMonth,SendCardCfg.struValid.struBeginTime.byDay,SendCardCfg.struValid.struBeginTime.byHour,SendCardCfg.struValid.struBeginTime.byMinute,SendCardCfg.struValid.struBeginTime.bySecond,
		SendCardCfg.byDoorRight[1],SendCardCfg.wCardRightPlan[0][0]);
	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&SendCardCfg,sizeof(SendCardCfg)) == FALSE){
		printf("-------SendRemoteConfig faild:errno:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	return 0;
}