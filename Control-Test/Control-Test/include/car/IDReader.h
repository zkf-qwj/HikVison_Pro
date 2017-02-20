#include <string>
#include <windows.h>

#include <iostream>
#include<map>
#include "sdtapi.h"
#include <process.h>
#include "Windows.h"
#include <deque>
#pragma comment(lib,"IDReader.lib")
#pragma comment(lib,"Sdtapi.lib")
class IDReader
{
public:
	IDReader();
	~IDReader();

public:
	int ReaderInit(int (*pCallBack)(char * Name, char * Gender, char * Folk,char *BirthDay, char * Code, char * Address,char *Agency, char * ExpireStart,char* ExpireEnd),int (*pCallBack2)( char * Name));

public:
	char Name[256];
	char Gender[256];
	char Folk[256];
	char BirthDay[256];
	char Code[256];
	char Address[256];
	char Agency[256];
	char ExpireStart[256];
	char ExpireEnd[256];

	char IC[256];
	char Tmp[256];
	int m_Handle;
	int iRet;
public:
	int (*Callback1)( char * Name, char * Gender, char * Folk,char *BirthDay, char * Code, char * Address,char *Agency, char * ExpireStart,char* ExpireEnd);
	int (*Callback2)( char * Name);

public:
	static DWORD  WINAPI ReadID(LPVOID  pUser);
	static DWORD  WINAPI ReadIC(void * pUser);


};