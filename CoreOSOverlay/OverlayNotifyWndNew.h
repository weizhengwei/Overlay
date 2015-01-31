#pragma  once

#include "CoreMsgOverlay2Svr.h"
#include "tstring/tstring.h"
#include "datatype.h"
#include "im/im_constant.h"

enum{
	BASE = 0,
	INBOX_MESSAGE,
	FRIEND_REQUEST,
	NOTIFICATION,
	FRIEND_STATUS,
	GAME_READY_PLAY,
	PDL,
	CHAT_MESSAGE,
};

class CTempPopUp;
class ListItemBottomNew{

public:
	ListItemBottomNew(){};
	virtual ~ListItemBottomNew(){};
	
	virtual BOOL InsertItem(CTempPopUp* pDlg, PVOID) = 0;
	virtual int GetClassType() = 0;
	virtual void OnClick(ISonicBase* , LPVOID pReserve) = 0;
};

class ListItemInboxNew : public ListItemBottomNew
{
public:

	ListItemInboxNew() ;
	virtual ~ListItemInboxNew() ;

	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData) ;
	void InitItems(CTempPopUp* pDlg, PVOID psData);
	int GetClassType(){ return INBOX_MESSAGE; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return OnClickBottom(NULL, pReserve); };

protected:
	void OnClickTop(ISonicBase *, LPVOID pReserve) ;
	void OnClickBottom(ISonicBase *, LPVOID pReserve) ;

	ISonicString* m_pIcon;
	ISonicString* m_pTitle;
	ISonicString* m_pContent;
	core_msg_inbox m_sData;
private:
	const static int TITLE_WIDTH    = 250;
	const static int CONTENT_WIDTH  = 280;
	const static int CONTENT_HEIGHT = 30;
};

class ListItemFriReqNew : public ListItemBottomNew
{
public:

	ListItemFriReqNew() ;
	virtual ~ListItemFriReqNew() ;

	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData) ;
	void InitItems(CTempPopUp* pDlg, PVOID psData);
	int GetSenderId(){ return m_sData.nSenderId; }
	int GetClassType(){ return FRIEND_REQUEST; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return; }
protected:
	void OnClickTop(ISonicBase *, LPVOID pReserve) ;
	void OnClickAccept(ISonicBase *, LPVOID pReserve) ;
	void OnClickIgnore(ISonicBase *, LPVOID pReserve) ;
	void OnClickContent(ISonicBase *, LPVOID pReserve);

	ISonicString* m_pTitle ;
	ISonicImage*  m_pImgAvatar ;
	ISonicString* m_pBtnAvatar ;
	ISonicString* m_pTextContent ;
	ISonicString* m_pBtnAccept ;
	ISonicString* m_pBtnCancel ;
	ISonicString* m_pIcon;

	core_msg_friendreq m_sData ;
private:
	const static int TITLE_WIDTH    = 250;
	const static int CONTENT_WIDTH  = 280;
	const static int CONTENT_HEIGHT = 30;
};

class ListItemNotifyNew : public ListItemBottomNew
{
public:

	ListItemNotifyNew() ;
	virtual ~ListItemNotifyNew() ;

	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData) ;
	void InitItems(CTempPopUp* pDlg, PVOID psData);
	int GetClassType(){ return NOTIFICATION; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return OnClickContent(NULL, pReserve); }
protected:
	void OnClickContent(ISonicBase *, LPVOID pReserve) ;
	ISonicString* m_pIcon;
	ISonicString* m_pTextTitle ;
	ISonicString* m_pTextContent ;
	core_msg_notification m_sData ;
private:
	const static int TITLE_WIDTH    = 250;
	const static int CONTENT_WIDTH  = 280;
	const static int CONTENT_HEIGHT = 30;
};

class ListItemFRStatusNew : public ListItemBottomNew
{
public:
	enum{
		LOGIN_STATUS = 0,
		PLAYGAME_STATUS
	};
	ListItemFRStatusNew();
	virtual ~ListItemFRStatusNew();

	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData);
	void InitItems(CTempPopUp* pDlg, PVOID psData);
	void SetType(const int nType){ m_nType = nType; }	
	const int GetType(){ return m_nType; }
	int GetClassType(){ return FRIEND_STATUS; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return; }
protected:
	void OnClickContent(ISonicBase*, LPVOID pReserve);
	ISonicString* m_pText;
	core_msg_FRStatus m_sData;
	int m_nType;
};

class ListItemGameReadyNew : public ListItemBottomNew
{
public:
	ListItemGameReadyNew();
	virtual ~ListItemGameReadyNew();

	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData);
	void InitItems(CTempPopUp* pDlg, PVOID psData);	
	int GetClassType(){ return GAME_READY_PLAY; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return; }
protected:
	void OnClickContent(ISonicBase*, LPVOID pReserve);

	ISonicString* m_pIcon;
	ISonicString* m_pTitle;
	ISonicString* m_pText;
	core_msg_ready_play m_sData;
};
class ListItemImMessage : public ListItemBottomNew
{
public:
	ListItemImMessage();
	virtual ~ListItemImMessage();
	BOOL InsertItem(CTempPopUp* pDlg, PVOID psData);
	void InitItems(CTempPopUp* pDlg, PVOID psData);	
	int GetClassType(){ return CHAT_MESSAGE; }
	virtual void OnClick(ISonicBase* , LPVOID pReserve){ return OnClickContent(NULL, pReserve) ;}
protected:
	void OnClickContent(ISonicBase *, LPVOID pReserve);
private:
	ISonicString* m_pIcon;
	ISonicString* m_pTitle;
	ISonicString* m_pContent;
	const static int TITLE_WIDTH = 250;
	const static int CONTENT_WIDTH = 280;
	const static int CONTENT_HEIGHT = 30;
	_tstring m_strSendUserName;
};

class CTempPopUp : public CWindowImpl<CTempPopUp>
{
public:
	CTempPopUp();
	~CTempPopUp();

	DECLARE_WND_CLASS(_T("CTempPopUp"))
	BEGIN_MSG_MAP_EX(CTempPopUp)
		MSG_WM_CREATE(OnCreate)
		/*MSG_WM_LBUTTONDOWN(OnLButtonDown)*/
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOUSEMOVE(OnMouseMove)
	END_MSG_MAP()

	
public:
	PVOID GetDataPtr(){ return m_pDataPtr; }
	BOOL InitInboxTipWnd(core_msg_inbox& stInboxData);
	BOOL InitFriTipWnd(core_msg_friendreq& stFriReqData);
	BOOL InitNotifyTipWnd(core_msg_notification& stNotifyData);
	BOOL InitFRStatusWnd(core_msg_FRStatus& stFRStatus);
	BOOL InitGameReadyPlayWnd(core_msg_ready_play& stGameReady);
	BOOL InitImMessageWnd(im_msg_rcv_msg& stImMsg);
	BOOL ShowWindow(int nCmdShow);
	void OnResponseFriendReq(BOOL bAccept, BOOL bResult);
	const int GetType(){ return m_nType; }
protected:
	void OnClickClose(ISonicBase* , LPVOID);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnTimer(UINT_PTR nIDEvent) ;
	void OnMouseLeave();
	void OnMouseHover(WPARAM wParam, CPoint ptPos);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL IsMouseInTipWindow();
	void OnFinalMessage(HWND);
	void OnLButtonDown(UINT nFlags, CPoint point);
private:
	BOOL m_bMouseTrack;
	ListItemBottomNew* m_pDataPtr;
	int m_nWidth;
	int m_nHeight;
	int m_nType;
};