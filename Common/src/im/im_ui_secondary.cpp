#pragma once
#include "im\im_ui_secondary.h"
#include "OverlayImpl.h"
#include "OverlayImImpl.h"
//#include "resource.h"
#include "data/DataPool.h"


CUIChatMgrSecondary::CUIChatMgrSecondary()
{
	m_bIsInGame = true;
}

CUIChatMgrSecondary::~CUIChatMgrSecondary()
{
}

CUIFriendDlgBase*  CUIChatMgrSecondary::CreateFriendDlgObj()
{
	return new CUIFriendDlgSecondary(this,NULL);
}

CUIChatDlgBase*  CUIChatMgrSecondary::CreateChatDlgObj(_ITEM_BASE_INFO info)
{
	return new CUIChatDlgSecondary(this,info);
}

CUIStatusDlgBase*  CUIChatMgrSecondary::CreateStatusDlgObj()
{
	return new CUIStatusDlgSecondary(this);
}
CUITabMgrDlg *CUIChatMgrSecondary::CreateTabMgrDlgObj()
{
	return new CUITabMgrDlgSecondary(this);
}

im_dialog_params   CUIChatMgrSecondary::GetDefaultTabMgrParams()
{
	return COverlayImImpl::GetInstance()->GetDefaultTabMgrParams();
}

void CUIChatMgrSecondary::OnCoreMessage(HWND hFrom, core_msg_header * pHeader)
{
	CUIChatMgrBase::OnCoreMessage(hFrom,pHeader);

	switch (pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_CONNECT_DONE:
		{

		}
		break;
	case ARC_CHAT_MSG_ITEM_INFO:
		{

		}
		break;
	case ARC_CHAT_MSG_RCV_MSG:
		{
			//CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_rcv_msg*)pHeader)->msg._szUserName);
			//if (!pDlg || !::IsWindowVisible(pDlg->m_hWnd))
			_IM_BASE_CONFIG config;
			LoadConfig(config);
			if(config.bChatMsgNotify && m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				SendMessage(::GetParent(m_pFriendDlg->m_hWnd), WM_COREMESSAGE, 0, (LPARAM)pHeader);
			}

		}
		break;
	case ARC_CHAT_MSG_FRIEND_LIST_RESP:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				SendCoreMsgDirectly(m_pFriendDlg->m_hWnd,pHeader);
			}
		}
		break;
	case ARC_CHAT_MSG_PRESENCE_CHANGED:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				SendMessage(::GetParent(m_pFriendDlg->m_hWnd), WM_COREMESSAGE, 0, (LPARAM)pHeader);
			}
		}
		break;
	case ARC_CHAT_MSG_AVATAR_CHANGED:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				SendMessage(::GetParent(m_pFriendDlg->m_hWnd),WM_COREMESSAGE,0,(LPARAM)pHeader);
			}
		}
		break;
	case ARC_CHAT_MSG_ROSTER_ARRIVE_FOR_OVERLAY:
		{
			if (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd))
			{
				::PostMessage(m_pFriendDlg->m_hWnd, WM_INITFRIENDLIST_IN_OVERLAY, 0, 0);
			}
		}
		break;
	default:
		break;
	}

	return;
}

bool  CUIChatMgrSecondary::InitChatSystem(HWND hWnd)
{ 
	BOOL bRet = m_coreMsgClient.Initialize(this,(DWORD)hWnd,ARC_IM_SERVER_NAME);
	if (bRet)
	{
		m_coreMsgClient.Connect(TRUE);
	}
	return (bRet == TRUE) ? true : false;
}

bool CUIChatMgrSecondary::UnInitChatSystem()
{
   m_coreMsgClient.Uninitialize();
   return true;
}
_tstring CUIChatMgrSecondary::GetStringFromId( LPCTSTR  lpszStr)
{
	im_msg_load_string msg;
	lstrcpyn(msg.szId,lpszStr,MAX_PATH);
	SendCoreMsg2Svr(&msg);
    return _tstring(msg.szContent);
}

_tstring CUIChatMgrSecondary::GetUserProfilePath()
{
	return theDataPool.GetUserProfilePath();
}

_tstring CUIChatMgrSecondary::GetBaseDir()
{
	return theDataPool.GetBaseDir();
}

bool CUIChatMgrSecondary::GetFriendInfo(_tstring szUser, LPITEM_BASE_INFO lpInfo, bool bByNick /*= false*/)
{
	if (!lpInfo)
		return false;

	im_msg_item_info msg;
	lstrcpyn(msg.info._szUser,szUser.c_str(),MAX_PATH);
	msg.bByNick = bByNick;

	SendCoreMsg2Svr(&msg);

	if (_tcslen(msg.info._szUser) <= 0)
		return false;

	*lpInfo = msg.info;
	return true;
}

bool CUIChatMgrSecondary::GetSelfUserInfo(LPITEM_BASE_INFO lpInfo)
{
	if (!lpInfo)
		return false;

	im_msg_self_info msg;
	SendCoreMsg2Svr(&msg);

	if (_tcslen(msg.info._szUser) <= 0) 
		return false;

	*lpInfo = msg.info;
	return true;
}

