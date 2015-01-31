//
#include <time.h>
#include <atltime.h>

#include "toolkit.h"
#include "im\im_ui_mgr.h"

#define DEFAULT_AVATAR_PATH		_T("themes\\sonic\\default_charactor.png")
#define TREEITEM_AVATAR_NAME	_T("TreeItem_Avatar_")

const static unsigned int CSU_LOGIN_TIMER = 100;
const static unsigned int CSU_LOGIN_TIMER_ELAPSE = 2000;
const static unsigned int CUS_SEARCH_TIMER = 101;
const static unsigned int CUS_SEARCH_TIMER_ELAPSE_BASE = 300;


LRESULT CIMMessageWndBase::OnUpdateLoadedAvatar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	if (m_pUIChatMgr->IsFriendDlgValid())
	{
		m_pUIChatMgr->GetFriendDlg()->HandleFriendAvatar(_tstring((LPCTSTR)lParam), NULL);
	}
	free((void*)lParam);
	return 0L;
}

LRESULT CIMMessageWndBase::OnUpdateRefreshPresence( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	if (m_pUIChatMgr->IsFriendDlgValid())
	{
		m_pUIChatMgr->GetFriendDlg()->HandleFriendPresence(_tstring((LPCTSTR)lParam), ArcPresenceType::Available);
	}
	free((void*)lParam);
	return 0L;
}

//end of the CIMMessageWndBase class
/*********************************************************************************************/

CUITabMgrDlg::CUITabMgrDlg(CUIChatMgrBase* pMgr):
m_pTabCtrl(NULL),
m_pMgr(pMgr),
m_ListWnd(this),
m_pListbar(NULL),
m_hIcon(NULL),
m_nCurTabItemWidth(CHAT_TAB_MAX_LIMIT),
m_nCurSelIndex(0)
{

}

CUITabMgrDlg::~CUITabMgrDlg()
{
	SONICUI_DESTROY(m_pListbar);
	SONICUI_DESTROY(m_pTabCtrl);
}

LRESULT CUITabMgrDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("tabchat_dialog"));
	m_pTabCtrl = GetCoreUI()->GetObjByID<ISonicTab>(m_hWnd, _T("chat_tabs"));
	if (m_pTabCtrl && m_pMgr)
	{
		m_pTabCtrl->SetAttr(SATTR_STRING_BTNTEXT_ALIGN, 0);
		m_pTabCtrl->SetAttr(SATTR_STRING_LEFT_PADDING, 15);
		m_pTabCtrl->SetAttr(SATTR_STRING_RIGHT_PADDING, 30);
		m_pTabCtrl->SetAttr(SATTR_TAB_SIMPLE_BORDER, 1);
		m_pTabCtrl->SetAttr(SATTR_TAB_BORDER_COLOR, (LPVOID)(RGB(57, 62, 72)));
		m_pTabCtrl->SetAttr(SATTR_TAB_DRAG_LEVEL, 2);
		m_pTabCtrl->SetDragDropCallback(m_pMgr);
        m_pTabCtrl->Delegate(DELEGATE_EVENT_TAB_DELETING,NULL,this,&CUITabMgrDlg::OnDeleteItem);
        m_pTabCtrl->Delegate(DELEGATE_EVENT_SEL_CHANGED,NULL,this,&CUITabMgrDlg::OnSelItemChanged);
		m_pTabCtrl->SetTabPanelHorMargin(5, 30);
		m_pTabCtrl->SetTabBtnSpace(0);

		ISonicImage* pBkImg = GetCoreUI()->GetImage(_T("IM_TAB_CLOSE_BTN"));
		m_pTabCtrl->SetCloseButton(pBkImg, 12, 10);
	}

	if (m_hIcon != NULL)
	{
		SetIcon(m_hIcon, TRUE);//Set big icon
		SetIcon(m_hIcon, FALSE);//Set small icon
	}

	m_ListWnd.Create(m_hWnd,WTL::CRect(0,0,0,0),NULL,WS_POPUP);
	m_pListbar = GetSonicUI()->CreateString();
	m_pListbar->Format(_T("/a,linkc=0, p4=%d/"), GetCoreUI()->GetImage(_T("IM_TAB_LIST_BTN"))->GetObjectId());
	m_pListbar->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUITabMgrDlg::OnShowList);

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("tabchat_dialog_btn_exit"))->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUITabMgrDlg::OnClickExit);
	ISonicString *pMinBtn = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("tabchat_dialog_btn_min"));
	if (pMinBtn)
	{
		pMinBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUITabMgrDlg::OnClickMin);
	}
	
	ISonicPaint * pPaint = m_pTabCtrl->GetPaint();
	pPaint->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&CUITabMgrDlg::DrawTab);
	//m_ListWnd.SetListBoxMargin(11,12);

	return TRUE;
}

LRESULT CUITabMgrDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	if (m_pMgr)
	{
      m_pMgr->RemoveTabMgrDlg(this);
	}
	return 0;
}

void CUITabMgrDlg::OnSelItemChanged(ISonicBase *, LPVOID)
{
	if (!m_pTabCtrl || !m_pTabCtrl->IsValid())
	{
		return;
	}

	int nSel = m_pTabCtrl->GetSel();
	if (nSel < 0 || nSel >= m_pTabCtrl->GetTabCount())
	{
		return;
	}

	DoFlash(nSel,false);
	if (m_nCurSelIndex != nSel)
	{
		DoFlash(m_nCurSelIndex,false);
		m_nCurSelIndex = nSel;
	}

	CUIChatDlgBase *pChat = GetChatDlgByIndex(nSel);
	if (pChat != NULL && pChat->IsWindow())
	{
		::SetWindowText(m_hWnd, pChat->GetBaseInfo()._szNick);
		if (pChat->GetInputEdit())
		{
			m_pMgr->ShowCursorItemOnInputEdit(pChat);
		}
	}
}

void CUITabMgrDlg::OnDeleteItem(ISonicBase *, LPVOID pReserve)
{
	if (m_pTabCtrl && m_pTabCtrl->GetTabCount() == 1)
	{
		PostMessage(WM_CLOSE);
	}
	CUIChatDlgBase *pChat = GetChatDlgByIndex((int)pReserve);
	if (m_mapChatFlashing.find(pChat) != m_mapChatFlashing.end())
	{
		m_mapChatFlashing.erase(pChat);
	}
	if (m_ListWnd.IsWindow())
	{
		m_ListWnd.DeleteItem((int)(pReserve));
	}
	if ((int)pReserve < m_nCurSelIndex)
	{
		--m_nCurSelIndex;
	}
	Invalidate();
}

void CUITabMgrDlg::OnClickExit(ISonicString *, LPVOID pReserve)
{
	::DestroyWindow(m_hWnd);
}

void CUITabMgrDlg::OnClickMin(ISonicString *, LPVOID pReserve)
{
	ShowWindow(SW_MINIMIZE);
}

int CUITabMgrDlg::GetChatIndexInTab(CUIChatDlgBase *pChat)
{
	if (!pChat || !pChat->IsWindow()) return -1;

	for (int nIndex = 0; nIndex < m_pTabCtrl->GetTabCount(); ++nIndex)
	{
		if (pChat->m_hWnd == m_pTabCtrl->GetTabWnd(nIndex))
		{
			return nIndex;
		}
	}

	return -1;
}

void CUITabMgrDlg::DoFlash(CUIChatDlgBase *pChat, bool bEable /*= true*/, bool bJustTaskBar /*= false*/)
{
	DoFlash(GetChatIndexInTab(pChat), bEable, bJustTaskBar);
}

 void CUITabMgrDlg::DoFlash(int nIndex, bool bEable /*= true*/, bool bJustTaskBar /*= false*/)
{
	if (!m_pTabCtrl || nIndex < 0 || nIndex >= m_pTabCtrl->GetTabCount() || !m_pMgr)
	{
		return;
	}

	CUIChatDlgBase *pChat = GetChatDlgByIndex(nIndex);
	if (!pChat || !pChat->IsWindow())
	{
		return;
	}

	FLASHWINFO flashinfo = {sizeof(FLASHWINFO)};
	flashinfo.hwnd = m_hWnd;
	flashinfo.dwTimeout = 500;

	_IM_BASE_CONFIG settings;
	m_pMgr->LoadConfig(settings);
	if (settings.bDisableFlashWindow || !bEable)
	{
		if (!bJustTaskBar)
		{
			SetTabItemFlash(nIndex, false);
		}
		flashinfo.dwFlags = FLASHW_STOP;
		flashinfo.uCount =0;
		if (::FlashWindowEx(&flashinfo))
		{
			m_mapChatFlashing[pChat] &= ~EFS_TYPE_TASKBAR;
		}
		return;
	}
	else
	{
		flashinfo.dwFlags = FLASHW_TRAY|FLASHW_TIMER;
		flashinfo.uCount = 5;

		if (bJustTaskBar)
		{
			if (::FlashWindowEx(&flashinfo))
			{
				m_mapChatFlashing[pChat] |= EFS_TYPE_TASKBAR;
			}
			return;
		}

		if ((m_pMgr->IsInGame() && m_pTabCtrl->GetSel() != nIndex) ||
			(!m_pMgr->IsInGame() && (m_pMgr->GetTabMgrDlgByHandle(GetForegroundWindow()) != this || m_pTabCtrl->GetSel() != nIndex)))
		{
			SetTabItemFlash(nIndex, true);
			if (::FlashWindowEx(&flashinfo))
			{
				m_mapChatFlashing[pChat] |= EFS_TYPE_TASKBAR;
			}
		}
	}
}

void CUITabMgrDlg::ShowListWnd(BOOL bShow)
{
	if (bShow && m_pTabCtrl && m_pTabCtrl->IsValid())
	{
		int nCount = m_pTabCtrl->GetTabCount();
		m_ListWnd.SelectItem(m_pTabCtrl->GetSel());
		m_ListWnd.SetListFocus();
		WTL::CRect rc (*(m_pListbar->GetRect()));
		ClientToScreen(rc);
		int nHeight = nCount * LISTITEMHEIGHT ;
		if (nHeight > LISTMAXHEIGHT)
		{
			nHeight = LISTMAXHEIGHT;
		}
		int nWidthMargin,nHeightMargin;
		m_ListWnd.GetListBoxMargin(nWidthMargin,nHeightMargin);
		::SetWindowPos(m_ListWnd.m_hWnd,NULL, rc.left - 70, rc.bottom, LISTWIDTH + nWidthMargin*2, nHeight + nHeightMargin*2,SWP_NOZORDER);
		//m_ListWnd.ShowWindow(SW_SHOW);
		if (m_pMgr)
		{
			m_pMgr->ShowWindow(m_ListWnd.m_hWnd,SW_SHOW);
		}
		if (m_ListWnd.GetCoreListBox() && m_ListWnd.GetCoreListBox()->IsWindow())
		{
			m_ListWnd.GetCoreListBox()->SetFocus();
		}
	}
	else if (m_ListWnd.IsWindow())
	{
		::ShowWindow(m_ListWnd.m_hWnd,SW_HIDE);
	}
}
void CUITabMgrDlg::DrawTab(ISonicPaint * pPaint, LPVOID) 
{
	if (pPaint && pPaint->IsValid())
	{
		if (m_pListbar == NULL || !m_pListbar->IsValid())
		{
			return;
		}
		CURRENT_PAINT * pCurrentPaint =  pPaint->GetCurrentPaint();
		int nWidth = m_pTabCtrl->GetVisibleTabsTotalWidth();
	
		int nLeftMargin = 5;
		if ( m_pTabCtrl->GetTabCount() > m_pTabCtrl->GetVisibleTabCount())
		{
			if (!m_pListbar->IsVisible())
			{
				m_pListbar->Show(TRUE,FALSE);
			}
			m_pListbar->TextOut(pCurrentPaint->hdc, pCurrentPaint->left + nWidth + nLeftMargin + 2, pCurrentPaint->top - 1, m_hWnd);

		}
		else
		{
			if (m_pListbar->IsVisible())
			{
				m_pListbar->Show(FALSE,FALSE);
			}
		}
	}
}
void CUITabMgrDlg::OnShowList(ISonicBase *,LPVOID)
{
	if (!m_ListWnd.IsWindow())
	{
		return;
	}

	if (m_ListWnd.IsWindowVisible())
	{
		ShowListWnd(FALSE);
	}
	else
	{
		ShowListWnd(TRUE);
	}
}
void CUITabMgrDlg::SetSelTab (int nIndex,HWND hWnd)
 {
	 if (m_pTabCtrl && m_pTabCtrl->IsValid())
	 {
		 m_pTabCtrl->SetSel(nIndex);
		 m_pTabCtrl->SetTabVisible(nIndex);
	 }
 }
void CUITabMgrDlg::OnSize(UINT nType, WTL::CSize size)
{
// 	if (m_pTabCtrl)
// 	{
// 		WTL::CRect rcClent;
// 		GetClientRect(&rcClent);
// 		m_pTabCtrl->Move(6, 36, rcClent.Width()-12, rcClent.Height() - 42, true);
// 		m_pTabCtrl->SetTabVisible(m_pTabCtrl->GetSel());
// 	}
}
void CUITabMgrDlg::AddTab(HWND hDlg, LPCTSTR lpszTabText, int nIndex)
{
	if (m_pTabCtrl && m_pTabCtrl->IsValid())
	{
		m_pTabCtrl->AddTab(hDlg,lpszTabText,nIndex);
		m_ListWnd.AddListItem(nIndex,lpszTabText);
		CUIChatDlgBase *pChat = m_pMgr->GetChatDlgByHandle(hDlg);
		if (pChat && pChat->IsWindow())
		{
			m_mapChatFlashing[pChat] = EFS_TYPE_NONE;
		}
	}
}
void CUITabMgrDlg::DeleteListItem(int nIndex)
{
	m_ListWnd.DeleteItem(nIndex);
}
/////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUITabMgrDlg::SetTabText(int nIndex, LPCTSTR lpItem)
{
	if (m_pTabCtrl && m_pTabCtrl->IsValid() && nIndex >= 0 && nIndex < m_pTabCtrl->GetTabCount())
	{
		_tstring strTextFormat;
		
		_tstring tsNickName;
		tsNickName = lpItem;

		DWORD col = (nIndex == m_pTabCtrl->GetSel()) ? 0xffffff : 0x999999;

		strTextFormat.Format(3, _T("/c=%x, align=0x1, line_width=%d, single_line=2, font, font_height=13, font_face='Open Sans'/%s"),
			col, m_nCurTabItemWidth - 45, GetSonicUI()->HandleRawString(tsNickName.c_str(), RSCT_DOUBLE));

		m_pTabCtrl->SetTabText(nIndex, strTextFormat.c_str());

		return TRUE;
	}
	return FALSE;
}

