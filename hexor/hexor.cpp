// hexor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "hexor.h"
#include "hexorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHexorApp

BEGIN_MESSAGE_MAP(CHexorApp, CWinApp)
	//{{AFX_MSG_MAP(CHexorApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHexorApp construction

CHexorApp::CHexorApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHexorApp object

CHexorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CHexorApp initialization

BOOL CHexorApp::InitInstance()
{
	// Standard initialization

	CHexorDlg dlg;
	m_pMainWnd = &dlg;
	dlg.xor = 0x00;
	SetDialogBkColor(RGB(0, 0, 0), 0x0011E734);
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void string_hex_xor(u_char *value, u_char *result, int xor, DWORD len) 
{
	DWORD i;
	u_char *p = result;
	
	memset(result, 0x0, (len * 2) + 1);
	for(i=0;i<len;i++) {
		sprintf((char*) p, "%02X", xor ? value[i] ^ xor : value[i]);
		p += 2;
	}
}

void xor_hex_string(u_char *value, u_char *result, int xor)
{
	DWORD i = 0x0;
	u_char *p = result;
	u_char byte[3] = { 0x0, 0x0, 0x0 };
	u_char chr;

	memset((char*) result, 0x0, strlen((char*) value));
	while(value[i]) {
		byte[0] = value[i++];
		byte[1] = value[i++];
		sscanf((char*) byte, "%x", &chr);
		(*p) = xor ? chr ^ xor : chr;
		//deb("hex: 0x%02x xor: 0x%02x", chr, (*p));
		p += 1;
	}
}
