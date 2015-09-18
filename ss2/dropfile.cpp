#include "stdafx.h"

int DropFile(char *fname,BYTE data[],DWORD size)
{
	DWORD write;
	HANDLE hFile;
	char path[MAX_PATH];

	GetWindowsDirectory(path,MAX_PATH);

	lstrcat(path,"\\");
	lstrcat(path,fname);

	hFile = CreateFile(path, GENERIC_ALL, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		deb("dropfile: failed to create file %s error: %s\n",
			path,FORMATERROR);
		CloseHandle(hFile);
		return ERR;
	}

	WriteFile(hFile,&data[0],size,&write,NULL);
	if(write != size)
	{
		deb("dropfile: not all bytes written! written:%lu need:%lu\n",
			write,size);
		CloseHandle(hFile);
		return ERR;
	}

	CloseHandle(hFile);

	return SUCCESS;
}