// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifdef NDEBUG
#pragma comment(linker,"/merge:.text=.x /merge:.data=.x")
#pragma comment(linker,"/filealign:512 /SECTION:.x,EWRX /IGNORE:4078")
//#pragma comment(linker,"/nodefaultlib")
#pragma optimize("gsy",on)
#endif

#if !defined(AFX_STDAFX_H__9FC4ED6E_91F8_478E_82EA_AAFA276B323E__INCLUDED_)
#define AFX_STDAFX_H__9FC4ED6E_91F8_478E_82EA_AAFA276B323E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define u_char unsigned char
void xor_hex_string(u_char *value, u_char *result, int xor);
void string_hex_xor(u_char *value, u_char *result, int xor, DWORD len) ;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9FC4ED6E_91F8_478E_82EA_AAFA276B323E__INCLUDED_)
