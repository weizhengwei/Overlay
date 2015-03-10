
#include <afxwin.h>
#include "im\im_ui_Primary.h"
#include "ui\PWPDlg.h"
#include "data\UIString.h"

CUIChatMgrPrimary::CUIChatMgrPrimary()
{
	m_bIsInGame = false;
}

 CUIChatMgrPrimary::~CUIChatMgrPrimary()
{
}

void CUIChatMgrPrimary::HandlePresence(_tstring tsUserName, Presence::PresenceType presence)
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished() && presence != Presence::PresenceType::Unavailable)
		return;

	im_msg_presence_changed msg;
	lstrcpyn(msg.szUserName,tsUserName.c_str(),MAX_PATH-1);
	msg.iType = presence;

	::SendMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_COPYDATE_MESSAGE, 0, (LPARAM)&msg);
}

void CUIChatMgrPrimary::HandleDivorce( _tstring tsName, _tstring tsNick )
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished())
		return;

	im_msg_friend_divorce msg;
	lstrcpyn(msg.szUserName, tsName.c_str(), MAX_PATH-1);
	lstrcpyn(msg.szNickName, tsNick.c_str(), MAX_PATH-1);
	::SendMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_COPYDATE_MESSAGE, 0, (LPARAM)&msg);
}

void CUIChatMgrPrimary::HandleAvartarChanged(_tstring tsUserName,_tstring szPath)
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished())
		return;

	im_msg_avatar_changed msg;
	lstrcpyn(msg.szUserName,tsUserName.c_str(),MAX_PATH-1);
	lstrcpyn(msg.szPath,szPath.c_str(),MAX_PATH-1);
	::SendMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_COPYDATE_MESSAGE, 0, (LPARAM)&msg);
}

void CUIChatMgrPrimary::HandleSendChatMsgDone(im_msg_send_msg_done msg)
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished())
		return;

	::SendMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_COPYDATE_MESSAGE, 0, (LPARAM)&msg);
}

void CUIChatMgrPrimary::HandleRcvChatMsg(MSG_LOG_ELEMENT msgEelement, bool bOfflineMessage /*= false*/)
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished())
		return;

	im_msg_rcv_msg msg;
	msg.msg = msgEelement;
	msg.bOfflineMessage = bOfflineMessage;
	::SendMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_COPYDATE_MESSAGE, 0, (LPARAM)&msg);
}

void CUIChatMgrPrimary::HandleRosterArrive()
{//init member list when after login timer
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (m_pFriendDlg->IsInitFriendListFinished())
		return;

	if (m_pFriendDlg)
	{
		::SendMessage(m_pFriendDlg->m_hWnd, WM_INITFRIENDLIST, 0, 0);
	}
}

void CUIChatMgrPrimary::HandleSyncSentMessage( LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId )
{
	if (m_pFriendDlg == NULL || !::IsWindow(m_pFriendDlg->m_hWnd))
		return;

	if (!m_pFriendDlg->IsInitFriendListFinished())
		return;

	DWORD dwContentLen = _tcslen(lpcContent)+1;
	DWORD dwMessageLen = sizeof(im_msg_sync_sent_message) + sizeof(TCHAR)*dwContentLen;
	im_msg_sync_sent_message* pMsg = (im_msg_sync_sent_message*)malloc(dwMessageLen);
	memset(pMsg, 0, dwMessageLen);
	pMsg->dwCmdId = ARC_CHAT_MSG_SYNC_SENT_MESSAGE;
	pMsg->dwSize = dwMessageLen;
	pMsg->nTime = nTime;
	pMsg->nSyncID = nSyncId;
	pMsg->nType = nType;
	pMsg->dwLen = dwContentLen;
	_tcsncpy(pMsg->szSelfName, lpcSelfName, _tcslen(lpcSelfName));
	_tcsncpy(pMsg->szUser, lpcUser, _tcslen(lpcUser));
	_tcsncpy(pMsg->szContent, lpcContent, dwContentLen-1);

	
	SendCoreMsg(HWND_BROADCAST, pMsg);
	SendCoreMsg(m_msgServer.GetSelfWindow(), pMsg);

	free((void*)pMsg);
}

CUIFriendDlgBase*  CUIChatMgrPrimary::CreateFriendDlgObj()
{
	return new CUIFriendDlgPrimary(this,NULL);
}

CUIChatDlgBase*  CUIChatMgrPrimary::CreateChatDlgObj(_ITEM_BASE_INFO info)
{
	return new CUIChatDlgPrimary(this,info);
}

CUIStatusDlgBase*  CUIChatMgrPrimary::CreateStatusDlgObj()
{
	return new CUIStatusDlgPrimary(this);
}

im_dialog_params CUIChatMgrPrimary::GetDefaultTabMgrParams()
{
	im_dialog_params pm;
	pm.pParentWnd = NULL;

	CPWPDlg* pDlg = (CPWPDlg*)AfxGetApp()->GetMainWnd();
	if (pDlg == NULL)
	{
		return pm;
	}

	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT);
	pDlg->GetWindowPlacement(&wndPlacement);
	RECT rcClient = wndPlacement.rcNormalPosition;
	//pDlg->GetWindowRect(&rcClient);
	pm.rt.left = rcClient.left + ( (rcClient.right-rcClient.left)/2-260 );
	pm.rt.right = pm.rt.left + 520;
	pm.rt.top = rcClient.top + ( (rcClient.bottom-rcClient.top)/2-251 );
	pm.rt.bottom = pm.rt.top + 502;

	return pm;
}