CUIChatDlgBase * CUITabMgrDlg::GetChatDlgByIndex(int nIndex)
{
	if (!m_pTabCtrl || !m_pTabCtrl->IsValid() || nIndex < 0 || nIndex >= m_pTabCtrl->GetTabCount())
	{
		return NULL;
	}

	return m_pMgr->GetChatDlgByHandle(m_pTabCtrl->GetTabWnd(nIndex));
}

BOOL CUITabMgrDlg::SetTabItemFlash(int nIndex, bool bFlashing)
{
	if (!m_pTabCtrl || !m_pTabCtrl->IsValid())
	{
		return FALSE;
	}

	LPCTSTR lpItemText = m_pTabCtrl->GetTabText(nIndex);
	if (NULL == lpItemText)
	{
		return FALSE;
	}

	_tstring strTextFormat;

	DWORD dwCol = (nIndex == m_pTabCtrl->GetSel()) ? 0xffffff : 0x999999;
	LPCTSTR lpcItemText = GetSonicUI()->HandleRawString(lpItemText, RSCT_DOUBLE);
	int nLineWidth = m_nCurTabItemWidth-45;
	if (bFlashing)
	{
		strTextFormat.Format(4, _T("/c=%x, line_width=%d, single_line=2, font, font_height=13, font_face='Open Sans', sparkle, sparkle_color=%x|0x00a7e0, sparkle_interval=300/%s"), 
			dwCol, nLineWidth, dwCol, lpcItemText);
	}
	else
	{
		strTextFormat.Format(3, _T("/c=%x, line_width=%d, single_line=2, font, font_height=13, font_face='Open Sans'/%s"),
			dwCol, nLineWidth, lpcItemText);
	}

	m_pTabCtrl->SetTabText(nIndex, strTextFormat.c_str());
	CUIChatDlgBase *pChat = GetChatDlgByIndex(nIndex);
	if (pChat && pChat->IsWindow())
	{
		m_mapChatFlashing[pChat] = bFlashing ? 
			(m_mapChatFlashing[pChat] | EFS_TYPE_TABITEM) :
			(m_mapChatFlashing[pChat] & ~EFS_TYPE_TABITEM);
	}
	return TRUE;
}

void CUITabMgrDlg::ResizeTabItems()
{
	//tab verify
	if (!m_pTabCtrl || !m_pTabCtrl->IsValid())
	{
		return;
	}

	//get count
	int nCount = m_pTabCtrl->GetTabCount();
	if (nCount <= 0)
	{
		return;
	}

	//get default width
	int nMaxAvailableWidth = m_pTabCtrl->GetWidth() - 40;
	int nEachItemWidth = nMaxAvailableWidth / nCount;

	//get the best match width for each table
	do 
	{
		//The following condition means the available width of the table can hold all of the items
		if (nEachItemWidth > CHAT_TAB_MAX_LIMIT)
		{
			nEachItemWidth = CHAT_TAB_MAX_LIMIT;
			break;
		}

		//The following condition means the number of the items is too much so that if we force to hold all of the items, 
		//every item just has only a little width for displaying the item-text
		while (nEachItemWidth < CHAT_TAB_MIN_LIMIT)
		{
			if (--nCount <= 0)
			{
				//error (the tab width is so little that can's hold only one item)
				return;
			}
			nEachItemWidth = nMaxAvailableWidth / nCount;
		}

		if (nEachItemWidth > CHAT_TAB_MAX_LIMIT)
		{
			//The code will never get here, because ( 84*(x-1) < 180*x ) can't be set up 
			//when the "x" which means the visible count is a integer larger than 0,
			nEachItemWidth = CHAT_TAB_MAX_LIMIT;
		}

	} while (false);

	m_nCurTabItemWidth = nEachItemWidth;
	m_nCurSelIndex = m_pTabCtrl->GetSel();
	for (int i = 0; i < m_pTabCtrl->GetTabCount(); ++i)
	{
		m_pTabCtrl->SetTabWidth(i, nEachItemWidth);
		CUIChatDlgBase *pChat = GetChatDlgByIndex(i);
		if (pChat && pChat->IsWindow())
		{
			SetTabText(i, pChat->GetBaseInfo()._szNick);
			if (m_mapChatFlashing.find(pChat) != m_mapChatFlashing.end() && m_mapChatFlashing[pChat] == EFS_TYPE_ALL)
			{
				SetTabItemFlash(i, true);
			}
		}
	}
}

void CUITabMgrDlg::ReorderListWndItems()
{
	if (!m_pTabCtrl || !m_pTabCtrl->IsValid()) return;

	if (!m_ListWnd.IsWindow()) return;

	m_ListWnd.Clear();

	for (int i = 0; i < m_pTabCtrl->GetTabCount(); ++i)
	{
		LPCTSTR lpItemName = NULL;
		CUIChatDlgBase *pChat = GetChatDlgByIndex(i);
		if (pChat && pChat->IsWindow())
		{
			lpItemName = pChat->GetBaseInfo()._szNick;
		}
		m_ListWnd.AddListItem(-1, lpItemName);
	}
}

LRESULT CUITabMgrDlg::OnResizeTabItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_TABMGR_RESIZE)
	{
		ResizeTabItems();
	}
	return 0L;
}

LRESULT CUITabMgrDlg::OnShowSel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (m_pMgr)
	{
		m_pMgr->ShowTabSel((CUIChatDlgBase*)wParam);
	}
	return 0L;
}

LRESULT CUITabMgrDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return DestroyWindow();
}

bool CUITabMgrDlg::IsChatFlashing(CUIChatDlgBase *pChat)
{
	if (m_mapChatFlashing.find(pChat) != m_mapChatFlashing.end())
	{
		return m_mapChatFlashing[pChat] != EFS_TYPE_NONE;
	}
	return false;
}

LRESULT CUITabMgrDlg::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = false;

	if (m_pTabCtrl == NULL || !m_pTabCtrl->IsValid())
	{
		return 1L;
	}

	if (::GetForegroundWindow() == m_hWnd)
	{
		SetTabItemFlash(m_pTabCtrl->GetSel(), false);
		CUIChatDlgBase *pChat = GetChatDlgByIndex(m_pTabCtrl->GetSel());
		if (pChat && pChat->GetInputEdit())
		{
			pChat->GetInputEdit()->SetFocus();
		}
	}
	return 1L;
}

 /////////////////////////////////////////////////////////////////////////////////////////////
//IM main dialog begin			

CUIFriendDlgBase::CUIFriendDlgBase(CUIChatMgrBase* pMgr,CWindow* pParent)
:m_pSearchBar(NULL)
,m_pSearchQuit(NULL)
,m_pSearchFailure(NULL)
,m_pTree(NULL)
,m_pOnLineString(NULL)
,m_pOffLineString(NULL)
,m_nFLItemNickNameX(52)
,m_nFLItemNickNameY(12)
,m_nFLItemAvatarX(16)
,m_nFLItemAvatarY(12)
,m_nFLItemStatusImageX(10)
,m_nFLItemStatusImageY(6)
,m_uDefaultItemHeight(44)
,m_nSearchCount(0)
,m_nOnLineVisibleItemNum(0)
,m_nOffLineVisibleItemNum(0)
,m_bInitFriendListFinished(false)
,m_bFirstSearch(true)
,m_bDoSearch(false)
,m_bSearchBarFocused(false)
,m_bExitWorkThread(false)
,m_bForcing(true)
,m_tsSearchDefaultText(_T(""))
,m_tsLastSearchText(_T(""))
,m_nCurPressence(ArcPresenceType::Unavailable)
{
	m_pUIChatMgr = pMgr;
	m_pTree = GetSonicUI()->CreateObject<ISonicTreeView>();
	m_pSearchQuit = GetSonicUI()->CreateString();
	m_pSearchFailure = GetSonicUI()->CreateString();

	m_pDefaultAvatar = GetSonicUI()->CreateImage();

	_tstring tsPath = m_pUIChatMgr->GetBaseDir();
	tsPath += DEFAULT_AVATAR_PATH;

	m_pDefaultAvatar->Load(tsPath.c_str());
}

CUIFriendDlgBase:: ~CUIFriendDlgBase()
{
	SONICUI_DESTROY(m_pTree);
	SONICUI_DESTROY(m_pSearchQuit);
	SONICUI_DESTROY(m_pSearchFailure);
	SONICUI_DESTROY(m_pDefaultAvatar);

	_MapFriendImgItr itr = m_mapAvatar.begin();
	for (; itr != m_mapAvatar.end(); ++itr)
	{
		SONICUI_DESTROY(itr->second);
	}

	if (_threadHandle != NULL && _threadId != NULL)
	{
		stop();
	}
}

int CUIFriendDlgBase::EndInit()
{
	m_pUIChatMgr->InitIMMessageWnd();
	RECT rc;
	GetClientRect(&rc);
	if (m_pTree->IsValid())
	{
		ISonicImage *pFriendItemBKImage = GetCoreUI()->GetImage(_T("IM_FRIEND_LIST_SELECT_IMAGE"));
		m_pTree->SetSelectBackgroundImage(pFriendItemBKImage);

		ISonicAnimation *pOnLineAni = m_pTree->AddItem(29);
		ISonicAnimation *pOffLineAni = m_pTree->AddItem(29);
		ISonicImage *pFriendHeaderBKImage = GetCoreUI()->GetImage(_T("IM_FRIEND_LIST_HEADER_ITEM"));
		ISonicImage *pExpandImage = GetCoreUI()->GetImage(_T("IM_FRIEND_LIST_EXPAND_BTN"));

		ISonicString *pExpandOnLineString = GetSonicUI()->CreateString();
		pExpandOnLineString->Format(_T("/p=%d/"), pExpandImage);
		pExpandOnLineString->SetAttr(SATTR_TREE_EXPANDSIGN, 1);

		ISonicString *pExpandOffLineString = GetSonicUI()->CreateString();
		pExpandOffLineString->Format(_T("/p=%d/"), pExpandImage);
		pExpandOffLineString->SetAttr(SATTR_TREE_EXPANDSIGN, 1);

		m_pOnLineString = GetSonicUI()->CreateString();
		m_pOffLineString = GetSonicUI()->CreateString();
		UpdateFriendListHeaderNum(true);
		UpdateFriendListHeaderNum(false);

		pOnLineAni->SetAttr(SATTR_TREE_ITEMSELECTIMAGE, pFriendHeaderBKImage);
		pOnLineAni->SetAttr(SATTR_TREE_ITEMEXPAND, 1);
		pOnLineAni->AddObject(pExpandOnLineString->GetObjectId(), m_nFLItemAvatarX, 10, TRUE);
		pOnLineAni->AddObject(m_pOnLineString->GetObjectId(), 32, 5, TRUE);

		pOffLineAni->SetAttr(SATTR_TREE_ITEMSELECTIMAGE, pFriendHeaderBKImage);
		pOffLineAni->SetAttr(SATTR_TREE_ITEMEXPAND, 1);
		pOffLineAni->AddObject(pExpandOffLineString->GetObjectId(), m_nFLItemAvatarX, 10, TRUE);
		pOffLineAni->AddObject(m_pOffLineString->GetObjectId(), 32, 5, NULL);

		m_pTree->ShowItem(pOnLineAni, FALSE);
		m_pTree->ShowItem(pOffLineAni, FALSE);
		m_pTree->Show(FALSE);

	}

	if (m_pSearchQuit && m_pSearchQuit->IsValid())
	{
		m_pSearchQuit->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIFriendDlgBase::OnClickBtnSearchQuit);
		return 1;
	}

	return 0;
}

void CUIFriendDlgBase::OnClickBtnSearchQuit(ISonicString *, LPVOID pReserve)
{
	/*
	 *@ verify variables
	 */
	if (!m_bDoSearch || !m_pSearchBar || !m_pSearchQuit)
		return;

	/*
	 *@ reset variables and control status
	 *@ here not need reset the m_tsLastSearchText
	 *@ because m_pSearchBar->SetWindowText(...) will reset it
	 */
	m_bDoSearch=false;
	m_bForcing = TRUE;
	m_pSearchFailure->Show(FALSE);
	m_pSearchQuit->Show(FALSE);
	this->SetFocus();
	m_pSearchBar->SetEditFont(0);
	m_pSearchBar->SetWindowText(m_tsSearchDefaultText.c_str());

	/*
	 *@ verify variables
	 */
	if (m_tsLastSearchText.size() > 0)
	{
		StartSearch();
		DetectionFriends();
	}
}

void CUIFriendDlgBase::OnAddFriendClick(ISonicString *, LPVOID pReserve)
{
	AddFriendClick(NULL, NULL);
}

LRESULT CUIFriendDlgBase::OnEnChangeRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	/*
	 *@ check if searching is allowed
	 */
	if(!m_bSearchBarFocused)
		return 0L;

	/*
	 *@ get the search text
	 */
	ATL::CString  sText ;
	m_pSearchBar->GetWindowText(sText);

	/*
	 *@ do nothing if the search text is not change
	 */
	if (m_tsLastSearchText.CompareNoCase(sText.GetString()) == 0)
		return 0L;

	/*
	 *@ get the number of the tree-items at the first time when begin search
	 */
	if (m_bFirstSearch)
	{
		m_nSearchCount = m_pUIChatMgr->GetFriendsCount();
		m_bFirstSearch = false;
	}
	
	/*
	 *@ set search timer
	 */
	if (m_nSearchCount < 100)
	{
		DoSearch();
		m_bFirstSearch = true;
	}
	else if (m_nSearchCount < 2000)
	{
		::KillTimer(m_hWnd, CUS_SEARCH_TIMER);
		::SetTimer(m_hWnd, CUS_SEARCH_TIMER, CUS_SEARCH_TIMER_ELAPSE_BASE, NULL);
	}
	else
	{
		::KillTimer(m_hWnd, CUS_SEARCH_TIMER);
		::SetTimer(m_hWnd, CUS_SEARCH_TIMER, CUS_SEARCH_TIMER_ELAPSE_BASE*2, NULL);
	}

	/*
	 *@ save the last search text
	 */
	m_tsLastSearchText = sText.GetString();

	/*
	 *@ return
	 */
	return 1L;
}

LRESULT CUIFriendDlgBase::OnEnSetfocusRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	/*
	 *@ if the search-bar is focus, not do any thing
	 */
	if (m_bSearchBarFocused)
		return 0L;

	/*
	 *@ set the focus value true if search-bar is not focus
	 */
	m_bSearchBarFocused =true;

	/*
	 *@ init the search-bar when the search-barbecome focus
	 */
	if (m_pSearchBar && m_bForcing)
	{
		m_pSearchBar->SetEditFont(1);
		m_pSearchBar->SetWindowText(_T(""));
		m_bForcing =FALSE;
	}

	/*
	 *@ return
	 */
	return 0L;
}

