#include "StdAfx.h"
#include "extern.h"


/// Global variables

WORD IdentdStillNeed ;

/// Enable / Disable Ident server on port 113 with reply of user passed as an argument

DWORD IdentServer(WORD state,char* user)
{
	static DWORD dwIdentdThreadId = NULL;
	static HANDLE hIdentdThread = NULL; 
	static char tmpUser[64];
	
	if( state == ENABLE ) 
	{
		strncpy(tmpUser,user,64);
		IdentdStillNeed = 1;
		hIdentdThread = xCreateThread( NULL, 0, CoreIdentd, tmpUser, 0,	&dwIdentdThreadId); 

		if (hIdentdThread == NULL) 
		{
			deb( "xCreateThread for identd failed (%d)\n", GetLastError() ); 
			return ERR;
		}

		CloseHandle(hIdentdThread);

		return dwIdentdThreadId;
	} 
	else if( state == DISABLE ) 
	{
		IdentdStillNeed = 0;

		return dwIdentdThreadId;
	}
	
	return ERR;
}

DWORD WINAPI CoreIdentd( LPVOID user ) 
{
	SOCKET m_socket;
	int bytesInOut = SOCKET_ERROR;
	int remPort = 0,locPort = 0;
	DWORD nonBlock = 1;
	char rsbuf[128];
	char tmpbuf[64];
	char idbuf[64];
	char sUser[64];
	sockaddr_in service;
	SOCKET AcceptSocket;

	strncpy(sUser,(char*)user,64);
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	
	if ( m_socket == INVALID_SOCKET ) 
	{
		deb( "Error at socket(): %ld\n", WSAGetLastError() );
		udpdeb("socket() failed. err:%d\n",WSAGetLastError());
		ExitThread(ERR);
	}
	
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons( IDENTD_PORT );
	
	if ( bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) 
	{
		//udpdeb("port:%d bind() failed . err:%d\n",ntohs(service.sin_port),WSAGetLastError());
		deb("port:%d bind() failed . err:%d\n",ntohs(service.sin_port),WSAGetLastError());
		closesocket(m_socket);
		ExitThread(ERR);
	}
	
	if ( listen( m_socket, SOMAXCONN) == SOCKET_ERROR )
	{
		deb( "Identd: Error listening on socket.\n");
		ExitThread(ERR);
	}
	
	if(ioctlsocket(m_socket,FIONBIO,&nonBlock) ) 
	{
		deb( "Identd: Error ioctl on socket.\n");
		ExitThread(ERR);
	}
	
	while (1) 
	{
		AcceptSocket = (SOCKET) SOCKET_ERROR;
		while ( AcceptSocket == (SOCKET) SOCKET_ERROR ) 
		{
			if(!IdentdStillNeed) 
			{
				deb("identd exiting...\n");
				closesocket(m_socket);

				if(AcceptSocket != (SOCKET) SOCKET_ERROR)
					closesocket(AcceptSocket);

				ExitThread(SUCCESS);
			}

			if(rtr(m_socket,1) == SUCCESS) 
				AcceptSocket = accept( m_socket, NULL, NULL );

			deb("identd: accept socket = %d", AcceptSocket);

			if(AcceptSocket == (unsigned int) -1) 
			{
				deb("identd exiting...\n");
				closesocket(m_socket);
				ExitThread(SUCCESS);
			}
		}

		memset(rsbuf, 0x0, 128);
		memset(tmpbuf, 0x0, 64);
		memset(idbuf, 0x0, 64);

		bytesInOut = SockReadLine( AcceptSocket, tmpbuf, 64, 3);

		if(bytesInOut == SOCKET_ERROR) 
		{
			closesocket(AcceptSocket);
			continue;
		}

		sscanf(tmpbuf,"%d , %d", &locPort, &remPort);
		_snprintf(rsbuf,128,"%d , %d : USERID : UNIX : %s\n", locPort, remPort, sUser);
		bytesInOut = send( AcceptSocket, rsbuf, strlen(rsbuf), 0 );
		closesocket(AcceptSocket);
	}
}