void CUIChatMgrPrimary::OnCoreMessage(HWND hFrom, core_msg_header * pHeader)
{
	if (!pHeader)
	{
		return;
	}

	CUIChatMgrBase::OnCoreMessage(hFrom,pHeader);

	switch (pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_SEND_MSG:
		{
			MSG_LOG_ELEMENT msg = ((im_msg_send_msg*)pHeader)->msg;
			bool bOffline = ((im_msg_send_msg*)pHeader)->bOfflineMessage;
			SendChatMessage(msg, bOffline);
		}
		break;
	case ARC_CHAT_MSG_SET_PRESENCE:
		{
			Presence::PresenceType iType = (Presence::PresenceType)((im_msg_set_presence*)pHeader)->iType;
			SetPresence(iType);
		}
		break;
	case ARC_CHAT_MSG_GET_PRESENCE:
		{
			im_msg_get_presence* pMsg = (im_msg_get_presence*)pHeader;
			pMsg->iType = GetPresence(pMsg->szUserName);
		}
		break;
	case ARC_CHAT_MSG_LOAD_HISTOPRY_RECORDS:
		{
			im_msg_load_history_records* pMsg = (im_msg_load_history_records*)pHeader;

			std::list<MSG_LOG_ELEMENT> RecordList;
			LoadHistoryRecord(pMsg->szUserName,pMsg->nCount,RecordList);

			std::list<MSG_LOG_ELEMENT>::iterator  iter=RecordList.begin();
			for (int i=0;iter!=RecordList.end();iter++,i++)
			{
				pMsg->aRecords[i] = *iter;
			}
		}
		break;
	case ARC_CHAT_MSG_LOAD_STRING:
		{
			im_msg_load_string* pMsg = (im_msg_load_string*)pHeader;
			lstrcpyn(pMsg->szContent,GetStringFromId(pMsg->szId).c_str(),2047);
		}
		break;
	case ARC_CHAT_MSG_ITEM_INFO:
		{
			im_msg_item_info* pMsg = (im_msg_item_info*)pHeader;
			GetFriendInfo(pMsg->info._szUser, &(pMsg->info), pMsg->bByNick);
		}
		break;
	case ARC_CHAT_MSG_SELF_INFO:
		{
			im_msg_self_info* pMsg = (im_msg_self_info*)pHeader;
			GetSelfUserInfo(&(pMsg->info));
		}
		break;
	case ARC_CHAT_MSG_GET_TIME:
		{
			im_msg_get_time* pMsg = (im_msg_get_time*)pHeader;
			pMsg->nTime = GetTime();
		}
		break;
	case ARC_CHAT_MSG_SYNC_LOGIN_STATUS:
		{
			im_msg_sync_login_status *pMsg = (im_msg_sync_login_status*)pHeader;
			SetLoginStatus(pMsg->nStatus);
		}
		break;
	case ARC_CHAT_MSG_FRIEND_LIST_REQ:
		{
			/*
			 *@ get the friend list
			 */
			std::vector<CHAT_FRIEND> friends;
			if (!m_IMMgr.GetFriendManager().GetFriendList(friends))
				break;

			/*
			 *@ init the friends-list requested by the from-window
			 */
			int nRespSize = sizeof(im_msg_friend_list_response) + sizeof(CHAT_FRIEND) * friends.size();
			im_msg_friend_list_response* pResp = (im_msg_friend_list_response*)malloc(nRespSize);
			memset(pResp, 0, nRespSize);
			pResp->dwLen = friends.size();
			pResp->dwSize = nRespSize;
			pResp->dwCmdId = ARC_CHAT_MSG_FRIEND_LIST_RESP;

			/*
			 *@ get the data for the requested friends-list
			 */
			std::vector<CHAT_FRIEND>::iterator itr = friends.begin();
			int nCount = 0;
			for (; itr != friends.end(); ++itr)
			{
				pResp->aFriendList[nCount++] = *itr;
			}

			/*
			 *@ send the friends-list to the from window
			 */
			SendCoreMsg(hFrom,pResp); 
			free(pResp); 
		}
		break;
	case ARC_CHAT_MSG_CLEAR_HISTOPRY_RECORDS:
		{
			im_msg_clear_history_records * pRecord = (im_msg_clear_history_records *) pHeader;
			ClearHistroyRecord(pRecord->szUserName);
		}
		break;
	case ARC_CHAT_MSG_FRIENDLIST_INIT_FINISHED:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				im_msg_friend_list_is_inited_req* pMsg = (im_msg_friend_list_is_inited_req*)pHeader;
				pMsg->bIsInitFinished = m_pFriendDlg->IsInitFriendListFinished();
			}
		}
		break;
	case ARC_CHAT_MSG_GET_FRIENDS_COUNT:
		{
			im_msg_get_friends_count *pMsg = (im_msg_get_friends_count*)pHeader;
			pMsg->nCount = GetFriendsCount();
		}
		break;
	case ARC_CHAT_MSG_HANDLE_SYNC_SENT_MESSAGE:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				im_msg_handle_sync_sent_message *pMsg = (im_msg_handle_sync_sent_message*)pHeader;
				im_msg_handle_sync_sent_message *pNewMsg = (im_msg_handle_sync_sent_message*)malloc(pMsg->size());
				memcpy((void*)pNewMsg, (void*)pMsg, pMsg->size());
				::PostMessage(m_pFriendDlg->m_hWnd, WM_HANDLE_SYNC_SENT_MESSAGE, 0, (LPARAM)pNewMsg);
			}
		}
		break;
	default:
		break;
	}
	return;
}

bool  CUIChatMgrPrimary::InitChatSystem()
{ 
	bool bRet = m_msgServer.Initialize(this,-1,ARC_IM_SERVER_NAME);
	m_IMMgr.RegisterIMCallback(this);
	return bRet;
}

bool CUIChatMgrPrimary::UnInitChatSystem()
{
   m_msgServer.Uninitialize();
   m_IMMgr.UnRegisterIMCallback(this);
   return true;
}
_tstring CUIChatMgrPrimary::GetStringFromId( LPCTSTR  lpszStr)
{
	return theUIString._GetStringFromId(lpszStr);
}

_tstring CUIChatMgrPrimary::GetUserProfilePath()
{
   return theDataPool.GetUserProfilePath();
}

_tstring CUIChatMgrPrimary::GetBaseDir()
{
   return theDataPool.GetBaseDir();
}

bool CUIChatMgrPrimary::GetFriendInfo(_tstring szUser, LPITEM_BASE_INFO lpInfo, bool bByNick /*= false*/)
{
	if (!lpInfo)
		return false;
	
	CHAT_FRIEND fd;
	bool bRet = false;
	if (bByNick)
	{
		bRet = m_IMMgr.GetFriendManager().GetFriendByNick(szUser, fd);
	}
	else
	{
		bRet = m_IMMgr.GetFriendManager().GetFriendByName(szUser, fd);
	}

	if (bRet)
	{
		lpInfo->iType = fd.nPresence;
		lstrcpyn( lpInfo->_szUser, fd.szUserName,	MAX_PATH-1);
		lstrcpyn( lpInfo->_szNick, fd.szNick,		MAX_PATH-1);

		CWebData::WEB_CHARACT_MSG data = g_theSNSManager.GetCharactDataByNickName(lpInfo->_szNick);

		lstrcpyn( lpInfo->_szAvatarPath,	data._szHeaderPath.c_str(),	MAX_PATH-1);
        lstrcpyn( lpInfo->_szServer,		data._szServer.c_str(),		MAX_PATH-1);
		lstrcpyn( lpInfo->_szCharater,		data._szCharater.c_str(),	MAX_PATH-1);
		return true;
	}
	
	return false;
}