LRESULT CUIFriendDlgBase::OnEnKillfocusRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	/*
	 *@ kill focus
	 */
	m_bSearchBarFocused = false;

	/*
	 *@ if the search-text is null, refresh the UI by call the OnClickBtnSearchQuit function
	 */
	if (m_pSearchBar && m_pSearchBar->GetWindowTextLength() == 0)
	{
		m_bForcing = TRUE;
		m_pSearchFailure->Show(FALSE);
		m_pSearchQuit->Show(FALSE);
		this->SetFocus();
		m_pSearchBar->SetEditFont(0);
		m_pSearchBar->SetWindowText(m_tsSearchDefaultText.c_str());
	}

	/*
	 *@ return
	 */
	return 0L;
}

LRESULT CUIFriendDlgBase::OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;
	}
	switch(pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_PRESENCE_CHANGED:
		{
			//Notify relative UI update.
			im_msg_presence_changed *pCmd = (im_msg_presence_changed*)pHeader;
			if (pCmd)
			{
				if (_tcscmp(m_pUIChatMgr->GetUserName(), pCmd->szUserName) == 0)
				{//my status changed
					HandleSelfPresence((ArcPresenceType)(pCmd->iType));
				}
				else
				{//one of my friend's status changed
					_tstring tsUserName = pCmd->szUserName;
					m_wqPresenceFriends.enter(tsUserName);
				}
			}
		}
		break;
	case  ARC_CHAT_MSG_AVATAR_CHANGED:
		{
			//Notify relative UI update.
			im_msg_avatar_changed *pCmd = (im_msg_avatar_changed*)pHeader;
			if (pCmd)
			{
				if (_tcscmp(m_pUIChatMgr->GetUserName(), pCmd->szUserName) == 0)
				{//my avatar changed
					HandleSelfAvatar(pCmd->szPath);
				}
				else
				{//one of my friend's avatar changed
					_tstring tsUserName = pCmd->szUserName;
					m_wqAvatarFriends.enter(tsUserName);
				}
			}
		}
		break;
	case ARC_CHAT_MSG_FRIEND_DIVORCE:
		{
			//Notify relative UI update
			im_msg_friend_divorce *pCmd = (im_msg_friend_divorce*)pHeader;
			if (pCmd)
			{
				DeleteFriendItem(pCmd->szNickName);
				m_nOnLineVisibleItemNum = m_pTree->GetChildItemCount(m_pTree->GetItem(0), TRUE);
				m_nOffLineVisibleItemNum = m_pTree->GetChildItemCount(m_pTree->GetItem(1), TRUE);
				UpdateFriendListHeaderNum(true);
				UpdateFriendListHeaderNum(false);
				DetectionFriends();
				m_pUIChatMgr->HandleUserOffline(pCmd->szUserName);
			}
		}
		break;
	default:
		break;
	}
	return 0L;
}

bool CUIFriendDlgBase::InsertFriends( CHAT_FRIEND* friends, int nLen, LPCTSTR lpcSearch /*= NULL*/ )
{
	if (nLen <= 0 || !m_pUIChatMgr || !m_pTree) 
		return false;

	_tstring tsBaseDir = m_pUIChatMgr->GetBaseDir();

	int nOfflineNum = 0;
	int nNum = 0;

	std::vector<CHAT_FRIEND*> vecFriend;
	for (; nNum < nLen; ++nNum)
	{
		if (friends[nNum].nPresence == ArcPresenceType::Available)
		{
			vecFriend.push_back(&friends[nNum]);
		}
	}
	nNum = 0;

	for (; nNum < nLen; ++nNum)
	{
		if (friends[nNum].nPresence == ArcPresenceType::DND)
		{
			vecFriend.push_back(&friends[nNum]);
		}
	}
	nNum = 0;

	for (; nNum < nLen; ++nNum)
	{
		if (friends[nNum].nPresence == ArcPresenceType::Unavailable)
		{
			vecFriend.push_back(&friends[nNum]);
			++nOfflineNum;
		}
	}
	nNum = 0;

	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 1);

	for (; nNum < nLen; ++nNum)
	{
		/*
		 *@ init chat friend;
		 */
		CHAT_FRIEND& cf = (CHAT_FRIEND)(*vecFriend[nNum]);

		/*
		 *@ verify chat friend;
		 */
		if (NULL == cf.szNick || NULL == cf.szUserName || 
			_tcslen(cf.szNick) <= 0 || _tcslen(cf.szUserName) <= 0) 
		{
			return false;
		}

		/*
		 *@ insert the friend item to the tree;
		 */
		/**************************************/
		//init tree item
		ISonicAnimation *pParentItem = (cf.nPresence < ArcPresenceType::XA) ? m_pTree->GetItem(0) : m_pTree->GetItem(1);
		ISonicAnimation *pNewItem = m_pTree->AddItem(m_uDefaultItemHeight, -1, pParentItem);

		/**************************************/
		//insert header background image
		ISonicString *pImage = m_pUIChatMgr->GetStatusBkImg(cf.nPresence, false);
		if (pImage)
			pNewItem->AddObject(pImage->GetObjectId(), m_nFLItemStatusImageX, m_nFLItemStatusImageY, TRUE);

		/**************************************/
		//insert nick name string
		_tstring tsNick = cf.szNick;
		tsNick.Replace(_T("/"), _T("//"));//translate the control char to avoid conflict.
		ISonicString* pNickName = GetSonicUI()->CreateString();
		if (pNickName && pNickName->Format(m_tsFLItemNickNameFormatType.c_str(), m_colFLItemNickNameColor, tsNick.c_str()))
			pNewItem->AddObject(pNickName->GetObjectId(), m_nFLItemNickNameX, m_nFLItemNickNameY, TRUE);

		/**************************************/
		//insert avatar image
		ISonicImage *pAvatarImage = m_pDefaultAvatar;
		_MapFriendImgItr itr = m_mapAvatar.find(cf.szUserName);
		if (itr != m_mapAvatar.end())
			pAvatarImage = itr->second;

		ISonicString* pAvatarString = GetSonicUI()->CreateString();
		if (pAvatarString && pAvatarString->Format(m_tsFLItemAvatarFormatType.c_str(), pAvatarImage->GetObjectId()))
		{
			_tstring tsAvatarStringName = TREEITEM_AVATAR_NAME + _tstring(cf.szUserName);
			pAvatarString->SetName(tsAvatarStringName.c_str());
			pNewItem->AddObject(pAvatarString->GetObjectId(), m_nFLItemAvatarX, m_nFLItemAvatarY, TRUE);
		}
		else
		{
			SONICUI_DESTROY(pAvatarString);
		}

		/**************************************/
		//set item attribute data and set the delegate 
		pNewItem->SetName(cf.szNick);
		pNewItem->Delegate(DELEGATE_EVENT_DBLCLICK, NULL, this, &CUIFriendDlgBase::OnDBLClickFriendItem);

		if (lpcSearch != NULL && _tstring(cf.szNick).Find(lpcSearch) != 0)
		{
			m_pTree->ShowItem(pNewItem, FALSE);
		}
		else
		{
			if (cf.nPresence < ArcPresenceType::XA)
			{
				++m_nOnLineVisibleItemNum;
			}
			else
			{
				++m_nOffLineVisibleItemNum;
			}
		}
	}

	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 0);

	/*
	 *@ refresh the detection button in overlay;
	 */
	DetectionFriends();

	return true;
}

bool CUIFriendDlgBase::InsertFriendItemEx(ITEM_BASE_INFO &info, bool bSearch, unsigned int uItemHeight)
{
	/*
	 *@ chat manager
	 */
	if (!m_pUIChatMgr || !m_pTree) 
		return false;

	/*
	 *@ verify nick name
	 */
	if(_tstring(info._szNick).empty() || _tstring(info._szUser).empty())
		return false;

	/*
	 *@ if exist already ,no need insert
	 */
	if (NULL != GetSonicUI()->GetObjectByName(info._szNick)) 
		return false;

	/*
	 *@ get the index sorted by the user name and status
	 */
	int nIndex = GetFriendItemSortedIndexByUserName(info._szUser, info.iType);
	if (LB_ERR == nIndex) 
		return false;

	/*
	 *@ add the user item to the tree
	 */
	unsigned int uHeight = (uItemHeight != 0) ? uItemHeight : m_uDefaultItemHeight;
	ISonicAnimation *pParentItem = (info.iType < ArcPresenceType::XA) ? m_pTree->GetItem(0) : m_pTree->GetItem(1);
	ISonicAnimation *pNewItem = m_pTree->AddItem(uHeight, nIndex, pParentItem);

	/**************************************/
	//insert header background image
	ISonicString *pImage = m_pUIChatMgr->GetStatusBkImg(info.iType, false);
	if (pImage)
	{
		pNewItem->AddObject(pImage->GetObjectId(), m_nFLItemStatusImageX, m_nFLItemStatusImageY, TRUE);
	}

	/**************************************/
	//insert nick name string
	_tstring tsNick = info._szNick;
	tsNick.Replace(_T("/"), _T("//"));//translate the control char to avoid conflict.
	ISonicString* pNickName = GetSonicUI()->CreateString();
	if (pNickName && pNickName->Format(m_tsFLItemNickNameFormatType.c_str(), m_colFLItemNickNameColor, tsNick.c_str()))
	{
		pNewItem->AddObject(pNickName->GetObjectId(), m_nFLItemNickNameX, m_nFLItemNickNameY, TRUE);
	}

	/**************************************/
	//insert avatar image
	ISonicImage *pAvatarImage = m_pDefaultAvatar;
	_MapFriendImgItr itr = m_mapAvatar.find(info._szUser);
	if (itr != m_mapAvatar.end())
		pAvatarImage = itr->second;

	ISonicString* pAvatarString = GetSonicUI()->CreateString();
	if (pAvatarString && pAvatarString->Format(m_tsFLItemAvatarFormatType.c_str(), pAvatarImage->GetObjectId()))
	{
		_tstring tsAvatarStringName = TREEITEM_AVATAR_NAME + _tstring(info._szUser);
		pAvatarString->SetName(tsAvatarStringName.c_str());
		pNewItem->AddObject(pAvatarString->GetObjectId(), m_nFLItemAvatarX, m_nFLItemAvatarY, TRUE);
	}
	else
	{
		SONICUI_DESTROY(pAvatarString);
	}

	/**************************************/
	//set item attribute data and set the delegate 
	pNewItem->SetName(info._szNick);
	pNewItem->Delegate(DELEGATE_EVENT_DBLCLICK, NULL, this, &CUIFriendDlgBase::OnDBLClickFriendItem);

	/*
	 *@ when do search,can not show the new item
	 */
	if (!bSearch && m_bDoSearch) 
		m_pTree->ShowItem(pNewItem, FALSE);

	/*
	 *@ show the parent item 
	 */
	int nParentIndex = info.iType < ArcPresenceType::XA ? 0 : 1;
	if (!m_pTree->GetItem(nParentIndex)->IsVisible())
		m_pTree->ShowItem(m_pTree->GetItem(nParentIndex), TRUE);
	
	DetectionFriends();

	return true;
}

ISonicAnimation * CUIFriendDlgBase::GetFriendItemByUserName( _tstring tsUserName )
{
	if (tsUserName.size() <= 0 || !m_pTree || !m_pTree->IsValid())
		return NULL;

	ISonicAnimation *pParentItem = m_pTree->GetItem(0);
	ISonicAnimation *pAni = m_pTree->GetFirstChildItem(pParentItem);

	while (pAni != NULL && pAni->IsValid())
	{
		/*
		 *@ verify the control name
		 */
		if (pAni->GetName() == NULL) 
			return NULL;

		/*
		 *@ if the user name is equal to the control name, return the index
		 */
		if ( (_tstring(pAni->GetName())).CompareNoCase(tsUserName) == 0 )
			return pAni;

		/*
		 *@ increasing the control and the index
		 */
		pAni = m_pTree->GetNextSiblingItem(pAni);
	}

	pParentItem = m_pTree->GetItem(1);
	pAni = m_pTree->GetFirstChildItem(pParentItem);

	while (pAni != NULL && pAni->IsValid())
	{
		/*
		 *@ verify the control name
		 */
		if (pAni->GetName() == NULL) 
			return NULL;

		/*
		 *@ if the user name is equal to the control name, return the index
		 */
		if ( (_tstring(pAni->GetName())).CompareNoCase(tsUserName) == 0 )
			return pAni;

		/*
		 *@ increasing the control and the index
		 */
		pAni = m_pTree->GetNextSiblingItem(pAni);
	}

	return NULL;
}

int CUIFriendDlgBase::GetFriendItemIndexByUserName(_tstring tsUserName, bool bOnLine)
{
	if (tsUserName.size() <= 0 || !m_pTree || !m_pTree->IsValid())
		return LB_ERR;

	ISonicAnimation *pParentItem = m_pTree->GetItem(bOnLine ? 0 : 1);
	ISonicAnimation *pAni = m_pTree->GetFirstChildItem(pParentItem);

	int nIndex = 0;
	while (pAni != NULL && pAni->IsValid())
	{
		/*
		 *@ verify the control name
		 */
		if (pAni->GetName() == NULL) 
			return LB_ERR;

		/*
		 *@ if the user name is equal to the control name, return the index
		 */
		if ( (_tstring(pAni->GetName())).CompareNoCase(tsUserName) == 0 )
			return nIndex;

		/*
		 *@ increasing the control and the index
		 */
		pAni = m_pTree->GetNextSiblingItem(pAni);
		++nIndex;
	}

	//return error
	return LB_ERR;
}

int CUIFriendDlgBase::GetFriendItemSortedIndexByUserName(_tstring tsUserName, int iStatus)
{
	if (tsUserName.size() <= 0 || !m_pTree || !m_pTree->IsValid()) 
		return LB_ERR;

	ISonicAnimation *pParentItem = m_pTree->GetItem(iStatus < ArcPresenceType::XA ? 0 : 1);
	ISonicAnimation *pAni = m_pTree->GetFirstChildItem(pParentItem);
	
	int nIndex = 0;
	while (pAni != NULL && pAni->IsValid())
	{
		/*
		 *@ verify the control name
		 */
		if (pAni->GetName() == NULL) 
			return LB_ERR;

		/*
		 *@ verify the control status
		 */
		int nAniStatus = m_pUIChatMgr->GetFriendPresence(pAni->GetName());
		if (nAniStatus == -1)
		{
			pAni = m_pTree->GetNextSiblingItem(pAni);
			++nIndex;
			continue;
		}

		/*
		 *@ if the user's status is less then the control's status, return the index
		 *@ else if the user's status is lager then the control's status, return error
		 */
		if (iStatus < nAniStatus)
		{
			return nIndex;
		}
		else if (iStatus > nAniStatus)
		{
			pAni = m_pTree->GetNextSiblingItem(pAni);
			++nIndex;
			continue;
		}

		/*
		 *@ when the process be here,it means user's status is equal to the control's status
		 *@ so, if the user's name is less then the control's name, return the index
		 */
		if ( (_tstring(pAni->GetName())).CompareNoCase(tsUserName) > 0 )
			return nIndex;

		/*
		 *@ increasing the control and the index
		 */
		pAni = m_pTree->GetNextSiblingItem(pAni);
		++nIndex;
	}
	return nIndex;
}

