#pragma once
#include <map>
#include <list>
#include <Windows.h>

#include "atltime.h"

#include "ui/ICoreUI.h"
#include "ui/CoreWidget.h"
#include "ui/CoreEdit.h"

#include "constant.h"
#include "thread/WQueue.h"

#define WIN_DESTROY(pWin) if(pWin && ::IsWindow(pWin->m_hWnd)){ pWin->DestroyWindow();pWin = NULL; }

//tab item width control
#define CHAT_TAB_MIN_LIMIT		84
#define CHAT_TAB_MAX_LIMIT		130

//tab mgr dlg message
#define WM_TABMGR_RESIZE				WM_USER + 5001
#define WM_TABMGR_SHOWSEL				WM_USER + 5002
#define WM_FRLIST_SHOW_CHAT				WM_USER + 5010
#define WM_INITFRIENDLIST				WM_USER + 5030
#define WM_INITFRIENDLIST_IN_OVERLAY	WM_USER + 5031
#define WM_UPDATE_LOADED_AVATAR			WM_USER + 5032
#define WM_UPDATE_REFRESH_PRESENCE		WM_USER + 5033

//chat dlg message
#define WM_SETINPUT_EDIT_FOCUS	WM_USER + 5101


#include "im_constant.h"

using namespace String;

class CUIChatMgrBase;

class CUIFriendDlgBase: public CDialogImpl<CUIFriendDlgBase>, public Thread
{
	//pair<nickname,AvatarImage>
	typedef	std::pair<_tstring, ISonicImage*>	_PairFriendImg;
	typedef std::map<_tstring, ISonicImage*>	_MapFriendImg;
	typedef _MapFriendImg::iterator				_MapFriendImgItr;
	typedef _MapFriendImg::const_iterator		_MapFriendImgCItr;

public:
	CUIFriendDlgBase(CUIChatMgrBase* pMgr,CWindow* pParent = NULL);
	virtual ~CUIFriendDlgBase();

public:
	enum { IDD = IDD_FRIEND_DLG };

	// message map and handlers
	BEGIN_MSG_MAP(CUIFriendDlgBase)
		MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
		MESSAGE_HANDLER(WM_FRLIST_SHOW_CHAT, OnShowChat)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_INITFRIENDLIST, OnInitFriendList)
		MESSAGE_HANDLER(WM_INITFRIENDLIST_IN_OVERLAY, OnInitFriendListInOverlay)
		MESSAGE_HANDLER(WM_UPDATE_LOADED_AVATAR, OnUpdateLoadedAvatar)
		MESSAGE_HANDLER(WM_UPDATE_REFRESH_PRESENCE, OnUpdateRefreshPresence)
		COMMAND_HANDLER(IDC_RICHEDIT_FRIEND_SEARCH, EN_SETFOCUS, OnEnSetfocusRicheditFriendSearch)
		COMMAND_HANDLER(IDC_RICHEDIT_FRIEND_SEARCH, EN_KILLFOCUS, OnEnKillfocusRicheditFriendSearch)
		COMMAND_HANDLER(IDC_RICHEDIT_FRIEND_SEARCH, EN_CHANGE, OnEnChangeRicheditFriendSearch)
	END_MSG_MAP()

public:
	//Init common objs by follow function
	int EndInit();
	void ClearAllItems(bool bClearFriendNum = true);
	int GetFriendItemCount(){ 
		if (m_pTree) { 
			return m_pTree->GetItemCount() - 2; 
		}else {
			return 0;
		}
	}
	int GetVisibleFriendItemCount(){
		if (m_pTree) {
			return m_pTree->GetChildItemCount(m_pTree->GetItem(0), TRUE) 
				+ m_pTree->GetChildItemCount(m_pTree->GetItem(1), TRUE);
		}else {
			return 0;
		}
	}

	void KillLoginTimer();


/********************************************************/
/*					virtual functions					*/
/********************************************************/
public:
	virtual void HandleFriendPresence(_tstring sUserName, ArcPresenceType presence){}
	virtual void HandleSelfPresence(ArcPresenceType presence){}
	virtual void HandleSelfAvatar(_tstring lpPath){}
	virtual void HandleFriendAvatar(_tstring sUserName, LPCTSTR lpPath){}

