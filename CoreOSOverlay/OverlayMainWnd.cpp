#include "StdAfx.h"
#include "OverlayMainWnd.h"
#include "BrowserImpl.h"
#include "CoreMsgBrowser2Svr.h"

#include "data/DataPool.h"
#include "data/Utility.h"
#include <atltime.h>
#include "OverlayImImpl.h"
#include "OverlayImImpl.h"
#include "OverlayFileWnd.h"

#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
//#include "toolkit.h"

COverlayMainWnd _OverlayMainWnd;

COverlayMainWnd::COverlayMainWnd(void)
:m_bCreateWnd(false)
,m_bOverlayShowing(false)
{
	m_TimeTemp = GetTickCount();
}

COverlayMainWnd::~COverlayMainWnd(void)
{

}

void COverlayMainWnd::OnDestroy()
{
//	SONICUI_DESTROY(m_pAvatar);
	SetMsgHandled(FALSE);
	PostQuitMessage(0);
	m_bCreateWnd = false;
}
int COverlayMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    _tstring  strNotifyTip;
	COverlayImpl::GetInstance()->SetOverlayMainWnd(this);
	BOOL bNotify = COverlayImpl::GetInstance()->SetMgrWnd(m_hWnd,strNotifyTip);

	UISkinInit();
    FitWndSize();

    m_vcHideWnd.push_back(m_hWnd);
    m_UIMainWndframe.Init(m_hWnd);
    SetTimer(1000,2000);
   
    SetMsgHandled(FALSE);
   
	m_bCreateWnd = true;
    if (bNotify)
    {
        COverlayImpl::GetInstance()->ShowTipNotify(strNotifyTip);
    }

	m_PopUpManager.Init();
	m_PopUpManager.RegisterListener(this);
	return 0;
}

BOOL COverlayMainWnd::ShowTipWnd()
{
	CSize szOverlaySize = COverlayImpl::GetInstance()->GetOverlaySize();
	::SetWindowPos(m_OverlayTipWnd.m_hWnd,NULL,-10000 + szOverlaySize.cx - 350,szOverlaySize.cy - 100,350,100,SWP_NOACTIVATE|SWP_NOZORDER);
	return ::ShowWindow(m_OverlayTipWnd.m_hWnd,SW_SHOWNA);
	//COverlayImpl::GetInstance()->ShowGently(hWnd,TRUE);
}

BOOL COverlayMainWnd::HideTipWnd()
{
    if(::IsWindow(m_OverlayTipWnd.m_hWnd))
        return ::ShowWindow(m_OverlayTipWnd.m_hWnd, SW_HIDE);
    return TRUE;
}

void COverlayMainWnd::SetBackBtnText()
{
	m_UIMainWndframe.SetBackToGameTip();
}

BOOL COverlayMainWnd::ShowTipNotify(_tstring & strText)
{
	if (!m_OverlayTipWnd.m_hWnd && ::IsWindow(m_hWnd) && m_bCreateWnd)
	{
		m_OverlayTipWnd.Create(m_hWnd,CRect(0,0,0,0),NULL,WS_POPUP);
		m_OverlayTipWnd.SetTitle(strText);
		return ShowTipWnd();
	}
	return FALSE;
}

void COverlayMainWnd::SetTipNotifyText(_tstring & strText)
{
	m_OverlayTipWnd.SetTitle(strText);
}


void COverlayMainWnd::ShowBrowserMainWnd(LPCTSTR lpUrl)
{
 	if (!m_OverlayBrowerMainWnd.m_hWnd)
 	{
		m_OverlayBrowerMainWnd.Create(m_hWnd,CRect(0,0,600,400),NULL,WS_POPUP|WS_THICKFRAME);
		if (CBrowserImpl::GetInstance()->GetMainBrowserWndRect()->Width() == 0)
		{
			COverlayImpl::GetInstance()->OlCenterWindow(m_OverlayBrowerMainWnd.m_hWnd,m_hWnd);
		}
		else
		{
			m_OverlayBrowerMainWnd.MoveWindow(*(CBrowserImpl::GetInstance()->GetMainBrowserWndRect()));
		}
		m_OverlayBrowerMainWnd.ShowWindow(SW_SHOWNA);
	}
	CBrowserImpl::GetInstance()->CreateBrowser(lpUrl,-1);
}