void CUIFriendDlgBase::DeleteFriendItem( LPCTSTR lpcNickName )
{
	ISonicAnimation *pAni = dynamic_cast<ISonicAnimation *>(GetSonicUI()->GetObjectByName(lpcNickName));
	if (pAni)
	{
		m_pTree->DeleteItem(pAni);
	}
}

void CUIFriendDlgBase::UpdateFriendPresence(_tstring tsUserName,int iStatus)
{
	if (m_nCurPressence >= ArcPresenceType::XA)
		return;

	/*
	 *@ get the user base info
	 */
	ITEM_BASE_INFO info;
	if (!m_pUIChatMgr->GetFriendInfo(tsUserName, &info))
		return;

	/****************************************************************************************/
	/*
	 *@ set no update attr
	 */
	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 1);

	/*
	 *@ delete the friend item by the user name
	 */
	DeleteFriendItem(info._szNick);

	/*
	 *@ insert item
	 */
	InsertFriendItemEx(info, CheckItemSatisfySearchCondtion(info._szNick));

	/*
	 *@ update item num
	 */
	m_nOnLineVisibleItemNum = m_pTree->GetChildItemCount(m_pTree->GetItem(0), TRUE);
	m_nOffLineVisibleItemNum = m_pTree->GetChildItemCount(m_pTree->GetItem(1), TRUE);
	UpdateFriendListHeaderNum(true);
	UpdateFriendListHeaderNum(false);

	/*
	 *@ update the tree
	 */
	m_pTree->UpdateLayout();

	/*
	 *@ release no update attr
	 */
	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 0);

	/****************************************************************************************/
}

void CUIFriendDlgBase::UpdateFriendAvatar( _tstring tsUserName, LPCTSTR lpPath )
{
	/*
	 *@ get the user base info
	 */
	ITEM_BASE_INFO info;
	if (!m_pUIChatMgr->GetFriendInfo(tsUserName, &info))
		return;

	/*
	 *@ init avatar image path
	 */
	_tstring tsPath = m_pUIChatMgr->GetBaseDir();
	tsPath += info._szAvatarPath;

	/*
	 *@ destroy the avatar image
	 */
	_MapFriendImgItr itr = m_mapAvatar.find(tsUserName);
	if (itr != m_mapAvatar.end())
	{
		itr->second->Load(tsPath.c_str());
	}
	else
	{
		ISonicImage *pAvatarImage = GetSonicUI()->CreateImage();
		pAvatarImage->Load(tsPath.c_str());
		m_mapAvatar[tsUserName] = pAvatarImage;
	}

	if (m_nCurPressence >= ArcPresenceType::XA)
	{
		if (GetFriendItemCount() > 0)
		{
			ClearAllItems(true);
		}
		return;
	}

	/*
	 *@ init the name of the avatar-string control
	 */
	_tstring tsAvatarStringName = TREEITEM_AVATAR_NAME + tsUserName;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 1);

	/*
	 *@ get the item from the tree
	 */
	ISonicAnimation *pAni = dynamic_cast<ISonicAnimation *>(GetSonicUI()->GetObjectByName(info._szNick));
	if (pAni)
	{//exist

		ISonicString *pAvatarString = dynamic_cast<ISonicString *>(GetSonicUI()->GetObjectByName(tsAvatarStringName.c_str()));
		if (pAvatarString)
		{
			pAvatarString->Format(m_tsFLItemAvatarFormatType.c_str(), m_mapAvatar[tsUserName]->GetObjectId());
		}
		else
		{
			pAvatarString = GetSonicUI()->CreateString();
			if (pAvatarString->Format(m_tsFLItemAvatarFormatType.c_str(), m_mapAvatar[tsUserName]->GetObjectId()))
			{
				pAvatarString->SetName(tsAvatarStringName.c_str());
				pAni->AddObject(pAvatarString->GetObjectId(), m_nFLItemAvatarX, m_nFLItemAvatarY, TRUE);
			}
			else
			{
				SONICUI_DESTROY(pAvatarString);
			}
		}
	}
	else
	{
		InsertFriendItemEx(info, CheckItemSatisfySearchCondtion(info._szNick));
	}

	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 0);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void CUIFriendDlgBase::ClearAllItems(bool bClearFriendNum /*= true*/)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 1);

	if (GetFriendItemCount() > 0)
	{
		/*
		 *@ init the parent items
		 */
		ISonicAnimation *pOnLineItem = m_pTree->GetItem(0);
		ISonicAnimation *pOffLineItem = m_pTree->GetItem(1);
		ISonicAnimation *pFirstItem = NULL;
		ISonicAnimation *pNextItem = NULL;

		/*
		 *@ delete all of the children of on-line item
		 */
		pFirstItem = m_pTree->GetFirstChildItem(pOnLineItem);
		while (pNextItem = m_pTree->GetNextSiblingItem(pFirstItem)) {
			m_pTree->DeleteItem(pNextItem);
		}
		m_pTree->DeleteItem(pFirstItem);

		/*
		 *@ delete all of the children of off-line item
		 */
		pFirstItem = m_pTree->GetFirstChildItem(pOffLineItem);
		while (pNextItem = m_pTree->GetNextSiblingItem(pFirstItem)) {
			m_pTree->DeleteItem(pNextItem);
		}
		m_pTree->DeleteItem(pFirstItem);

		/*
		 *@ refresh the num of online node and offline node
		 */
		if (bClearFriendNum)
		{
			m_nOnLineVisibleItemNum = 0;
			m_nOffLineVisibleItemNum = 0;
			UpdateFriendListHeaderNum(true);
			UpdateFriendListHeaderNum(false);
		}
	}
	
	m_pTree->SetAttr(SATTR_TREE_NOAUTO_UPDATELAYOUT, 0);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

bool CUIFriendDlgBase::CheckItemSatisfySearchCondtion(_tstring tsNickName)
{
	ATL::CString  sText ;
	m_pSearchBar->GetWindowText(sText);
	if(sText.GetLength()==0|| (0==tsNickName.Find(sText.GetString())))
	{
		return true;
	}
	return false;
}

void CUIFriendDlgBase::DoSearch(BOOL bForcing)
{
	if (m_nCurPressence >= ArcPresenceType::XA)
		return;

	if (!m_pSearchQuit || !m_pSearchBar)
		return;
	
	ATL::CString sText;
	m_pSearchBar->GetWindowText(sText);

	if(0 == sText.GetLength() || m_bForcing)
	{
		m_bDoSearch = false;
		m_pSearchQuit->Show(FALSE);

		StartSearch();
	}
	else
	{
		m_bDoSearch=true;
		m_pSearchQuit->Show(TRUE);

		StartSearch(sText.GetString());
	}
}

void CUIFriendDlgBase::KillLoginTimer()
{
	::KillTimer(m_hWnd, CSU_LOGIN_TIMER);
}

LRESULT CUIFriendDlgBase::OnShowChat(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	_ITEM_BASE_INFO *pInfo = (_ITEM_BASE_INFO*)wParam;
	if (pInfo != NULL)
	{
		m_pUIChatMgr->ShowChatDlg(*pInfo, (BOOL)lParam, false);
	}
	return 0L;
}

LRESULT CUIFriendDlgBase::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	this->ShowWindow(SW_HIDE);
	return 0L;
}

LRESULT CUIFriendDlgBase::OnInitFriendList( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	if (IsInitFriendListFinished())
		return 0L;

	::KillTimer(m_hWnd, CSU_LOGIN_TIMER);
	::SetTimer(m_hWnd, CSU_LOGIN_TIMER, CSU_LOGIN_TIMER_ELAPSE, NULL);
	return 0L;
}

LRESULT CUIFriendDlgBase::OnInitFriendListInOverlay( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	//if the arc client has been set off-line status, do not call the InitFriendList function
	if (m_pUIChatMgr->GetSefPresence() < (int)ArcPresenceType::XA)
		InitFriendList();
	
	return 0L;
}

LRESULT CUIFriendDlgBase::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	if ((UINT_PTR)wParam == CSU_LOGIN_TIMER)
	{
		::KillTimer(m_hWnd, CSU_LOGIN_TIMER);
		InitFriendList();
	}
	else if ((UINT_PTR)wParam == CUS_SEARCH_TIMER)
	{
		::KillTimer(m_hWnd, CUS_SEARCH_TIMER);
		DoSearch();
		m_bFirstSearch = true;
	}

	return 0L;
}

void CUIFriendDlgBase::OnDBLClickFriendItem( ISonicAnimation *pItem, LPVOID pReserve )
{
	if (!m_pUIChatMgr) 
		return;

	if (!m_pTree || !m_pTree->IsValid())
		return;

	if (!pItem || !pItem->IsValid())
		return;

	if (NULL == pItem->GetName())
		return;
	 

	ITEM_BASE_INFO info;
	if (!m_pUIChatMgr->GetFriendInfo(pItem->GetName(), &info, true))
		return;

	m_pUIChatMgr->ShowChatDlg(info, FALSE, true);//use the item relative data directly.
}

bool CUIFriendDlgBase::IsAvatarExist( _tstring tsUserName )
{
	if (m_mapAvatar.find(tsUserName) != m_mapAvatar.end())
	{
		if (m_mapAvatar[tsUserName] != NULL)
		{
			return true;
		}
	}
	return false;
}

bool CUIFriendDlgBase::InsertAvatar( _PairFriendImg pair )
{
	m_mapAvatar[pair.first] = pair.second;
	return true;
}

long CUIFriendDlgBase::run()
{
	_tstring tsBaseDir = m_pUIChatMgr->GetBaseDir();

	while (true)
	{
		if (m_bExitWorkThread)
		{
			m_bExitWorkThread = false;
			break;
		}

		if (m_wqPresenceFriends.size() > 0)
		{
			_tstring tsUserName = m_wqPresenceFriends.leave();
			if (tsUserName.size() <= 0)
				continue;

			LPTSTR lpUserName = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
			if (lpUserName == NULL)
				continue;

			_tcsncpy(lpUserName, tsUserName.c_str(), MAX_PATH-1);
			::PostMessage(m_pUIChatMgr->GetMessageHWND(), WM_UPDATE_REFRESH_PRESENCE, 0, (LPARAM)(lpUserName));
			Sleep(50);
		}
		else if (m_wqAvatarFriends.size() > 0)
		{
			_tstring tsUserName = m_wqAvatarFriends.leave();
			if (tsUserName.size() <= 0)
				continue;

			LPTSTR lpUserName = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
			if (lpUserName == NULL)
				continue;

			_tcsncpy(lpUserName, tsUserName.c_str(), MAX_PATH-1);
			::PostMessage(m_pUIChatMgr->GetMessageHWND(), WM_UPDATE_LOADED_AVATAR, 0, (LPARAM)(lpUserName));
			Sleep(100);
		}
		else 
		{
			Sleep(500);
			continue;
		}
	}

	return 0;
}

void CUIFriendDlgBase::StartSearch( _tstring szSearchText /*= _T("")*/ )
{
	/*
	 *@ get friends
	 */
	if (GetFriendItemCount() > 0)
	{
		if (szSearchText.empty())
		{
			ISonicAnimation *pParentItem = m_pTree->GetItem(0);
			ISonicAnimation *pChildItem = m_pTree->GetFirstChildItem(pParentItem);
			while (pChildItem != NULL && pChildItem->IsValid())
			{
				m_pTree->ShowItem(pChildItem, TRUE);
				pChildItem = m_pTree->GetNextSiblingItem(pChildItem);
			}

			m_nOnLineVisibleItemNum = m_pTree->GetChildItemCount(pParentItem, TRUE);

			pParentItem = m_pTree->GetItem(1);
			pChildItem = m_pTree->GetFirstChildItem(pParentItem);
			while (pChildItem != NULL && pChildItem->IsValid())
			{
				m_pTree->ShowItem(pChildItem, TRUE);
				pChildItem = m_pTree->GetNextSiblingItem(pChildItem);
			}

			m_nOffLineVisibleItemNum = m_pTree->GetChildItemCount(pParentItem, TRUE);
		}
		else
		{
			m_nOnLineVisibleItemNum = 0;
			m_nOffLineVisibleItemNum = 0;
			ISonicAnimation *pParentItem = m_pTree->GetItem(0);
			ISonicAnimation *pChildItem = m_pTree->GetFirstChildItem(pParentItem);
			while (pChildItem != NULL && pChildItem->IsValid())
			{
				if (_tstring(pChildItem->GetName()).Find(szSearchText.c_str()) != 0) {
					m_pTree->ShowItem(pChildItem, FALSE);
				}else {
					m_pTree->ShowItem(pChildItem, TRUE);
					++m_nOnLineVisibleItemNum;
				}
				pChildItem = m_pTree->GetNextSiblingItem(pChildItem);
			}

			pParentItem = m_pTree->GetItem(1);
			pChildItem = m_pTree->GetFirstChildItem(pParentItem);
			while (pChildItem != NULL && pChildItem->IsValid())
			{
				if (_tstring(pChildItem->GetName()).Find(szSearchText.c_str()) != 0) {
					m_pTree->ShowItem(pChildItem, FALSE);
				}else {
					m_pTree->ShowItem(pChildItem, TRUE);
					++m_nOffLineVisibleItemNum;
				}
				pChildItem = m_pTree->GetNextSiblingItem(pChildItem);
			}
		}
	}

	/*
	 *@ refresh the friends number distinguished by offline-online
	 */
	UpdateFriendListHeaderNum(true);
	UpdateFriendListHeaderNum(false);
}

//friend dialog base end
////////////////////////////////////////////////////////////////////////////////////////////////////////////

CUIChatDlgBase::CUIChatDlgBase(CUIChatMgrBase* pMgr,_ITEM_BASE_INFO info)
:m_pUIChatMgr(NULL)
,m_pEditInput(NULL)
,m_pEditOutPut(NULL)
,m_pAvatar(NULL)
,m_pAvatarPaint(NULL)
{
	m_tsLastMsgDate = _T("\0");
	m_pUIChatMgr = pMgr;
	m_info = info;
	m_bSpamming =false;
	m_bFirstOfflineMessageComing = false;
	GetSonicUI()->AddInputFilter(this);
}

LRESULT CUIChatDlgBase::OnEnChangeEditChatdlgInput(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	/*
	** get current input text length in character
	*/
	if (m_pEditInput == NULL || !m_pEditInput->IsWindow())
	{ 
		return TRUE;
	}
	int nLen = m_pEditInput->GetTextLengthEx(GTL_NUMCHARS,1200);
	nLen = (nLen < 0) ? 0 : nLen;

	/*
	** truncate text if text length is larger than 220
	*/
	if (nLen > CHAT_MAX_INPUT_CHARACTERS)
	{//avoid exceed the limitation of 220 when copying text from clipboard.
		ATL::CString sChatEditText ;
		m_pEditInput->GetWindowText(sChatEditText) ;
		m_pEditInput->SetWindowText(sChatEditText.Left(CHAT_MAX_INPUT_CHARACTERS).GetString());	
		nLen = CHAT_MAX_INPUT_CHARACTERS;
	}
	return TRUE;
}

