#pragma once

#include "ArcShareMem.h"

#include "CoreMsgOverlay2Svr.h"
#include "tstring/tstring.h"
using String::_tstring;
#include <queue>



class IOverlayMainWnd
{
public:
	virtual void ShowOverlay(BOOL bShow ,BOOL bNotifyGame = TRUE) = 0;
	virtual void ShowBrowserMainWnd(LPCTSTR lpUrl) = 0;
	virtual void SetBackBtnText() = 0;
	virtual BOOL ShowTipNotify(_tstring & strText) = 0;
	virtual void SetTipNotifyText(_tstring & strText) = 0;
	virtual void ShowSetting() = 0;
	virtual CWindow* GetMainWnd() = 0;
	virtual void HandleSelfAvatar(_tstring lpPath) = 0;
	virtual void HandleSelfPresence(UINT iStatus) = 0;
};

class COverlayImpl : public IOffscreenUICallBack
{
public:
	COverlayImpl(void);
	~COverlayImpl(void);
	static COverlayImpl * GetInstance()
	{
		static COverlayImpl Instance;
		return &Instance;
	}
	// return the window handle which will be the render target
	virtual void osGetRenderTarget(HWND hOSWindow, LPRECT pRectScreen) ;

	// notify the host to change its cursor shape, the nCursorResID is system defined
	virtual void osChangeCursor(int nCursorResID) ;

	// notify the host the canvas should be repaint with a specified dirty rectangle
	virtual void osDraw(ISonicPaint * pPaint, const RECT * pRectDirty, BOOL bNullImage = FALSE);

	// notify the host to set or release capture(releasing if hOSWindow is null)
	virtual void osChangeCapture(HWND hOSWindow);

	// notify the host some off-screen is destroying
	virtual void osDestroy(HWND hOSWindow);

	// SonicUI will call this function to get the off-screen panel, that is considered as the bottom and main window
	virtual HWND osGetPanel();
	// notify the host focus is changing from hLosingFocus to hFocus
	virtual void osChangeFocus(HWND hFocus, HWND hLosingFocus) ;
	CCoreMsgClient *GetCoreMsgClient(){return &m_msgClient;}
	core_msg_cookie* GetMsgCookie(){return &m_msgCookie;}


	BOOL SetMgrWnd(HWND hwnd,_tstring & strNotifyTip);
	
    void	Init()	;
	int    OnProcessMsg(core_msg_header * pHeader);
	void   SetListBox(CCoreListBox*pListBox){m_pListBox = pListBox;}
	CArcShareMemMgr<overlay_sm_header> GetMenMgr(){return m_smMgr;}
	BOOL   OlCenterWindow(HWND hWnd,HWND hWndCenter = NULL);
	CSize GetOverlaySize();
	int GetGameId(){return _ttoi(m_strGameId.c_str());}
	_tstring &GetGameStrId(){return m_strGameId;}

	//send msg to game 
	inline BOOL SendCoreClientMsg(core_msg_header * pHeader);
	//send msg to arc server
	inline BOOL SendCoreMsg(core_msg_header * pHeader);

	BOOL ShowGently(HWND hWnd,BOOL bShow, BOOL bActivted = FALSE, int nStep = 0);
	void SetClientHwnd(HWND hWnd){m_hMsgClient = hWnd;}
	void InitMsg();

	//send cursor msg to game
	void SendCursorMsg(int nCursorResID);

	//send track to arc server
	void SendTrack(LPCTSTR lpcsTrack, ...);

	BOOL SetTipNotifyText();
	//BOOL ShowTipNotify();

	//IOverlayMainWnd
	void SetOverlayMainWnd(IOverlayMainWnd* pIOverlayMainWnd){m_pIOverlayMainWnd = pIOverlayMainWnd;}
	void ShowBrowserMainWnd(LPCTSTR lpUrl){m_pIOverlayMainWnd->ShowBrowserMainWnd(lpUrl);}
	void ShowOverlay(BOOL bShow,BOOL bNotifyGame = TRUE){m_pIOverlayMainWnd->ShowOverlay(bShow,bNotifyGame);}
	void SetBackBtnText(){m_pIOverlayMainWnd->SetBackBtnText();}
	void ShowSetting(){m_pIOverlayMainWnd->ShowSetting();}
	void HandleSelfAvatar(_tstring lpPath){m_pIOverlayMainWnd->HandleSelfAvatar(lpPath);}
	void HandleSelfPresence(UINT iStatus){m_pIOverlayMainWnd->HandleSelfPresence(iStatus);}
	CWindow* GetMainWnd() {return m_pIOverlayMainWnd->GetMainWnd();}
	BOOL ShowTipNotify(_tstring & strText);
	BOOL SendMainWndCoreMessage(UINT msgId,WPARAM wParam,LPARAM lParam);
private:
		CArcShareMemMgr<overlay_sm_header> m_smMgr;
		HWND m_hWnd;
		//ISonicTab *m_pTab;
		CCoreListBox* m_pListBox;
		CSize m_sOverlaySize;
		CCoreMsgClient m_msgClient;
		core_msg_cookie m_msgCookie;
		_tstring m_strGameId;
		_tstring m_strSessionId;
		HWND m_hMsgClient;
		IOverlayMainWnd * m_pIOverlayMainWnd;
		std::queue<_tstring> quStrNotify;
};
