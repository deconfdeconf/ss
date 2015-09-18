#include "stdafx.h"

	int winxp = 0, win98 = 0, winnt = 0, win2k = 0, win95 = 0, winunk = 0,
		winme = 0, win2k3 = 0;
	char szLocalIP[17];

void SetSystemInfo(void)
{
	lstrcpy( szLocalIP, GetLocalInetAddr());

	deb("Set local ip to: %s\n", szLocalIP);
	switch(OSVersionCheck()) {
	case OS_UNKNOWN:
		winunk = 1;
		break;
	case OS_WIN95:
		win95 = 1;
		break;
	case OS_WINNT:
		winnt = 1;
		break;
	case OS_WIN98:
		win98 = 1;
		break;
	case OS_WINME:
		winme = 1;
		break;
	case OS_WIN2K:
		win2k = 1;
		break;
	case OS_WINXP:
		winxp = 1;
		break;
	case OS_WIN2K3:
		win2k3 = 1;
		break;
	default:
		winunk = 1;
		break;
	}
}