BOOL CUIChatDlgBase::OnInputMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if((uMsg == WM_KEYDOWN ||uMsg == WM_CHAR) && (wParam==VK_RETURN )&& (m_pEditInput && m_pEditInput->m_hWnd == hWnd))
	{
		if(!(GetKeyState(VK_CONTROL)&0x8000))
		{
			if (uMsg == WM_KEYDOWN)
			{
				SendChatMsg();	
			}
            return FALSE;		
		}
	}
	return TRUE;	
}

LRESULT CUIChatDlgBase::OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;  

	}
	switch(pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_PRESENCE_CHANGED:
		{
			im_msg_presence_changed *pCmd = (im_msg_presence_changed*)pHeader;
			UpdatePresence(pCmd->iType);
		}
		break;
	case  ARC_CHAT_MSG_SEND_MSG_DONE:
		{
			//Notify relative UI update.
			im_msg_avatar_changed *pCmd = (im_msg_avatar_changed*)pHeader;
		}
		break;
	case  ARC_CHAT_MSG_RCV_MSG:
		{
			//Notify relative UI update.
			im_msg_rcv_msg *pCmd = (im_msg_rcv_msg*)pHeader;
			HandleRecvMsg(pCmd->msg, pCmd->bOfflineMessage);
		}
		break;
	case  ARC_CHAT_MSG_AVATAR_CHANGED:
		{
			//Notify relative UI update.
			im_msg_avatar_changed *pCmd = (im_msg_avatar_changed*)pHeader;

			if (::GetFileAttributes(pCmd->szPath) != INVALID_FILE_ATTRIBUTES)
				m_pAvatar->Load(pCmd->szPath);

			UpdateAvatar(m_pAvatar,m_info.iType);
		}
		break;
	case ARC_CHAT_MSG_SYNC_SENT_MESSAGE:
		{
			im_msg_sync_sent_message *pCmd = (im_msg_sync_sent_message*)pHeader;
			ShowSentMessage(pCmd->szSelfName, pCmd->szContent, pCmd->szUser, pCmd->nType, pCmd->nTime, pCmd->nSyncID);
		}
		break;
	default:
		break;
	}
	return 0L;
}

LRESULT CUIChatDlgBase::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	switch(wParam)
	{
	case TIMER_NID_SET_SPAMMING:
		{
			KillTimer(wParam);
			m_bSpamming=false;
			m_listTm.clear( );	
		}
		break;
	default:
		break;
	}
	return 0L;
}

LRESULT CUIChatDlgBase::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	WTL::CPaintDC dc(m_hWnd);
	if (m_pAvatarPaint && m_pAvatarPaint->IsValid())
	{
		m_pAvatarPaint->Move(5,2);
		m_pAvatarPaint->Render(dc, m_hWnd);
	}
	return 0L;
}

LRESULT CUIChatDlgBase::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pUIChatMgr)
	{
		CWindow *pParentWnd = &GetParent();
		if (pParentWnd != NULL && pParentWnd->IsWindow())
		{
			CUITabMgrDlg *pTabMgr = m_pUIChatMgr->GetTabMgrDlgByHandle(pParentWnd->m_hWnd);
			if (pTabMgr && pTabMgr->IsWindow() && pTabMgr->GetTabCtrl() && pTabMgr->GetTabCtrl()->IsValid())
			{
				if (pTabMgr->GetTabCtrl()->GetTabCount() > 1)
				{
					pTabMgr->PostMessage(WM_TABMGR_RESIZE);
				}
			}
		}
		m_pUIChatMgr->RemoveChatDailog(m_info._szUser);
	}
	SONICUI_DESTROY(m_pAvatarPaint);
	return 0L;
}

LRESULT CUIChatDlgBase::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   return 0L;
}

void CUIChatDlgBase::UpdatePresence(UINT iStatus,BOOL bFirst)
{
	int nCorlorIndex=5, nMsgCorlorIndex=10, nTimeCorlorIndex=12;
	_tstring szMsg(_T(""));
	if (bFirst)
	{
		if (iStatus < ArcPresenceType::DND)
		{
			nCorlorIndex =6;
			nMsgCorlorIndex=11;
			nTimeCorlorIndex=13;
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION3")).c_str());
		}
		else if (iStatus == ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION1")).c_str());
		}
		else 
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION2")).c_str());
		}
	}
	else
	{
		if (iStatus == ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION1")).c_str());
		}
		else if (iStatus > ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION7")).c_str());
		}
	}

	if (!szMsg.empty())
	{
		ATL::CTime tmNow = m_pUIChatMgr->GetTime(); 
		ATL::CString strTime = tmNow.Format(_T("(%H:%M)"));

		_tstring sTimeStamp(_T(""));
		//verify whether to display timestamp
        _IM_BASE_CONFIG settings;
        m_pUIChatMgr->LoadConfig(settings);
		if (settings.bShowTimeStamp)
		{
			sTimeStamp.Format(1,_T(" %s"),strTime.GetString());
		}
		if (m_pEditOutPut && m_pEditOutPut->IsWindow())
		{
			m_pEditOutPut->AddLine(nCorlorIndex,m_info._szNick);
			m_pEditOutPut->SetLineSpacing(LINE_SPACE);
			m_pEditOutPut->AddText(nMsgCorlorIndex,szMsg.c_str());
			m_pEditOutPut->AddText(nTimeCorlorIndex,sTimeStamp.c_str());
			m_pEditOutPut->AddLine(1,_T(""));
			m_pEditOutPut->SetLineSpacing(0);
			m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);
		}
	}
	UpdateAvatar(m_pAvatar,iStatus);
	m_info.iType = iStatus;
}

void CUIChatDlgBase::UpdateAvatar( UINT nStatus )
{
	UpdateAvatar(m_pAvatar, nStatus);
	m_info.iType = nStatus;
}

void CUIChatDlgBase::UpdateAvatar(ISonicImage* pImg,UINT nStatus)
{
	if (pImg == NULL || m_pAvatarPaint==NULL)
	{
		return;
	}

	m_pAvatarPaint->DelAllObject();

	ISonicString* pBk = GetSonicUI()->CreateString();
	ISonicImage* pBkImg = GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"));
	switch (nStatus)
	{
	case ArcPresenceType::Available:
	case ArcPresenceType::Chat:
		{
			pBk->Format(_T("/p=%d,pclip=0|0|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetWidth(),pBkImg->GetHeight()/3);
		}
		break;
	case ArcPresenceType::DND:
		{
			pBk->Format(_T("/p=%d,pclip=0|%d|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetHeight()/3,pBkImg->GetWidth(),pBkImg->GetHeight()*2/3);
		}
		break;
	default:
		{
			pBk->Format(_T("/p=%d,pclip=0|%d|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetHeight()*2/3,pBkImg->GetWidth(),pBkImg->GetHeight());
		}
		break;
	}

	ISonicString* pAvatarStr = GetSonicUI()->CreateString();
	pAvatarStr->Format(_T("/a,p=%d, pstretch=43|43/"),pImg->GetObjectId());
	pAvatarStr->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIChatDlgBase::OnDelegateClickedChatFriend);

	m_pAvatarPaint->AddObject(pBk->GetObjectId(),0,0,TRUE);
	m_pAvatarPaint->AddObject(pAvatarStr->GetObjectId(),(m_pAvatarPaint->GetWidth()-pAvatarStr->GetWidth())/2,(m_pAvatarPaint->GetHeight()-pAvatarStr->GetHeight())/2,TRUE);		
	m_pAvatarPaint->Redraw(FALSE);
}

void CUIChatDlgBase::HandleRecvMsg(const MSG_LOG_ELEMENT &msg, bool bOfflineMessage /*= false*/)
{
	_tstring sNick(m_info._szNick);
	if (sNick.empty())
	{
		sNick = m_info._szUser;
	}

	_tstring tsHead(_T(""));
	//verify whether to display timestamp
	_IM_BASE_CONFIG settings;
	m_pUIChatMgr->LoadConfig(settings);
	if (settings.bShowTimeStamp)
	{
		tsHead.append(_T(" ("));

		_tstring tsTime = msg._szTime;
		_tstring::SplitList timeList = tsTime.Split(_T(":"));
		_tstring::SplitList::iterator itr = timeList.begin();
		if (itr == timeList.end()) return;
		tsTime = (*itr);
		tsTime += _tstring(_T(":"));
		++itr;
		if (itr == timeList.end()) return;
		tsTime += (_tstring)(*itr);
		tsHead.append(tsTime.c_str());

		tsHead.append(_T("):"));

		//display date.
		do 
		{
			ATL::CString csExistText;
			m_pEditOutPut->GetWindowText(csExistText);
			if ( (!_tstring(msg._szDate).empty() && _tstring(msg._szDate).CompareNoCase(m_tsLastMsgDate) != 0) ||
				 (bOfflineMessage && !m_bFirstOfflineMessageComing) )
			{
				_tstring::SplitList sDatelist = _tstring(msg._szDate).Split(_T(":"));

				_tstring tsDate;
				if (sDatelist.size() == 2 && 
					sDatelist.front().ConvToInt32() > 0 && 
					sDatelist.front().ConvToInt32() < 13)
				{
					_tstring tsMonthId;
					tsMonthId.Format(1,_T("IDS_MONTH_%d"),sDatelist.front().ConvToInt32());
					tsMonthId = m_pUIChatMgr->GetStringFromId(tsMonthId.c_str());

					sDatelist.pop_front();
					_tstring tsDayAndYear = sDatelist.front();

					tsDate.Format(2,_T("%s %s"),tsMonthId.c_str(),tsDayAndYear.c_str());
					if (!tsDate.CompareNoCase(m_tsLastMsgDate))
					{
						m_tsLastMsgDate = msg._szDate;
						break;
					}
				}
				else
				{
					tsDate=msg._szDate;
				}

				m_pEditOutPut->AddLine(7,tsDate.c_str());
				m_pEditOutPut->SetLineSpacing(LINE_SPACE);
				m_tsLastMsgDate = msg._szDate;
			}
		} while (FALSE);
	}
	else
	{
		sNick.append(_T(":"));
	}

	m_bFirstOfflineMessageComing = true;

	if (m_pEditOutPut && m_pEditOutPut->IsWindow())
	{
		m_pEditOutPut->AddLine(0,sNick.c_str());
		m_pEditOutPut->SetLineSpacing(LINE_SPACE);
		m_pEditOutPut->AddText(7,tsHead.c_str());
		m_pEditOutPut->AddLine(1,msg._szContent);
		m_pEditOutPut->AddLine(1,_T(""));
		m_pEditOutPut->SetLineSpacing(0);
		m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);
	}

	SaveChatMessageLog(msg);
}

void CUIChatDlgBase::SendChatMsg()
{
	if (!m_pUIChatMgr)
		return;

	if (m_pEditOutPut == NULL || !m_pEditOutPut->IsWindow())
		return;

	if (m_pEditInput == NULL || !m_pEditInput->IsWindow())
		return;

	//check message whether is empty.
	ATL::CString csContent;
	m_pEditInput->GetWindowText(csContent) ;

	if(csContent.GetLength()==0)   
		return ;

	//check self presence whether is offline
	int iStatus = m_pUIChatMgr->GetSefPresence();
	if (iStatus >= ArcPresenceType::XA)
	{
		m_pEditOutPut->AddLine(10,m_pUIChatMgr->GetStringFromId(_T("IDC_DIALOG_CHAT_OFFLINE")).c_str());
		m_pEditOutPut->SetLineSpacing(LINE_SPACE);
		m_pEditOutPut->AddLine(1,_T(""));
		m_pEditOutPut->SetLineSpacing(0);
		m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);
		return;
	}

	//check sending message whether too frequent.
	time_t tm;
	time(&tm);
	m_listTm.push_back(tm);
	while(m_listTm.size()>10)
	{
		m_listTm.pop_front();
	}
	if(10==m_listTm.size()&&m_listTm.back()-m_listTm.front()<5)
	{
		m_bSpamming=true;
		KillTimer(TIMER_NID_SET_SPAMMING);
		SetTimer(TIMER_NID_SET_SPAMMING,30000,NULL);
	}
	if(m_bSpamming)
	{
		m_pEditOutPut->AddLine(1,m_pUIChatMgr->GetStringFromId(_T("IDC_DIALOG_CHAT_FAST")).c_str());
		m_pEditOutPut->SetLineSpacing(LINE_SPACE);
		m_pEditOutPut->AddLine(1,_T(""));
		m_pEditOutPut->SetLineSpacing(0);
		m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);
		return;
	}

	_tstring tsName = (_tcslen(m_pUIChatMgr->GetNickName()) <= 0) ? m_pUIChatMgr->GetUserName() : m_pUIChatMgr->GetNickName();

	ATL::CTime tmNow = m_pUIChatMgr->GetTime(); 
	ATL::CString strTime = tmNow.Format(_T("%H:%M:%S")); 

	_tstring tsCurDate;
	tsCurDate.Format(3,_T("%d:%d,%d"),tmNow.GetMonth(),tmNow.GetDay(),tmNow.GetYear());
	_tstring tsHead(_T(""));

	//verify whether to display timestamp
	_IM_BASE_CONFIG settings;
	m_pUIChatMgr->LoadConfig(settings);
	if (settings.bShowTimeStamp)
	{
		tsHead.append(_T(" ("));

		_tstring tsTime = strTime;
		_tstring::SplitList timeList = tsTime.Split(_T(":"));
		_tstring::SplitList::iterator itr = timeList.begin();
		if (itr == timeList.end()) return;
		tsTime = (*itr);
		tsTime += _tstring(_T(":"));
		++itr;
		if (itr == timeList.end()) return;
		tsTime += (_tstring)(*itr);
		tsHead.append(tsTime.c_str());

		tsHead.append(_T("):"));

		do 
		{
			if (!tsCurDate.empty() && tsCurDate.CompareNoCase(m_tsLastMsgDate) != 0 )
			{
				_tstring::SplitList sDatelist = tsCurDate.Split(_T(":"));

				_tstring tsDate;
				if (sDatelist.size()==2 && sDatelist.front().ConvToInt32()>0 && sDatelist.front().ConvToInt32()<13)
				{
					_tstring tsMonthId;
					tsMonthId.Format(1,_T("IDS_MONTH_%d"),sDatelist.front().ConvToInt32());
					tsMonthId = m_pUIChatMgr->GetStringFromId(tsMonthId.c_str());

					sDatelist.pop_front();
					_tstring tsDayAndYear = sDatelist.front();

					tsDate.Format(2,_T("%s %s"),tsMonthId.c_str(),tsDayAndYear.c_str());
					if (!tsDate.CompareNoCase(m_tsLastMsgDate))
					{
						m_tsLastMsgDate = tsCurDate;
						break;
					}
				}
				else
				{
					tsDate = tsCurDate;
				}

				m_pEditOutPut->AddLine(7,tsDate.c_str());
				m_pEditOutPut->SetLineSpacing(LINE_SPACE);
				m_tsLastMsgDate = tsCurDate;
			}
		} while (FALSE);
	}
	else
	{
		tsName.append(_T(":"));
	}

	m_pEditOutPut->AddLine(2, tsName.c_str( ));
	m_pEditOutPut->SetLineSpacing(LINE_SPACE);
	m_pEditOutPut->AddText(8,tsHead.c_str());
	m_pEditOutPut->AddLine(1,csContent.GetString());
	m_pEditOutPut->AddLine(1,_T(""));
	m_pEditOutPut->SetLineSpacing(0);
	m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);

	m_pEditInput->SetWindowText(_T(""));

	ITEM_BASE_INFO friendInfo;
	if (m_pUIChatMgr->GetFriendInfo(m_info._szUser, &friendInfo))
	{
		SyncSentMessage(tsName.c_str(), csContent.GetString(), m_info._szUser, m_info.iType, tmNow.GetTime(), GetCurrentProcessId());
		UpdateXmppStatus((UINT)m_info.iType, FALSE);

		MSG_LOG_ELEMENT msg;
		lstrcpyn(msg._szUserName,m_info._szUser,MAX_PATH-1);
		lstrcpyn(msg._szRecvNick,m_info._szNick,MAX_PATH-1);
		lstrcpyn(msg._szSenderNick,tsName.c_str(),MAX_PATH-1);
		lstrcpyn(msg._szTime,strTime.GetString(),MAX_PATH-1);
		lstrcpyn(msg._szDate,tsCurDate.c_str(),MAX_PATH-1);
		lstrcpyn(msg._szContent,csContent.GetString(),MAX_PATH-1);

		m_pUIChatMgr->SendChatMessage(msg, m_info.iType > ArcPresenceType::DND);
	}
	else
	{
		UpdateXmppStatus((UINT)ArcPresenceType::Unavailable, FALSE);
	}

}

void CUIChatDlgBase::ShowSentMessage( LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId )
{
	if (GetCurrentProcessId() == nSyncId)
		return;

	_tstring tsName = lpcSelfName;

	ATL::CTime tmNow(nTime); 
	ATL::CString strTime = tmNow.Format(_T("%H:%M:%S")); 

	_tstring tsCurDate;
	tsCurDate.Format(3,_T("%d:%d,%d"),tmNow.GetMonth(),tmNow.GetDay(),tmNow.GetYear());
	_tstring tsHead(_T(""));

	//verify whether to display timestamp
	_IM_BASE_CONFIG settings;
	m_pUIChatMgr->LoadConfig(settings);
	if (settings.bShowTimeStamp)
	{
		tsHead.append(_T(" ("));

		_tstring tsTime = strTime;
		_tstring::SplitList timeList = tsTime.Split(_T(":"));
		_tstring::SplitList::iterator itr = timeList.begin();
		if (itr == timeList.end()) return;
		tsTime = (*itr);
		tsTime += _tstring(_T(":"));
		++itr;
		if (itr == timeList.end()) return;
		tsTime += (_tstring)(*itr);
		tsHead.append(tsTime.c_str());

		tsHead.append(_T("):"));

		do 
		{
			if (!tsCurDate.empty() && tsCurDate.CompareNoCase(m_tsLastMsgDate) != 0 )
			{
				_tstring::SplitList sDatelist = tsCurDate.Split(_T(":"));

				_tstring tsDate;
				if (sDatelist.size()==2 && sDatelist.front().ConvToInt32()>0 && sDatelist.front().ConvToInt32()<13)
				{
					_tstring tsMonthId;
					tsMonthId.Format(1,_T("IDS_MONTH_%d"),sDatelist.front().ConvToInt32());
					tsMonthId = m_pUIChatMgr->GetStringFromId(tsMonthId.c_str());

					sDatelist.pop_front();
					_tstring tsDayAndYear = sDatelist.front();

					tsDate.Format(2,_T("%s %s"),tsMonthId.c_str(),tsDayAndYear.c_str());
					if (!tsDate.CompareNoCase(m_tsLastMsgDate))
					{
						m_tsLastMsgDate = tsCurDate;
						break;
					}
				}
				else
				{
					tsDate = tsCurDate;
				}

				m_pEditOutPut->AddLine(7,tsDate.c_str());
				m_pEditOutPut->SetLineSpacing(LINE_SPACE);
				m_tsLastMsgDate = tsCurDate;
			}
		} while (FALSE);
	}
	else
	{
		tsName.append(_T(":"));
	}

	m_pEditOutPut->AddLine(2, tsName.c_str( ));
	m_pEditOutPut->SetLineSpacing(LINE_SPACE);
	m_pEditOutPut->AddText(8, tsHead.c_str());
	m_pEditOutPut->AddLine(1, lpcContent);
	m_pEditOutPut->AddLine(1, _T(""));
	m_pEditOutPut->SetLineSpacing(0);
	m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);

	UpdateXmppStatus((UINT)nType, FALSE);
}