bool CUIChatMgrPrimary::GetSelfUserInfo(LPITEM_BASE_INFO lpInfo)
{
	if (!lpInfo)
		return false;
	
	lstrcpyn(lpInfo->_szUser,		theDataPool.GetUserName().c_str(),						MAX_PATH-1);
	lstrcpyn(lpInfo->_szNick,		theDataPool.GetUserNick().c_str(),						MAX_PATH-1);
	lstrcpyn(lpInfo->_szAvatarPath,	g_theSNSManager.GetUserCharData()._szHeaderPath.c_str(),MAX_PATH-1);
    lstrcpyn(lpInfo->_szServer,		g_theSNSManager.GetUserCharData()._szServer.c_str(),	MAX_PATH-1);
	lstrcpyn(lpInfo->_szCharater,	g_theSNSManager.GetUserCharData()._szCharater.c_str(),	MAX_PATH-1);
	lpInfo->iType = m_IMMgr.GetXmppClient().GetPresence();
	return true;
}

bool CUIChatMgrPrimary::SendChatMessage(MSG_LOG_ELEMENT msgEelement, bool bOffline /*= false*/)
{
	if (bOffline)
	{
		if (m_IMMgr.GetXmppClient().IsSupportOfflineMessage())
		{
			m_IMMgr.GetXmppClient().SendChatMessage(msgEelement._szUserName,msgEelement._szContent);
		}
	}
	else
	{
		m_IMMgr.GetXmppClient().SendChatMessage(msgEelement._szUserName,msgEelement._szContent);
	}
	
    _tstring sSenderNick(msgEelement._szSenderNick);
    _tstring sTime(msgEelement._szTime);
	_tstring sDate(msgEelement._szDate);
	m_IMMgr.GetMsgLogMgr().AppendLog(msgEelement._szUserName,msgEelement._szRecvNick,sSenderNick.toUTF8().c_str(),sTime.toNarrowString().c_str(),sDate.toNarrowString().c_str(),msgEelement._szContent);
	return true;
}

bool CUIChatMgrPrimary::SetPresence(int iType)
{
	/*
	 *@ must kill the login timer immediately
	 */
	if (iType == ArcPresenceType::Unavailable && m_pFriendDlg != NULL && m_pFriendDlg->IsWindow())
		m_pFriendDlg->KillLoginTimer();

	/*
	 *@ set presence by using Xmpp-Client, and the Xmpp-Client will handle presence after this call end
	 */
	m_IMMgr.GetXmppClient().SetPresece((ArcPresenceType)iType);

	/*
	*@ return, this annotation just write to make the typesetting has a Good-looking
	 */
	return true;
}

int CUIChatMgrPrimary::GetPresence(_tstring szUser)
{ 
	int iType = Presence::Unavailable;

	if (!szUser.CompareNoCase(theDataPool.GetUserName().c_str()))
	{
		iType = m_IMMgr.GetXmppClient().GetPresence();
	}
	else
	{
		iType = m_IMMgr.GetFriendManager().GetFriendPresenceByName(szUser.c_str());
	}

	return iType;
}

bool CUIChatMgrPrimary::LoadHistoryRecord(_tstring szUser,UINT nCount,std::list<MSG_LOG_ELEMENT>& RecordList) 
{
	UINT nMax = (nCount>MAX_HISTORY_RECORDS_COUNT) ? MAX_HISTORY_RECORDS_COUNT:nCount;

	RecordList = m_IMMgr.GetMsgLogMgr().ReadLog(szUser.c_str(),nMax);
	return true;
}

bool CUIChatMgrPrimary::ClearHistroyRecord(_tstring szUser)
{
	m_IMMgr.GetMsgLogMgr().ClearLog(szUser.c_str());
	return true;
}

 BOOL CUIChatMgrPrimary::SendCoreMsg(HWND hWnd, core_msg_header * pHeader)
{
	return m_msgServer.SendCoreMsg(hWnd,pHeader);
}

void CUIChatMgrPrimary::SetTabIcon( CUITabMgrDlg* pTab )
{
	if (pTab)
	{
		pTab->SetTabIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	}
}

bool CUIChatMgrPrimary::IsForegroundWndBelongMe(HWND hWnd)
{
	if (hWnd == NULL || !::IsWindow(hWnd))
	{
		return false;
	}
	CPWPDlg* pDlg = (CPWPDlg*)AfxGetApp()->GetMainWnd();
	if (pDlg == NULL || m_pFriendDlg == NULL || !pDlg->IsWindowVisible() || pDlg->IsIconic())
	{
		return false;
	}

	DWORD dwCurProcessID = GetCurrentProcessId();
	DWORD dwCurThreadIID = GetCurrentThreadId();
	DWORD dwForegroundProcessID = 0;
	if (hWnd != NULL && ::IsWindow(hWnd))
	{
		if (dwCurThreadIID == ::GetWindowThreadProcessId(hWnd, &dwForegroundProcessID))
		{
			if (dwCurProcessID == dwForegroundProcessID)
			{
				return true;
			}
		}
	}
	return false;
}

