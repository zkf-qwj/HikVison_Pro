#include "CardManagement.h"

NET_DVR_CARD_CFG_V50* g_pCM_CardCfg;//获取数据

LONG CM_SetOrGetCardNoFun(LONG lUserID,void *CallBack,int type)
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
//2017-02-16/12:13:34
void CM_GetDateTime(char *aTime, NET_DVR_TIME_EX *Date)
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

BOOL CM_HandleCardSet(Json::Value rRoot,NET_DVR_CARD_CFG_V50 *struCond)
{
	string SJson;
	int Card_Vaild,Date_Vaild,Card_Type,Card_Passwd,LeaderCard,DoorRight,CardRightPlan;
	SJson = rRoot["ModifyParamType"].asString();
	if ((char *)(SJson.c_str()) == NULL){
		printf("get ModifyParamType faild\n");
		return  false;
	}
	int ModifyParamType = atoi((char *)(SJson.c_str()));
	struCond->dwModifyParamType = ModifyParamType;
	Card_Vaild = CARD_PARAM_CARD_VALID & ModifyParamType;   //卡有效
	Date_Vaild = CARD_PARAM_VALID & ModifyParamType;	    //有效期参数
	Card_Type = CARD_PARAM_CARD_TYPE & ModifyParamType;		//卡类型
	Card_Passwd = CARD_PARAM_PASSWORD & ModifyParamType;	//卡密码
	LeaderCard = CARD_PARAM_LEADER_CARD & ModifyParamType;	//首卡
	DoorRight = CARD_PARAM_DOOR_RIGHT & ModifyParamType;	//门权限
	CardRightPlan = CARD_PARAM_RIGHT_PLAN & ModifyParamType;//卡权限计划参数
	SJson = rRoot["channel"].asString();
	if ((char *)SJson.c_str() == NULL){
		printf("get channel faild\n");
		return  false;
	}
	int DoorNo = atoi((char *)SJson.c_str());
	SJson = rRoot["CardNo"].asString();
	if (SJson.c_str() == NULL){
		printf("get CardNo faild\n");
		return false;
	}
	strcpy((char *)struCond->byCardNo,SJson.c_str());//获取卡号
	SJson = rRoot["CardValid"].asString();
	if (Card_Vaild){
		if ((char *)SJson.c_str() == NULL){
			printf("get CardVaild faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			struCond->byCardValid = 1;
		}else{
			struCond->byCardValid = 0;
		}
	}else {
		struCond->byCardValid = 0;//默认有效
	}
	SJson = rRoot["CardType"].asString();
	if (Card_Type){//默认是空白卡
		if ((char *)SJson.c_str() == NULL){
			printf("get CardType faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			struCond->byCardType = 1;
		}else if (strcmp(SJson.c_str(),"2") == 0){
			struCond->byCardType = 2;
		}else if (strcmp(SJson.c_str(),"3") == 0){
			struCond->byCardType = 3;
		}else if (strcmp(SJson.c_str(),"4") == 0){
			struCond->byCardType = 4;
		}else if (strcmp(SJson.c_str(),"5") == 0){
			struCond->byCardType = 5;
		}else if (strcmp(SJson.c_str(),"6") == 0){
			struCond->byCardType = 6;
		}else if (strcmp(SJson.c_str(),"7") == 0){
			struCond->byCardType = 7;
		}else if (strcmp(SJson.c_str(),"8") == 0){
			struCond->byCardType = 8;
		}else{
			struCond->byCardType = 0;
		}
	}else{
		struCond->byCardType = 0;
	}
	SJson = rRoot["LeaderCard"].asString();
	if (LeaderCard){ //默认是非首卡
		if ((char *)SJson.c_str() == NULL){
			printf("get LeaderCard faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			struCond->byLeaderCard = 1;
		}else{
			struCond->byLeaderCard = 0;
		}
	}else{
		struCond->byLeaderCard = 0;
	}
	SJson = rRoot["ValidEnable"].asString();
	if (Date_Vaild){//默认是不使能日期
		if ((char *)SJson.c_str() == NULL){
			printf("get ValidEnable faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			struCond->struValid.byEnable = 1;
			if ((rRoot["ValidBeginTime"].asString().c_str() == NULL) || (rRoot["ValidEndTime"].asString().c_str() == NULL)){
				printf("get ValidBeginTime faild\n");
				return false;
			}
			SJson = rRoot["ValidBeginTime"].asString();
			CM_GetDateTime((char *)SJson.c_str(),&(struCond->struValid.struBeginTime));
			SJson = rRoot["ValidEndTime"].asString();
			CM_GetDateTime((char *)SJson.c_str(),&(struCond->struValid.struEndTime));
		}else{
			struCond->struValid.byEnable = 0;
		}
	}else {
		struCond->struValid.byEnable = 0;
	}
	SJson = rRoot["DoorRight"].asString();
	if (DoorRight){//门权限
		if ( (char *)SJson.c_str() == NULL){
			printf("get DoorRight faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			struCond->byDoorRight[DoorNo] = 1;
		}else{
			struCond->byDoorRight[DoorNo] = 0;
		}

	}else{
		struCond->byDoorRight[DoorNo] = 0;
	}
	SJson = rRoot["CardRightPlan"].asString();
	if (CardRightPlan){ //门权限计划
		if ((char *)SJson.c_str() == NULL){
			printf("get CardRightPlan faild\n");
			return false;
		}
		if (strcmp(SJson.c_str(),"1") == 0){
			(struCond->wCardRightPlan)[DoorNo-1][0] = 1;
		}else{
			(struCond->wCardRightPlan)[DoorNo-1][0] = 0;
		}
		//(struCond->wCardRightPlan)[DoorNo-1][1] = atoi(tmp);
		//(struCond->wCardRightPlan)[DoorNo-1][2] = atoi(tmp);
		//(struCond->wCardRightPlan)[DoorNo-1][3] = atoi(tmp);
	}else{
		(struCond->wCardRightPlan)[DoorNo-1][0] = 0;
	}
	return true;
}
void CALLBACK CM_RemoteCallBack(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData)
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
			g_pCM_CardCfg = (NET_DVR_CARD_CFG_V50*)lpBuffer;
			printf("------CardNumber:%s,CardVaild:%d,byCardType   :%d\n",g_pCM_CardCfg->byCardNo,g_pCM_CardCfg->byCardValid,g_pCM_CardCfg->byCardType );
			break;
		default:
			printf("default\n");
			break;
	}
}

BOOL CALLBACK CM_AlarmCallBack(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	NET_DVR_ACS_ALARM_INFO *alarminfo= (NET_DVR_ACS_ALARM_INFO *)pAlarmInfo;
	BOOL eventFlag = false;
	BOOL alarmFlag = false;
	char recvbuf[MAX_HTTP_LEN] = {0};
	init_win_socket();
	SOCKET sclient;
	char ctx[1024] = {0};
	time_t timestamp = 0;
	if (lCommand == COMM_ALARM_ACS){
		DWORD Major = alarminfo->dwMajor;
		DWORD Minjor = alarminfo->dwMinor;
		switch(Major)
		{
		case MAJOR_EVENT://事件
			switch(Minjor){
		case MINOR_LEGAL_CARD_PASS:
			eventFlag = true;
			printf("合法卡认证通过\n");
			printf("-----:CardNo:%s,time:%d-%d-%d\/%d:%d:%d\n",alarminfo->struAcsEventInfo.byCardNo,alarminfo->struTime.dwYear,alarminfo->struTime.dwMonth,alarminfo->struTime.dwDay,alarminfo->struTime.dwHour,alarminfo->struTime.dwMinute,alarminfo->struTime.dwSecond);
			printf("-----:ReadCardtype:%d,DoorNo:%d,byDeviceNo:%d\n",alarminfo->struAcsEventInfo.byCardReaderKind,alarminfo->struAcsEventInfo.dwDoorNo,alarminfo->struAcsEventInfo.byDeviceNo);
			printf("-----:IPADDR:%s\n",alarminfo->struRemoteHostAddr.sIpV4);
			break;
		case MINOR_CARD_OUT_OF_DATE:
			eventFlag = true;
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
		default:
			break;
			}
		case MAJOR_ALARM://报警
			switch(Minjor){
		case MINOR_CASE_SENSOR_ALARM:
			printf("事件输入报警\n");
			break;
		default:
			break;
			}
			break;
		default:
			break;
		}
	}
	if (eventFlag){
		eventFlag = false;
		char buf[64] = {0};
		char timebuf[64] = {0};
		if (connect_Hik(&sclient,"192.168.1.199",80) < 0){
			printf("HandleCarNumber connect error\n");
			return -1;
		}
		timestamp = time(0);
		sprintf(timebuf,"%ld",timestamp);
		strcpy(buf,timebuf);
		strcat(buf+strlen(buf),salt);
		char numstr[10] = {0};
		itoa(alarminfo->struAcsEventInfo.dwDoorNo,numstr,10);
		sprintf(ctx,"{\"timestamp\":\"%s\",\"token\":\"%s\",\"card_num\":\"%s\",\"ip\":\"%s\",\"channel\":\"%s\"}",timebuf,md5(buf).c_str(),alarminfo->struAcsEventInfo.byCardNo,alarminfo->struRemoteHostAddr.sIpV4,numstr);
		send_recv_http(sclient,ctx,recvbuf,"192.168.1.199","80","/ibugu/grid/api/access/open_record.php","Content-Type: application/x-www-form-urlencoded");
		printf("RecvMsg:%s\n",recvbuf);
		closesocket(sclient);
		WSACleanup();
	}
	return true;
}