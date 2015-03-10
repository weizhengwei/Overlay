#include "StdAfx.h"
#include "OverlayImpl.h"
#include "CoreMsgOverlay2Browser.h"
#include "BrowserImpl.h"
#include "OverlayImImpl.h"


COverlayImpl::COverlayImpl(void)
:m_pListBox(NULL)
,m_pIOverlayMainWnd(NULL)
{
}

COverlayImpl::~COverlayImpl(void)
{
}
void COverlayImpl::osDraw(ISonicPaint * pPaint, const RECT * pRectDirty, BOOL bNullImage)
{
	if (!pPaint || !pRectDirty)
	{
		return;
	}
	CArcAutoLock lock;
	int nWidth = pRectDirty->right - pRectDirty->left;
	int nHeight = pRectDirty->bottom - pRectDirty->top;
	overlay_sm_header * pHeader = (overlay_sm_header *) m_smMgr.GetHeader();
	if (!m_smMgr.GetBits() || !pPaint->GetBits())
	{
		return;
	}
	CRect rect;
	::GetClientRect(m_hWnd,&rect);
	BYTE * buffer = pPaint->GetBits() + (pRectDirty->top * rect.Width() + pRectDirty->left) *4;
	BYTE * pData = m_smMgr.GetBits() + (pRectDirty->top * rect.Width()+ pRectDirty->left) *4;
	//GLOG(_T("pRectDirty.top = %d pRectDirty->bottom = %d"),pRectDirty->top,pRectDirty->bottom);
	if (!buffer || !pData)
	{
		return ;
	}
	for (int i = pRectDirty->top ; i < pRectDirty->bottom ; i++)
	{
		memcpy(pData,buffer,nWidth *4);
		pData += rect.Width() *4;
		buffer += rect.Width() *4;
	}
	RECT  rc (pHeader->rtDirty);
	::UnionRect(&pHeader->rtDirty,&rc,(RECT *)pRectDirty);
	pHeader->bUpdate = TRUE;
	pHeader->bHasImage = !bNullImage;
}
void COverlayImpl::osGetRenderTarget(HWND hOSWindow, LPRECT pRectScreen)
{
	if(hOSWindow == m_hWnd)
	{
		CWindow wnd(m_smMgr.GetHeader()->hGame);
		wnd.GetClientRect(pRectScreen);
		wnd.ClientToScreen(pRectScreen);
	}
}
void COverlayImpl::osChangeCapture(HWND hOSWindow)
{

}
void COverlayImpl::osDestroy(HWND hOSWindow)
{

}
void  COverlayImpl::osChangeCursor(int nCursorResID)
{
	SendCursorMsg(nCursorResID);
}
void COverlayImpl::SendCursorMsg(int nCursorResID)
{
	core_msg_cursorchange msgCursorChange;
	msgCursorChange.lCursorId = nCursorResID;
	SendCoreClientMsg(&msgCursorChange);
	//GLOG(_T("nCursorResID = %d thread = %d"),nCursorResID,GetCurrentThreadId());
}
void COverlayImpl::Init()
{
    TCHAR szGameId [64] = {0};
    if(!GetEnvironmentVariable(_T("CoreGameId"), szGameId, 64))
    {
        //szGameId[0] = _T('0');
    }
    m_strGameId = szGameId;

    TCHAR szShareMemName [64] = {0};
    if(!GetEnvironmentVariable(_T("ShareMemName"), szShareMemName, 64))
    {
        //szGameId[0] = _T('0');
    }


    //WTL::CString str;
    //str.Format(_T("%s_s"),szGameId);
    if (!m_smMgr.Open(szShareMemName))
    {
        TerminateProcess(GetCurrentProcess(),0);
    }
}
CSize COverlayImpl::GetOverlaySize()
{
	CSize sOverlaySize;
	overlay_sm_header * pHeader = (overlay_sm_header *) m_smMgr.GetHeader();
	if (pHeader)
	{
		CArcAutoLock lock;
		sOverlaySize.cx = pHeader->nWidth;
		sOverlaySize.cy = pHeader->nHeight;
	}
	else
	{
		sOverlaySize.cx = 1024;
		sOverlaySize.cy = 768;
	}
	return sOverlaySize;
}
BOOL COverlayImpl::SetMgrWnd(HWND hwnd,_tstring & strNotifyTip)
{
	m_hWnd = hwnd;
	overlay_sm_header * pHeader = (overlay_sm_header *) m_smMgr.GetHeader();
	int dwOverlayPID = 0;
	if (pHeader)
	{
		CArcAutoLock lock;
		dwOverlayPID = pHeader->dwOverlayPID;
		pHeader->hOSPanel = hwnd;
		pHeader->dwOverlayPID = GetCurrentProcessId();
	}
	InitMsg();

	BOOL bres = FALSE;
	if (dwOverlayPID == 0)
	{
		core_msg_gamestate gamestart;
		gamestart.nState = 0;
		lstrcpyn(gamestart.csGameId,GetGameStrId().c_str(),128);
		SendCoreMsg(&gamestart);

		core_msg_first_fixed_notification msg;
		if (SendCoreMsg(&msg) && !msg.dwRet)
		{
			strNotifyTip = msg.szText;
			bres = TRUE;
			//quStrNotify.push(strNotifyTip);
			//ShowTipNotify(_tstring(msg.szText));
		}
	}
	
	return bres;
}
void COverlayImpl::InitMsg()
{
	m_msgClient.Initialize(m_hWnd,_ttoi(m_strGameId.c_str()), MSG_SERVER_MAIN);
	m_msgClient.Connect();
	core_msg_overlay_init overlayinit;
	overlayinit.hMsgClient = m_msgClient.GetSelfWindow();
	if (m_hMsgClient == NULL)
	{
		return;
	}
	while(!SendCoreClientMsg(&overlayinit))
	{
		if (!IsWindow(m_hMsgClient))
		{
			break;
		}
	}
}
int COverlayImpl::OnProcessMsg(core_msg_header * pHeader)
{
	if (!pHeader)
	{
		return -1;
	}
	switch (pHeader->dwCmdId)
	{
	
	case CORE_MSG_COOKIE:
		{
			core_msg_cookie * pBody = (core_msg_cookie *)pHeader;
			memcpy(&m_msgCookie,pBody,sizeof(core_msg_cookie));
		}
		break;
	case CORE_MSG_CHECK_GAME_WINDOWED:
		{
			core_msg_check_game_windowed*pMsgCheck = (core_msg_check_game_windowed*)pHeader;
			pMsgCheck->dwRet = !m_smMgr.GetHeader()->bFullScreen;
		}
		break;
	case  CORE_MSG_NAVIGATE:
		{
			core_msg_navigate* pNavigate = (core_msg_navigate*)pHeader;
			ShowOverlay(TRUE);
			USES_CONVERSION;
			ShowBrowserMainWnd(A2T(pNavigate->szUrl));
			overlay_sm_header *psmHeader = m_smMgr.GetHeader();
			if (psmHeader)
			{
				lstrcpynA(psmHeader->csPage,pNavigate->szUrl,512);
			}
		}
		break;
	case CORE_MSG_FIRST_FIXED_NOTIFICATION:
		{
			core_msg_first_fixed_notification *pNotif = (core_msg_first_fixed_notification *)pHeader;
			ShowTipNotify(_tstring(pNotif->szText));
			
		}
		break;
	case CORE_MSG_SHOWOVERLAY:
		{
			core_msg_showoverlay *pMsg = (core_msg_showoverlay*)pHeader;
			ShowOverlay(pMsg->bShow);
		}
		break;
	default:
		break;
	}
	return 0;
}
void COverlayImpl::osChangeFocus(HWND hFocus, HWND hLosingFocus) 
{
	//::SendMessage(m_hWnd,WM_USER + 1000,WPARAM(hFocus),LPARAM(hLosingFocus));
	if (m_pListBox && m_pListBox->m_hWnd == hLosingFocus )
	{
		::ShowWindow(m_pListBox->GetParent(),SW_HIDE);
	}
	else if (COverlayImImpl::GetUIChatMgr()->GetStatusDlg() && COverlayImImpl::GetUIChatMgr()->GetStatusDlg()->m_LBSetting.m_hWnd == hLosingFocus)
	{
		::ShowWindow(COverlayImImpl::GetUIChatMgr()->GetStatusDlg()->m_hWnd,SW_HIDE);
	}
	//GLOG(_T("hFocus = %d  hlosingfocus = %d"),hFocus ,hLosingFocus);
}