__int64 CUIChatMgrSecondary::GetTime()
{
	im_msg_get_time msg;
	SendCoreMsg2Svr(&msg);
	return msg.nTime;
}

void CUIChatMgrSecondary::SetLoginStatus( int loginStatus )
{
	im_msg_sync_login_status msg;
	msg.nStatus = loginStatus;
	SendCoreMsg2Svr(&msg);
}

bool CUIChatMgrSecondary::RequestFriendList()
{
	im_msg_friend_list_req msg;
	return SendCoreMsg2Svr(&msg);
}

bool CUIChatMgrSecondary::GetInitFriendListIsFinished()
{
	im_msg_friend_list_is_inited_req msg;
	SendCoreMsg2Svr(&msg);
	return msg.bIsInitFinished;
}

bool CUIChatMgrSecondary::SendChatMessage(MSG_LOG_ELEMENT msgEelement, bool bOffline/* = false*/)
{
	im_msg_send_msg msg;
	msg.msg= msgEelement;
	msg.bOfflineMessage = bOffline;
	return SendCoreMsg2Svr(&msg);
}

bool CUIChatMgrSecondary::SetPresence(int iType)
{
	im_msg_set_presence msg;
	msg.iType = iType;
	bool bRet = SendCoreMsg2Svr(&msg);
	return bRet;
}

bool CUIChatMgrSecondary::LoadHistoryRecord(_tstring szUser,UINT nCount,std::list<MSG_LOG_ELEMENT>& RecordList) 
{
	RecordList.clear();
	im_msg_load_history_records msg;
	lstrcpyn(msg.szUserName,szUser.c_str(),MAX_PATH-1);
	msg.nCount = nCount;
	bool bRet = SendCoreMsg2Svr(&msg);
	if (bRet)
	{
		UINT nMax = (nCount>MAX_HISTORY_RECORDS_COUNT) ? MAX_HISTORY_RECORDS_COUNT:nCount;
		for (int i=0;i<nMax;i++)
		{
			if (_tstring(msg.aRecords[i]._szDate).empty())
				break;

			RecordList.push_back(msg.aRecords[i]);
		}
	}
	return bRet;
}

bool CUIChatMgrSecondary::ClearHistroyRecord(_tstring szUser)
{
	im_msg_clear_history_records msg;
	lstrcpyn(msg.szUserName,szUser.c_str(),MAX_PATH-1);
	return SendCoreMsg2Svr(&msg);
}

int CUIChatMgrSecondary::GetPresence(_tstring szUser)
{
	im_msg_get_presence msg;
	lstrcpyn(msg.szUserName,szUser.c_str(),MAX_PATH-1);
	SendCoreMsg2Svr(&msg);
	return msg.iType;
}

int CUIChatMgrSecondary::GetFriendsCount()
{
	im_msg_get_friends_count msg;
	SendCoreMsg2Svr(&msg);
	return msg.nCount;
}

void CUIChatMgrSecondary::SyncSentMessage( LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId )
{
	DWORD dwContentLen = _tcslen(lpcContent)+1;
	DWORD dwMessageLen = sizeof(im_msg_handle_sync_sent_message) + sizeof(TCHAR)*dwContentLen;
	im_msg_handle_sync_sent_message* pMsg = (im_msg_handle_sync_sent_message*)malloc(dwMessageLen);
	memset(pMsg, 0, dwMessageLen);
	pMsg->dwCmdId = ARC_CHAT_MSG_HANDLE_SYNC_SENT_MESSAGE;
	pMsg->dwSize = dwMessageLen;
	pMsg->nTime = nTime;
	pMsg->nSyncID = nSyncId;
	pMsg->nType = nType;
	pMsg->dwLen = dwContentLen;
	_tcsncpy(pMsg->szSelfName, lpcSelfName, _tcslen(lpcSelfName));
	_tcsncpy(pMsg->szUser, lpcUser, _tcslen(lpcUser));
	_tcsncpy(pMsg->szContent, lpcContent, dwContentLen-1);

	SendCoreMsg2Svr(pMsg);

	free((void*)pMsg);
}

BOOL CUIChatMgrSecondary::SendCoreMsg(HWND hWnd, core_msg_header * pHeader)
{
	//if (m_pMsgClient)
	{
		return m_coreMsgClient.SendCoreMsg(hWnd,pHeader);
	}
	return false;
}

