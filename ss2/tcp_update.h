#define MONITORING_INTERVAL 57600000

#pragma pack(1)
struct update_msg
{
	u_int version;
	u_int size;
	u_int checksum;
} ;
#pragma pack()

int write_file( char *pbuf, u_int size, u_int checksum );
DWORD WINAPI monitor_updates(LPVOID param);