void COverlayMainWnd::ShowSetting()
{
 	if (!m_OverlaySettingWnd.m_hWnd)
 	{
 		m_OverlaySettingWnd.Create(m_hWnd,CRect(0,0,400,300),NULL,WS_POPUP);
 	}
	SetBackBtnText();
	COverlayImpl::GetInstance()->OlCenterWindow(m_OverlaySettingWnd.m_hWnd,m_hWnd);
 	m_OverlaySettingWnd.ShowWindow(SW_SHOWNA);

}
void COverlayMainWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	int nFunction = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME, INI_CFG_CLIENT_INGAME_FUNCTION, 16, theDataPool.GetUserProfilePath().c_str());
	int nShortcut = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME ,INI_CFG_CLIENT_INGAME_SHORTCUT, 9, theDataPool.GetUserProfilePath().c_str());
	////GLOG(_T("OnKeyDownnFunction = %d,OnKeyDownnShortcut=%d,nRepCnt = %d,nFlags=%d nChar=%d ,m_TimeTemp = %d"),GetKeyState(nFunction),GetKeyState(nShortcut),nRepCnt,nFlags,nChar,m_TimeTemp);
	if ( nChar != nFunction && nChar != nShortcut)
	{
		return;
	}
	if ((nFunction == 0 || (GetKeyState(nFunction)&0x8000)!=0) && (GetKeyState(nShortcut)&0x8000)!=0)
	{
		UINT nTimeTemp = m_TimeTemp;
		m_TimeTemp = GetTickCount();
		if (m_TimeTemp - nTimeTemp < 100)
		{            
			return;
		}
		if (IsWindowVisible())
		{
			ShowOverlay(FALSE);
		}
		else
		{
			ShowOverlay(TRUE);
		}
		////GLOG(_T("nFunction = %d,nShortcut=%d m_TimeTemp=%d"),GetKeyState(nFunction),GetKeyState(nShortcut),m_TimeTemp);
	}
	//SetMsgHandled(FALSE);
}
LRESULT COverlayMainWnd::OnCoreMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
	_tstring sDir = theDataPool.GetBaseDir();
	core_msg_header* pheader = (core_msg_header*)lParam;
	if (!pheader)
	{
		return 0;
	}
	switch(pheader->dwCmdId)
	{
 	case CORE_MSG_RESIZE:
 		{
			PostMessage(WM_CHANGESIZDE/*WM_CLOSE*/);
 		}
		break;
	case CORE_MSG_NOTIFICATION:
		{
			core_msg_notification * pstNotify = (core_msg_notification *)pheader;
			
			if(pstNotify->dwClientId == COverlayImpl::GetInstance()->GetGameId())
			{
				//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
				CTempPopUp *pTempDlg = new CTempPopUp;
				pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
				if (pTempDlg->InitNotifyTipWnd(*pstNotify))
				{
					m_PopUpManager.ShowPopUpWindow(pTempDlg);
				}
			}
		}
		break;
	case CORE_MSG_INBOX:
		{
			core_msg_inbox *pstInbox = (core_msg_inbox *)pheader;
			if (pstInbox->dwClientId == COverlayImpl::GetInstance()->GetGameId())
			{
				//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
				CTempPopUp *pTempDlg = new CTempPopUp;
				pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
				if(pTempDlg->InitInboxTipWnd(*pstInbox))
				{
					m_PopUpManager.ShowPopUpWindow(pTempDlg);
				}
				
			}
		}
		break;
	case CORE_MSG_FRIENDREQ:
		{
			core_msg_friendreq *pstFriendReq = (core_msg_friendreq *)pheader;
			if (pstFriendReq->dwClientId == COverlayImpl::GetInstance()->GetGameId())
			{
				//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
				CTempPopUp *pTempDlg = new CTempPopUp;
				pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
				if(pTempDlg->InitFriTipWnd(*pstFriendReq))
				{
					m_PopUpManager.ShowPopUpWindow(pTempDlg);
				}
			}
		}
		break;
	case CORE_MSG_FRSTATUS:
		{
			core_msg_FRStatus * pstFRStatus = (core_msg_FRStatus*)pheader;
			if (pstFRStatus->dwClientId == COverlayImpl::GetInstance()->GetGameId())
			{
				//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
				CTempPopUp *pTempDlg = new CTempPopUp;
				pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
				if (pTempDlg->InitFRStatusWnd(*pstFRStatus))
				{
					m_PopUpManager.ShowPopUpWindow(pTempDlg);
				}
				
				
				
			}
		}
		break;
	case CORE_MSG_READY_PLAY:
		{
			core_msg_ready_play * pstGameReady = (core_msg_ready_play*)pheader;
			if (pstGameReady->dwClientId == COverlayImpl::GetInstance()->GetGameId())
			{
				//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
				CTempPopUp *pTempDlg = new CTempPopUp;
				pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
				if (pTempDlg->InitGameReadyPlayWnd(*pstGameReady))
				{
					m_PopUpManager.ShowPopUpWindow(pTempDlg);
				}
			}
		}
	case CORE_MSG_COOKIE:
		{
			COverlayImpl::GetInstance()->OnProcessMsg(pheader);
			CBrowserImpl::GetInstance()->SetCookie();
			CBrowserImpl::GetInstance()->SetBillingCookie();
			
		}		
		break; 
	case CORE_MSG_NAVIGATE:
	case CORE_MSG_CHECK_GAME_WINDOWED:
	case CORE_MSG_FIRST_FIXED_NOTIFICATION:
	case CORE_MSG_SHOWOVERLAY:
		{
			COverlayImpl::GetInstance()->OnProcessMsg(pheader);
		}
		break;
	case CORE_MSG_BROWSER_SETCOOOKIE:
		{
			core_msg_setcookie* pSetCookie = (core_msg_setcookie*)pheader;
			core_msg_cookie * pMsgCookie = COverlayImpl::GetInstance()->GetMsgCookie();
			USES_CONVERSION;
			lstrcpyn(pMsgCookie->csSessionId,A2T(pSetCookie->csSessionId),128);
			CBrowserImpl::GetInstance()->SetCookie();
		}
		break;
	case CORE_MSG_FR_RESULT:
		{
			core_msg_FR_result* pFRResult = (core_msg_FR_result*)pheader;

		}
		break;
	case ARC_CHAT_MSG_RCV_MSG:
		{
			//core_msg_immessage * pF
			if (!::IsWindowVisible(m_hWnd))
			{
				im_msg_rcv_msg * pRcvMsg = (im_msg_rcv_msg*)pheader;
				COverlayImImpl::_IMMESSAGE_INFO ImMessageInfo;
				ImMessageInfo._szSenderNick = pRcvMsg->msg._szSenderNick;
				ImMessageInfo._szUserName = pRcvMsg->msg._szUserName;
				ImMessageInfo.nTimeCur = GetCurrentTime();
				if (COverlayImImpl::GetInstance()->InsertImMessage(ImMessageInfo))
				{
					//CTempPopUp will delete itself when the window is destroyed.OnFinalMessage
					CTempPopUp *pTempDlg = new CTempPopUp;
					pTempDlg->Create(m_hWnd, CRect(0,0,0,0), NULL, WS_POPUP);
					if (pTempDlg->InitImMessageWnd(*pRcvMsg))
					{
						m_PopUpManager.ShowPopUpWindow(pTempDlg);
					}
				}
			}
			
		}
	break;
	case ARC_CHAT_MSG_PRESENCE_CHANGED:
		{
			//SendMessage(::GetParent(m_pFriendDlg->m_hWnd),WM_COREMESSAGE,0,(LPARAM)pHeader);
		}
		break;
	case ARC_CHAT_MSG_AVATAR_CHANGED:
		{
			//SendMessage(::GetParent(m_pFriendDlg->m_hWnd),WM_COREMESSAGE,0,(LPARAM)pHeader);
		}
		break;
	default:
		break;
	}
	return 0;
}

