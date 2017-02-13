#include <includes.h>
void CALLBACK callback(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData);
int main()
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
	if ( (lHandle = NET_DVR_StartRemoteConfig(lUserID,NET_DVR_SET_CARD_CFG,(LPVOID)&card_cfg,(DWORD)sizeof(card_cfg),(fRemoteConfigCallback)callback,&lHandle)) < 0){
		printf("start remoteConfig error:%d\n",NET_DVR_GetLastError());
		return -1;
	}


	NET_DVR_CARD_CFG pSendBuf;
	pSendBuf.dwSize = sizeof(pSendBuf);
	pSendBuf.dwModifyParamType = 0;
	strcpy_s((char *)pSendBuf.byCardNo, ACS_CARD_NO_LEN, "34120212");
	pSendBuf.byCardValid = 1;
	pSendBuf.byCardType = 1;
	/**/
	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&pSendBuf,sizeof(pSendBuf)) == FALSE){
		printf("SendRemoteCfg error:%d\n",NET_DVR_GetLastError());
		return -1;
	}

	card_cfg.dwSize = sizeof(card_cfg);
	card_cfg.dwCardNum = 0xffffffff;
	card_cfg.byCheckCardNo = 1;
	card_cfg.wLocalControllerID = 0;
	for(int i = 0; i < sizeof(card_cfg.byRes)/sizeof(card_cfg.byRes[0]); i++){
		card_cfg.byRes[i] = 0;
	}
	for(int i = 0; i < sizeof(card_cfg.byRes1)/sizeof(card_cfg.byRes1[0]); i++){
		card_cfg.byRes1[i] = 0;
	}

	if ( (lHandle = NET_DVR_StartRemoteConfig(lUserID,NET_DVR_GET_CARD_CFG,(LPVOID)&card_cfg,(DWORD)sizeof(card_cfg),(fRemoteConfigCallback)callback,&lHandle)) < 0){
		printf("start remoteConfig error:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	NET_DVR_CARD_CFG_SEND_DATA Get_Card_Cfg;
	Get_Card_Cfg.dwSize = sizeof(Get_Card_Cfg);
	strcpy_s((char *)Get_Card_Cfg.byCardNo, ACS_CARD_NO_LEN, "34120212");

	if (NET_DVR_SendRemoteConfig(lHandle,ENUM_ACS_SEND_DATA,(char *)&Get_Card_Cfg,sizeof(Get_Card_Cfg)) == FALSE){
		printf("SendRemoteCfg error:%d\n",NET_DVR_GetLastError());
		return -1;
	}
	printf("file end\n");
	//获取卡参数
	getchar();
	return -1;
}
void CALLBACK callback(DWORD dwType, void *lpBuffer, DWORD dwBufLen, void *pUserData)
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
			/**/
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
	/*
	printf("CallBack lHandle:%d\n",UserData);
	if (NET_DVR_StopRemoteConfig(UserData) == FALSE){
		printf("NET_DVR_StopRemoteConfig error:%d\n",NET_DVR_GetLastError());
		return ;
	}*/
}