
class XorConnection
{
private:
	SOCKET	sock;
	char	host[128];
	u_int	port;
	u_int	xor;
public:
	XorConnection(bool connect=false, char* host=NULL, int port=0);
	~XorConnection(void);
	int connect(char* host, u_int port, u_int timeout);
	int recv(char* buffer, u_int len);
	int send(char* buffer, u_int len);
	void disconnect(void);
	int close(void);
	int reset(void);
};