LRESULT COverlayMainWnd::OnShowTipWindow(UINT msgId, WPARAM wParam, LPARAM lParam, BOOL bHandled)
{
	CTempPopUp* pDlg = (CTempPopUp *)lParam;
	if (pDlg)
	{
		CRect rect;
		pDlg->GetClientRect(rect);
		CSize szOverlaySize = COverlayImpl::GetInstance()->GetOverlaySize();
		::SetWindowPos(pDlg->m_hWnd, NULL, -10000 + szOverlaySize.cx - rect.Width(), szOverlaySize.cy - rect.Height(),\
			0, 0, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
		pDlg->ShowWindow(SW_SHOWNA);

		_tstring sUserProPath = theDataPool.GetUserProfilePath();
		BOOL bPlaySound = GetPrivateProfileInt(INI_CFG_NOTIFY, INI_CFG_NOTIFY_PLAY_SOUND, 1, sUserProPath.c_str());
		if (bPlaySound)
		{
			int nType = pDlg->GetType();
			_tstring sCmd;
			_tstring sFilePath;
			switch(nType)
			{
			case INBOX_MESSAGE:
				sFilePath = theDataPool.GetBaseDir() + _T("Sounds\\Message_Recieved.wav");
				break;
			case FRIEND_REQUEST:
				sFilePath = theDataPool.GetBaseDir() + _T("Sounds\\Message_Recieved.wav");
				break;
			case FRIEND_STATUS:
				sFilePath = theDataPool.GetBaseDir() + _T("Sounds\\Basic_Notification.wav");
				break;
			case NOTIFICATION:
				sFilePath = theDataPool.GetBaseDir() + _T("Sounds\\Basic_Notification.wav");
				break;
			case GAME_READY_PLAY:
				sFilePath = theDataPool.GetBaseDir() + _T("Sounds\\Download_Complete.wav");
				break;
			default:
				break;
			}
			sCmd.Format(1, _T("open \"%s\" alias NewMessageSound"), sFilePath.c_str());
			mciSendString(_T("close NewMessageSound"), NULL, 0, NULL);
			int nRet = mciSendString(sCmd.c_str(), NULL, 0, NULL);
			if (nRet == 0)
			{
				mciSendString(_T("play NewMessageSound"), NULL, 0, NULL);
			}
		}
	}
	return 0L;
}

void COverlayMainWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1000)
	{
		m_UIMainWndframe.SetSystemTime();
		m_UIMainWndframe.SetSystemDate();
		
	}	
}

