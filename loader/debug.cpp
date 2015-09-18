#include "loader.h"

#ifndef FRELEASE
char* fmterr(void)
{
	LPVOID lpMsgBuf = NULL;
	static char szInternal[255];
	
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,0,NULL);
	
	lstrcpy(szInternal,(char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return szInternal;
}
/* ------------------------------------------------------------------------------*/
void fdeb(char *msg,...) 
{
	va_list ap;
	char *string;
	
	string = (char*) a(65535);
	
	va_start(ap,msg);
	wvsprintf(string,msg,ap);
	va_end(ap);
	OutputDebugString(string);
	f(string);
}
/* ------------------------------------------------------------------------------*/
void * a(WORD size) 
{
	void *p;
	p = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
	if(p == NULL) {
		//deb("HEAP FAILED!\n");
		return NULL;
	}
	return p;
	
}
/* ------------------------------------------------------------------------------*/
int f(char* p)
{
	if(p == NULL) {
		//deb("FREE NULL BLOCK!\n");
		return -1;
	}
	HeapFree(GetProcessHeap(),0,p);
	return 0;
}
/* ------------------------------------------------------------------------------*/
void delservice(char *name)
{
	SC_HANDLE sch = NULL;
	SC_HANDLE scs = NULL;
	
	sch = OpenSCManager(NULL,NULL,GENERIC_WRITE);
	if(sch == NULL)
	{
		//deb("Del OpenSCManager: %s\n",fmterr());
		return;
	}
	scs = OpenService(sch,name,SERVICE_ALL_ACCESS);
	if(scs == NULL)
	{
		//deb("Del OpenService: %s\n",fmterr());
		return;
	}
	if(DeleteService(scs) == 0)
	{
		//deb("Del DeleteService: %s\n",fmterr());
	}
	else
	{
		//deb("Service %s deleted.\n",name);
	}
	CloseServiceHandle(sch);
	return;
}
#endif
