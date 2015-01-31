#pragma once

#include "im/im_ui_mgr.h"
#include "im/IMManager.h"

#include "data/SNSManager.h"
#include "data/DataPool.h"


/********************************/
/*	friend list panel constant	*/
/********************************/
#define FLP_SEARCH_TEXT_MAXSIZE			40
#define FLP_LB_LEFT_OS					5
#define FLP_LB_TOP_OS					139
#define FLP_LB_RIGHT_OS					5
#define FLP_LB_BOTTOM_OS				38
#define FLP_SEARCH_FAILURE_COLOR		RGB(238,238,238)
#define FLP_NICKNAME_COLOR				RGB(251,251,251)
#define FLP_TMI_COLOR					RGB(168,168,168)

#define WM_HANDLE_SYNC_SENT_MESSAGE		WM_USER + 6001
#define WM_HANDLE_COPYDATE_MESSAGE		WM_USER + 6002

/********************************/
/*	chat in tab constant		*/
/********************************/
#define CIT_TBS_DEF_COUNT				3
#define CIT_TBS_MAX_COUNT				12
#define CIT_EDIT_SCROLL_BG_COLOR		RGB(15,15,15)


//end of macro definition
//////////////////////////////////////////////////////////////////////////
// arc main chat system,responsible for all actually im logic and support secondary im system.
//////////////////////////////////////////////////////////////////////////
//begin the class

/*
 *@ class CIMMessageWndPrimary
 *@ using for the message center
 */
class CIMMessageWndPrimary : public CIMMessageWndBase
{
public:
	CIMMessageWndPrimary(CUIChatMgrBase* pMgr):CIMMessageWndBase(pMgr){}
	virtual ~CIMMessageWndPrimary(){}
	BEGIN_MSG_MAP(CIMMessageWndPrimary)
		MESSAGE_HANDLER(WM_HANDLE_COPYDATE_MESSAGE, OnHandleCopyDateMessage)
		MESSAGE_HANDLER(WM_HANDLE_SYNC_SENT_MESSAGE, OnHandleSyncSentMessage)
		CHAIN_MSG_MAP(CIMMessageWndBase)
	END_MSG_MAP()

	LRESULT OnHandleCopyDateMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnHandleSyncSentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
};

class CUIChatMgrPrimary : public CUIChatMgrBase,public CIMEventCallBack
{
public:
	CUIChatMgrPrimary();
	virtual ~CUIChatMgrPrimary();	

public://CIMEventCallBack interface
	virtual void HandlePresence(_tstring tsUserName, Presence::PresenceType presence);
	virtual void HandleDivorce(_tstring tsName, _tstring tsNick);
	virtual void HandleSendChatMsgDone(im_msg_send_msg_done msg);
	virtual void HandleRcvChatMsg(MSG_LOG_ELEMENT msgEelement, bool bOfflineMessage = false);
	virtual void HandleAvartarChanged(_tstring sUserName,_tstring szPath);
	virtual void HandleRosterArrive();

public:
	virtual CIMMessageWndBase*	CreateMessageWndObj();
	virtual CUIFriendDlgBase*  CreateFriendDlgObj();
	virtual CUIChatDlgBase*    CreateChatDlgObj(_ITEM_BASE_INFO info);
	virtual CUIStatusDlgBase*  CreateStatusDlgObj();
    virtual im_dialog_params   GetDefaultTabMgrParams();
	virtual void			   SetTabIcon(CUITabMgrDlg* pTab);
public:
	virtual void  OnCoreMessage(HWND hFrom, core_msg_header * pHeader);
	virtual bool  InitChatSystem();
    virtual bool  UnInitChatSystem();
	virtual bool GetFriendInfo(_tstring szUser, LPITEM_BASE_INFO lpInfo, bool bByNick = false);
	virtual bool GetSelfUserInfo(LPITEM_BASE_INFO lpInfo);
    virtual _tstring GetStringFromId( LPCTSTR  lpszStr);
	virtual _tstring GetUserProfilePath();
	virtual _tstring GetBaseDir();
	virtual bool IsForegroundWndBelongMe(HWND hWnd);
	virtual void ShowNewMinimizeTab(HWND hWnd);
	virtual void ShowCursorItemOnInputEdit(CUIChatDlgBase *pChat);

public:
	virtual void SyncSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId);

