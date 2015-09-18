/*#if !defined(AFX_STDAFX_H__0366F203_612A_4CAB_AE40_E339990B0603__INCLUDED_)
#define AFX_STDAFX_H__0366F203_612A_4CAB_AE40_E339990B0603__INCLUDED_
#endif*/

#pragma comment(linker,"/filealign:512 /section:.x,RWX /merge:.text=.x /merge:.data=.x /merge:.rsrc=.x /merge:.rdata=.x /IGNORE:4078")
#pragma comment(linker,"/nodefaultlib")
#pragma optimize("gsy",on)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <initguid.h>
#include <exdisp.h>        // the IID_IWebBrowser is located over there

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif
#include <atlimpl.cpp>


void fdeb(char *msg,...) ;
char* fmterr(void);


/*
EXTERN_C const IID LIBID_intLib;

EXTERN_C const CLSID CLSID_Int;

#ifdef __cplusplus

class DECLSPEC_UUID("5C372352-90D0-4214-BF20-8E4A2B82F980") Gopher;
#endif
typedef interface IInt Iint;
EXTERN_C const IID IID_IInt;
/////////////////////////////////////////////////////////////////////////////
// CInt
class ATL_NO_VTABLE CInt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CInt, &CLSID_Int>,
	public IObjectWithSiteImpl<CInt>
	//public IDispatchImpl<IInt, &IID_IInt, &LIBID_intLib>
{
public:
	CInt()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_Int)

BEGIN_COM_MAP(CInt)
	COM_INTERFACE_ENTRY(IInt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

// IInt

// IDispatch
public:    
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, 
		              VARIANT*, EXCEPINFO*, UINT*);
// IObjectWithSite
public:
    STDMETHOD(SetSite)(IUnknown *pUnkSite);
    

private:
    DWORD m_dwCookie;
    HWND m_hwndBrowser;
    CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> m_spWebBrowser2;
    CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> m_spCPC;

};*/

