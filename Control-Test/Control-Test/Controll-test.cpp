#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include <process.h>
#include "HCNetSDK.h"
#pragma comment(lib,"HCNetSDk.lib") 
using namespace std;
typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindows;
int iNum = 0;
int g_count = 0;
LONG g_DlUserID = 0;
LONG g_ClUserID = 0;

unsigned int __stdcall CameraThreadFun(PVOID pM)
{	
	printf(" child thread beginthread ID:%d,g_count:%d\n",GetCurrentThreadId(),g_count);
g_count++;
	return 0;
}
unsigned int __stdcall DoorThreadFun(PVOID pM)
{	
	printf(" child thread beginthread ID:%d,g_count:%d\n",GetCurrentThreadId(),g_count);
g_count++;
	return 0;
}

void main()
{

	NET_DVR_Init();
	NET_DVR_SetConnectTime(2000, 1);
	LONG lUserID;
	BOOL BRet;
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	g_DlUserID = NET_DVR_Login_V30("192.168.1.64", 8000, "admin", "@vsea.tv", &struDeviceInfo);
	if (g_DlUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	g_ClUserID = NET_DVR_Login_V30("192.168.1.66", 8000, "admin", "@vsea.tv", &struDeviceInfo);
	if (g_ClUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}


	//NET_DVR_GetDeviceConfig(lUserID,);
	const int THREAD_NUM = 2;
	HANDLE handle[THREAD_NUM]; 
	handle[0] =  (HANDLE)_beginthreadex(NULL, 0, CameraThreadFun, NULL, 0, NULL);
	handle[1] =  (HANDLE)_beginthreadex(NULL, 0, DoorThreadFun, NULL, 0, NULL);
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE ,INFINITE); 
	NET_DVR_Logout(g_DlUserID);
	NET_DVR_Logout(g_ClUserID);
	NET_DVR_Cleanup();
	while(1);
	return;
}