public:
	virtual void StartSearch(_tstring szSearchText = _T(""));
    virtual void ShowChatDlg(_ITEM_BASE_INFO info) {};

public:
	virtual void InitFriendList(){}
	virtual void AddFriendClick(ISonicString *, LPVOID pReserve){}
	virtual void DetectionFriends(){}
	virtual void UpdateFriendListHeaderNum(bool bOnLine){}

public:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		// override to do something, if needed
		delete this;
	}

public:
	virtual long run();

//virtual functions end
/********************************************************/
/*						set functions					*/
/********************************************************/
public:
	inline void SetListBoxDefaultItemHeight(unsigned int uHeight)
	{
		m_uDefaultItemHeight = uHeight;
	}

	inline void SetListBoxNickNameFormatType(LPCTSTR lpType)
	{
		m_tsFLItemNickNameFormatType = lpType;
	}

	inline void SetListBoxAvatarFormatType(LPCTSTR lpType)
	{
		m_tsFLItemAvatarFormatType = lpType;
	}

	inline void SetListBoxNickNameColor(COLORREF color)
	{
		m_colFLItemNickNameColor = color;
	}

	inline void SetListBoxNickNamePos(int x, int y)
	{
		m_nFLItemNickNameX = x;
		m_nFLItemNickNameY = y;
	}

	inline void SetListBoxAvatarPos(int x, int y)
	{
		m_nFLItemAvatarX = x;
		m_nFLItemAvatarY = y;
	}

	inline void SetListBoxStatusImagePos(int x, int y)
	{
		m_nFLItemStatusImageX = x;
		m_nFLItemStatusImageY = y;
	}

	inline void SetSearchDefaultText(LPCTSTR lpText)
	{
		m_tsSearchDefaultText = lpText;
	}

//set functions end
/********************************************************/
/*						get functions					*/
/********************************************************/
public:
	bool IsSearching(){ return m_bDoSearch; }
	bool IsForcing(){ return m_bForcing; }
	bool IsSearchBarFocused(){ return m_bSearchBarFocused; }
	bool IsInitFriendListFinished(){ return m_bInitFriendListFinished; }
	ArcPresenceType GetCurPresence(){ return m_nCurPressence; }

