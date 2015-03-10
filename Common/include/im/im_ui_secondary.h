#pragma once
#include "im/im_ui_mgr.h"
//#include "resource.h"

/********************************/
/*	friend list panel constant	*/
/********************************/
#define FLP_SEARCH_TEXT_MAXSIZE			40
#define FLP_LB_LEFT_OS					2
#define FLP_LB_TOP_OS					88
#define FLP_LB_RIGHT_OS					2
#define FLP_LB_BOTTOM_OS				50
#define FLP_SEARCH_FAILURE_COLOR		RGB(238,238,238)
#define FLP_NICKNAME_COLOR				ARGB(255,251,251,251)
#define FLP_TMI_COLOR					RGB(168,168,168)

//////////////////////////////////////////////////////////////////////////
// secondary chat system.
//////////////////////////////////////////////////////////////////////////

class CUIChatMgrSecondary : public CUIChatMgrBase
{
public:
	CUIChatMgrSecondary();
	virtual ~CUIChatMgrSecondary();
public:
	virtual void			OnCoreMessage(HWND hFrom, core_msg_header * pHeader);
    virtual bool			InitChatSystem(HWND hWnd);
    virtual bool			UnInitChatSystem();
	virtual bool			SendChatMessage(MSG_LOG_ELEMENT msgEelement, bool bOffline = false);
	virtual bool			SetPresence(int iType);
	virtual bool			LoadHistoryRecord(_tstring szUser,UINT nCount,std::list<MSG_LOG_ELEMENT>& RecordList) ;
	virtual bool			ClearHistroyRecord(_tstring szUser);
	virtual bool			GetFriendInfo(_tstring szUser, LPITEM_BASE_INFO lpInfo, bool bByNick = false);
	virtual bool			GetSelfUserInfo(LPITEM_BASE_INFO lpInfo);
	virtual int				GetPresence(_tstring szUser);
	virtual BOOL			SendCoreMsg(HWND hWnd, core_msg_header * pHeader);
	virtual _tstring		GetUserProfilePath();
	virtual _tstring		GetBaseDir();
	virtual __int64			GetTime();
	virtual void			SetLoginStatus(int loginStatus);
	virtual int				GetSefPresence();
	virtual int				GetFriendsCount();
	virtual void			SyncSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId);

public:
	virtual CUIFriendDlgBase*	CreateFriendDlgObj();
	virtual CUIChatDlgBase*		CreateChatDlgObj(_ITEM_BASE_INFO info);
	virtual CUIStatusDlgBase*	CreateStatusDlgObj();
	virtual CUITabMgrDlg*		CreateTabMgrDlgObj();

    virtual im_dialog_params	GetDefaultTabMgrParams();
	virtual CUIChatDlgBase*		ShowChatDlg(_ITEM_BASE_INFO info, BOOL bOfflineMessage, bool bManual = true, bool bShow = true, bool bActive = true);
	virtual void				ShowWindow( HWND hWnd,  int nCmdShow);
	virtual void				ShowListWindow(CCoreListBox *pList,int nCmdShow);
	//virtual BOOL OnDrop(ISonicBaseWnd * pDragFrom, ISonicBaseWnd * pDragTo, ISonicAnimation * pDraggingItem);
	
public:
	_tstring	GetStringFromId( LPCTSTR  lpszStr);
	void		ShowTabMgrChatDlg(UINT nShowCmd);
	bool		RequestFriendList();
	bool		GetInitFriendListIsFinished();

protected:
    BOOL SendCoreMsg2Svr(core_msg_header * pHeader);
	
private:
	CCoreMsgClient m_coreMsgClient;
};

class CUIFriendDlgSecondary : public CUIFriendDlgBase
{
public:
	CUIFriendDlgSecondary(CUIChatMgrSecondary* pMgr,CWindow* pParent = NULL);
	~CUIFriendDlgSecondary();
	BEGIN_MSG_MAP(CUIFriendDlgSecondary)
        CHAIN_MSG_MAP(CUIFriendDlgBase)
		MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT(OnPaint)
	END_MSG_MAP()
public:
	LRESULT OnProcessMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
public:
   void HandleFriendListResp(im_msg_friend_list_response * pMsg);
   void AddFriendClick(ISonicString *, LPVOID pReserve);
   virtual _tstring GetBaseDataDir(){ return m_pUIChatMgr->GetBaseDir(); }
   virtual void StartSearch(_tstring szSearchText=_T(""));
   void OnSize(UINT nType, WTL::CSize size);
   void OnPaint(HDC);
   void DetectionFriends();
  // virtual void ShowChatDlg(_ITEM_BASE_INFO info);:

protected:
	virtual void UpdateFriendListHeaderNum(bool bOnLine);
	virtual void InitFriendList();

protected:
	virtual void HandleSelfAvatar(_tstring lpPath);
	virtual void HandleSelfPresence(ArcPresenceType presence);
	virtual void HandleFriendAvatar(_tstring sUserName, LPCTSTR lpPath);
	virtual void HandleFriendPresence(_tstring sUserName, ArcPresenceType presence);


	void OnInviteFriendsClick(ISonicBase *, LPVOID pReserve);
private:
	ISonicString*               m_pInviteFriendTilte;
	ISonicString*               m_pInviteFriendBtn;

private:
	bool m_bHandleSelfPresenceWhenInitIM;

  
};

class CUIChatDlgSecondary : public CUIChatDlgBase
{
public:
	CUIChatDlgSecondary(CUIChatMgrSecondary* pMgr,_ITEM_BASE_INFO info);
	virtual ~CUIChatDlgSecondary();
	BEGIN_MSG_MAP(CUIChatDlgSecondary)
		CHAIN_MSG_MAP(CUIChatDlgBase)
	//	MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	void UpdateProfile();
	void ClickFriend();
	

};

#include <map>

class CUIStatusDlgSecondary : public CUIStatusDlgBase
{
public:
	CUIStatusDlgSecondary(CUIChatMgrSecondary* pMgr);
	virtual ~CUIStatusDlgSecondary();
	BEGIN_MSG_MAP(CUIChatDlgSecondary)
		CHAIN_MSG_MAP(CUIStatusDlgBase)
		//	MESSAGE_HANDLER(WM_COREMESSAGE, OnProcessMsg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()
public:
	void UpdateStatus(UINT iStatus);
	void AddSettingItem(int iStatus);
	void LoadStatusIcon();
protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   void Init();
private:
	std::map<int,ISonicImage*>	m_mapStatusIcon;
	_tstring m_szStatus[9];
};

class CUITabMgrDlgSecondary : public CUITabMgrDlg
{
public:
	CUITabMgrDlgSecondary(CUIChatMgrSecondary* pMgr);
	virtual ~CUITabMgrDlgSecondary();
	BEGIN_MSG_MAP(CUITabMgrDlgSecondary)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		CHAIN_MSG_MAP(CUITabMgrDlg)
	END_MSG_MAP()
protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	void ShowListWnd(BOOL bShow);
};