void CUIChatMgrPrimary::ShowNewMinimizeTab( HWND hWnd )
{
	OSVERSIONINFOEX sOsVersion = {0} ;
	sOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX) ; 
	GetVersionEx((OSVERSIONINFO*)&sOsVersion);
	if (sOsVersion.dwMajorVersion >= 6)
	{//vista win7 win8
		typedef HRESULT (WINAPI *LPDWMISCOMPOSITIONENABLED)(BOOL*);
		HMODULE hModule = ::GetModuleHandle(_T("dwmapi.dll"));
		if (hModule != NULL)
		{
			static LPDWMISCOMPOSITIONENABLED lpDwmIsCompositionEnabled = (LPDWMISCOMPOSITIONENABLED) ::GetProcAddress(hModule, "DwmIsCompositionEnabled");
			if (lpDwmIsCompositionEnabled != NULL)
			{
				GetSonicUI()->EffectFromHwnd(hWnd)->SetLayeredAttributes(0, 0, LWA_ALPHA);
				ShowWindow(hWnd, SW_SHOWNA);
				if (!g_appManager.IsAnyAppFullScreen())
				{
					ShowWindow(hWnd, SW_MINIMIZE);
				}
				GetSonicUI()->EffectFromHwnd(hWnd)->SetLayeredAttributes(0, 255, LWA_ALPHA);
				return;
			}
		}
	}

	ShowWindow(hWnd, SW_MINIMIZE);
}

void CUIChatMgrPrimary::ShowCursorItemOnInputEdit( CUIChatDlgBase *pChat )
{
	if (!g_appManager.IsAnyAppFullScreen())
	{
		pChat->PostMessage(WM_SETINPUT_EDIT_FOCUS, 0, 0);
	}
}

__int64 CUIChatMgrPrimary::GetTime()
{
	return theDataPool.GetServerTimer();
}

void CUIChatMgrPrimary::SetLoginStatus( int loginStatus )
{
	m_IMMgr.GetXmppClient().SetLoginWithStatus(loginStatus);
}

void CUIChatMgrPrimary::SyncSentMessage( LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId )
{
	HandleSyncSentMessage(lpcSelfName, lpcContent, lpcUser, nType, nTime, nSyncId);
}

void CUIChatMgrPrimary::HandleCopyDateMessage( core_msg_header *pHeader )
{
	if (pHeader && pHeader->dwSize > 0)
	{
		SendCoreMsg(HWND_BROADCAST,pHeader);
		SendCoreMsg(m_msgServer.GetSelfWindow(),pHeader);
	}
}

/*		---CUIChatMgrPrimary class end---		*/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*		---CUIFriendDlgPrimary class begin---		*/

CUIFriendDlgPrimary::CUIFriendDlgPrimary(CUIChatMgrPrimary* pMgr,CWindow* pParent)
	:CUIFriendDlgBase(pMgr,pParent),
	m_pAvatarPaint(NULL),
	m_pAvatar(NULL),
	m_pStatusDlg(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAvatar = GetSonicUI()->CreateImage(); 

	SetListBoxNickNameFormatType(_T("/line_width=210, single_line=2//c=%x, font, font_height=12, font_face='Open Sans',align=1/%s"));
	SetListBoxAvatarFormatType(_T("/p=%d, pstretch=20|20/"));
	SetListBoxNickNameColor(RGB(197,197,198));
}

CUIFriendDlgPrimary::~CUIFriendDlgPrimary()
{
	m_pAvatarPaint->DelAllObject();
	SONICUI_DESTROY(m_pAvatarPaint);
	SONICUI_DESTROY(m_pAvatar);
}

LRESULT CUIFriendDlgPrimary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if (!m_pSearchQuit || !m_pUIChatMgr)
	{
		return 0;
	}

	// atach window from .xml UI config
	GetCoreUI()->AttachWindow(m_hWnd, _T("friendlist_popup_dialog"));
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_title_status"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnSetStatus);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("firendlist_popup_dialog_btn_min"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnMin);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("firendlist_popup_dialog_btn_add"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnAddFriendClick);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_sep"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnSetStatus);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("firendlist_popup_dialog_btn_exit"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnExit);

	SetWindowText(m_pUIChatMgr->GetStringFromId(_T("IDS_POPDLG_FRIENDLIST_TITLE")).c_str()) ;
	SetIcon(m_hIcon, TRUE);//Set big icon
	SetIcon(m_hIcon, FALSE);//Set small icon

	CRect rcClient;
	GetClientRect(&rcClient);

	m_pTree->Create(m_hWnd, FLP_LB_LEFT_OS, FLP_LB_TOP_OS, 
		(rcClient.right - rcClient.left) - FLP_LB_RIGHT_OS - FLP_LB_LEFT_OS, 
		(rcClient.bottom - rcClient.top) - FLP_LB_BOTTOM_OS - FLP_LB_TOP_OS, 
		GetCoreUI()->GetImage(_T("SCROLLBAR_THUMB")), 
		GetCoreUI()->GetImage(_T("SCROLLBAR_CHANNEL_TREE")), NULL, NULL);

	m_pSearchBar = GetCoreUI()->GetObjByID<WTL::CCoreEdit>(m_hWnd,_T("firendlist_popup_dialog_edit_serach"));
	m_pSearchBar->LimitText(FLP_SEARCH_TEXT_MAXSIZE);
	CString csDefault;
	m_pSearchBar->GetWindowText(csDefault);
	SetSearchDefaultText(csDefault.GetString());

	m_pSearchFailure->Format(
		_T("/c=%x, font, font_height=13, font_face='Open Sans', align=1/%s"),
		FLP_SEARCH_FAILURE_COLOR,
		m_pUIChatMgr->GetStringFromId(_T("IDS_CHAT_SEARCH_FAILURE")).c_str()
		);
	m_pSearchFailure->Show(FALSE);

	m_pSearchQuit->Format(_T("/a,p2=%d/"),GetCoreUI()->GetImage(_T("BTN_CHAT_SEARCH"))->GetObjectId());
	m_pSearchQuit->Show(FALSE);

	m_pAvatarPaint = GetSonicUI()->CreatePaint();
	if (!m_pAvatarPaint) return 0;
	m_pAvatarPaint->Create(
		FALSE,
		GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetWidth(),
		GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetHeight()/3
		);

	//update nickname.
	_tstring sNick = theDataPool.GetUserNick().c_str();
	sNick.Replace(_T("/"),_T("//"));
	ISonicString* pNickStr = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_nick"));
	pNickStr->Format(_T("/c=%x, font, font_height=14, font_face='Open Sans',align=1/%s"), FLP_NICKNAME_COLOR, sNick.c_str());
	pNickStr->Redraw();

	SendMessage(WM_UPDATE_CHAT_STATUS,0,Presence::Unavailable);//ensure the default head image can be displayed at the beginning.
	
	return EndInit();
}

