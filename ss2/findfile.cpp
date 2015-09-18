#include "stdafx.h"




DWORD WINAPI FindFile(LPVOID addr)
{
	WIN32_FIND_DATA data;
	char path[MAX_PATH];
	char mask[64];
	char fname[MAX_PATH];
	char fullpath[MAX_PATH];
	int  send;
	findfile_param *param, *second;
	HANDLE hSearch;
	BOOL ret = 1;
	
	param = (findfile_param*) addr;

	strcpy(path, param->sdir);
	strcpy(mask, param->mask);
	send = param->send;

	/*deb("called FindFile: addr: 0x%p sdir: %s mask: %s addr->sdir: %s addr->mask: %s",
		addr,path, mask, param->sdir, param->mask);*/

	wsprintf(fname, "%s\\%s", path, mask);
	hSearch = FindFirstFile(fname, &data);
	if(hSearch != INVALID_HANDLE_VALUE) {
		while(ret && GetLastError() != ERROR_NO_MORE_FILES) {
			wsprintf(fullpath, "%s\\%s", path, data.cFileName);
			udpdeb("Found target: %s Size: %d\n", fullpath, data.nFileSizeLow);
			if(send) {
				send_this_file(fullpath);
			}
			ret = FindNextFile(hSearch,&data);
		}
		FindClose(hSearch);
	}
	
	ret = 0;
	wsprintf(fname, "%s\\*.*", path);
	SetLastError(0);
	hSearch = FindFirstFile(fname, &data);
	//deb("hSearch: %x ret: %x", hSearch, ret);

	if(hSearch != INVALID_HANDLE_VALUE) 
	{
		while(ret != ERROR_NO_MORE_FILES) 
		{
			//deb("found: %s", data.cFileName);
			if( strcmp(data.cFileName,".") != 0 && strcmp(data.cFileName,"..") != 0) 
			{
				if( (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))	
				{
					//deb("directory %s", data.cFileName);
					second = (findfile_param*) malloc(sizeof(findfile_param));
					wsprintf(second->sdir, "%s\\%s", path, data.cFileName);
					wsprintf(second->mask, "%s", mask);
					second->send = send;
					FindFile((LPVOID) second);
					SetLastError(0);
					free(second);
				}
				//else
					//deb("file %s\\%s", path, data.cFileName);		
			}
			FindNextFile(hSearch,&data);
			ret = GetLastError();
			//deb("while_end: ret: %x", ret);
		}
		FindClose(hSearch);
	} 
	else 
	{
		deb("invalid search handle. %s", FORMATERROR);
	}
	//deb("done search.");

	return 0;
}

int DoFindFile(char *stdir, char *mask, int send )
{
	static findfile_param param;
	DWORD id;

	strcpy(param.sdir, stdir);
	strcpy(param.mask, mask);
	param.send = send;

	xCreateThread(NULL, 0, FindFile, (LPVOID) &param, 0, &id);
	udpdeb("Searching for %s from %s...\n", mask, stdir);

	return 0;
}
int send_this_file(char *name)
{
	DWORD thread, dwCode = STILL_ACTIVE;
	HANDLE hThread;

	hThread = xCreateThread(NULL, 0, &udp_sendfile, name, 0, &thread);

	if(hThread == NULL) 
	{
		deb("xCreateThread failed for udp_sendfile: %s", FORMATERROR);
		return -1;
	}

	while(dwCode == STILL_ACTIVE) 
	{
		if(!GetExitCodeThread(hThread, &dwCode)) 
		{
			deb("getexitcodethread: %s", FORMATERROR);
			break;
		}
	}

	return 0;
}