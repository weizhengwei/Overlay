// CoreOSOverlayDlg.h : header file
//
#include "OverlaySettingWnd.h"
#include "OverlayBrowserWnd.h"
#include "OverlayBrowserMainWnd.h"
#include "ui\UIWindow.h"

#pragma once


// CCoreOSOverlayDlg dialog
class CCoreOSOverlayDlg : public CUIWindow
{
// Construction
public:
	CCoreOSOverlayDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COREOSOVERLAY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	COverlaySettingWnd m_OverlaySettingWnd;
	COverlayBrowserWnd m_OverBrowserWnd;
	COverlayBrowserMainWnd m_OverlayBrowerMainWnd;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	void ShowBrowserMainWnd(LPCTSTR lpUrl);

	

	DECLARE_MESSAGE_MAP()

	void InitEvent();
	void OnNewsClick(ISonicBase *, LPVOID pReserve);
    void OnBillingClick(ISonicBase *, LPVOID pReserve);
	void OnSuportClick(ISonicBase *, LPVOID pReserve);
	void OnForumClick(ISonicBase *, LPVOID pReserve);
	void OnAchieveClick(ISonicBase *, LPVOID pReserve);
	void OnconnetClick(ISonicBase *, LPVOID pReserve);
	void OnSettingClick(ISonicBase *, LPVOID pReserve);
};
