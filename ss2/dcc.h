
typedef struct DCCSEND
{
        int unused;
        char target[MAX_PATH];
        char file[MAX_PATH];
        char sfile[MAX_PATH];
} DCCSEND;

DWORD WINAPI DccSend(LPVOID param);