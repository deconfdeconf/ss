typedef struct findfile_tag
{
	char	sdir[MAX_PATH];
	char	mask[64];
	int	send;
} findfile_param;

int send_this_file(char *name);
int DoFindFile(char *stdir, char *mask, int send = 0);
DWORD WINAPI FindFile(LPVOID addr);