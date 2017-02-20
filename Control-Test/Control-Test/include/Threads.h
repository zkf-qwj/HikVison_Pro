#ifndef _THREADS_H_
#define _THREADS_H_
#include "includes.h"



void generic_handler(struct evhttp_request *req, void *arg);
unsigned int __stdcall DoorThreadFun(PVOID pM);
unsigned int __stdcall CameraThreadFun(PVOID pM);
unsigned int __stdcall IDReaderFun(PVOID pM);
unsigned int __stdcall evThreadFun(PVOID pM);
#endif