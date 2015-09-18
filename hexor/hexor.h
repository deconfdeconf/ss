// hexor.h : main header file for the HEXOR application
//

#if !defined(AFX_HEXOR_H__080518B7_8A63_4C46_8BFC_B83D050A993C__INCLUDED_)
#define AFX_HEXOR_H__080518B7_8A63_4C46_8BFC_B83D050A993C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHexorApp:
// See hexor.cpp for the implementation of this class
//

class CHexorApp : public CWinApp
{
public:
	CHexorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHexorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHexorApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEXOR_H__080518B7_8A63_4C46_8BFC_B83D050A993C__INCLUDED_)
