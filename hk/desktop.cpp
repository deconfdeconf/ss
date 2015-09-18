#include "hk.h"
#pragma code_seg(".yaxaxaxaxaxa")
int SetUserDesktop( int mode )
{
    static HWINSTA hwinstaSave; 
    static HDESK hdeskSave; 
    static HWINSTA hwinstaUser; 
    static HDESK hdeskUser; 
	
	if(mode == 1) {
		hwinstaSave = GetProcessWindowStation(); 
		hdeskSave = GetThreadDesktop(GetCurrentThreadId()); 
		
		hwinstaUser = OpenWindowStation("WinSta0", FALSE, MAXIMUM_ALLOWED); 
		if (hwinstaUser == NULL) 
			return 0; 
		SetProcessWindowStation(hwinstaUser); 
		hdeskUser = OpenDesktop("Default", 0, FALSE, MAXIMUM_ALLOWED); 
		if (hdeskUser == NULL) { 
			SetProcessWindowStation(hwinstaSave); 
			CloseWindowStation(hwinstaUser); 
			return 0; 
		} 
		SetThreadDesktop(hdeskUser); 
	} 
	else if(mode == 0) {
		SetThreadDesktop(hdeskSave); 
		SetProcessWindowStation(hwinstaSave); 
		CloseDesktop(hdeskUser); 
		CloseWindowStation(hwinstaUser); 
	}
	
    return 0; 
} 