BOOL CUIChatMgrSecondary::SendCoreMsg2Svr(core_msg_header * pHeader)
{
//	if (m_pMsgClient)
	{
		return m_coreMsgClient.SendCoreMsg(pHeader);
	}
	return false;
}
void CUIChatMgrSecondary::ShowTabMgrChatDlg(UINT nShowCmd)
{
	std::vector<CUITabMgrDlg*>::iterator it = m_vecTabMgr.begin();
	std::vector<CUITabMgrDlg*>::iterator end = m_vecTabMgr.end();
	while(it != end)
	{
		(*it)->ShowWindow(nShowCmd);
		++it;
	}
	
}
CUIChatDlgBase*  CUIChatMgrSecondary::ShowChatDlg(_ITEM_BASE_INFO info, BOOL bOfflineMessage, 
												  bool bNeedShow, bool bShow,bool bActive /*= true*/)
{
	if(::IsWindowVisible(COverlayImpl::GetInstance()->GetMainWnd()->m_hWnd))
	{
		return CUIChatMgrBase::ShowChatDlg(info, bOfflineMessage, bNeedShow, bShow, false);
	}
	else
	{
		return CUIChatMgrBase::ShowChatDlg(info, bOfflineMessage, bNeedShow, false, false);
	}
}
void CUIChatMgrSecondary::ShowWindow( HWND hWnd,  int nCmdShow)
{
	if (nCmdShow != 0)
	{
		::ShowWindow(hWnd,SW_SHOWNA);
	}
	else
	{
		::ShowWindow(hWnd,nCmdShow);
	}
		
}
void CUIChatMgrSecondary::ShowListWindow(CCoreListBox *pList,int nCmdShow)
{
	if(!pList)
	{
		return;
	}
	if (nCmdShow != 0)
	{
		pList->ShowWindow(SW_SHOWNA);
	}
	else
	{
		pList->ShowWindow(nCmdShow);
	}
}

int CUIChatMgrSecondary::GetSefPresence()
{
	ITEM_BASE_INFO info; 
	GetSelfUserInfo(&info);
	return info.iType;		
}
 //BOOL CUIChatMgrSecondary::OnDrop(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem)
//{
//	if (pDragTo == NULL)
//	{
//		ISonicTab* pTab = (ISonicTab*) pDragFrom;
//
//		//im_dialog_params para;
//		//para.pParentWnd = COverlayImpl::GetInstance()->GetMainWnd();
//		///*CRect rc = COverlayImImpl::GetInstance()->GetChatTabDlgDefaultRect();
//		//para.rt.left = rc.left;
//		//para.rt.top = rc.top;
//		//para.rt.bottom = rc.bottom;
//		//para.rt.right = rc.right;*/
//		//para.nShowCmd = SW_SHOW;
//
//		CUITabMgrDlg* pTabMgr = CreateTabMgrDlg(COverlayImImpl::GetInstance()->GetDefaultTabMgrParams());
//		if (pTabMgr && pTabMgr->GetTabCtrl())
//		{
//	
//			pTabMgr->GetTabCtrl()->AddTab(pTab->GetTabWnd(pDraggingItem), pTab->GetTabText(pDraggingItem));
//	    }
//		//pTab->DeleteTab(nOldIndex);
//	}
//	return TRUE;
//}

CUIFriendDlgSecondary::CUIFriendDlgSecondary(CUIChatMgrSecondary* pMgr,CWindow* pParent)
:CUIFriendDlgBase(pMgr,pParent)
,m_pInviteFriendTilte(NULL)
,m_pInviteFriendBtn(NULL)
,m_bHandleSelfPresenceWhenInitIM(false)
{
	SetListBoxNickNameFormatType(_T("/line_width=210, single_line=2//c=%x, font, font_height=12, font_face='Open Sans',align=1/%s"));
	SetListBoxAvatarFormatType(_T("/p=%d, pstretch=20|20/"));
	SetListBoxNickNameColor(RGB(197,197,198));
}

CUIFriendDlgSecondary::~CUIFriendDlgSecondary()
{
	SONICUI_DESTROY(m_pInviteFriendBtn);
	SONICUI_DESTROY(m_pInviteFriendTilte);
}

LRESULT CUIFriendDlgSecondary::OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;
	}

	CUIFriendDlgBase::OnProcessMsg(uMsg, wParam, lParam, bHandled);

	switch(pHeader->dwCmdId)
	{
	case ARC_CHAT_MSG_FRIEND_LIST_RESP:
		{
			HandleFriendListResp((im_msg_friend_list_response*)pHeader);
		}
		break;
	default:
		break;
	}
	return 0L;
}