void CUIChatDlgBase::UpdateXmppStatus(UINT iStatus,BOOL bFirst)
{
	if (!m_pUIChatMgr)
	{
		return;
	}
	int nCorlorIndex=5, nMsgCorlorIndex=10, nTimeCorlorIndex=12;
	_tstring szMsg(_T(""));
	if (bFirst)
	{
		if (iStatus < ArcPresenceType::DND)
		{
			nCorlorIndex =6;
			nMsgCorlorIndex=11;
			nTimeCorlorIndex=13;
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION3")).c_str());
		}
		else if (iStatus == ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION1")).c_str());
		}
		else 
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION2")).c_str());
		}
	}
	else
	{
		if (iStatus == ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION1")).c_str());
		}
		else if (iStatus > ArcPresenceType::DND)
		{
			szMsg.Format(1,_T(" %s"),m_pUIChatMgr->GetStringFromId(_T("IDS_DIALOG_CHAT_OPTION7")).c_str());
		}
	}

	if (!szMsg.empty())
	{
		ATL::CTime tmNow = m_pUIChatMgr->GetTime(); 
		ATL::CString   strTime   = tmNow.Format(_T("(%H:%M)"));

		_tstring sTimeStamp(_T(""));
		//verify whether to display timestamp
		_IM_BASE_CONFIG settings;
		m_pUIChatMgr->LoadConfig(settings);
		if (settings.bShowTimeStamp)
		{
			sTimeStamp.Format(1,_T(" %s"),strTime.GetString());
		}
		if (m_pEditOutPut && m_pEditOutPut->IsWindow())
		{
			m_pEditOutPut->AddLine(nCorlorIndex,m_info._szNick);
			m_pEditOutPut->SetLineSpacing(LINE_SPACE);
			m_pEditOutPut->AddText(nMsgCorlorIndex,szMsg.c_str());
			m_pEditOutPut->AddText(nTimeCorlorIndex,sTimeStamp.c_str());
			m_pEditOutPut->AddLine(1,_T(""));
			m_pEditOutPut->SetLineSpacing(0);
			m_pEditOutPut->PostMessage(WM_VSCROLL,SB_BOTTOM,0);
		}
	}
	UpdateAvatar(m_pAvatar,iStatus);
}

void CUIChatDlgBase::ShowHistoryRecords()
{
	if (!m_pEditOutPut || !m_pEditOutPut->IsWindow() || !m_pUIChatMgr)
	{
		return;
	}
	//get config about whether display timestamp in chat box or not
	_IM_BASE_CONFIG settings;
	m_pUIChatMgr->LoadConfig(settings);

	//insert all of messages into chat list box
	std::list<MSG_LOG_ELEMENT>  vecLogMsg;
	m_pUIChatMgr->LoadHistoryRecord(m_info._szUser,100,vecLogMsg);

	typedef std::map<ATL::CTime, std::vector<LPMSG_LOG_ELEMENT>> TMapLogMsg;
	typedef TMapLogMsg::iterator								 TMapLogMsgItr;

	TMapLogMsg mapLogMsg;
	std::list<MSG_LOG_ELEMENT>::iterator msgItr = vecLogMsg.begin();
	for (; msgItr != vecLogMsg.end(); ++msgItr)
	{
		//init month
		_tstring::SplitList timeList = _tstring(msgItr->_szDate).Split(_T(":"));
		_tstring::SplitList::iterator itr = timeList.begin();
		if (itr == timeList.end()) 
			return;
		_tstring tsMonth = *itr;
		++itr;

		//init day
		if (itr == timeList.end()) 
			return;
		timeList = (*itr).Split(_T(","));
		itr = timeList.begin();
		if (itr == timeList.end()) 
			return;
		_tstring tsDay = *itr;
		++itr;

		//init year
		if (itr == timeList.end()) 
			return;
		_tstring tsYear = *itr;

		//init hour
		timeList = _tstring(msgItr->_szTime).Split(_T(":"));
		itr = timeList.begin();
		if (itr == timeList.end()) 
			return;
		_tstring tsHour = *itr;
		++itr;

		//init min
		if (itr == timeList.end()) 
			return;
		_tstring tsMin = *itr;
		++itr;

		//init sec, keep compatible(old time do not has second)
		_tstring tsSec = _T("0");
		if (itr != timeList.end())
			tsSec = *itr;

		CTime messageTime(
				tsYear.ConvToInt32(), 
				tsMonth.ConvToInt32(), 
				tsDay.ConvToInt32(), 
				tsHour.ConvToInt32(), 
				tsMin.ConvToInt32(), 
				tsSec.ConvToInt32()
				);

		//save the log by time-key
		if (mapLogMsg.find(messageTime) == mapLogMsg.end())
		{
			std::vector<LPMSG_LOG_ELEMENT> vecTemp;
			mapLogMsg[messageTime] = vecTemp;
		}

		mapLogMsg[messageTime].push_back(&(*msgItr));
	}

	TMapLogMsgItr  mapItr = mapLogMsg.begin();
	for (; mapItr != mapLogMsg.end(); ++mapItr)
	{
		for (int nLogIndex = 0; nLogIndex < mapItr->second.size(); ++nLogIndex)
		{
			LPMSG_LOG_ELEMENT& lpLogElement = mapItr->second[nLogIndex];

			if (_tstring(lpLogElement->_szContent).empty())
				continue;
			//display timestamp based on config
			_tstring  tsName = lpLogElement->_szSenderNick;
			_tstring   tsHead(_T(""));
			if (settings.bShowTimeStamp)
			{
				tsHead.append(_T(" ("));

				_tstring tsTime = lpLogElement->_szTime;
				_tstring::SplitList timeList = tsTime.Split(_T(":"));
				_tstring::SplitList::iterator itr = timeList.begin();
				if (itr == timeList.end()) return;
				tsTime = (*itr);
				tsTime += _T(":");
				++itr;
				if (itr == timeList.end()) return;
				tsTime += (_tstring)(*itr);
				tsHead.append(tsTime.c_str());

				tsHead.append(_T("):"));

				//display date.
				do 
				{
					if (!_tstring(lpLogElement->_szDate).empty() && _tstring(lpLogElement->_szDate).CompareNoCase(m_tsLastMsgDate) != 0 )
					{
						_tstring::SplitList sDatelist = _tstring(lpLogElement->_szDate).Split(_T(":"));

						_tstring tsDate;
						if (sDatelist.size() == 2 && sDatelist.front().ConvToInt32() > 0 && sDatelist.front().ConvToInt32() < 13)
						{
							_tstring tsMonthId;
							tsMonthId.Format(1,_T("IDS_MONTH_%d"), sDatelist.front().ConvToInt32());
							tsMonthId = m_pUIChatMgr->GetStringFromId(tsMonthId.c_str());

							sDatelist.pop_front();
							_tstring tsDayAndYear = sDatelist.front();

							tsDate.Format(2, _T("%s %s"), tsMonthId.c_str(), tsDayAndYear.c_str());
							if (!tsDate.CompareNoCase(m_tsLastMsgDate))
							{
								m_tsLastMsgDate = lpLogElement->_szDate;
								break;
							}
						}
						else
						{
							tsDate = lpLogElement->_szDate;
						}

						m_pEditOutPut->AddLine(4, tsDate.c_str());
						m_pEditOutPut->SetLineSpacing(LINE_SPACE);
						m_tsLastMsgDate = lpLogElement->_szDate;
					}
				} while (FALSE);
			}
			else
			{
				tsName.append(_T(":"));
			}

			//display chat message from db
			m_pEditOutPut->AddLine(3, tsName.c_str());
			m_pEditOutPut->SetLineSpacing(LINE_SPACE);
			m_pEditOutPut->AddText(9, tsHead.c_str());
			m_pEditOutPut->AddLine(4, lpLogElement->_szContent);	
			m_pEditOutPut->AddLine(4, _T(""));
			m_pEditOutPut->SetLineSpacing(0);
		}
	}
	m_pEditOutPut->PostMessage( WM_VSCROLL,  SB_BOTTOM,0);
}

void CUIChatDlgBase::OnStaticClickedChatClear(ISonicString *, LPVOID pReserve)
{
	if (m_pEditOutPut &&m_pEditOutPut->IsWindow())
	{
		m_pEditOutPut->SetWindowText(_T(""));
		m_pEditOutPut->Invalidate();//fixed bug 0001561:The old scrollbar appears if user click "Clear History".
	}
	if (m_pUIChatMgr)
	{
		m_pUIChatMgr->ClearHistroyRecord(m_info._szUser);
	}
}

void CUIChatDlgBase::OnDelegateClickedChatFriend( ISonicString *, LPVOID pReserve )
{
	ClickFriend();
}

LRESULT CUIChatDlgBase::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CUITabMgrDlg *pTabMgr = m_pUIChatMgr->GetChatDlgTabMgr(this);
	if (pTabMgr && ::IsWindow(pTabMgr->m_hWnd) && pTabMgr->IsChatFlashing(this))
	{
		pTabMgr->DoFlash(this, false);
	}
	return 0L;
}

LRESULT CUIChatDlgBase::OnEnInputFocus(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	return OnLButtonDown(uMsg, wParam, (LPARAM)hwnd, bHandled);
}

LRESULT CUIChatDlgBase::OnEnOutputFocus(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	return OnLButtonDown(uMsg, wParam, (LPARAM)hwnd, bHandled);
}

LRESULT CUIChatDlgBase::OnSetInputEditFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (m_pEditInput)
	{
		m_pEditInput->SetFocus();
	}
	return 1L;
}

void CUIChatDlgBase::SyncSentMessage( LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId )
{
	m_pUIChatMgr->SyncSentMessage(lpcSelfName, lpcContent, lpcUser, nType, nTime, nSyncId);
}