void SendOverlayNotifyMessage(BOOL bShow)
{
	char szWnd[32], szMsgId[32];
	memset(szWnd, 0, sizeof(szWnd));
	memset(szMsgId, 0, sizeof(szMsgId));
	HWND hGameWnd = NULL;
	DWORD dwMsgId = 0;
	if(GetEnvironmentVariableA("ArcGameWindowHandle", szWnd, 30) && 
		GetEnvironmentVariableA("ArcGameMessageId", szMsgId, 30))
	{
		hGameWnd = (HWND)atoi(szWnd);
		dwMsgId = (DWORD)atoi(szMsgId);
	}

	if(hGameWnd && dwMsgId)
	{
		DWORD dwRes = 0;
		LRESULT res = ::SendMessageTimeout(hGameWnd, dwMsgId, 0, bShow, SMTO_ABORTIFHUNG, 5000, &dwRes);

		if(res == 0)
		{
			_tstringA dbgstr;
			DWORD lastError = GetLastError();
			dbgstr.Format(3, "ERROR: send message to game, hwnd: %s, msgId: %s, lastErr: %u\n", szWnd, szMsgId, lastError);
			OutputDebugStringA(dbgstr.c_str());
		}
	}
}

void COverlayMainWnd::ShowOverlay(BOOL bShow,BOOL bNotifyGame)
{
	overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
    m_bOverlayShowing = bShow;

	if (bShow)
	{
        HideTipWnd();
		// show overlay which means now resizing was disabled [12/4/2014 liuyu]
		if (pHeader)
		{
			pHeader->bShow = TRUE;
		}

		//notify game overlay is about to open
		if(!::IsWindowVisible(m_hWnd) && bNotifyGame)
		{
			SendOverlayNotifyMessage(bShow);
		}        

        FitWndSize();
        ShowWndByResolution();


		COverlayImpl::GetInstance()->SendCursorMsg(32512);
		SetBackBtnText();
		COverlayImpl::GetInstance()->SendTrack(_T("in-game-overlay"));
		static BOOL bFirst = FALSE;
		if (!bFirst)
		{
			COverlayImpl::GetInstance()->SendTrack(_T("overlayuse"));

			overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
			if (pHeader && !pHeader->bFullScreen)
			{
				CBrowserImpl::GetInstance()->ActiveThreadWnd();
			}
			bFirst = TRUE;
		}	


	}
	else
	{
		if (pHeader)
		{
			pHeader->bShow = FALSE;
			memset(pHeader->csPage,0,512);	
		}	


		m_UIMainWndframe.HideGameBackTip();

        bool bNotifyGame = ShowWindow(SW_HIDE);
        m_OverlaySettingWnd.ShowWindow(SW_HIDE);
        m_OverlayBrowerMainWnd.ShowWindow(SW_HIDE);

		SendMessage(WM_SETFOCUS);
		COverlayImpl::GetInstance()->SendCursorMsg(0);
		
		COverlayImImpl::GetInstance()->ShowOverlayChatDlg(bShow);

		//notify game overlay is closed
		if(bNotifyGame)
		{
			SendOverlayNotifyMessage(bShow);
		}
	}
}
void COverlayMainWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
// 	if (bShow)
// 	{
// 		SetBackBtnText();
// 	}
	SetMsgHandled(FALSE);
}