void CUIFriendDlgSecondary::HandleFriendListResp(im_msg_friend_list_response * pMsg)
{
	/*
	 *@ verify the parameter
	 */
	if (!pMsg)
		return;

	/*
	 *@ check weather to do search now
	 */
	ATL::CString sText;
	m_pSearchBar->GetWindowText(sText);
	bool bSearch = sText.GetLength() > 0 && !m_bForcing;

	/*
	 *@ insert the friends
	 */
	if (pMsg->dwLen > 0)
	{
		InsertFriends(&pMsg->aFriendList[0], pMsg->dwLen, bSearch ? sText.GetString() : NULL);
	}

	/*
	 *@ refresh the fiends avatar in the chat-dialog
	 */
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
		 *@ update the friend's avatar
		 */
		for (int i = 0; i < pMsg->dwLen; ++i)
		{
			if (pMsg->aFriendList[i].nPresence >= ArcPresenceType::XA) 
				continue;
			//online user
			CUIChatDlgBase* pDlg = m_pUIChatMgr->GetChatDailog(pMsg->aFriendList[i].szUserName);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				pDlg->UpdateAvatar((UINT)pMsg->aFriendList[i].nPresence);
			}
		}

		/*
		 *@ must init the presence variable of the base class here
		 */
		m_nCurPressence = (ArcPresenceType)m_pUIChatMgr->GetSefPresence();

		/*
		 *@ when init im-system, check whether need to handle self presence
		 */
		if (m_bHandleSelfPresenceWhenInitIM)
		{
			m_bHandleSelfPresenceWhenInitIM = false;
			HandleSelfPresence(m_nCurPressence);
		}

		/*
		 *@ get the avatars of the friends whose presence is online or busy
		 */
		for (int nIndex = 0; nIndex < pMsg->dwLen; ++nIndex)
		{
			if (pMsg->aFriendList[nIndex].nPresence < ArcPresenceType::XA)
			{
				_tstring tsFriendUser = pMsg->aFriendList[nIndex].szUserName;
				m_wqAvatarFriends.enter(tsFriendUser);
			}
		}

		m_bInitFriendListFinished = true;
	}

	/*
	 *@ refresh the friends number distinguished by offline-online
	 */
	UpdateFriendListHeaderNum(true);
	UpdateFriendListHeaderNum(false);

	/*
	 *@ refresh the UI
	 */
	if (bSearch)
	{//search
		m_bDoSearch=true;
		m_pSearchQuit->Show(TRUE);
		if (GetVisibleFriendItemCount() <= 0)
		{
			m_pTree->Show(FALSE);
			m_pSearchFailure->Show(TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
		}
		else
		{
			m_pTree->Show(TRUE);
			m_pSearchFailure->Show(FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(0), m_nOnLineVisibleItemNum > 0);
			m_pTree->ShowItem(m_pTree->GetItem(1), m_nOffLineVisibleItemNum > 0);
		}
		m_pInviteFriendBtn->Show(FALSE);
		m_pInviteFriendTilte->Show(FALSE);
	}
	else
	{//not search
		m_bDoSearch=false;
		m_pSearchQuit->Show(FALSE);
		if (GetFriendItemCount() <= 0)
		{
			m_pTree->Show(FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
			m_pInviteFriendBtn->Show(TRUE);
			m_pInviteFriendTilte->Show(TRUE);
		}
		else
		{
			m_pTree->Show(TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(0), TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(1), TRUE);
			m_pInviteFriendBtn->Show(FALSE);
			m_pInviteFriendTilte->Show(FALSE);
		}
		
		m_pSearchFailure->Show(FALSE);
	}

	m_pTree->UpdateLayout();
}

LRESULT CUIFriendDlgSecondary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	// atach window from .xml UI config
	GetCoreUI()->AttachWindow(m_hWnd, _T("friendlist_popup_dialog"));
	if (!COverlayImImpl::GetInstance()->InitIM())
		return 0L;
	//GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_title_status"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnSetStatus);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("firendlist_popup_dialog_btn_add"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgSecondary::OnAddFriendClick);
	//GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_sep"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgPrimary::OnClickBtnSetStatus);
	SetWindowText(m_pUIChatMgr->GetStringFromId(_T("IDS_POPDLG_FRIENDLIST_TITLE")).c_str()) ;
	//SetIcon(m_hIcon, TRUE);//Set big icon
	//SetIcon(m_hIcon, FALSE);//Set small icon

	ISonicSkin *pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);

	CRect rcClient;
	GetClientRect(&rcClient);

	m_pTree->Create(m_hWnd, FLP_LB_LEFT_OS, FLP_LB_TOP_OS, 
		(rcClient.right - rcClient.left) - FLP_LB_RIGHT_OS - FLP_LB_LEFT_OS, 
		(rcClient.bottom - rcClient.top) - FLP_LB_BOTTOM_OS - FLP_LB_TOP_OS, 
		GetCoreUI()->GetImage(_T("SCROLLBAR_THUMB")), 
		GetCoreUI()->GetImage(_T("SCROLLBAR_CHANNEL_TREE")), NULL, NULL);

	CUIChatMgrSecondary *pChatMgr = dynamic_cast<CUIChatMgrSecondary *>(m_pUIChatMgr);

	m_pSearchBar = GetCoreUI()->GetObjByID<WTL::CCoreEdit>(m_hWnd,_T("firendlist_popup_dialog_edit_serach"));
	m_pSearchBar->LimitText(FLP_SEARCH_TEXT_MAXSIZE);
	ATL::CString csDefault;
	m_pSearchBar->GetWindowText(csDefault);
	SetSearchDefaultText(csDefault.GetString());

	m_pSearchFailure->Format(
		_T("/c=%x, font, font_height=13, font_face='Open Sans', align=1/%s"),
		FLP_SEARCH_FAILURE_COLOR,
		m_pUIChatMgr->GetStringFromId(_T("IDS_CHAT_SEARCH_FAILURE")).c_str()
		);
	m_pSearchFailure->Show(FALSE);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	m_pSearchQuit->Format(_T("/a,p2=%d/"),GetCoreUI()->GetImage(_T("BTN_CHAT_SEARCH"))->GetObjectId());
	m_pSearchQuit->Show(FALSE);

   m_pInviteFriendTilte = GetSonicUI()->CreateString();
   m_pInviteFriendTilte->Format(_T("/c=%x, font, font_height=14, font_face='Open Sans', align=1/%s"),
	   RGB(170,170,170),
	   m_pUIChatMgr->GetStringFromId(_T("IDS_INVITE_FRIENDS")).c_str());
	   //m_pInviteFriendTilte->Show(FALSE);
   m_pInviteFriendBtn = GetSonicUI()->CreateString();
   core_msg_cookie* pMsgCookie =  COverlayImpl::GetInstance()->GetMsgCookie();
   _tstring strText;
   if (pMsgCookie)
   {
		strText = pMsgCookie->csInvite;
   }
   m_pInviteFriendBtn->Format(_T("/a,p=%d,btn_width=116,btn_text='/c=%x,align=0x11,single_line=2,font, font_height=13,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BTN_OVERLAY_INVITE"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),strText.c_str());
 //  m_pInviteFriendBtn->Show(FALSE);
   m_pInviteFriendBtn->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgSecondary::OnInviteFriendsClick);
 //  pPaint->AddObject(m_pInviteFriendTilte->GetObjectId(),10,88,TRUE);
