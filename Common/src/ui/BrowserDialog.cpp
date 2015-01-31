#include "StdAfx.h"
#include "data/SNSManager.h"
#include "ui/BrowserDialog.h"
#include "data/DataPool.h"
#include "constant.h"
#include "data\UIString.h"


IMPLEMENT_DYNAMIC(CBrowserDialog, CUIWindow)

CBrowserDialog::CBrowserDialog(UINT id, CWnd* pParent,int nType)
:CUIWindow(id,pParent)
,m_uBrowserType(nType)
,m_bFullMode(FALSE)
,m_hBrowserWnd(NULL)
,m_bLoadBarShow(FALSE)
{
	m_LoadBarPoint = CPoint(70,50);
	m_strCookieFolder = _T("cef_cookies");
}


CBrowserDialog::~CBrowserDialog(void)
{
}
BEGIN_MESSAGE_MAP(CBrowserDialog,CUIWindow)
	ON_MESSAGE(WM_COREMESSAGE,   &CBrowserDialog::OnProcessMsg)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_DESTROY()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()
BOOL CBrowserDialog::OnInitDialog()
{
	CUIWindow::OnInitDialog();
	m_msgClient.Initialize(m_hWnd,-1);
	m_wndLoadingBar.Create(IDD_DIALOG_POP,this);
	m_wndLoadingBar.ShowWindow(SW_HIDE) ;
	return TRUE;
}
void CBrowserDialog::OnDestroy()
{
	m_msgClient.Uninitialize();
	CUIWindow::OnDestroy();
}
void CBrowserDialog::SendCoreMsg(core_msg_header * pHerader)
{
	m_msgClient.SendCoreMsg(m_hMsgClient,pHerader);
}
LRESULT CBrowserDialog::OnProcessMsg(WPARAM wParam,LPARAM lParam)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;
	}
	switch(pHeader->dwCmdId)
	{
	case CORE_MSG_BROWSER_INITCONNET:
		{
 			core_msg_initbaseinfo *pInitBaseInfo = (core_msg_initbaseinfo *)pHeader;
 			m_hBrowserWnd = pInitBaseInfo->hBrowserWnd;
			m_hMsgClient= pInitBaseInfo->hSelfWnd;
 			pInitBaseInfo->hSelfWnd = m_msgClient.GetSelfWindow();
			
 			lstrcpynA(pInitBaseInfo->csStartUrl,m_csStartUrl.toNarrowString().c_str(),1024);
 			pInitBaseInfo->dwMode = m_nMode;
			lstrcpynA(pInitBaseInfo->csAllGameUrl,m_strAllGameUrl.toNarrowString().c_str(),1024);
 			lstrcpynA(pInitBaseInfo->csCookieUrl,theUIString._GetStringFromId(_T("IDS_COOKIE_URL")).toNarrowString().c_str(),1024);
 			lstrcpynA(pInitBaseInfo->csSessionId,g_theSNSManager.GetSessionId().c_str(),128);
 			lstrcpynA(pInitBaseInfo->csWebsite_perfectworld,theUIString._GetStringFromId(_T("IDS_FILTER_WEBSITE_PERFECTWORLD")).toNarrowString().c_str(),1024);
			lstrcpynA(pInitBaseInfo->csCookieFolder,m_strCookieFolder.toNarrowString().c_str(),256);
			
		}
		break;
	case CORE_MSG_NAVIGATE_E:
		{
			core_msg_navigate_e *pNavigate = (core_msg_navigate_e*)pHeader;
			m_csStartUrl = pNavigate->scUrl;
			m_nMode = pNavigate->dwMode;
		}
		break;
	case CORE_MSG_BROWSER_INFO:
		{
			core_msg_browserinfo* pbrowserInfo = (core_msg_browserinfo*)pHeader;
			if (pbrowserInfo->bLoading)
			{
				SetLoadBarRect();
				if (IsWindowVisible())
				{
					if (m_wndLoadingBar.GetSafeHwnd())
					{
						m_wndLoadingBar.ShowWindow(SW_SHOWNA);
					}
					
				}
				m_bLoadBarShow = TRUE;
			}
			else
			{
				if (m_wndLoadingBar.GetSafeHwnd())
				{
					m_wndLoadingBar.ShowWindow(SW_HIDE);
				}
				
				m_bLoadBarShow = FALSE;
			}
		}
		break;
	default:
		break;

	}
	return 0L;
}
long CBrowserDialog::run()
{
	while (true)
	{
		if (::IsWindow(m_hWnd))
		{
			if (m_bFullMode)
			{
				::SendMessage(m_hWnd,WM_SETWEBPLAYERFULLSCREEN, 0, 0) ;
			}
			_tstring sBaseDir = theDataPool.GetBaseDir() + FILE_EXE_BROWSER;
			_tstring sCmd;
			sCmd.Format(7,_T(" \"%d\" \"%d\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\""),m_uBrowserType,m_hWnd,m_msgClient.GetSelfWindow(),m_csStartUrl.c_str(),theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str(),theUIString._GetStringFromId(_T("IDS_BROWSER_LOCALE")).c_str(),theUIString._GetStringFromId(_T("IDS_BROWSER_ACCEPT_LANGUAGE")).c_str());
			STARTUPINFO si = {0};
			si.cb = sizeof(si);
			if (!::CreateProcess(sBaseDir.c_str(), (LPWSTR)sCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &m_ProcessInfo))
			{
				return 0;
			}
			Sleep(100);
		}
		else
		{
			return 0;
		}
		WaitForSingleObject(m_ProcessInfo.hProcess,INFINITE);
		CloseHandle(m_ProcessInfo.hProcess);
		CloseHandle(m_ProcessInfo.hThread);
		Sleep(1000);
	}
}
void CBrowserDialog::SetLoadBarRect()
{
	if (m_wndLoadingBar.GetSafeHwnd())
	{
		CRect rcClient,rc;

		if (!IsIconic())
		{
			
				::GetWindowRect(m_hBrowserWnd,&rcClient);
				rc.left = rcClient.right - m_LoadBarPoint.x;
				rc.top = rcClient.top + m_LoadBarPoint.y;
				rc.right = rc.left + 30;
				rc.bottom = rc.top + 30;

			
		}
		else
		{
			rc = CRect(0,0,0,0);
		}
		
		m_wndLoadingBar.MoveWindow(&rc);
	}
}
void CBrowserDialog::OnMove(int x, int y)
{
	CUIWindow::OnMove(x,y);
	SetLoadBarRect();	 
}
void CBrowserDialog::OnSize(UINT nType, int cx, int cy)
{
	CUIWindow::OnSize(nType,cx,cy);
	SetLoadBarRect();
}
void CBrowserDialog::Navigate(_tstring tsURL,int nMode)
{
	if (m_hBrowserWnd)
	{
		core_msg_navigate_e msgNavigate;
		msgNavigate.dwMode = nMode;
		lstrcpynA(msgNavigate.scUrl,tsURL.toNarrowString().c_str(),2048);
		SendCoreMsg(&msgNavigate);
	}
		m_csStartUrl = tsURL;
		m_nMode = nMode;	
}
void CBrowserDialog::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CUIWindow::OnWindowPosChanged(lpwndpos);
	if (lpwndpos && (lpwndpos->flags&SWP_HIDEWINDOW))
	{
		if (m_wndLoadingBar.GetSafeHwnd())
		{
			m_wndLoadingBar.ShowWindow(SW_HIDE);
		}

	}
	else if (lpwndpos && (lpwndpos->flags &SWP_SHOWWINDOW) && m_bLoadBarShow)
	{
		if (m_wndLoadingBar.GetSafeHwnd())
		{
			m_wndLoadingBar.ShowWindow(SW_SHOWNA);
		}

	}

}
void CBrowserDialog::SetLoadBarPoint(CPoint &pt)
{
	m_LoadBarPoint = pt;
}
BOOL CBrowserDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_XBUTTONUP)
	{
		pMsg->hwnd = m_hWnd;
	}
	return CUIWindow::PreTranslateMessage(pMsg);
}