long COverlayMainWnd::run()
{
	while (true)
	{
		Sleep(2000);
		overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
		if (!pHeader|| !::IsWindow(pHeader->hGame))
		{
			if (pHeader)
			{
				int nTemp = (UINT)time(NULL) - pHeader->uStartTime;
				if (nTemp > 0)
				{
					COverlayImpl::GetInstance()->SendTrack(_T("spenttime:%u"), nTemp);
				}
				core_msg_gamestate gameclose;
				gameclose.nState = 1;
				lstrcpyn(gameclose.csGameId,COverlayImpl::GetInstance()->GetGameStrId().c_str(),128);
				COverlayImpl::GetInstance()->SendCoreMsg(&gameclose);
			}
			
			PostMessage(WM_CLOSE,0,0);
			Sleep(2000);
			TerminateProcess(GetCurrentProcess(),0);
			return 0;
		}
	}
	return 0;
}

void COverlayMainWnd::ShowTipWindow(PVOID pTempPopUp)
{
	PostMessage(WM_SHOWTIPWINDOW, 0, (LPARAM)pTempPopUp);
}
void COverlayMainWnd::HandleSelfAvatar(_tstring lpPath)
{
	m_UIMainWndframe.HandleSelfAvatar(lpPath.c_str());
}
void COverlayMainWnd::HandleSelfPresence(UINT iStatus)
{
	m_UIMainWndframe.HandleSelfPresence(iStatus);
}

LRESULT  COverlayMainWnd::OnOpenFileWnd (UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
	COverlayFileWnd FileWnd;
	FileWnd.Create(m_hWnd,CRect(0,0,480,440),NULL,WS_POPUP|WS_VISIBLE);
	COverlayImpl::GetInstance()->OlCenterWindow(FileWnd.m_hWnd,m_hWnd);
	return FileWnd.OpenFileWnd(wParam, lParam);
}

LRESULT COverlayMainWnd::OnChangeSize( UINT msgId, WPARAM wParam, LPARAM lParam, BOOL bHandled )
{
    if (m_bOverlayShowing)
    {
        // generelly speaking when overlay was showing we would not got a resize message,
        // but some games could resize itself, so we have to handle it [1/13/2015 liuyu]
        FitWndSize();
    }
    //FitWndSize();
    //OutputDebugString(L"resize");
    return 0;
}

void COverlayMainWnd::FitWndSize()
{
    CSize szOverlaySize = COverlayImpl::GetInstance()->GetOverlaySize();
    SetWindowPos(NULL,0,0,szOverlaySize.cx,szOverlaySize.cy,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

    Invalidate();
    m_UIMainWndframe.Resize();

    GLOG(_T("FitWndSize:width:%d, height:%d"), szOverlaySize.cx,szOverlaySize.cy);
}

void COverlayMainWnd::UISkinInit()
{
    if (GetSonicUI()->GetOffscreenUICallback())
    {
        Thread::start();
    }
    GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_mainwnd"));
}

void COverlayMainWnd::ShowWndByResolution()
{
    CSize szOverlaySize = COverlayImpl::GetInstance()->GetOverlaySize();

    ShowWindow(SW_SHOW);
    m_OverlaySettingWnd.ShowWindow(SW_SHOW);
    m_OverlayBrowerMainWnd.ShowWindow(SW_SHOW);
    COverlayImImpl::GetInstance()->ShowOverlayChatDlg(SW_SHOW);

    // browser here [12/12/2014 liuyu]
    CRect rc;
    m_OverlayBrowerMainWnd.GetClientRect(&rc);
    m_OverlayBrowerMainWnd.ClientToScreen(&rc);

    CRect rcClient;
    GetClientRect(&rcClient);
    ClientToScreen(&rcClient);

    if ((rc.Width() > szOverlaySize.cx || rc.Height() > szOverlaySize.cy) ||
        (rc.left  > rcClient.right))
    {
        m_OverlayBrowerMainWnd.SetWindowPos(NULL,rcClient.left, rcClient.top, szOverlaySize.cx,szOverlaySize.cy,SWP_NOZORDER|SWP_NOACTIVATE);
    }

    // settings wnd here [12/12/2014 liuyu]
    if (szOverlaySize.cx < RESOLUTION_SIGN)
    {
        m_OverlaySettingWnd.ShowWindow(SW_HIDE);
        COverlayImImpl::GetInstance()->ShowOverlayChatDlg(SW_HIDE);
    }


}