//define 'LEAN_AND_MEAN';
#include "stdafx.h"
#include <windows.h>

HWINSTA hwinstaSave;
HDESK hdeskSave;
HWINSTA hwinstaUser;
HDESK hdeskUser;

int SetUserDesktop(int mode)
{
	hwinstaSave = GetProcessWindowStation();
	hdeskSave = GetThreadDesktop(GetCurrentThreadId());
	hwinstaUser = OpenWindowStation("WinSta0",
		FALSE, MAXIMUM_ALLOWED);
	if(hwinstaUser!=NULL)
	{
		SetProcessWindowStation(hwinstaUser);

		hdeskUser = OpenDesktop("Default", 0, FALSE,
			MAXIMUM_ALLOWED);
		if((int)hdeskUser!=NULL)
		{
			SetThreadDesktop(hdeskUser);
			deb("Switched to user desktop.\n");
			return 0;
		}

		SetProcessWindowStation(hwinstaSave);
		CloseWindowStation(hwinstaUser);
		return 0;
	}
	else
	{
		return -1;
	}
}
