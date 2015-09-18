#define BIO_BUF_SIZE 16384
#define BIO_SOCKET_TIMEOUT 5

typedef BOOL (WINAPI* PInternetGetConnectedState) (LPDWORD, DWORD);
int sockets_bio(SOCKET first, SOCKET two);
int check_inet_state(void);