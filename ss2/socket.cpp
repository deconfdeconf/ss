#include "StdAfx.h"

/// Socket functions

int ReadyToRead(SOCKET Socket,WORD timeout)
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
			return SUCCESS;
	}

	if(dwResult == SOCKET_ERROR)
		return SOCKET_ERROR;
	
	return ERR;
}

int fast_select(SOCKET Socket,WORD timeout)
{
	fd_set          fds;
	struct timeval  tm;
	int dwResult;
	
	memset(&tm, 0x0,sizeof(tm));
	tm.tv_sec = 0;
	tm.tv_usec = timeout;
	FD_ZERO(&fds);
	FD_SET(Socket, &fds);
	
	if ( (dwResult = select(Socket + 1, &fds, &fds, NULL, &tm)) > 0) 
	{
		if(FD_ISSET(Socket,&fds))
			return SUCCESS;
	}
	
	if(dwResult == SOCKET_ERROR) 
	{
		deb("in fast_select(): WSA : %d\n",WSAGetLastError());
		return SOCKET_ERROR;
	}
	
	return 1;
}

int SockReadLine(SOCKET socket,char *buf,WORD bufsiz,WORD timeout) {
	int res;
	
	while(ReadyToRead(socket,timeout) == SUCCESS) 
	{
		res = recv(socket,buf++,1,0);
		if(res == SOCKET_ERROR) 
			break;

		if(*(buf - 1) == 0x0 || *(buf - 1) == 0xd ) 
		{
			*buf = 0x0;
			break;
		}
	}
	
	return res;
}

WORD SockRead(SOCKET socket,char* buf,WORD bufsiz,WORD timeout)
{
	DWORD nonBlock = 0;
	
	memset(buf,0x0,bufsiz);
	
	if(ioctlsocket(socket, FIONBIO, &nonBlock) ) 
	{
		deb( "Error ioctl on socket.\n");
		ExitThread(ERR);
	}
	
	return recv(socket,buf,bufsiz,0);
}

DWORD SockReadAll(SOCKET socket,char* buf,long bufsiz,WORD timeout)
{
	int  curRead,totRead = 0;
	DWORD nonBlock = 0;
	WORD int_count = 0;
	char  *tmpBuf,*bData;
	int dwResult;
	
	
	memset(buf,0x0,bufsiz);
	
	if(ioctlsocket(socket,FIONBIO,&nonBlock) ) 
	{
		deb( "Error ioctl on socket.\n");
		return ERR;
	}
	
	bData = (char*) a(1024);
	tmpBuf = (char*) a(INTERNAL_TEMP_BUF);
	
	if(tmpBuf == NULL || bData == NULL) 
	{
		deb("malloc() error\n");
		return ERR;
	}
	
	while(1) 
	{
		dwResult = ReadyToRead(socket,timeout);
		if( dwResult == SUCCESS) 
		{
			curRead = recv(socket,bData,1024,0);
			if(curRead == SOCKET_ERROR || curRead == 0) 
			{
				f(bData);
				f(tmpBuf);
				if(int_count) 
				{
					return SUCCESS;
				} else {
					return curRead;
				}
			}
			
			/*if(curRead <= 0 || curRead == SOCKET_ERROR) {
			break;
		}*/
			if((totRead + curRead)> bufsiz) 
			{
				break;
			}
			
			memcpy((void*) (tmpBuf + totRead), bData,curRead);
			totRead += curRead;
			int_count++;
		} else if(dwResult == SOCKET_ERROR) 
		{
			f(bData);
			f(tmpBuf);
			if(int_count) 
			{
				return totRead;
			} else {
				return curRead;
			}
		}
	}
	
	if(totRead > bufsiz) 
		totRead = bufsiz;
	
	memcpy( buf, tmpBuf,bufsiz);
	
	f(bData);
	f(tmpBuf);
	return totRead;
}
