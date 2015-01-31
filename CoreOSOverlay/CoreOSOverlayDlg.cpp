// CoreOSOverlayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CoreOSOverlay.h"
#include "CoreOSOverlayDlg.h"
#include "BrowserImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCoreOSOverlayDlg dialog




CCoreOSOverlayDlg::CCoreOSOverlayDlg(CWnd* pParent /*=NULL*/)
	: CUIWindow(CCoreOSOverlayDlg::IDD, pParent)
	,m_OverBrowserWnd(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCoreOSOverlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCoreOSOverlayDlg, CUIWindow)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()


	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CCoreOSOverlayDlg message handlers

BOOL CCoreOSOverlayDlg::OnInitDialog()
{
	CUIWindow::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_mainwnd"));
	// TODO: Add extra initialization here

	InitEvent();

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CCoreOSOverlayDlg::InitEvent()
{
	
		GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_news_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnNewsClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_billing_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnBillingClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_support_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnSuportClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_forum_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnForumClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_achieve_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnAchieveClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_connect_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnconnetClick) ;
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("overlay_setting_btn"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CCoreOSOverlayDlg::OnSettingClick) ;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCoreOSOverlayDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CUIWindow::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCoreOSOverlayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCoreOSOverlayDlg::OnBillingClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("https://billing.perfectworld.com/core/"));
}
void CCoreOSOverlayDlg::ShowBrowserMainWnd(LPCTSTR lpUrl)
{
	if (!m_OverlayBrowerMainWnd.GetSafeHwnd())
	{
		m_OverlayBrowerMainWnd.Create(IDD_OVERLAYBROWSERMAINWND,this);
	}
	m_OverlayBrowerMainWnd.ShowWindow(SW_SHOWNORMAL);
	CBrowserImpl::GetInstance()->CreateBrowser(lpUrl,-1);
}

void CCoreOSOverlayDlg::OnNewsClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("http://core.perfectworld.com/clientgamenews2/?game=woi"));	
}
void CCoreOSOverlayDlg::OnSuportClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("https://support.perfectworld.com/app/home"));	
}
void CCoreOSOverlayDlg::OnForumClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("http://woi-forum.perfectworld.com/"));
}
void CCoreOSOverlayDlg::OnAchieveClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("http://core.perfectworld.com/achievement/?style=client"));
}
void CCoreOSOverlayDlg::OnconnetClick(ISonicBase *, LPVOID pReserve)
{
	ShowBrowserMainWnd(_T("http://core.perfectworld.com/home/?style=client"));
}
void CCoreOSOverlayDlg::OnSettingClick(ISonicBase *, LPVOID pReserve)
{
	if (!m_OverlaySettingWnd.GetSafeHwnd())
	{
		m_OverlaySettingWnd.Create(IDD_OVERLAYSETTINGWND,this);
	}
	m_OverlaySettingWnd.ShowWindow(SW_SHOWNORMAL);
}