//   pPaint->AddObject(m_pInviteFriendBtn->GetObjectId(),10,115,TRUE);


// 	m_pAvatarPaint = GetSonicUI()->CreatePaint();
// 	if (!m_pAvatarPaint) return 0;
// 	m_pAvatarPaint->Create(
// 		FALSE,
// 		GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetWidth(),
// 		GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetHeight()/3
// 		);

	//update nickname.
	/*_tstring sNick = theDataPool.GetUserNick();
	sNick.Replace(_T("/"),_T("//"));
	ISonicString* pNickStr = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_nick"));
	pNickStr->Format(_T("/c=%x, font, font_height=14, font_face='Open Sans',align=1/%s"), FLP_NICKNAME_COLOR, sNick.c_str());
	pNickStr->Redraw();*/

//	SendMessage(WM_UPDATE_CHAT_STATUS,0,Presence::Unavailable);//ensure the default head image can be displayed at the beginning.

	 EndInit();
	 m_bHandleSelfPresenceWhenInitIM = pChatMgr->GetInitFriendListIsFinished();
	 return 0;
}
void CUIFriendDlgSecondary::AddFriendClick(ISonicString *, LPVOID pReserve)
{
	_tstring szUrl = m_pUIChatMgr->GetStringFromId(_T("IDS_SEARCH_FRIENDS"));
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(szUrl.c_str());
}
void CUIFriendDlgSecondary::OnSize(UINT nType, WTL::CSize size)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_pTree->IsValid())
	{
		m_pTree->Move(FLP_LB_LEFT_OS, FLP_LB_TOP_OS, 
			rcClient.right-FLP_LB_RIGHT_OS - FLP_LB_RIGHT_OS, 
			rcClient.bottom-FLP_LB_BOTTOM_OS - FLP_LB_TOP_OS);
	}
}