public:
	CIMManager& GetIMMgr(){return m_IMMgr;};
	virtual bool SendChatMessage(MSG_LOG_ELEMENT msgEelement, bool bOffline = false);

	virtual __int64	GetTime(); 
	virtual void	SetLoginStatus(int loginStatus);
	virtual int		GetSefPresence()						{ return (int)m_IMMgr.GetXmppClient().GetPresence(); }
	virtual int		GetFriendPresence(LPCTSTR lpUserName)	{ return m_IMMgr.GetFriendManager().GetFriendPresenceByName(lpUserName); }
	virtual int		GetFriendsCount()						{ return m_IMMgr.GetFriendManager().GetFriendsCount(); }
	virtual int		GetPresence(_tstring szUser);
	virtual bool	SetPresence(int iType);
	virtual bool	LoadHistoryRecord(_tstring szUser,UINT nCount,std::list<MSG_LOG_ELEMENT>& RecordList);
	virtual bool	ClearHistroyRecord(_tstring szUser);
    virtual BOOL	SendCoreMsg(HWND hWnd, core_msg_header * pHeader);
	virtual void	HandleCopyDateMessage(core_msg_header *pHeader);

public:
	void HandleSyncSentMessage(LPCTSTR lpcSelfName, LPCTSTR lpcContent, LPCTSTR lpcUser, int nType, __int64 nTime, int nSyncId);

private:
	CIMManager		m_IMMgr;
	CCoreMsgServer  m_msgServer;
};


class CUIStatusDlgPrimary: public CUIStatusDlgBase
{
public:
	CUIStatusDlgPrimary(CUIChatMgrPrimary* pMgr);
	virtual ~CUIStatusDlgPrimary();

public:
	BEGIN_MSG_MAP(CUIStatusDlgPrimary)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		CHAIN_MSG_MAP(CUIStatusDlgBase)
	END_MSG_MAP()

protected:
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
};

class CUIFriendDlgPrimary: public CUIFriendDlgBase
{
public:
	CUIFriendDlgPrimary(CUIChatMgrPrimary* pMgr,CWindow* pParent = NULL);
	~CUIFriendDlgPrimary();
	// message map and handlers
	BEGIN_MSG_MAP(CUIFriendDlgPrimary)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_PAINT(OnPaint)
		CHAIN_MSG_MAP(CUIFriendDlgBase)
	END_MSG_MAP()
protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

/********************************************************/
/*					virtual functions					*/
/********************************************************/
protected:
	virtual void HandleSelfAvatar(_tstring lpPath);
	virtual void HandleSelfPresence(ArcPresenceType presence);
	virtual void HandleFriendAvatar(_tstring sUserName, LPCTSTR lpPath);
	virtual void HandleFriendPresence(_tstring sUserName, ArcPresenceType presence);

protected:
	virtual void StartSearch(_tstring szSearchText = _T(""));
	virtual void UpdateFriendListHeaderNum(bool bOnLine);
	virtual void InitFriendList();

protected:
	virtual void AddFriendClick(ISonicString *, LPVOID pReserve);

protected:
	 void OnPaint(HDC);
	 void OnClickBtnSetStatus(ISonicString *, LPVOID pReserve);
	 void OnClickBtnMin(ISonicString *, LPVOID pReserve);
	 void OnClickBtnExit(ISonicString *, LPVOID pReserve);
	 void OnClickedBtnAvatar(ISonicString *, LPVOID pReserve);

public:
	void UpdateAvatarPaint(ISonicImage* pImg, UINT iStatus);
	void UpdateStatus(UINT iStatus);

public:
	void LoadStatusIcon();
	void AddSettingItem(int iStatus);
	void SetStatusDlg(CUIStatusDlgPrimary *pStatus);

private:
	ISonicPaint*				m_pAvatarPaint;
	ISonicImage*				m_pAvatar;
	CUIStatusDlgPrimary*		m_pStatusDlg;
	HICON						m_hIcon;
	//WQueue<_XMPP_STATUS>		m_wTaskQueue;
	std::map<int,ISonicImage*>	m_mapStatusIcon;//pair<istatus,statusIcon>

};


class CUIChatDlgPrimary: public CUIChatDlgBase
{
public:
	CUIChatDlgPrimary(CUIChatMgrPrimary* pMgr,_ITEM_BASE_INFO info);
	virtual ~CUIChatDlgPrimary();

public:
	BEGIN_MSG_MAP(CUIChatDlgPrimary)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CUIChatDlgBase)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

protected:
	virtual void ClearLog(LPCTSTR lpUserName);
	virtual void ClickFriend();
	virtual void SaveChatMessageLog(const MSG_LOG_ELEMENT &msg);

private:
	void UpdateProfile();

private:
	HICON              m_hIcon;

};
