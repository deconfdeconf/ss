// hexorDlg.h : header file
//

#if !defined(AFX_HEXORDLG_H__4197B92F_BC08_4E84_88A5_AA722BE37FAF__INCLUDED_)
#define AFX_HEXORDLG_H__4197B92F_BC08_4E84_88A5_AA722BE37FAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CHexorDlg dialog

class CHexorDlg : public CDialog
{
// Construction
public:
	CHexorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CHexorDlg)
	enum { IDD = IDD_HEXOR_DIALOG };
	CEdit	m_hex;
	CEdit	m_xor;
	CEdit	m_string;
	u_char xor;
	CBrush* m_pEditBkBrush;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHexorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHexorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeString();
	afx_msg void OnChangeXor();
	afx_msg void OnChangeHex();
	afx_msg void OnCrypt();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEXORDLG_H__4197B92F_BC08_4E84_88A5_AA722BE37FAF__INCLUDED_)