void CUIFriendDlgSecondary::StartSearch( _tstring szSearchText /*= _T("")*/ )
{
	if (!m_pUIChatMgr || m_nCurPressence >= (int)ArcPresenceType::XA)
		return;

	if (!m_pTree || !m_pTree->IsValid() || 
		!m_pSearchFailure || !m_pSearchFailure->IsValid())
	{
		return;
	}

	CUIFriendDlgBase::StartSearch(szSearchText);

	if( !szSearchText.empty() )
	{//SEARCH
		if ((m_nOnLineVisibleItemNum + m_nOffLineVisibleItemNum) <= 0)
		{
			m_pTree->Show(FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
			m_pSearchFailure->Show(TRUE);
			Invalidate();
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
	{//NOT SEARCH
		m_pSearchFailure->Show(FALSE);
		if (GetFriendItemCount() == 0)
		{
			m_pTree->Show(FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
			m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);
		}
		else
		{
			m_pTree->Show(TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(0), TRUE);
			m_pTree->ShowItem(m_pTree->GetItem(1), TRUE);
		}
	}
	
	m_pTree->UpdateLayout();
}

void CUIFriendDlgSecondary::OnInviteFriendsClick(ISonicBase *, LPVOID pReserve)
{
	core_msg_cookie* pMsgCookie =  COverlayImpl::GetInstance()->GetMsgCookie();
	if (pMsgCookie)
	{
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(pMsgCookie->csFriendSearchPage);
	}
	
}
void CUIFriendDlgSecondary::OnPaint(HDC)
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
		dp.cx = dp.cx - 20;
		pImg->Draw(dc.m_hDC, rcSearch.left-1, rcSearch.top-1, &dp);

		rcSearchQuit.SetRect(rcSearch.right,rcSearch.top,rcSearch.right,rcSearch.bottom);
		GetSonicUI()->FillSolidRect(dc.m_hDC,&rcSearchQuit,RGB(19,19,19));
		if (m_pSearchQuit)
		{
			m_pSearchQuit->TextOut(dc.m_hDC, rcSearch.right + 2,rcSearch.top+7,m_hWnd);
		}

		if (m_pSearchFailure)
		{
			m_pSearchFailure->TextOut(dc.m_hDC, rcFriendList.left+(rcFriendList.Width()-m_pSearchFailure->GetWidth())/2,rcFriendList.top+7,m_hWnd);
		}
		if (m_pInviteFriendBtn)
		{
			m_pInviteFriendBtn->TextOut(dc.m_hDC,10,120,m_hWnd);
		}
		if (m_pInviteFriendTilte)
		{
			m_pInviteFriendTilte->TextOut(dc.m_hDC,10,93,m_hWnd);
		}
	}
}
void CUIFriendDlgSecondary::HandleSelfPresence(ArcPresenceType presence)
{
	COverlayImpl::GetInstance()->HandleSelfPresence(presence);
	if (presence > ArcPresenceType::XA)
	{
		ClearAllItems();
		m_pTree->Show(FALSE);
		m_pTree->ShowItem(m_pTree->GetItem(0), FALSE);
		m_pTree->ShowItem(m_pTree->GetItem(1), FALSE);

		if (m_pSearchFailure->IsVisible()) m_pSearchFailure->Show(FALSE);
		if (m_pInviteFriendTilte) m_pInviteFriendTilte->Show(TRUE);
		if (m_pInviteFriendBtn) m_pInviteFriendBtn->Show(TRUE);

		if (_threadHandle != NULL) 
		{
			m_bExitWorkThread = true;
			wait();
			_threadHandle = 0;
			_threadId = 0;
		}

		m_bInitFriendListFinished = false;
		m_pUIChatMgr->HandleSelfOffline();
	}
	else if (m_nCurPressence >= ArcPresenceType::XA && presence < ArcPresenceType::XA)
	{
	}
	m_nCurPressence = presence;

}
void CUIFriendDlgSecondary::HandleSelfAvatar(_tstring lpPath)
{
	m_pUIChatMgr->SetSelfAvatarPath(lpPath.c_str());
	COverlayImpl::GetInstance()->HandleSelfAvatar(m_pUIChatMgr->GetBaseDir() + lpPath);
}

void CUIFriendDlgSecondary::HandleFriendAvatar(_tstring sUserName, LPCTSTR lpPath)
{
	UpdateFriendAvatar(sUserName, lpPath);
}

void CUIFriendDlgSecondary::HandleFriendPresence(_tstring sUserName, ArcPresenceType presence)
{
	UpdateFriendPresence(sUserName, presence);
}

void CUIFriendDlgSecondary::DetectionFriends()
{
	if (!m_pInviteFriendBtn || !m_pInviteFriendTilte)
	{
		return;
	}
	if ( GetFriendItemCount() == 0)
	{
		m_pInviteFriendBtn->Show(TRUE);
		m_pInviteFriendTilte->Show(TRUE);
		Invalidate();
	}
	else
	{
		m_pInviteFriendBtn->Show(FALSE);
		m_pInviteFriendTilte->Show(FALSE);
	}
}

void CUIFriendDlgSecondary::UpdateFriendListHeaderNum( bool bOnLine )
{
	ISonicString *pSonicString = bOnLine ? m_pOnLineString : m_pOffLineString;

	int nCount =  bOnLine ? m_nOnLineVisibleItemNum : m_nOffLineVisibleItemNum;
	_tstring tsCount;
	//tsCount.Format(1, _T(" %d"), bOnLine ? m_nOnLineFriendNum : m_nOffLineFriendNum);
	tsCount.Format(1, _T(" %d"), nCount);

	_tstring tsOnLineNum = bOnLine ? m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE")) : m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"));
	tsOnLineNum += tsCount;

	pSonicString->Format(_T("/c=%x, font, font_height=13, font_face='Open Sans', align=1/%s"), FLP_TMI_COLOR, tsOnLineNum.c_str());	
}

void CUIFriendDlgSecondary::InitFriendList()
{
	CUIChatMgrSecondary *pChatMgr = dynamic_cast<CUIChatMgrSecondary *>(m_pUIChatMgr);
	if (!pChatMgr) 
		return;

	pChatMgr->RequestFriendList();
}

CUIChatDlgSecondary::CUIChatDlgSecondary(CUIChatMgrSecondary* pMgr,_ITEM_BASE_INFO info):CUIChatDlgBase(pMgr,info)
{
	m_pAvatar = GetSonicUI()->CreateImage();
}