void CUIFriendDlgPrimary::HandleFriendPresence(_tstring sUserName, ArcPresenceType presence)
{
	UpdateFriendPresence(sUserName, presence);
}

void CUIFriendDlgPrimary::HandleSelfPresence(ArcPresenceType presence)
{
	_tstring szPath = m_pUIChatMgr->GetBaseDir() + g_theSNSManager.GetUserCharData()._szHeaderPath.c_str();
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		m_pAvatar->Load(szPath.c_str());

	// update the header image and the presence back
	UpdateAvatarPaint(m_pAvatar, (UINT)presence);
	// update the status text
	UpdateStatus(presence);

	if (presence > ArcPresenceType::XA)
	{
		ClearAllItems();
		m_pTree->Show(FALSE);
		m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
		m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);

		if (_threadHandle != NULL) 
		{
			m_bExitWorkThread = true;
			wait();
			_threadHandle = 0;
			_threadId = 0;
		}

		m_bInitFriendListFinished = false;
		KillLoginTimer();
		m_pUIChatMgr->HandleSelfOffline();
	}
	else if (m_nCurPressence >= ArcPresenceType::XA && presence < ArcPresenceType::XA)
	{
		//m_pTree->Show(TRUE);
		//InitFriendList(FALSE);
	}
	m_nCurPressence = presence;
}

void CUIFriendDlgPrimary::HandleSelfAvatar(_tstring lpPath)
{
	_tstring szPath = m_pUIChatMgr->GetBaseDir() + g_theSNSManager.GetUserCharData()._szHeaderPath.c_str();
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		m_pAvatar->Load(szPath.c_str());

	UpdateAvatarPaint(m_pAvatar, m_nCurPressence);
}

void CUIFriendDlgPrimary::HandleFriendAvatar(_tstring sUserName, LPCTSTR lpPath)
{
	UpdateFriendAvatar(sUserName, lpPath);
}

void CUIFriendDlgPrimary::OnPaint(HDC)
{
	WTL::CPaintDC dc(m_hWnd); // device context for painting
	ISonicImage * pImg = GetCoreUI()->GetImage(_T("BG_CHAT_TEXTAREA"));
	if (pImg)
	{
		CRect rcFriendList,rcSearch,rcSearchQuit;
		rcFriendList = m_pTree->GetRect();

		::GetWindowRect(::GetParent(m_pSearchBar->m_hWnd),&rcSearch);
		ScreenToClient(&rcSearch);

		DRAW_PARAM dp = { 0 };
		dp.dwMask = DP_TILE;
		dp.cyDiv = 10;
		dp.cy = rcFriendList.Height();
		dp.cx = rcFriendList.Width();
		pImg->Draw(dc.m_hDC, rcFriendList.left, rcFriendList.top, &dp);

		dp.cy = rcSearch.Height()+2;
		pImg->Draw(dc.m_hDC, rcSearch.left-1, rcSearch.top-1, &dp);

		rcSearchQuit.SetRect(rcSearch.right,rcSearch.top,rcFriendList.right,rcSearch.bottom);
		GetSonicUI()->FillSolidRect(dc.m_hDC,&rcSearchQuit,RGB(19,19,19));
		if (m_pSearchQuit)
		{
			m_pSearchQuit->TextOut(dc.m_hDC, rcFriendList.right-18,rcSearch.top+7,m_hWnd);
		}

		if (m_pSearchFailure)
		{
			m_pSearchFailure->TextOut(dc.m_hDC, rcFriendList.left+(rcFriendList.Width()-m_pSearchFailure->GetWidth())/2,rcFriendList.top+7,m_hWnd);
		}
	}
	if (m_pAvatarPaint)
	{
		m_pAvatarPaint->Move(12,38);
		m_pAvatarPaint->Render(dc, m_hWnd);
	}
}

void CUIFriendDlgPrimary::OnClickBtnSetStatus(ISonicString *, LPVOID pReserve)
{
	if (!m_pStatusDlg) return ;
	if(m_pStatusDlg->IsWindowVisible())
	{
		m_pStatusDlg->ShowWindow(SW_HIDE);
		return;
	}
	CRect rect;
	GetWindowRect(&rect);
	const RECT* pRect = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("friendlist_popup_dialog_title_status"))->GetRect();

	int x = rect.left +(pRect->left+pRect->right)/2-60;
	if (x > 0)
	{
		m_pStatusDlg->MoveWindow(x, rect.top+96, 121, 93);
	}

	m_pStatusDlg->ShowWindow(SW_SHOW);
	m_pStatusDlg->m_LBSetting.ShowWindow(SW_SHOW);
}

void CUIFriendDlgPrimary::OnClickBtnMin(ISonicString *, LPVOID pReserve)
{
	ShowWindow(SW_MINIMIZE);
}

void CUIFriendDlgPrimary::OnClickBtnExit(ISonicString *, LPVOID pReserve)
{
	ShowWindow(SW_HIDE);
}

void CUIFriendDlgPrimary::OnClickedBtnAvatar(ISonicString *, LPVOID pReserve)
{
	//navigate friend's core connect main page.
	
	if (!m_pUIChatMgr) return;
	 
	_tstring szUrl = m_pUIChatMgr->GetStringFromId( _T("IDS_CONFIG_WEBSITE_ACCOUNT"));

	CPWPDlg* pDlg = (CPWPDlg*)AfxGetApp()->GetMainWnd();
	if (pDlg )
	{
		pDlg->Navigate(szUrl);
		if(pDlg->IsIconic()|| !pDlg->IsWindowVisible())
		{
			pDlg->ShowWindow(SW_RESTORE);
		}
	}
}

void CUIFriendDlgPrimary::UpdateAvatarPaint(ISonicImage* pImg, UINT iStatus)
{
	if (m_pAvatarPaint == NULL || pImg == NULL)
	{
		return;
	}

	ISonicString* pAvatarStr = GetSonicUI()->CreateString();
	pAvatarStr->Format(_T("/a,p=%d, pstretch=43|43/"),pImg->GetObjectId());
	pAvatarStr->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickedBtnAvatar);

	m_pAvatarPaint->DelAllObject();
	m_pAvatarPaint->AddObject(m_pUIChatMgr->GetStatusBkImg(iStatus,true)->GetObjectId(),0,0,TRUE);
	m_pAvatarPaint->AddObject(pAvatarStr->GetObjectId(), 
		(m_pAvatarPaint->GetWidth()-pAvatarStr->GetWidth())/2, 
		(m_pAvatarPaint->GetHeight()-pAvatarStr->GetHeight())/2, 
		TRUE);		
	m_pAvatarPaint->Redraw(FALSE);
}