CUIStatusDlgBase::CUIStatusDlgBase(CUIChatMgrBase* pMgr)
:m_pUIChatMgr(pMgr)
{
	m_pWndEffect = GetSonicUI()->CreateWndEffect();
}

CUIStatusDlgBase::~CUIStatusDlgBase()
{
	SONICUI_DESTROY(m_pWndEffect);
}

LRESULT CUIStatusDlgBase::OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader || !m_pUIChatMgr)
	{
		return 0L;  
	}
	switch(pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_PRESENCE_CHANGED:
		{
			//Notify relative UI update.
			im_msg_presence_changed *pCmd = (im_msg_presence_changed*)pHeader;
			if (!pCmd) break;

			if (lstrcmp(m_pUIChatMgr->GetUserName(), pCmd->szUserName) != 0) 
				break;

			int nIndex = -1;
			switch(pCmd->iType)
			{
			case ArcPresenceType::Available:
				nIndex = 0;
				break;
			case ArcPresenceType::DND:
				nIndex = 1;
				break;
			case ArcPresenceType::Unavailable:
				nIndex = 2;
				break;
			default:
				break;
			}
			m_LBSetting.SetCurSel(nIndex);
		}
		break;
	default:
		break;
	}
	return 0L;
}

LRESULT CUIStatusDlgBase::OnItemClick(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled)
{
	int Index = m_LBSetting.GetCurSel();
	if (Index == LB_ERR || !m_pUIChatMgr || !m_pUIChatMgr->GetFriendDlg())
	{
		return FALSE;
	}

	if ((int)m_LBSetting.GetItemData(Index) != m_pUIChatMgr->GetFriendDlg()->GetCurPresence())
	{
		int loginStatus = (int)m_LBSetting.GetItemData(Index);
		m_pUIChatMgr->SetLoginStatus(loginStatus);
		m_pUIChatMgr->SetPresence(loginStatus);
	}
	ShowWindow(SW_HIDE);
	return TRUE;
}

LRESULT CUIStatusDlgBase::OnSetPresence(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if (m_pUIChatMgr)
	{
		m_pUIChatMgr->SetPresence((ArcPresenceType)wParam);
		return 0L;
	}
	return 0L;
}

CUIChatMgrBase::CUIChatMgrBase()
	:m_pFriendDlg(NULL)
	,m_pStatusDlg(NULL)
	,m_pCurTabMgrDlg(NULL)
	,m_pMessageWnd(NULL)
{
	m_ChatDlgMap.clear();
	m_vecTabMgr.clear();
}
CUIChatMgrBase::~CUIChatMgrBase()
{
	DestroyChats();
	if (m_pMessageWnd) 
	{
		if (::IsWindow(m_pMessageWnd->m_hWnd))
		{
			m_pMessageWnd->DestroyWindow();
		}
		delete m_pMessageWnd;
	}
}

void CUIChatMgrBase::OnCoreMessage(HWND hFrom, core_msg_header * pHeader)
{
	if (!pHeader || !IsFriendDlgValid() || !IsStatusDlgValid())
	{
		return ;
	}

	switch (pHeader->dwCmdId)
	{
	case  ARC_CHAT_MSG_PRESENCE_CHANGED:
		{  //Notify relative UI update.
			im_msg_presence_changed *pCmd = (im_msg_presence_changed*)pHeader;
			if (!pCmd) 
				return;

			if (_tcscmp(GetUserName(), pCmd->szUserName) == 0)
			{//my status changed
				//handle self presence to the friend dlg;
				//handle self presence to the status dlg;
				m_pFriendDlg->HandleSelfPresence((ArcPresenceType)(pCmd->iType));
				SendCoreMsgDirectly(m_pStatusDlg->m_hWnd, pHeader);
			}
			else
			{//one of my friend's status changed
				_tstring tsUserName = pCmd->szUserName;
				m_pFriendDlg->EnterFriendPresenceChangeTask(tsUserName);
			}

			CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_presence_changed*)pHeader)->szUserName);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				SendCoreMsgDirectly(pDlg->m_hWnd,pHeader);
			}
		}
		break;
	case ARC_CHAT_MSG_AVATAR_CHANGED:
		{
			//Notify relative UI update.
			im_msg_avatar_changed *pCmd = (im_msg_avatar_changed*)pHeader;
			if (!pCmd) return;
			if (_tcscmp(GetUserName(), pCmd->szUserName) == 0)
			{//my avatar changed
				m_pFriendDlg->HandleSelfAvatar(pCmd->szPath);
			}
			else
			{//one of my friend's avatar changed
				_tstring tsUserName = pCmd->szUserName;
				m_pFriendDlg->EnterFriendAvatarChangeTask(tsUserName);
			}

			CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_avatar_changed*)pHeader)->szUserName);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				SendCoreMsgDirectly(pDlg->m_hWnd,pHeader);
			}
		}
		break;
	case ARC_CHAT_MSG_SEND_MSG_DONE:
		{  //Notify relative UI update.
			CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_send_msg_done*)pHeader)->msg._szUserName);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				SendCoreMsgDirectly(pDlg->m_hWnd,pHeader);
			}	
		}
		break;
	case ARC_CHAT_MSG_RCV_MSG:
		{
			//Notify relative UI update.
			ITEM_BASE_INFO info;
			if (GetFriendInfo(((im_msg_rcv_msg*)pHeader)->msg._szUserName, &info))
			{
				BOOL bOfflineMessage = ((im_msg_rcv_msg*)pHeader)->bOfflineMessage ? 1 : 0;
				if (_tcslen(info._szUser) == 0 || !m_pFriendDlg || !::IsWindow(m_pFriendDlg->m_hWnd))
				{
					break;
				}

				::SendMessage(m_pFriendDlg->m_hWnd, WM_FRLIST_SHOW_CHAT, (WPARAM)&info, (LPARAM)bOfflineMessage);

				CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_rcv_msg*)pHeader)->msg._szUserName);
				if (pDlg && ::IsWindow(pDlg->m_hWnd))
				{
					SendCoreMsgDirectly(pDlg->m_hWnd,pHeader);
				}
			}
		}
		break;
	case ARC_CHAT_MSG_FRIEND_DIVORCE:
		{
			//Notify relative UI update
			SendCoreMsgDirectly(m_pFriendDlg->m_hWnd,pHeader);
		}
		break;
	case ARC_CHAT_MSG_SYNC_SENT_MESSAGE:
		{
			CUIChatDlgBase* pDlg = GetChatDailog(((im_msg_sync_sent_message*)pHeader)->szUser);
			if (pDlg && ::IsWindow(pDlg->m_hWnd))
			{
				SendCoreMsgDirectly(pDlg->m_hWnd,pHeader);
			}
		}
		break;
	default:
		break;
	}
	return;
}