CUIChatDlgSecondary:: ~CUIChatDlgSecondary()
{
	SONICUI_DESTROY(m_pAvatarPaint);
}
LRESULT CUIChatDlgSecondary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	GetCoreUI()->AttachWindow(m_hWnd, _T("chat_popup_dialog"));
	GetSonicUI()->SkinFromHwnd(m_hWnd)->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);


	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_clear"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIChatDlgSecondary::OnStaticClickedChatClear);
	SetWindowText(m_info._szNick) ;
//	SetIcon(m_hIcon, TRUE);//Set big icon
//	SetIcon(m_hIcon, FALSE);//Set small icon

	ISonicString* pNickNameString = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_nickname"));
	_tstring sNick = m_info._szNick;
	sNick.Replace(_T("/"),_T("//"));//translate the control char to avoid conflict.

	pNickNameString->Format(_T("/line_width=200, single_line=2//a,c=%x,font,font_height=14,font_face='Open Sans'/%s"),RGB(225,226,228),sNick.c_str());
	pNickNameString->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIChatDlgSecondary::OnDelegateClickedChatFriend);
	pNickNameString->Redraw(FALSE);

	m_pEditInput = GetCoreUI()->GetObjByID<WTL::CCoreRichEdit>(m_hWnd,_T("chat_popup_dialog_redit_input"));
	m_pEditInput->LimitText(CHAT_MAX_INPUT_CHARACTERS);
	HWND hScroll = m_pEditInput->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollSkin = GetSonicUI()->SkinFromHwnd(hScroll);
	pScrollSkin->GetPaint()->SetBkColor(RGB(15,15,15));

	m_pEditOutPut = GetCoreUI()->GetObjByID<WTL::CCoreRichEdit>(m_hWnd,_T("chat_popup_dialog_redit_output"));
	m_pEditOutPut->SetReadOnly(TRUE);
	HWND hScrollTmp2 = m_pEditOutPut->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollTmp2 = GetSonicUI()->SkinFromHwnd(hScrollTmp2);
	pScrollTmp2->GetPaint()->SetBkColor(RGB(15,15,15));

	//GetSonicUI()->EffectFromHwnd(m_hWnd)->EnableWholeDrag(TRUE);
	//GetSonicUI()->SkinFromHwnd(m_hWnd)->SetSkin(_T("bg"), _T("image:%d;color_key:%d"),GetCoreUI()->GetImage(_T("BG_WND"))->GetObjectId(),RGB(255,0,255));

	m_pAvatarPaint = GetSonicUI()->CreatePaint();
	m_pAvatarPaint->Create(FALSE,GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetWidth(),GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetHeight()/3);

	ShowHistoryRecords();//show history message from db.
	UpdateProfile();
	return 0;
}
void CUIChatDlgSecondary::UpdateProfile()
{
	ISonicString* pGameCharacter = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("chat_popup_dialog_static_character"));
	pGameCharacter->Format(_T("/global,c=0x949495,font,font_height=11,font_face='Open Sans'/%s/br/%s"),m_info._szCharater,m_info._szServer);
	pGameCharacter->Redraw();

	//set friend's avatar image on the chat window
	_tstring szPath = m_pUIChatMgr->GetBaseDir() + m_info._szAvatarPath ;
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		m_pAvatar->Load(szPath.c_str());

	UpdateAvatar(m_pAvatar, m_info.iType);
}

void CUIChatDlgSecondary::ClickFriend()
{
	_tstring szUrl;
	_tstring szTemp = m_pUIChatMgr->GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS"));
	szUrl.Format(2,_T("%s/user/%s/activity"),szTemp.c_str( ),m_info._szNick);
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(szUrl.c_str());
}


CUIStatusDlgSecondary::CUIStatusDlgSecondary(CUIChatMgrSecondary* pMgr)
:CUIStatusDlgBase(pMgr)
{

}

CUIStatusDlgSecondary:: ~CUIStatusDlgSecondary()
{

}
LRESULT CUIStatusDlgSecondary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{

	RECT rect;
	GetCoreUI()->AttachWindow(m_hWnd, _T("chat_status_popup_dialog")) ;
	rect.left = 11;
	rect.top = 12;
	rect.right = 128;
	rect.bottom = 100;
	
	m_LBSetting.Create(m_hWnd, &rect, NULL,WS_CHILD|WS_VISIBLE|LBS_NOTIFY,NULL,IDC_LISTBOX_SET_PRESENCE);
	GetSonicUI()->SkinFromHwnd(m_LBSetting)->SetSkin(_T("filter"), _T("type:1; src:%d"), GetSonicUI()->SkinFromHwnd(m_hWnd));
	m_LBSetting.SetSelectColor(RGB(14,18,29));
	m_pWndEffect->Attach(::GetParent(m_LBSetting));//fixed bug: the list box doesn't disappear when going to shut down PC.

	// EXCEPTION: OCX Property Pages should return FALSE
	Init();
	return 0L;
}
void CUIStatusDlgSecondary::UpdateStatus(UINT iStatus)
{

	//GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("friendlist_popup_dialog_title_status"))->Format(_T("/a,c=%x, font, font_height=12, font_face='Open Sans',align=1/%s"),RGB(197,197,198),m_szStatus[iStatus].c_str());
}
void CUIStatusDlgSecondary::Init()
 {
	 m_szStatus[0] = m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE"));
	 m_szStatus[1] = _T("");
	 m_szStatus[2] = _T("");
	 m_szStatus[3] = m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_BUSY"));
	 m_szStatus[4] = _T("");
	 m_szStatus[5] = m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"));
	// LoadStatusIcon();
	 AddSettingItem(ArcPresenceType::Available);
	 AddSettingItem(ArcPresenceType::DND);
	 AddSettingItem(ArcPresenceType::Unavailable);
 }