//get functions end
/********************************************************/
/*					message functions					*/
/********************************************************/
protected:
	LRESULT OnEnChangeRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnEnSetfocusRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnEnKillfocusRicheditFriendSearch(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnShowChat(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitFriendList(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitFriendListInOverlay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnUpdateLoadedAvatar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnUpdateRefreshPresence(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	
	void OnClickBtnSearchQuit(ISonicString *, LPVOID pReserve);
	void OnAddFriendClick(ISonicString *, LPVOID pReserve);
	void OnDBLClickFriendItem(ISonicAnimation *pItem, LPVOID pReserve);

//message functions end
/********************************************************/
/*					internal functions					*/
/********************************************************/
protected:
	bool InsertFriends(CHAT_FRIEND* friends, int nLen, LPCTSTR lpcSearch = NULL);
	bool InsertFriendItemEx(ITEM_BASE_INFO &info, bool bSearch = false, unsigned int uItemHeight = 0);

	ISonicAnimation *GetFriendItemByUserName(_tstring tsUserName);
	int GetFriendItemIndexByUserName(_tstring tsUserName, bool bOnLine);
	int GetFriendItemSortedIndexByUserName(_tstring tsUserName, int iStatus);
	void DeleteFriendItem(LPCTSTR lpcNickName);
	void UpdateFriendPresence(_tstring tsUserName, int iStatus);
	void UpdateFriendAvatar(_tstring tsUserName, LPCTSTR lpPath);
	bool CheckItemSatisfySearchCondtion(_tstring tsNickName);

	void DoSearch(BOOL bForcing=FALSE);

private:
	bool IsAvatarExist(_tstring tsUserName);
	bool InsertAvatar(_PairFriendImg pair);

//internal functions end
/********************************************************/
/*						member Variable					*/
/********************************************************/
//controls
protected:
	WTL::CCoreEdit*     m_pSearchBar;
	ISonicString*		m_pSearchQuit;
	ISonicString*		m_pSearchFailure;
	ISonicTreeView*		m_pTree;
	ISonicImage*		m_pDefaultAvatar;

protected:
	ISonicString*		m_pOnLineString;
	ISonicString*		m_pOffLineString;

//chat manager
protected:
	CUIChatMgrBase*		m_pUIChatMgr;

	unsigned int		m_uDefaultItemHeight;

//tree about
protected:
	_tstring			m_tsFLItemNickNameFormatType;
	_tstring			m_tsFLItemAvatarFormatType;
	COLORREF			m_colFLItemNickNameColor;

	int					m_nFLItemNickNameX;
	int					m_nFLItemNickNameY;
	int					m_nFLItemAvatarX;
	int					m_nFLItemAvatarY;
	int					m_nFLItemStatusImageX;
	int					m_nFLItemStatusImageY;
	int					m_nOnLineVisibleItemNum;
	int					m_nOffLineVisibleItemNum;

	bool volatile		m_bInitFriendListFinished;

	_MapFriendImg		m_mapAvatar;

//search about
protected:
	_tstring			m_tsSearchDefaultText;
	_tstring			m_tsLastSearchText;
	int					m_nSearchCount;
	bool				m_bFirstSearch;
	bool				m_bDoSearch;
	bool				m_bForcing;
	bool				m_bSearchBarFocused;

public:
	ArcPresenceType		m_nCurPressence;

//load pictures
protected:
	WQueue<_tstring> m_wqPresenceFriends;
	WQueue<_tstring> m_wqAvatarFriends;
	bool volatile m_bExitWorkThread;

};
#include "im/TabListWnd.h"

class CUIChatDlgBase;
class CUITabMgrDlg: public CDialogImpl<CUITabMgrDlg>,public TabListImplBase
{
public:
	enum{IDD = IDD_TAB_DLG};
	enum _EFS_TYPE
	{
		EFS_TYPE_NONE		= 0,
		EFS_TYPE_TASKBAR	= 1,
		EFS_TYPE_TABITEM	= 2,
		EFS_TYPE_ALL		= 3,
	};

	CUITabMgrDlg(CUIChatMgrBase* pMgr);
	virtual~CUITabMgrDlg();
	BEGIN_MSG_MAP(CUITabMgrDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_TABMGR_RESIZE, OnResizeTabItem)
		MESSAGE_HANDLER(WM_TABMGR_SHOWSEL, OnShowSel)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnResizeTabItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowSel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDeleteItem(ISonicBase *, LPVOID pReserve);
	void OnSize(UINT nType, WTL::CSize size);
    void OnSelItemChanged(ISonicBase *, LPVOID);

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		// override to do something, if needed
		delete this;
	}

public:
	ISonicTab* GetTabCtrl(){return m_pTabCtrl;};
	int GetChatIndexInTab(CUIChatDlgBase *pChat);//return -1 means not in tab, other is the index
	CUIChatDlgBase * GetChatDlgByIndex(int nIndex);

	void AddTab(HWND hDlg, LPCTSTR lpszTabText, int nIndex = -1);
	void SetSelTab (int nIndex,HWND hWnd = NULL);
	CTabListWnd * GetListWnd(){return &m_ListWnd;} 
	void DeleteListItem(int nIndex);
	bool IsChatFlashing(CUIChatDlgBase *pChat);

	void DoFlash(CUIChatDlgBase *pChat, bool bEable = true, bool bJustTaskBar = false);
    void DoFlash(int nIndex, bool bEable = true, bool bJustTaskBar = false);
	virtual	void ShowListWnd(BOOL bShow);
	ISonicString * GetListBar(){return m_pListbar;}
	CUIChatMgrBase* GetChatMgr(){return m_pMgr;}

public:
	BOOL SetTabText(int nIndex, LPCTSTR lpItem);
	BOOL SetTabItemFlash(int nIndex, bool bFlashing);
	void SetTabIcon(HICON hIcon){ m_hIcon = hIcon; }
	void ResizeTabItems();
	void ReorderListWndItems();
	int GetCurTableWidth(){ return m_nCurTabItemWidth; }

private:
	void OnClickExit(ISonicString *, LPVOID pReserve);
	void OnClickMin(ISonicString *, LPVOID pReserve);
	void OnShowList(ISonicBase *,LPVOID);
	void DrawTab(ISonicPaint * pPaint, LPVOID);

private:
	ISonicTab* m_pTabCtrl;
	int m_nCurTabItemWidth;
	int m_nCurSelIndex;
	std::map<CUIChatDlgBase*, int> m_mapChatFlashing;

    CUIChatMgrBase* m_pMgr;

	CTabListWnd m_ListWnd;
	ISonicString *m_pListbar;

	HICON m_hIcon;
};

class CUIChatDlgBase: public CDialogImpl<CUIChatDlgBase>,public ISonicInputFilter
{
public:
	enum
	{
		IDD = IDD_CHAT_DLG,
		CHAT_MAX_INPUT_CHARACTERS = 220,
		LINE_SPACE = 280,
        TIMER_NID_SET_SPAMMING=1,
	};
	CUIChatDlgBase(CUIChatMgrBase* pMgr,_ITEM_BASE_INFO info);
	virtual ~CUIChatDlgBase(){};
	
	// message map and handlers
	
	BEGIN_MSG_MAP(CUIChatDlgBase)
      //  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETINPUT_EDIT_FOCUS, OnSetInputEditFocus)
		COMMAND_HANDLER(IDC_EDIT_CHATDLG_INPUT,EN_CHANGE,OnEnChangeEditChatdlgInput)
		COMMAND_HANDLER(IDC_EDIT_CHATDLG_INPUT,EN_SETFOCUS,OnEnInputFocus)
		COMMAND_HANDLER(IDC_EDIT_CHATDLG_OUTPUT,EN_SETFOCUS,OnEnOutputFocus)
	END_MSG_MAP()

//child control event handler
	LRESULT OnEnChangeEditChatdlgInput(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnEnInputFocus(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnEnOutputFocus(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);
	LRESULT OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetInputEditFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    virtual BOOL OnInputMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		// override to do something, if needed
		GetSonicUI()->RemoveInputFilter(this);
		delete this;
	}
//common functions.

public:
	virtual void ClearLog(LPCTSTR lpUserName){}
public:
	void UpdatePresence(UINT iStatus,BOOL bFirst =TRUE);
	void UpdateAvatar(UINT nStatus);
	void UpdateAvatar(ISonicImage* pImg,UINT nStatus);
	void HandleRecvMsg(const MSG_LOG_ELEMENT &msg, bool bOfflineMessage = false);
	void SendChatMsg();
	void SyncSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId);
	void ShowSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId);
    void UpdateXmppStatus(UINT iStatus,BOOL bFirst =TRUE);
    void ShowHistoryRecords();
	//void AttachTabMgr(CUITabMgrDlg* pTabMgr);

public:
	const _ITEM_BASE_INFO& GetBaseInfo(){ return m_info; }
	WTL::CCoreRichEdit* GetInputEdit(){ return m_pEditInput; }

public:
	void OnStaticClickedChatClear(ISonicString *, LPVOID pReserve);
	void OnDelegateClickedChatFriend(ISonicString *, LPVOID pReserve);

protected:
	virtual void ClickFriend(){}

protected:
	CUIChatMgrBase* m_pUIChatMgr;
    //CUITabMgrDlg* m_pTabMgr;
    _ITEM_BASE_INFO m_info;
	WTL::CCoreRichEdit* m_pEditInput;
	WTL::CCoreRichEdit* m_pEditOutPut;
	ISonicImage*        m_pAvatar;
	ISonicPaint*        m_pAvatarPaint;
	bool				m_bSpamming;
	bool				m_bFirstOfflineMessageComing;
	std::list<time_t>	m_listTm;
	_tstring			m_tsLastMsgDate;
};	


class CUIStatusDlgBase: public CDialogImpl<CUIStatusDlgBase>
{
public:
	CUIStatusDlgBase(CUIChatMgrBase* pMgr);
	virtual ~CUIStatusDlgBase();
public:
	enum { IDD = IDD_PRESENCE_DLG };

	// message map and handlers
	BEGIN_MSG_MAP(CUIStatusDlgBase)
		MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
        COMMAND_HANDLER(IDC_LISTBOX_SET_PRESENCE,LBN_SELCHANGE,OnItemClick)
	END_MSG_MAP()
	LRESULT OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSetPresence(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnItemClick(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL & bHandled);

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		// override to do something, if needed
		delete this;
	}

public:
	WTL::CCoreListBox	m_LBSetting;
	ISonicWndEffect*	m_pWndEffect;
	CUIChatMgrBase*		m_pUIChatMgr;
	
};

class CUIChatMgrBase: public ICoreMsgCallback,public IDragDropCallback
{
public:
	CUIChatMgrBase();
	virtual ~CUIChatMgrBase();
public:
	virtual void		HandleCopyDateMessage(core_msg_header *pHeader)								{}
	virtual void		OnCoreMessage(HWND hFrom, core_msg_header * pHeader);
	virtual bool		SendChatMessage(MSG_LOG_ELEMENT msgEelement, bool bOffline = false)			{ return true; }
	virtual bool		SetPresence(int iType)														{ return true; }
	virtual int			GetPresence(_tstring szUser)												{ return ArcPresenceType::Unavailable; }
	virtual bool		LoadHistoryRecord(_tstring szUser,UINT nCount,std::list<MSG_LOG_ELEMENT>& RecordList)	{ RecordList.clear(); return true; }
	virtual bool		ClearHistroyRecord(_tstring szUser)											{ return false; }
	virtual BOOL		SendCoreMsg(HWND hWnd, core_msg_header * pHeader)							{ return false; }
	virtual bool		InitChatSystem()															{ return false; }
    virtual bool		UnInitChatSystem()															{ return false; }
	virtual bool		GetFriendInfo(_tstring szUser, LPITEM_BASE_INFO lpInfo, bool bByNick = false)	{ return false; }
	virtual bool		GetSelfUserInfo(LPITEM_BASE_INFO lpInfo)									{ return false; }
	virtual _tstring	GetStringFromId( LPCTSTR  lpszStr)											{ return _tstring(); }
	virtual _tstring	GetUserProfilePath()														{ return _tstring(); }
	virtual _tstring	GetBaseDir()																{ return _tstring(); }
	virtual __int64		GetTime()																	{ return 0; }
	virtual void		SetLoginStatus(int loginStatus)												{}
	virtual int			GetSefPresence();
	virtual int			GetFriendPresence(LPCTSTR lpUserName)										{ ITEM_BASE_INFO info; if (GetFriendInfo(lpUserName, &info)) return info.iType; }
	virtual int			GetFriendsCount()															{ return 0; }
public:
	virtual CUIFriendDlgBase*	CreateFriendDlgObj()												{ return NULL; }
	virtual CUIChatDlgBase*		CreateChatDlgObj(_ITEM_BASE_INFO info)								{ return NULL; }
	virtual CUIStatusDlgBase*	CreateStatusDlgObj()												{ return NULL; }
	virtual CUITabMgrDlg*		CreateTabMgrDlgObj()												{ return new CUITabMgrDlg(this); }
	virtual im_dialog_params	GetDefaultTabMgrParams()											{ return im_dialog_params(); }
	virtual bool				IsForegroundWndBelongMe(HWND hWnd)									{ return true; }
	virtual void				SetTabIcon(CUITabMgrDlg* pTab)										{}
	virtual void				ShowWindow( HWND hWnd,  int nCmdShow)								{ ::ShowWindow(hWnd,nCmdShow); }
	virtual void				ShowListWindow(CCoreListBox *pList,int nCmdShow)					{ if (pList) pList->ShowWindow(nCmdShow); }

public:
	virtual bool				ShowFriendDlg(im_dialog_params para);
	virtual CUIChatDlgBase*		ShowChatDlg(_ITEM_BASE_INFO info,im_dialog_params para, bool bNeedShow = true);
	virtual CUIChatDlgBase*		ShowChatDlg(_ITEM_BASE_INFO info, BOOL bOfflineMessage = FALSE, bool bManual =true,bool bShow = true,bool bActive = true);
	virtual bool				ShowStatusDlg(im_dialog_params para);
	virtual void				ShowCursorItemOnInputEdit(CUIChatDlgBase *pChat)					{}
	virtual BOOL				OnDrop(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem);

public:
	virtual void SyncSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId){}

public:
	CUITabMgrDlg*   CreateTabMgrDlg(im_dialog_params para);
	CUITabMgrDlg*   CreateTabMgrDlgDirectly();
	CUIChatDlgBase* GetChatDailog(_tstring szUser);
	CUIChatDlgBase* GetChatDlgByHandle(HWND hWnd);
	CUITabMgrDlg*   GetTabMgrDlg(int nIndex);
	CUITabMgrDlg*   GetTabMgrDlg(ISonicTab *pTab);
	CUITabMgrDlg*	GetTabMgrDlgByHandle(HWND hWnd);
	CUIFriendDlgBase* GetFriendDlg() { return (m_pFriendDlg != NULL && ::IsWindow(m_pFriendDlg->m_hWnd)) ? m_pFriendDlg : NULL; }
	CUIStatusDlgBase* GetStatusDlg()  {return m_pStatusDlg;};
	CUITabMgrDlg*    GetBestTabMgrDlg(bool& bNewCreate);
	CUITabMgrDlg*    GetCurTabMgrDlg();
	CUITabMgrDlg*    GetChatDlgTabMgr(CUIChatDlgBase *pChat);
	bool             SetCurTabMgrDlg(CUITabMgrDlg* pTabMgr);
	bool             SetCurTabMgrDlg(int nIndex);
	bool            RemoveTabMgrDlg(CUITabMgrDlg* pTabMgr);
	bool            RemoveChatDailog(_tstring szUser);
	bool            LoadConfig(_IM_BASE_CONFIG& settings);
	ISonicString*   GetStatusBkImg(UINT iStatus,bool bBig);

	bool            SendCoreMsgDirectly(HWND hWnd, core_msg_header * pHeader);
	void			ShowTabSel(CUIChatDlgBase *pChat, UINT nShowTab = SW_SHOW);
	void			DestroyChats();
	void			HandleSelfOffline();
	void			HandleUserOffline(_tstring tsUser);
	bool			IsInGame(){ return m_bIsInGame; }
	void			SetNickName(LPCTSTR lpcNickName) { m_tsNickName = lpcNickName; }
	void			SetUserName(LPCTSTR lpcUserName) { m_tsUserName = lpcUserName; }
	void			SetSelfAvatarPath(LPCTSTR lpcSelfAvatarPath) { m_tsSelfAvatarPath = lpcSelfAvatarPath; }
	LPCTSTR			GetNickName(){ return m_tsNickName.c_str(); }
	LPCTSTR			GetUserName(){ return m_tsUserName.c_str(); }
	LPCTSTR			GetSelfAvatarPath(){ return m_tsSelfAvatarPath.c_str(); }


public:
	virtual void	ShowNewMinimizeTab(HWND hWnd);

protected:
	CUIFriendDlgBase* m_pFriendDlg;
	CUIStatusDlgBase*  m_pStatusDlg;
	CUITabMgrDlg*      m_pCurTabMgrDlg;
	std::map<_tstring,CUIChatDlgBase*> m_ChatDlgMap;
	std::vector<CUITabMgrDlg*> m_vecTabMgr;
	RECT     m_rtTabMgr; 
	Mutex    m_lockChatDlgMap;    
	bool	 m_bIsInGame;
	_tstring m_tsUserName;
	_tstring m_tsNickName;
	_tstring m_tsSelfAvatarPath;
};