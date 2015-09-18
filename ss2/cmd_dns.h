
enum { CMD_SET = 0x01, CMD_DEL, CMD_DMP };

int process_cmd_dns(char *cmdline);
int process_cmd(HANDLE hHostsfile, int cmd, char *szHostname, char *szIp);
