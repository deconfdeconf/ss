
#define HEADERS_BUF 1024
#define HTTP_MESSAGE_TEMPLATE "<html><body bgcolor=black><font color=red>%message</font></body></html>"

typedef struct SETUP {
	int port;
	int proxy_allowed;
	char root[MAX_PATH];
	SOCKET s;
} SETUP;

typedef struct REQUEST {
	char method[12];
	char version[10];
	char url[512];
	char host[256];
	char connection[128];
} REQUEST;

int HttpServer(WORD state,int port, char *root = NULL);
DWORD WINAPI CoreHttp( LPVOID mysetup );
DWORD WINAPI ProcessHttpClient( LPVOID s );
int HttpReadHeaders( SOCKET , char *headers);
int ParseHeaders(SOCKET client, char *headers, REQUEST *request);
int MarkHackerIp(SOCKET client);
int MakeLocalPath(char *url,char *requrl,char *root);
int HttpMessage(SOCKET client, int code, char *msg);
char *GetContentType(char *url);
int HttpListing(SOCKET client, char *root);