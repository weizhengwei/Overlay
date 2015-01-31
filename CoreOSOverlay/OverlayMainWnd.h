#pragma once
#include "OverlayBrowserMainWnd.h"
#include "OverlaySettingWnd.h"
#include "OverlayImpl.h"

#include "CoreMsgOverlay2Svr.h"
#include "OverlayNotifyWnd.h"
#include "OverlayTipWnd.h"
#include "OverlayNotifyWndNew.h"
#include "tstring/tstring.h"
#include "thread/thread.h"
#include <vector>
#include "PopUpManager.h" 
#include "UIMainWndframe.h"
#include "im/im_ui_secondary.h"

using std::vector;
using String::_tstring;
class COverlayMainWnd : public CWindowImpl<COverlayMainWnd>
						,public IOverlayMainWnd, public Thread,
						public IPopUpManagerListener
{
public:
	DECLARE_WND_CLASS(_T("COverlayMainWnd"))

	BEGIN_MSG_MAP_EX(COverlayMainWnd)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SYSKEYDOWN(OnKeyDown)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MESSAGE_HANDLER(WM_COREMESSAGE,OnCoreMsg)
		MESSAGE_HANDLER(WM_SHOWTIPWINDOW, OnShowTipWindow)
		MESSAGE_HANDLER(WM_OPENFILEWND,OnOpenFileWnd)
        MESSAGE_HANDLER(WM_CHANGESIZDE, OnChangeSize)
	END_MSG_MAP()

public:
	COverlayMainWnd(void);
	~COverlayMainWnd(void);

    // rewrite from  IOverlayMainWnd
    virtual void ShowOverlay(BOOL bShow, BOOL bNotifyGame = TRUE);
    virtual void ShowBrowserMainWnd(LPCTSTR lpUrl);
    virtual void SetBackBtnText();
    virtual BOOL ShowTipNotify(_tstring & strText);
    virtual void SetTipNotifyText(_tstring & strText);
    virtual void ShowSetting();
    virtual CWindow* GetMainWnd(){return this;}
    virtual void HandleSelfAvatar(_tstring lpPath);
    virtual void HandleSelfPresence(UINT iStatus);
	
    // rewrite from IPopUpManagerListener
	virtual void ShowTipWindow(PVOID pTempPopUp);

    // rewrite from thread
	virtual long run();

protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnDestroy();
	void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnTimer(UINT_PTR nIDEvent);
	LRESULT  OnCoreMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	LRESULT  OnOpenFileWnd (UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	LRESULT  OnShowTipWindow(UINT msgId, WPARAM wParam, LPARAM lParam, BOOL bHandled);
    LRESULT  OnChangeSize(UINT msgId, WPARAM wParam, LPARAM lParam, BOOL bHandled);

private:
    void FitWndSize();
    void UISkinInit();

    BOOL ShowTipWnd();
    BOOL HideTipWnd();
    void ShowWndByResolution();
private:
	COverlaySettingWnd m_OverlaySettingWnd;
	COverlayBrowserMainWnd m_OverlayBrowerMainWnd;
	COverlayTipWnd m_OverlayTipWnd;
    CPopUpManager m_PopUpManager;
    CUIMainWndframe m_UIMainWndframe;

	vector<HWND> m_vcHideWnd;
	vector<_tstring> m_tipMsgVec;
	bool m_bCreateWnd;
	UINT m_TimeTemp;
    bool m_bOverlayShowing;

};

extern COverlayMainWnd _OverlayMainWnd;
