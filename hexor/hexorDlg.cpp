// hexorDlg.cpp : implementation file
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
// CHexorDlg dialog

CHexorDlg::CHexorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHexorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHexorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pEditBkBrush = new CBrush(RGB(0, 0, 0));
}

void CHexorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHexorDlg)
	DDX_Control(pDX, IDC_EDIT3, m_hex);
	DDX_Control(pDX, IDC_EDIT2, m_xor);
	DDX_Control(pDX, IDC_EDIT1, m_string);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHexorDlg, CDialog)
	//{{AFX_MSG_MAP(CHexorDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT1, OnChangeString)
	ON_EN_CHANGE(IDC_EDIT2, OnChangeXor)
	ON_EN_CHANGE(IDC_EDIT3, OnChangeHex)
	ON_BN_CLICKED(IDOK, OnCrypt)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHexorDlg message handlers

BOOL CHexorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHexorDlg::OnPaint() 
{
	COLORREF bk,green;
	CDC* pDC;
	PAINTSTRUCT ps;
	CBrush hkbrush;
	char buf[4];
	

	itoa(xor, buf, 10);
	m_xor.SetWindowText(buf);
	bk = RGB(0,0,0);
	green = 0x0011E734;
	pDC = this->GetDC();

	if(pDC != NULL) {
		BeginPaint(&ps);

		hkbrush.CreateSolidBrush(bk);
		pDC->FillRect(&ps.rcPaint,&hkbrush);

		EndPaint(&ps);
	} else {
		MessageBox("null pDC");
	}

	m_xor.SetLimitText(2);

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CHexorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CHexorDlg::OnChangeString() 
{
  u_char buf[1024], out[2050];

	if(m_string.m_hWnd == ::GetFocus()) {
		m_string.GetWindowText((char*)buf, 1024);
		string_hex_xor(buf, out, xor, strlen((char*) buf));
		m_hex.SetWindowText((char*)out);
		m_hex.UpdateData();
	} else {
		OnChangeHex();
	}
}

void CHexorDlg::OnChangeXor() 
{
	char hex[4];
	u_char buf[1024], out[2050];

	m_xor.GetWindowText(hex,3);
	sscanf(hex,"%x", &xor);

	m_string.GetWindowText((char*)buf, 1024);
	string_hex_xor(buf, out, xor, strlen((char*) buf));
	m_hex.SetWindowText((char*)out);
	m_hex.UpdateData();
}

void CHexorDlg::OnChangeHex() 
{
	u_char buf[2050], out[1024];

	if(m_hex.m_hWnd == ::GetFocus()) {
		m_hex.GetWindowText((char*)buf, 2050);
		xor_hex_string(buf, out, xor);
		m_string.SetWindowText((char*)out);
		m_string.UpdateData();
	} else {
		OnChangeString();
	}
}

void CHexorDlg::OnCrypt() 
{
	
}

HBRUSH CHexorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch (nCtlColor) {
	case CTLCOLOR_EDIT:
	case CTLCOLOR_MSGBOX:
		pDC->SetTextColor(RGB(0, 255, 0));
		pDC->SetBkColor(RGB(0, 0, 0));
		return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());
		
	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    }

	return hbr;
}
