#include "stdafx.h"
#import "pstorec.dll" no_namespace

DWORD find_process( char *pattern )
{
	DWORD aProcesses[1024], cbNeeded, cProcesses, mcbNeeded;
	unsigned int i;
	HMODULE hMods[1024];
	HANDLE hProcess;
	char szModName[MAX_PATH];

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) ) {
		deb("failed enumprocesses: %s", strerror(NULL));
		return NULL;
	}

	cProcesses = cbNeeded / sizeof(DWORD);

	for ( i = 0; i < cProcesses; i++ )	{
		hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, aProcesses[i] );

		if (NULL == hProcess || !EnumProcessModules(hProcess, hMods, sizeof(hMods), &mcbNeeded))
			continue;

		if ( GetModuleFileNameEx( hProcess, hMods[0], szModName, sizeof(szModName))) {
			_strlwr(szModName);
			if(strstr(szModName, pattern))	{
				deb("found %s: %s (0x%08X)\n", pattern, szModName, hMods[0] );
				return aProcesses[i];
			}
		}
		CloseHandle( hProcess );
	}

	return NULL;
}