HWND COverlayImpl::osGetPanel()
{
	return m_hWnd;
}
BOOL COverlayImpl::SendCoreClientMsg(core_msg_header * pHeader)
{
	return m_msgClient.SendCoreMsg(m_hMsgClient,pHeader);
}
BOOL COverlayImpl::SendCoreMsg(core_msg_header * pHeader)
{
	return m_msgClient.SendCoreMsg(pHeader);
}
BOOL COverlayImpl::OlCenterWindow(HWND hWnd,HWND hWndCenter)
{
	ATLASSERT(::IsWindow(hWnd));

	// determine owner window to center against
	DWORD dwStyle = ::GetWindowLong(hWnd,GWL_STYLE);
	if(hWndCenter == NULL)
	{
		if(dwStyle & WS_CHILD)
			hWndCenter = ::GetParent(hWnd);
		else
			hWndCenter = ::GetWindow(hWnd, GW_OWNER);
	}

	// get coordinates of the window relative to its parent
	RECT rcDlg;
	::GetWindowRect(hWnd, &rcDlg);
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
	// center within parent client coordinates
	hWndParent = ::GetParent(hWnd);
	ATLASSERT(::IsWindow(hWndParent));

	::GetWindowRect(hWndParent, &rcArea);
	ATLASSERT(::IsWindow(hWndCenter));
	::GetWindowRect(hWndCenter, &rcCenter);
	::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	//}

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;


	// map screen coordinates to child coordinates
	return ::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
BOOL COverlayImpl::ShowGently(HWND hWnd,BOOL bShow, BOOL bActivted, int nStep)
{
	ISonicWndEffect* pEffect = GetSonicUI()->EffectFromHwnd(hWnd) ;
	if (pEffect)
	{
		return pEffect->ShowGently(bShow, bActivted,nStep) ;
	}
	return FALSE;
}
void COverlayImpl::SendTrack(LPCTSTR lpcsTrack, ...)
{
	// format the input string
	if(!lpcsTrack)
	{
		return;
	}
	TCHAR message[1024];
	va_list argList;
	va_start(argList, lpcsTrack);
	_vsntprintf_s(message, sizeof(message) / sizeof(TCHAR), _TRUNCATE, lpcsTrack, argList);
	message[1023] = 0;
	va_end(argList);

	// send core_msg_track message to server
	core_msg_track msgTrack;
	msgTrack.dwClientId = (DWORD)GetGameId();
	lstrcpyn(msgTrack.csTrack, message, sizeof(msgTrack.csTrack) / sizeof(TCHAR));
	SendCoreMsg(&msgTrack);
}
BOOL COverlayImpl::SetTipNotifyText()
{
	if (quStrNotify.empty() || !m_pIOverlayMainWnd)
	{
		return FALSE;
	}
	m_pIOverlayMainWnd->SetTipNotifyText(quStrNotify.front());
	quStrNotify.pop();
	
	return TRUE;
}
BOOL COverlayImpl::ShowTipNotify(_tstring & strText)
{
	 if ( m_pIOverlayMainWnd && !m_pIOverlayMainWnd->ShowTipNotify(strText))
	 {
		quStrNotify.push(strText);
		return TRUE;
	 }	 
	 return FALSE;
}
BOOL COverlayImpl::SendMainWndCoreMessage(UINT msgId,WPARAM wParam,LPARAM lParam )
{
	return ::SendMessage(m_hWnd,msgId,wParam,lParam);
}