void CUIFriendDlgPrimary::UpdateStatus(UINT iStatus)
{
	if (!m_pUIChatMgr) return;

	_tstring szStatus[9]= { 
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE")),
		_T(""),_T(""), 
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_BUSY")),_T(""),
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"))
	};

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_title_status"))->Format(_T("/a,c=%x, font, font_height=12, font_face='Open Sans',align=1/%s"),RGB(197,197,198),szStatus[iStatus].c_str());
}


void CUIFriendDlgPrimary::AddSettingItem(int iStatus)
{
	if (!m_pUIChatMgr) return;
	if (!m_pStatusDlg) return ;

	_tstring szStatus[9]= { 
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE")), 
		_T(""), _T(""), 
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_BUSY")),_T(""), 
		m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"))
	};

// 	ISonicString* pStatus = GetSonicUI()->CreateString();
// 	pStatus->Format(_T("/p=%d/"),m_mapStatusIcon[iStatus]->GetObjectId());

	ISonicString* pStatusString = GetSonicUI()->CreateString();
	pStatusString->Format(_T("/c=%x, font, font_height=13, font_face='Open Sans',align=1/%s"),RGB(223,223,223),szStatus[iStatus].c_str());

	int nIndex = m_pStatusDlg->m_LBSetting.GetCount();
	//m_pStatusDlg->m_LBSetting.AddObject(nIndex,pStatus,6,6,TRUE,TRUE);
	m_pStatusDlg->m_LBSetting.AddObject(nIndex,pStatusString,25,3,TRUE,FALSE);

	m_pStatusDlg->m_LBSetting.SetItemHeight(nIndex,27);
	m_pStatusDlg->m_LBSetting.SetItemData(nIndex,(DWORD_PTR)iStatus);
}

void CUIFriendDlgPrimary::AddFriendClick(ISonicString *, LPVOID pReserve)
{
	if (!m_pUIChatMgr) return;

	_tstring szUrl = m_pUIChatMgr->GetStringFromId(_T("IDS_SEARCH_FRIENDS"));

	CPWPDlg* pDlg = (CPWPDlg*)AfxGetApp()->GetMainWnd();
	if (pDlg )
	{
		pDlg->Navigate(szUrl);
		if(pDlg->IsIconic()|| !pDlg->IsWindowVisible())
		{
			pDlg->ShowWindow(SW_RESTORE);
		}
	}
}

void CUIFriendDlgPrimary::LoadStatusIcon()
{
	m_mapStatusIcon.clear();
	ISonicImage* pImage = GetCoreUI()->GetImage(_T("BTN_FOOT_CHAT"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(Presence::Available,pImage));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(Presence::Chat,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_AWAY"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(Presence::Away,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_BUSY"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(Presence::DND,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_OFFLINE"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(Presence::Unavailable,pImage));
}

void CUIFriendDlgPrimary::SetStatusDlg(CUIStatusDlgPrimary *pStatus)
{
	if (pStatus && !IsBadReadPtr(pStatus, sizeof(CUIStatusDlgPrimary)))
	{
		m_pStatusDlg = pStatus;
		LoadStatusIcon();
		AddSettingItem(Presence::Available);
		AddSettingItem(Presence::DND);
		AddSettingItem(Presence::Unavailable);

		//set selected item.
		int i = 0;
		for (int i=0; i<m_pStatusDlg->m_LBSetting.GetCount(); i++)
		{

			CUIChatMgrPrimary *pMgr = dynamic_cast<CUIChatMgrPrimary *>(m_pUIChatMgr);
			if (pMgr && pMgr->GetIMMgr().GetXmppClient().GetPresence() == (ArcPresenceType)m_pStatusDlg->m_LBSetting.GetItemData(i))
			{
				m_pStatusDlg->m_LBSetting.SetCurSel(i);
				break;
			}
		}
		if (i == m_pStatusDlg->m_LBSetting.GetCount())
		{
			m_pStatusDlg->m_LBSetting.SetCurSel(m_pStatusDlg->m_LBSetting.GetCount()-1);
		}
	}
}

LRESULT CUIFriendDlgPrimary::OnUpdateStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	_tstring szPath = m_pUIChatMgr->GetBaseDir() + g_theSNSManager.GetUserCharData()._szHeaderPath.c_str();
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		if (m_pAvatar == NULL) 
			return 0L;

		m_pAvatar->Load(szPath.c_str());

		// update the status image for new status
		UpdateAvatarPaint(m_pAvatar, m_nCurPressence);
	}
	return 0L;
}

void CUIFriendDlgPrimary::InitFriendList()
{
	/*
	 *@ verify the chat manager 
	 */
	CUIChatMgrPrimary *pChatMgr = dynamic_cast<CUIChatMgrPrimary *>(m_pUIChatMgr);
	if (!pChatMgr) 
		return;

	/*
	 *@ get friends
	 */
	std::vector<CHAT_FRIEND> vecFriends;
	pChatMgr->GetIMMgr().GetFriendManager().GetFriendList(vecFriends);
	
	/*
	 *@ decide whether do search 
	 */
	CString sText;
	m_pSearchBar->GetWindowText(sText);
	bool bSearch = ((0 != sText.GetLength()) && !m_bForcing);
	
	if (!m_bInitFriendListFinished)
	{
		/*
		 *@ wait the load avatar work exit
		 */
		if (_threadHandle != NULL) 
		{
			m_bExitWorkThread = true;
			wait();
			_threadHandle = 0;
			_threadId = 0;
		}

		/*
		 *@ clear the avatar friends and presence friends
		 */
		while (m_wqAvatarFriends.size() > 0) {
			m_wqAvatarFriends.leave();
		}
		while (m_wqPresenceFriends.size() > 0) {
			m_wqPresenceFriends.leave();
		}

		m_nOnLineVisibleItemNum = 0;
		m_nOffLineVisibleItemNum = 0;

		if (bSearch)
		{
			/*
			 *@ insert friends with search
			 */
			if (vecFriends.size() > 0)
			{
				InsertFriends(&vecFriends[0], vecFriends.size(), sText.GetString());
			}
			m_bDoSearch=true;
			m_pSearchQuit->Show(TRUE);
			
			if (GetVisibleFriendItemCount() <= 0)
			{
				m_pTree->Show(FALSE);
				m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
				m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
				m_pSearchFailure->Show(TRUE);
			}
			else
			{
				m_pTree->Show(TRUE);
				m_pSearchFailure->Show(FALSE);
				m_pTree->ShowItem(m_pTree->GetItem(0), m_nOnLineVisibleItemNum > 0);
				m_pTree->ShowItem(m_pTree->GetItem(1), m_nOffLineVisibleItemNum > 0);
			}
		} 
		else
		{
			/*
			 *@ insert friends without search
			 */
			m_pTree->Show(TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(0), TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(1), TRUE);
			if (vecFriends.size() > 0)
			{
				InsertFriends(&vecFriends[0], vecFriends.size());
			}
			m_bDoSearch=false;
			m_pSearchQuit->Show(FALSE);
		}

		/*
		 *@ start the avatar thread
		 */
		if (_threadHandle == NULL)
		{
			start();

			if (_threadHandle == NULL) 
				return;
		}

		/*
		 *@ refresh the fiends avatar in the chat-dialog
		 */
		std::vector<CHAT_FRIEND>::iterator itr = vecFriends.begin();
		for (; itr != vecFriends.end(); ++itr)
		{
			if (itr->nPresence >= ArcPresenceType::XA) continue;
			//online user
			CUIChatDlgBase* pDlg = pChatMgr->GetChatDailog(itr->szUserName);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				pDlg->UpdateAvatar((UINT)itr->nPresence);
			}
		}

		/*
		 *@ refresh the friends number distinguished by offline-online
		 */
		UpdateFriendListHeaderNum(true);
		UpdateFriendListHeaderNum(false);

		/*
		 *@ init self status when show the friend list after login IM system success
		 */
		m_bInitFriendListFinished = true;
		m_nCurPressence = (ArcPresenceType)pChatMgr->GetIMMgr().GetXmppClient().GetPresence();
		pChatMgr->GetIMMgr().GetXmppClient().SetPresece(m_nCurPressence);

		/*
		 *@ init friend list in overlay(POST MESSAGE IN OVERLAY)
		 */
		im_msg_roster_arrive_for_overlay msg;
		pChatMgr->SendCoreMsg(HWND_BROADCAST,&msg);
		
		/*
		 *@ check the off-line message
		 */
		pChatMgr->GetIMMgr().CheckOfflineMessage();

		/*
		 *@ get the avatars of the friends whoes presence is online or busy
		 */
		std::vector<CHAT_FRIEND>::iterator friendItr = vecFriends.begin();
		for (; friendItr != vecFriends.end(); ++friendItr)
		{
			if (friendItr->nPresence < ArcPresenceType::XA)
			{
				g_theSNSManager.GetGameCharactorService(friendItr->szUserName, friendItr->szNick);
			}
		}

		m_pTree->UpdateLayout();
	}
}

