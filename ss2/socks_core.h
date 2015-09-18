

/* socks 5 request and reply packets */
struct s5_packet {
	char ver;
	char cmd;
	char rsv;
	char atyp;
	struct in_addr  sin_addr;
	WORD d_port;
} ;

struct s5_reply {
	char ver;
	char rep;
	char rsv;
	char atyp;
	struct in_addr  local_addr;
	WORD local_port;
} ;

/* socks 4 request and reply packets */
struct s4_packet {
	char ver;
	char cd;
	WORD d_port;
	struct in_addr sin_addr;
	char userid[10];
} ;

struct s4_reply {
	char vn;
	char cd;
	WORD d_port;
	struct in_addr sin_addr;
} ;