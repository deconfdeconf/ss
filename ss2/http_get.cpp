#include "StdAfx.h"
#include "extern.h"

struct httpget_config {
	char url[255];
	char out_file[255];
	WORD max_minutes;
	WORD loop;
} conf;

DWORD HttpGet(WORD threads_num,WORD time,char *url,char *out_file)
{
	DWORD dwGetThreadId = NULL;
	HANDLE hGetThread = NULL; 
	int threads_ok = 0;
	
	strncpy(conf.url,url,sizeof(conf.url));
	conf.max_minutes = time;
	if(out_file) {
		strncpy(conf.out_file,out_file,sizeof(conf.out_file));
		conf.loop = 0;
	} else {
		conf.out_file[0] = 0x0;
	}
	if(threads_num > 1) {
		conf.loop = 1;
	}
	
	while(threads_num) 
	{
		hGetThread = xCreateThread( NULL, 0, HttpGetThread, (void*) &conf, 0, &dwGetThreadId);  
		if (hGetThread == NULL) 
		{
			deb( "xCreateThread for HttpGet failed (%d)\n", GetLastError() ); 
		} 
		else 
		{
			threads_ok++;
		}
		threads_num--;
	}
	
	if(threads_num > 1) {
		return threads_ok;
	} else {
		return (DWORD) hGetThread;
	}
}