void CUIFriendDlgPrimary::UpdateFriendListHeaderNum( bool bOnLine )
{
	ISonicString *pSonicString = bOnLine ? m_pOnLineString : m_pOffLineString;
	
	int nCount = bOnLine ? m_nOnLineVisibleItemNum : m_nOffLineVisibleItemNum;
	_tstring tsCount;
	tsCount.Format(1, _T(" %d"), nCount);

	_tstring tsOnLineNum = bOnLine ? m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE")) : m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"));
	tsOnLineNum += tsCount;

	pSonicString->Format(_T("/c=%x, font, font_height=13, font_face='Open Sans', align=1/%s"), FLP_TMI_COLOR, tsOnLineNum.c_str());	
}

LRESULT CUIFriendDlgPrimary::OnHandleSyncSentMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	CUIChatMgrPrimary *pPrimary = dynamic_cast<CUIChatMgrPrimary*>(m_pUIChatMgr);
	im_msg_handle_sync_sent_message *pMsg = (im_msg_handle_sync_sent_message*)lParam;
	if (pPrimary && pMsg)
	{
		pPrimary->HandleSyncSentMessage(pMsg->szSelfName, pMsg->szContent, pMsg->szUser, pMsg->nType, pMsg->nTime, pMsg->nSyncID);
	}

	if (pMsg)
	{
		free((void*)pMsg);
	}

	return 0L;
}

LRESULT CUIFriendDlgPrimary::OnHandleCopyDateMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	core_msg_header *pHeader = (core_msg_header*)lParam;
	if (pHeader)
	{
		m_pUIChatMgr->HandleCopyDateMessage(pHeader);
	}
	return 0L;
}

void CUIFriendDlgPrimary::StartSearch( _tstring szSearchText /*= _T("")*/ )
{
	if (!m_pUIChatMgr || m_nCurPressence >= (int)ArcPresenceType::XA)
		return;

	if (!m_pTree || !m_pTree->IsValid() || !m_pSearchFailure || !m_pSearchFailure->IsValid())
		return;

	CUIFriendDlgBase::StartSearch(szSearchText);

	if( !szSearchText.empty() )
	{
		if ((m_nOnLineVisibleItemNum + m_nOffLineVisibleItemNum) <= 0)
		{
			m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
			m_pTree->Show(FALSE);
			
			m_pSearchFailure->Show(TRUE);
		}
		else
		{
			m_pTree->Show(TRUE);
			m_pSearchFailure->Show(FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(0), m_nOnLineVisibleItemNum > 0);
			m_pTree->ShowItem(m_pTree->GetItem(1), m_nOffLineVisibleItemNum > 0);
		}
	}
	else
	{
		m_pTree->Show(TRUE);
		m_pTree->ShowItem(m_pTree->GetItem(0), TRUE);
		m_pTree->ShowItem(m_pTree->GetItem(1), TRUE);
		m_pSearchFailure->Show(FALSE);
	}

	m_pTree->UpdateLayout();
}


//end of friend dlg primary
///////////////////////////////////////////////////////////////////////////////////////////

CUIStatusDlgPrimary::CUIStatusDlgPrimary(CUIChatMgrPrimary* pMgr):CUIStatusDlgBase(pMgr)
{
}