ISonicString* CUIChatMgrBase::GetStatusBkImg(UINT iStatus,bool bBig)
{
	ISonicString* pBk = GetSonicUI()->CreateString();
	ISonicImage* pBkImg =NULL;
	if (bBig)
	{
		pBkImg = GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"));
	}
	else
	{
		pBkImg = GetCoreUI()->GetImage(_T("CHAT_STATE_SMALL"));
	}

	switch (iStatus)
	{
	case ArcPresenceType::Available:
	case ArcPresenceType::Chat:
		{
			pBk->Format(_T("/p=%d,pclip=0|0|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetWidth(),pBkImg->GetHeight()/3);
		}
		break;
	case ArcPresenceType::DND:
		{
			pBk->Format(_T("/p=%d,pclip=0|%d|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetHeight()/3,pBkImg->GetWidth(),pBkImg->GetHeight()*2/3);
		}
		break;
	default:
		{
			pBk->Format(_T("/p=%d,pclip=0|%d|%d|%d/"),pBkImg->GetObjectId(),pBkImg->GetHeight()*2/3,pBkImg->GetWidth(),pBkImg->GetHeight());
		}
		break;
	}
	return pBk;
}

void CUIChatMgrBase::ShowNewMinimizeTab(HWND hWnd)
{
	ShowWindow(hWnd, SW_SHOWNA);
}

CUIChatDlgBase* CUIChatMgrBase::ShowChatDlg(_ITEM_BASE_INFO info,im_dialog_params para, bool bNeedShow /*= true*/)
{
	CUIChatDlgBase* pDlg = GetChatDailog(info._szUser);

	if (!pDlg)
	{
	/*	if (!para.bCreated)
		{
			return false;
		}*/

		CUITabMgrDlg* pTabMgr = (CUITabMgrDlg*) para.pParentWnd;
		if (!pTabMgr || !IsWindow(pTabMgr->m_hWnd))
		{
			return false;
		}
		pDlg = CreateChatDlgObj(info);
		HWND hWnd = pDlg->Create(pTabMgr->m_hWnd);
		if (hWnd == NULL || !::IsWindow(hWnd))
		{
			delete pDlg;
			return NULL;
		}

		m_lockChatDlgMap.lock();
		m_ChatDlgMap.insert(std::pair<_tstring,CUIChatDlgBase*>(info._szUser,pDlg));
		m_lockChatDlgMap.unlock();

		pTabMgr->AddTab(hWnd,info._szNick);
		pTabMgr->PostMessage(WM_TABMGR_RESIZE, 0, 0);
	}

	if (!::IsWindow(pDlg->m_hWnd))
	{
		m_lockChatDlgMap.lock();
		m_ChatDlgMap.erase(info._szUser);
		m_lockChatDlgMap.unlock();
		return false;
	}

	if (para.IsWinPosChanging())
	{
		//MoveWindow(para.rt,TRUE);
		::MoveWindow(pDlg->GetParent(),para.rt.left,para.rt.top,para.rt.right - para.rt.left,para.rt.bottom - para.rt.top,TRUE);
	}

	if (bNeedShow)
	{
		if (para.nShowCmd == SW_MINIMIZE && para.pParentWnd != NULL && para.pParentWnd->IsWindow())
		{
			HWND hWnd = para.pParentWnd->m_hWnd;
			ShowNewMinimizeTab(hWnd);
		}
		else
		{
			ShowWindow(pDlg->GetParent(), para.nShowCmd);
			if (pDlg->GetInputEdit())
			{
				pDlg->GetInputEdit()->SetFocus();
			}
		}
	}

	return pDlg;
}

CUIChatDlgBase*  CUIChatMgrBase::ShowChatDlg(_ITEM_BASE_INFO info, BOOL bOfflineMessage /*= FALSE*/, 
											 bool bManual /*= true*/, bool bShow /*= true*/, bool bActive /*= true*/)
{
	HWND hWnd = ::GetForegroundWindow();

	bool bNewCreate = false;
	CUITabMgrDlg *pTabMgr = GetChatDlgTabMgr(GetChatDailog(info._szUser));
	if (!pTabMgr || !pTabMgr->IsWindow())
	{
		pTabMgr = GetBestTabMgrDlg(bNewCreate);
	}

	if (bNewCreate && bActive)
	{
		::SetForegroundWindow(hWnd);
	}

	if (!pTabMgr || !pTabMgr->IsWindow() || !pTabMgr->GetTabCtrl())
	{
		return NULL;
	}

	im_dialog_params para;
	para.pParentWnd = pTabMgr;	
	bool bNeedShow = false;

	if (!bManual && bNewCreate && bShow)
	{
		if (bOfflineMessage == TRUE)
		{
			para.nShowCmd = SW_MINIMIZE;
		}
		else if (IsForegroundWndBelongMe(hWnd))
		{
			para.nShowCmd = SW_SHOWNORMAL;
		}
		else
		{
			para.nShowCmd = SW_MINIMIZE;
		}
		bNeedShow = true;
	}

	CUIChatDlgBase *pDlg = ShowChatDlg(info, para, bNeedShow);
	
	if (!pDlg || !pDlg->IsWindow())
	{
		return NULL;
	}

	if (bManual && bShow)
	{
		pTabMgr->PostMessage(WM_TABMGR_SHOWSEL, (WPARAM)pDlg, 0);
		pTabMgr->SetWindowText(info._szNick);
		if (pTabMgr->IsIconic())
		{
			pTabMgr->ShowWindow(SW_SHOWNORMAL);			
		}
		else if (bActive)
		{
			::SetWindowPos(pTabMgr->m_hWnd,HWND_TOP,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);	
		}
		else
		{
			::SetWindowPos(pTabMgr->m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);	
		}
	}
	else
	{
		//if the tab is new-created, the chat must be a new-created chat.
		//and do not care whether the new-created chat is pop-up, just flashing the task-bar without the table item.
		if (bNewCreate)
		{
			pTabMgr->SetWindowText(info._szNick);
		}
		RECT rcClient = { 0 };
		pTabMgr->GetClientRect(&rcClient);
		InvalidateRect(pTabMgr->m_hWnd, &rcClient, FALSE);
		UpdateWindow(pTabMgr->m_hWnd);
		pTabMgr->DoFlash(pDlg, true, bNewCreate);
	}
	return pDlg;
}

bool  CUIChatMgrBase::ShowStatusDlg(im_dialog_params para)
{
	if (!m_pStatusDlg)
	{
	/*	if (!para.bCreated)
		{
			return false;
		}*/
		m_pStatusDlg = CreateStatusDlgObj();
		if (!m_pStatusDlg)
		{
			return false;
		}
	}
	if (!::IsWindow(m_pStatusDlg->m_hWnd))
	{
	/*	if (!para.bCreated)
		{
			return false;
		}*/
		m_pStatusDlg->Create(para.pParentWnd->m_hWnd);
	}
	if (para.IsWinPosChanging())
	{
		m_pStatusDlg->MoveWindow(&para.rt);
	}
	m_pStatusDlg->ShowWindow(para.nShowCmd);
	return true;
}

bool CUIChatMgrBase::SendCoreMsgDirectly(HWND hWnd, core_msg_header * pHeader)
{
	return ::SendMessage(hWnd, WM_COREMESSAGE, 0, (LPARAM)pHeader);
}

bool CUIChatMgrBase:: LoadConfig(_IM_BASE_CONFIG& settings)
{
	//load some settings about im system.
    settings.bDisableIM				= GetPrivateProfileInt(INI_CFG_CLIENT_GENERAL, INI_CFG_CLIENT_GENERAL_1, 1, GetUserProfilePath().c_str());
	settings.bShowTimeStamp			= GetPrivateProfileInt(INI_CFG_CLIENT_GENERAL, INI_CFG_CLIENT_GENERAL_2, 1, GetUserProfilePath().c_str());
	settings.bMultiTabMgr			= GetPrivateProfileInt(INI_CFG_CHAT, INI_CFG_CHAT_OPEN_IN_NEW_WND, 0, GetUserProfilePath().c_str());
	settings.bDisableFlashWindow	= GetPrivateProfileInt(INI_CFG_CHAT, INI_CFG_CHAT_DISFLASH_NEW_MSG, 0, GetUserProfilePath().c_str());
	settings.bChatMsgNotify			= GetPrivateProfileInt(INI_CFG_NOTIFY, INI_CFG_NOTIFY_CHAT_MESSAGE, 1, GetUserProfilePath().c_str());
	return true;
}

bool CUIChatMgrBase::InitIMMessageWnd()
{
	m_pMessageWnd = CreateMessageWndObj();
	if (m_pMessageWnd)
	{
		m_pMessageWnd->Create(HWND_MESSAGE);
		if (m_pMessageWnd->IsWindow())
		{
			return true;
		}
		else
		{
			delete m_pMessageWnd;
		}
	}

	return false;
}

bool  CUIChatMgrBase::ShowFriendDlg(im_dialog_params para) 
{
	if (!m_pFriendDlg)
	{
		/*if (!para.bCreated)
		{
			return false;
		}*/
		m_pFriendDlg = CreateFriendDlgObj();
		if (!m_pFriendDlg)
		{
			return false;
		}
	}

	if (!::IsWindow(m_pFriendDlg->m_hWnd))
	{
	/*	if (!para.bCreated)
		{
			return false;
		}*/
		HWND hWnd = para.pParentWnd != NULL ? para.pParentWnd->m_hWnd : NULL;
		m_pFriendDlg->Create(hWnd);
	}
	if (para.IsWinPosChanging())
	{
		m_pFriendDlg->MoveWindow(&para.rt);
	}
	m_pFriendDlg->ShowWindow(para.nShowCmd);
	return true;
};

CUIChatDlgBase* CUIChatMgrBase::GetChatDailog(_tstring szUser)
{
	m_lockChatDlgMap.lock();
	CUIChatDlgBase* pItem = NULL;
	if (m_ChatDlgMap.find(szUser) != m_ChatDlgMap.end())
	{
		pItem = m_ChatDlgMap[szUser];
	}
	m_lockChatDlgMap.unlock();
	return pItem;
}

bool CUIChatMgrBase::RemoveChatDailog(_tstring szUser)
{
	m_lockChatDlgMap.lock();
	if (m_ChatDlgMap.find(szUser) != m_ChatDlgMap.end())
	{
        m_ChatDlgMap.erase(szUser);
	}
	m_lockChatDlgMap.unlock();
	return true;
}

CUITabMgrDlg* CUIChatMgrBase::CreateTabMgrDlg(im_dialog_params para)
{
	CUITabMgrDlg* pTabMgr = CreateTabMgrDlgObj();
	SetTabIcon(pTabMgr);

	HWND hWnd = para.pParentWnd != NULL ? para.pParentWnd->m_hWnd : NULL;
	pTabMgr->Create(hWnd);
	if (para.IsWinPosChanging())
	{
		//pTabMgr->MoveWindow(&para.rt);
		::SetWindowPos(pTabMgr->m_hWnd, HWND_TOP, para.rt.left, para.rt.top, para.rt.right-para.rt.left, para.rt.bottom-para.rt.top, SWP_NOACTIVATE | SWP_HIDEWINDOW);
	}

	m_vecTabMgr.push_back(pTabMgr);
   return pTabMgr;
}

CUITabMgrDlg* CUIChatMgrBase::CreateTabMgrDlgDirectly()
{
	im_dialog_params para = GetDefaultTabMgrParams();
	if (m_vecTabMgr.size()>0)
	{
		RECT rtWorkArea;
		if (para.pParentWnd == NULL)
		{
			::GetDesktopWorkingRect(rtWorkArea);
		}
		else
		{
			::GetWindowRect(para.pParentWnd->m_hWnd,&rtWorkArea);
		}
		if ((m_rtTabMgr.bottom+30 > rtWorkArea.bottom) || (m_rtTabMgr.right+30 > rtWorkArea.right))
		{
			para.rt.left = rtWorkArea.left + 100;
			para.rt.top = rtWorkArea.top + 100;
			para.rt.right = para.rt.left+ m_rtTabMgr.right-m_rtTabMgr.left;
			para.rt.bottom = para.rt.top+ m_rtTabMgr.bottom- m_rtTabMgr.top;
		}
		else
		{
			para.rt.left = m_rtTabMgr.left+30;
			para.rt.top = m_rtTabMgr.top+30;
			para.rt.right = para.rt.left+ m_rtTabMgr.right-m_rtTabMgr.left;
			para.rt.bottom = para.rt.top+ m_rtTabMgr.bottom- m_rtTabMgr.top;
		}
	}
	m_rtTabMgr = para.rt;
    return CreateTabMgrDlg(para);
}

CUITabMgrDlg* CUIChatMgrBase::GetChatDlgTabMgr(CUIChatDlgBase *pChat)
{
	if (!pChat)
	{
		return NULL;
	}
    CUITabMgrDlg* pMgr=NULL;
	
	for (int i=0;i<m_vecTabMgr.size();i++)
	{
		if (m_vecTabMgr[i]->m_hWnd == pChat->GetParent().m_hWnd)
		{
            pMgr= m_vecTabMgr[i];
			break;
		}
	}
    return pMgr;
}
CUITabMgrDlg*  CUIChatMgrBase::GetTabMgrDlg(int nIndex)
{
	if (nIndex < 0 || nIndex > ( (int)(m_vecTabMgr.size()) - 1) )
	{
		return NULL;
	}
	return m_vecTabMgr[nIndex];
}

CUITabMgrDlg* CUIChatMgrBase::GetTabMgrDlg(ISonicTab *pTab)
{
	if (!pTab) return NULL;

	for (std::vector<CUITabMgrDlg*>::iterator itr = m_vecTabMgr.begin();
		 itr != m_vecTabMgr.end();
		 ++itr)
	{
		CUITabMgrDlg *pDlg = *itr;
		if (pDlg && pDlg->IsWindow() && pDlg->GetTabCtrl() == pTab)
		{
			return pDlg;
		}
	}

	return NULL;
}

CUITabMgrDlg* CUIChatMgrBase::GetBestTabMgrDlg(bool& bNewCreate)
{
	bNewCreate = false;
	CUITabMgrDlg* pTabMgr = GetCurTabMgrDlg();
	if (!pTabMgr)
	{
		pTabMgr = CreateTabMgrDlgDirectly();
		SetCurTabMgrDlg(pTabMgr);
		bNewCreate = true;
	}
	return pTabMgr;
}

CUITabMgrDlg*  CUIChatMgrBase::GetCurTabMgrDlg()  
{
	_IM_BASE_CONFIG settings;
	LoadConfig(settings);

	if (settings.bMultiTabMgr)
	{
		return NULL;
	}
	if (m_pCurTabMgrDlg== NULL && m_vecTabMgr.size()>0)
	{
		int i=0;
		for (int i=0;i<m_vecTabMgr.size();i++)
		{
			if (m_vecTabMgr[i]->m_hWnd == ::GetFocus())
			{
				break;
			}
		}
		if (i == m_vecTabMgr.size())
		{
			i--;
		}
		m_pCurTabMgrDlg = m_vecTabMgr[i];

	}
	return m_pCurTabMgrDlg;
};

bool CUIChatMgrBase::SetCurTabMgrDlg(CUITabMgrDlg* pTabMgr)
{
	_IM_BASE_CONFIG settings;
	LoadConfig(settings);

	if (settings.bMultiTabMgr)
	{
		//m_pCurTabMgrDlg = NULL;
		return false;
	}

	m_pCurTabMgrDlg= pTabMgr; 
	return true;
};

bool CUIChatMgrBase::SetCurTabMgrDlg(int nIndex)
{
	if (nIndex < 0 || nIndex > ( (int)(m_vecTabMgr.size()) - 1) )
	{
		return false;
	}
	SetCurTabMgrDlg(m_vecTabMgr[nIndex]);
	return true;
}
bool CUIChatMgrBase::RemoveTabMgrDlg(CUITabMgrDlg* pTabMgr)
{
	if (pTabMgr == m_pCurTabMgrDlg)
	{
		m_pCurTabMgrDlg = NULL;
	}
	std::vector<CUITabMgrDlg*>::iterator iter = m_vecTabMgr.begin();
	for (;iter != m_vecTabMgr.end();iter++)
	{
		if (*iter == pTabMgr)
		{
			m_vecTabMgr.erase(iter);
			return true;
		}
	}
	return false;
}


BOOL CUIChatMgrBase::OnDrop(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem)
{
	CUIFriendDlgBase *pFriendDlg = GetFriendDlg();
	if (pFriendDlg == NULL)
		return FALSE;

    ISonicTab* pTab = dynamic_cast<ISonicTab*>(pDragFrom);
	CUITabMgrDlg* pFromTabMgr = GetTabMgrDlg(pTab);
	if (!pFromTabMgr || !pFromTabMgr->IsWindow())
	{
		return FALSE;
	}

	if (pDragFrom == pDragTo)
	{
		pFromTabMgr->PostMessage(WM_TABMGR_RESIZE, (WPARAM)0, (LPARAM)0);
		pFromTabMgr->ReorderListWndItems();
		return TRUE;
	}

	CUITabMgrDlg* pToTabMgr = GetTabMgrDlg(dynamic_cast<ISonicTab*>(pDragTo));

	if (pDragTo == NULL)
	{
		im_dialog_params para = GetDefaultTabMgrParams();
		POINT pt;
		GetSonicUI()->GetCursorPosInWindow(pFriendDlg->m_hWnd,&pt);
		ClientToScreen(pFriendDlg->m_hWnd,&pt);
	
		//the point must be in the item of the tab, so it need reset the point for the tab-mgr-dialog to move/create in
		//Offset to the left in 5 pix
		pt.x = pt.x-5-pFromTabMgr->GetCurTableWidth()/2;
		pt.y = pt.y-36-16;//Offset to the top in 36+32/2 pix

		para.rt.right = pt.x + para.rt.right - para.rt.left;;
		para.rt.bottom = pt.y + para.rt.bottom - para.rt.top;
        para.rt.left = pt.x;
		para.rt.top = pt.y;
		

		//if just has one item, only move the dialog holding the table;
		if (pTab->GetTabCount() == 0)
		{
			pFromTabMgr->MoveWindow(&para.rt);
			return FALSE;
		}
	
		pToTabMgr = CreateTabMgrDlg(para);
		if (pToTabMgr && pToTabMgr->GetTabCtrl())
		{
			//operator the to-tab-mgr-dialog
			CUIChatDlgBase *pChat = GetChatDlgByHandle(pTab->GetTabWnd(pDraggingItem));
			if (pChat && pChat->IsWindow())	
			{
				::SetParent(pChat->m_hWnd, pToTabMgr->m_hWnd);
				LPCTSTR lpItemText = pChat->GetBaseInfo()._szNick;
				pToTabMgr->AddTab(pTab->GetTabWnd(pDraggingItem), lpItemText);

				pToTabMgr->PostMessage(WM_TABMGR_RESIZE, (WPARAM)0, (LPARAM)0);
				//pToTabMgr->ShowWindow(SW_SHOW);
				ShowWindow(pToTabMgr->m_hWnd,SW_SHOW);
			}

			//operator the from-tab-mgr-dialog
		}
	}
	else
	{
		if (pToTabMgr && pToTabMgr->IsWindow())
		{
			pToTabMgr->PostMessage(WM_TABMGR_RESIZE, (WPARAM)0, (LPARAM)0);
			pToTabMgr->ReorderListWndItems();
		}
	}

	if (pTab->GetTabCount() > 0)
	{
		pFromTabMgr->PostMessage(WM_TABMGR_RESIZE, (WPARAM)0, (LPARAM)0);
		pFromTabMgr->ReorderListWndItems();
	}
    else if (pTab->GetTabCount() == 0)
    {
		::PostMessage(pFromTabMgr->m_hWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
    }
	
	return TRUE;
}

void CUIChatMgrBase::ShowTabSel(CUIChatDlgBase *pChat, UINT bShowTab /*= SW_SHOW*/)
{
	for (size_t i = 0; i < m_vecTabMgr.size(); ++i)
	{
		CUITabMgrDlg *pTab = m_vecTabMgr[i];
		if (!pTab || !pTab->IsWindow() || !pTab->GetTabCtrl() || !pTab->GetTabCtrl()->IsValid()) return;

		int nIndex = pTab->GetChatIndexInTab(pChat);
		if (nIndex != -1)
		{
			pTab->GetTabCtrl()->SetSel(nIndex);
			pTab->GetTabCtrl()->SetTabVisible(nIndex);
			pTab->GetTabCtrl()->Show(TRUE);
			return;
		}
	}
}

void CUIChatMgrBase::DestroyChats()
{
	while (m_vecTabMgr.size() > 0)
	{
		CUITabMgrDlg *pDlg = m_vecTabMgr[0];
		if (pDlg != NULL && pDlg->IsWindow())
		{
			pDlg->DestroyWindow();
		}
	}
	WIN_DESTROY(m_pFriendDlg);
	WIN_DESTROY(m_pStatusDlg);
}

CUIChatDlgBase* CUIChatMgrBase::GetChatDlgByHandle(HWND hWnd)
{
	std::map<_tstring,CUIChatDlgBase*>::iterator itr = m_ChatDlgMap.begin();
	for (; itr != m_ChatDlgMap.end(); ++itr)
	{
		if (NULL == itr->second || !(itr->second->IsWindow()))
		{
			continue;
		}
		if (itr->second->m_hWnd == hWnd)
		{
			return itr->second;
		}
	}
	return NULL;
}

CUITabMgrDlg* CUIChatMgrBase::GetTabMgrDlgByHandle(HWND hWnd)
{
	for (std::vector<CUITabMgrDlg*>::iterator itr = m_vecTabMgr.begin();
		 itr != m_vecTabMgr.end();
		 ++itr)
	{
		CUITabMgrDlg *pTabMgr = *itr;
		if (pTabMgr && pTabMgr->IsWindow() && pTabMgr->m_hWnd == hWnd)
		{
			return pTabMgr;
		}
	}
	return NULL;
}

void CUIChatMgrBase::HandleSelfOffline()
{
	std::map<_tstring,CUIChatDlgBase*>::iterator itr = m_ChatDlgMap.begin();
	for (; itr != m_ChatDlgMap.end(); ++itr)
	{
		if (itr->second->GetBaseInfo().iType < ArcPresenceType::XA)
		{
			itr->second->UpdateAvatar((UINT)ArcPresenceType::Unavailable);
		}
	}
}

void CUIChatMgrBase::HandleUserOffline( _tstring tsUser )
{
	std::map<_tstring,CUIChatDlgBase*>::iterator itr = m_ChatDlgMap.find(tsUser);
	if (itr != m_ChatDlgMap.end())
	{
		if (itr->second->GetBaseInfo().iType < ArcPresenceType::XA)
		{
			itr->second->UpdateAvatar((UINT)ArcPresenceType::Unavailable);
		}
	}
}

int CUIChatMgrBase::GetSefPresence()
{
	if(m_pFriendDlg != NULL && m_pFriendDlg->IsWindow()) 
		return m_pFriendDlg->m_nCurPressence;
	else
		return (int)ArcPresenceType::Unavailable;
}