DWORD WINAPI HttpGetThread( LPVOID config )
{
	SOCKADDR_IN sin;
	SOCKET s;
	WSADATA wsaData;
	const char *host, *path;
	char url[255];
	char hostname[256];
	int port, len;
	struct httpget_config *conf;
	char buf[HTTP_GET_BUF_SIZE];
	int done = 0;
	WORD max_seconds;
	time_t starttime,curtime;
	char out_file[255];
	FILE *f = NULL;
	WORD loop;
	char request[10];
	
	conf = (struct httpget_config*) config;
	
	loop = conf->loop;
	max_seconds = conf->max_minutes*60;
	strncpy(url,conf->url,255);
	strncpy(out_file,conf->out_file,255);
	
	//deb("httpgetthread: loop=%d,max_seconds=%d,url=%s,out_file=%s\n",loop,max_seconds,url,out_file);
	
	if (_strnicmp(url, "http:", 5) != 0)	
	{
		deb("Only http URLs are supported.\n");
		ExitThread(SUCCESS);
	}
	
	if (url[5] != '/' || url[6] != '/' || url[7] == 0) 
	{
		deb("Malformed URL: %s.\n", url);
		ExitThread(SUCCESS);
	}
	host = url + 7;
	path = strpbrk(host, ":/");
	len = path ? path - host : strlen(host);
	if (len == 0) 
	{
		deb("Hostname missing: %s.\n", url);
		ExitThread(SUCCESS);
	}
	if (len > 255) 
	{
		deb("Hostname too long, "
			"\"[must be] 255 octets or less\" (RFC 1035).\n");
		ExitThread(SUCCESS);
	}
	strncpy(hostname, host, len)[len] = 0;
	
	if (path && *path == ':' && *++path != 0 && *path != '/') 
	{
		if ((port = strtol(path, (char **)&path, 10)) >> 16 || (*path != 0 && *path != '/')) 
		{
			deb("Invalid port number.\n");
			ExitThread(SUCCESS);
		}
	} 
	else 
	{
		port = 80;
	}
	
	if (!path || *path == 0) 
		path = "/";
	
    sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if((sin.sin_addr.s_addr = DnsResolve(hostname)) == ERR) 
	{
		deb("httpget: resolve error\n");
		ExitThread(SUCCESS);
	}
	
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	
	if (iResult != NO_ERROR) 
	{
		deb("Error at WSAStartup() :%d\n",WSAGetLastError());
		ExitThread(SUCCESS);
	}
	
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (s == INVALID_SOCKET) 
	{
		deb("Error at socket(): %ld\n", WSAGetLastError());
		ExitThread(SUCCESS);
	}
	
	time(&starttime);
	DWORD total_read = 0;
	DWORD total_write = 0;
	/*&if(!loop)
	udpdeb("httpget: \"%s\" => \"%s\".\n",url,out_file);*/
	
	//SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);
	if((sin.sin_addr.s_addr = DnsResolve(hostname)) == ERR) 
	{
		deb("httpget: resolve error\n");
		ExitThread(SUCCESS);
	}
	
	int tryes = 0;
	if(!loop) 
	{
		max_seconds = 120;
		f = fopen(out_file,"wb");
		if(f == NULL) 
		{
			deb("httpget: fopen error.\n");
			fclose(f);
			_unlink(out_file);
			ExitThread(SUCCESS);
		}
	}
	
	while(!done) 
	{
s_connect:
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	
	if (s == INVALID_SOCKET) 
	{
		deb("Error at socket(): %ld\n", WSAGetLastError());
		fclose(f);
		_unlink(out_file);
		ExitThread(SUCCESS);
	}	
	
	time(&curtime);
	if( (curtime - starttime) >= max_seconds) 
	{
		//deb("httpget: threads max_seconds %d elapsed.\n",max_seconds);
		if(logmsgs_num<64) 
			udpdeb("httpget: threads max_seconds %d elapsed. url=%s\n",max_seconds,url);
		if(f) 
		{
			fclose(f);
			_unlink(out_file);
		}
		closesocket(s);
		ExitThread(SUCCESS);
	}
	if(connect( s, (SOCKADDR*) &sin, sizeof(sin) ) == SOCKET_ERROR) 
	{
		deb( "httpget: failed connect to %s:%d after %d tryes. error:%d\n",
			inet_ntoa(sin.sin_addr), port, tryes, WSAGetLastError());
		closesocket(s);
		SleepEx(CONNECT_SLEEP,FALSE);
		tryes++;
		continue;
	}
	tryes = 0;
	strncpy(request,"GET",10);
	_snprintf(buf, HTTP_GET_BUF_SIZE, "%s %s HTTP/1.0\r\n"
		"User-Agent: %s\r\n"
		"Host: %s\r\n\r\n", request, path, HTTP_USER_AGENT, hostname);
	//deb("HTTP=%s",buf);
	int res = send(s,buf,strlen(buf),0);
	if(res == SOCKET_ERROR) 
	{
		closesocket(s);
		continue;
	}
	if(!loop) 
	{
		int code = GetHTTPRespCode(s);
		if(code == HTTP_NOT_FOUND || code == HTTP_FORBIDDEN) 
		{
			udpdeb("%s => %d\n",url,code);
			closesocket(s);
			if(f)
				fclose(f),_unlink(out_file);
			ExitThread(SUCCESS);
		} 
		else if(code == ERR) 
		{
			udpdeb("%s => connection was reset.\n",url);
			closesocket(s);
			if(f)
				fclose(f),_unlink(out_file);
			ExitThread(SUCCESS);
		}
	}
	while(ReadyToRead(s,HTTP_TIMEOUT) == SUCCESS) 
	{
		res = recv(s,buf,HTTP_GET_BUF_SIZE,0);
		if(res == SOCKET_ERROR || res < 0) 
		{
			deb("httpget: error at recv() %d\n",WSAGetLastError());
			if(!loop) 
			{
				udpdeb("httpget: error at recv() %d\n",WSAGetLastError);
				if(f)
					fclose(f);
				closesocket(s);
				ExitThread(SUCCESS);
			}
			closesocket(s);
			goto s_connect;
		}
		if(res == 0) 
		{
			if(!loop) 
			{
				deb("httpget: %d bytes saved as \"%s\".\n",total_read,out_file);
				udpdeb("httpget: %d bytes saved as \"%s\".\n",total_read,out_file);
				if(f)
					fclose(f);
				closesocket(s);
				ExitThread(SUCCESS);
			}
			closesocket(s);
			goto s_connect;
		}
		total_read += res;
		if(!loop) 
		{
			int r;
			r = fwrite(buf,res,1,f);
			total_write += r;
		}
	}
	closesocket(s);
	if(!loop) 
	{
		done = 1;
		deb("httpget: timeout at byte %d.\n",total_read);
		udpdeb("httpget: timeout at byte %d.\n",total_read);
		if(f)
			fclose(f),_unlink(out_file);
		ExitThread(SUCCESS);
	}
	done = 1;
#ifndef FRELEASE
	Sleep(1000);
#endif
	}
	
	if(f)
		fclose(f);
	closesocket(s);
	ExitThread(SUCCESS);
}

int GetHTTPRespCode(SOCKET s)
{
	char *buf,*p;
	int res,bytes = 0;
	int code = 0;
	
	buf = (char*) a(1024);
	p = buf;
	
	do 
	{
		if(bytes++ > 6) 
			if(memcmp(p-4,"\r\n\r\n",4) == 0)
				break;
			res = recv(s,p++,1,0);
			if(res == 0) 
				return ERR;
	} while( bytes < 1023 );
	
	if( (p = strstr(buf,"HTTP/")) == NULL) {
		
		return ERR;
	}
	
	sscanf(p,"HTTP/%*d.%*d %d",&code);
	
	
	return code;
}