CUIStatusDlgPrimary:: ~CUIStatusDlgPrimary()
{

}

LRESULT CUIStatusDlgPrimary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	//attach window described in sonic.xml
	RECT rect;
	GetCoreUI()->AttachWindow(m_hWnd, _T("chat_status_popup_dialog")) ;
	if (::IsWindow(GetParent().m_hWnd))
	{
		GetParent().GetWindowRect(&rect);
		MoveWindow(rect.left+1, rect.top+25, 121, 93);
	}

	rect.left = 2;
	rect.top = 10;
	rect.right = 119;
	rect.bottom = 91;

	m_LBSetting.Create(m_hWnd, &rect, NULL,WS_CHILD|WS_VISIBLE|LBS_NOTIFY,NULL,IDC_LISTBOX_SET_PRESENCE);
	HWND hListWnd = m_LBSetting.m_hWnd;
	GetSonicUI()->SkinFromHwnd(hListWnd)->SetSkin(_T("filter"), _T("type:1; src:%d"), GetSonicUI()->SkinFromHwnd(m_hWnd));
	m_LBSetting.SetSelectColor(RGB(14,18,29));
	m_pWndEffect->Attach(::GetParent(m_LBSetting));//fixed bug: the list box doesn't disappear when going to shut down PC.

	// EXCEPTION: OCX Property Pages should return FALSE
	return 0L;
}

LRESULT CUIStatusDlgPrimary::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HWND hWnd = (HWND)wParam;
	if (!hWnd || ::GetParent(hWnd) != m_hWnd)
	{
		ShowWindow(SW_HIDE);
	}
	return 0;
}
CUIChatDlgPrimary::CUIChatDlgPrimary(CUIChatMgrPrimary* pMgr,_ITEM_BASE_INFO info):CUIChatDlgBase(pMgr,info)
{
	m_pAvatar = GetSonicUI()->CreateImage();
}
CUIChatDlgPrimary::~CUIChatDlgPrimary()
{
	SONICUI_DESTROY(m_pAvatarPaint)
}

LRESULT CUIChatDlgPrimary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CUIChatDlgBase::OnInitDialog(uMsg, wParam, lParam, bHandled);

	GetCoreUI()->AttachWindow(m_hWnd, _T("chat_popup_dialog"));

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_clear"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIChatDlgPrimary::OnStaticClickedChatClear);
	
	SetWindowText(m_info._szNick) ;
	SetIcon(m_hIcon, TRUE);//Set big icon
	SetIcon(m_hIcon, FALSE);//Set small icon

	ISonicString* pNickNameString = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_nickname"));
	_tstring sNick = m_info._szNick;
	sNick.Replace(_T("/"),_T("//"));//translate the control char to avoid conflict.

	pNickNameString->Format(_T("/line_width=200, single_line=2//a,c=%x,font,font_height=14,font_face='Open Sans'/%s"),RGB(248,248,248),sNick.c_str());
	pNickNameString->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIChatDlgPrimary::OnDelegateClickedChatFriend);
	pNickNameString->Redraw(FALSE);

	//input edit
	m_pEditInput = GetCoreUI()->GetObjByID<WTL::CCoreRichEdit>(m_hWnd,_T("chat_popup_dialog_redit_input"));
	m_pEditInput->LimitText(CHAT_MAX_INPUT_CHARACTERS);
	HWND hScroll = m_pEditInput->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollSkin = GetSonicUI()->SkinFromHwnd(hScroll);
	pScrollSkin->GetPaint()->SetBkColor(CIT_EDIT_SCROLL_BG_COLOR);

	//output edit
	m_pEditOutPut = GetCoreUI()->GetObjByID<WTL::CCoreRichEdit>(m_hWnd,_T("chat_popup_dialog_redit_output"));
	m_pEditOutPut->SetReadOnly(TRUE);
	HWND hScrollTmp2 = m_pEditOutPut->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollTmp2 = GetSonicUI()->SkinFromHwnd(hScrollTmp2);
	pScrollTmp2->GetPaint()->SetBkColor(CIT_EDIT_SCROLL_BG_COLOR);

	m_pAvatarPaint = GetSonicUI()->CreatePaint();
	m_pAvatarPaint->Create(FALSE,GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetWidth(),GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetHeight()/3);

	ShowHistoryRecords();
	UpdateProfile();

	return TRUE;
}

void CUIChatDlgPrimary::ClearLog(LPCTSTR lpUserName)
{
	CUIChatMgrPrimary *pPrtMgr = dynamic_cast<CUIChatMgrPrimary *>(m_pUIChatMgr);
	if (pPrtMgr)
	{
		pPrtMgr->GetIMMgr().GetMsgLogMgr().ClearLog(lpUserName); 
	}
}

void CUIChatDlgPrimary::ClickFriend()
{
	if (!m_pUIChatMgr) return;

	_tstring szUrl;
	szUrl.Format(2,_T("%s/user/%s/activity"), m_pUIChatMgr->GetStringFromId( _T("IDS_CONFIG_WEBSITE_SNS")).c_str( ),m_info._szNick);

	CPWPDlg* pDlg = (CPWPDlg*)AfxGetApp()->GetMainWnd();
	if (pDlg )
	{
		pDlg->Navigate(szUrl);
		if(pDlg->IsIconic()|| !pDlg->IsWindowVisible())
		{
			pDlg->ShowWindow(SW_RESTORE);
		}
	}
}

void CUIChatDlgPrimary::UpdateProfile()
{
	_tstring tsCharater = m_info._szCharater;
	_tstring tsServer = m_info._szServer;
	tsCharater.Replace(_T("/"),_T("//"));
	tsServer.Replace(_T("/"),_T("//"));
	//set friend's in-game avatar info on the chat window
	ISonicString* pGameCharacter = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_character"));
	pGameCharacter->Format(_T("/global,c=0x949495,font,font_height=11,font_face='Open Sans'/%s/br/%s"), tsCharater.c_str(), tsServer.c_str());
	pGameCharacter->Redraw();

	//set friend's avatar image on the chat window
	_tstring szPath = m_pUIChatMgr->GetBaseDir() + m_info._szAvatarPath ;
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		m_pAvatar->Load(szPath.c_str());
	}
	UpdateAvatar(m_pAvatar, m_info.iType);
}
