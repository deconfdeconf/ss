#include "stdafx.h"

void WindowsControl(WORD mode)
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
		
	if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		udpdeb("OpenProcessToken: %s",FORMATERROR); 
		
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
	
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
		
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
		
	if (GetLastError() != ERROR_SUCCESS) 
		udpdeb("AdjustTokenPrivileges: %s",FORMATERROR); 
	
	if( ExitWindowsEx((UINT) mode, 0) )
		udpdeb("Sent control event ok...\n");
	else
		udpdeb("Failed sent control event: %s\n",FORMATERROR);

	return;
}

int GetUptime(unsigned int s,int *days, int *hours, int *mins, int *secs) 
{
	s /= 1000;


	*days = s / 86400;
	s = s % 86400;
	*hours = s / 3600;
	s = s % 3600;
	*mins = s / 60;
	*secs = s % 60;

	return 1;
}