void CUIStatusDlgSecondary::AddSettingItem(int iStatus)
{
	ISonicString* pStatus = GetSonicUI()->CreateString();
//	pStatus->Format(_T("/p=%d/"),m_mapStatusIcon[iStatus]->GetObjectId());

	ISonicString* pStatusString = GetSonicUI()->CreateString();
	pStatusString->Format(_T("/c=%x, font, font_height=13, font_face='Open Sans',align=1/%s"),RGB(170,170,170),m_szStatus[iStatus].c_str());

	int nIndex = m_LBSetting.GetCount();
//	m_LBSetting.AddObject(nIndex,pStatus,6,6,TRUE,TRUE);
	m_LBSetting.AddObject(nIndex,pStatusString,15,3,TRUE,FALSE);

	m_LBSetting.SetItemHeight(nIndex,28);
	m_LBSetting.SetItemData(nIndex,(DWORD_PTR)iStatus);
}
void CUIStatusDlgSecondary::LoadStatusIcon()
{
	m_mapStatusIcon.clear();
	ISonicImage* pImage = GetCoreUI()->GetImage(_T("BTN_FOOT_CHAT"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(ArcPresenceType::Available,pImage));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(ArcPresenceType::Chat,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_AWAY"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(ArcPresenceType::Away,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_BUSY"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(ArcPresenceType::DND,pImage));
	pImage = GetCoreUI()->GetImage(_T("BG_CHAT_OFFLINE"));
	m_mapStatusIcon.insert(pair<int,ISonicImage*>(ArcPresenceType::Unavailable,pImage));
}
LRESULT CUIStatusDlgSecondary::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	HWND hWnd = (HWND)wParam;
	if (!hWnd || ::GetParent(hWnd) != m_hWnd)
	{
		ShowWindow(SW_HIDE);
	}
	return 0;
}

CUITabMgrDlgSecondary::CUITabMgrDlgSecondary(CUIChatMgrSecondary* pMgr)
:CUITabMgrDlg(pMgr)
{

}

CUITabMgrDlgSecondary:: ~CUITabMgrDlgSecondary()
{

}
LRESULT CUITabMgrDlgSecondary::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CUITabMgrDlg::OnInitDialog(uMsg,wParam,lParam,bHandled);
	GetListWnd()->SetListBoxMargin(11,12);
	return 0;
}
void CUITabMgrDlgSecondary::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (!bShow)
	{
		GetListWnd()->ShowWindow(SW_HIDE);
	}
	SetMsgHandled(FALSE);
}

void CUITabMgrDlgSecondary::ShowListWnd(BOOL bShow)
{
	if (bShow && GetTabCtrl() && GetTabCtrl()->IsValid())
	{
		int nCount = GetTabCtrl()->GetTabCount();
		GetListWnd()->SelectItem(GetTabCtrl()->GetSel());
		GetListWnd()->SetListFocus();
		WTL::CRect rc (*(GetListBar()->GetRect()));
		ClientToScreen(rc);
		int nHeight = nCount * LISTITEMHEIGHT ;
		if (nHeight > LISTMAXHEIGHT)
		{
			nHeight = LISTMAXHEIGHT;
		}
		int nWidthMargin,nHeightMargin;
		GetListWnd()->GetListBoxMargin(nWidthMargin,nHeightMargin);
		::SetWindowPos(GetListWnd()->m_hWnd,NULL, rc.left - 78, rc.bottom - 10, LISTWIDTH + nWidthMargin*2, nHeight + nHeightMargin*2,SWP_NOZORDER);
		//m_ListWnd.ShowWindow(SW_SHOW);
		if (GetChatMgr())
		{
			GetChatMgr()->ShowWindow(GetListWnd()->m_hWnd,SW_SHOW);
		}
		if (GetListWnd()->GetCoreListBox() && GetListWnd()->GetCoreListBox()->IsWindow())
		{
			GetListWnd()->GetCoreListBox()->SetFocus();
		}
	}
	else if (GetListWnd()->IsWindow())
	{
		::ShowWindow(GetListWnd()->m_hWnd,SW_HIDE);
	}
}