#include "stdafx.h"

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

void
fdeb(char *msg,...) 
{
        va_list ap;
		char string[32768];

        va_start(ap,msg);
		wvsprintf(string,msg,ap);
        va_end(ap);
		OutputDebugString(string);
}