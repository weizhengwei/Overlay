#pragma once
#include "thread/thread.h"
#include "CoreMsgBrowser2Svr.h"
#include "ui/UIWindow.h"
#include "tstring/tstring.h"
#include "ui/UILoadingBar.h"
using String::_tstring;
class CBrowserDialog : public CUIWindow
	,public Thread
{
	DECLARE_DYNAMIC(CBrowserDialog)
public:
	CBrowserDialog(UINT id, CWnd* pParent = NULL,int nType = 0);
	~CBrowserDialog(void);
	CCoreMsgClient * GetMsgClient(){return &m_msgClient;}
	void SendCoreMsg(core_msg_header * pHerader);
	void SetStartUrl(_tstring &strUrl){m_csStartUrl = strUrl;}
	virtual void SetLoadBarRect();
	virtual void Navigate(_tstring tsURL,int nMode = 1);
	//pt.x 相对父窗口右边向左偏移，pt.y 相对父窗口上边向下内偏移
	void SetLoadBarPoint(CPoint &pt);
	//must before  start
	void SetCookieFolder(_tstring & strCookieFolder){m_strCookieFolder = strCookieFolder;}
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bFullMode;
	HWND m_hBrowserWnd;
	_tstring m_strAllGameUrl;
	_tstring m_csStartUrl;
	//must before  start
	_tstring m_strCookieFolder;
	int      m_nMode;

protected:
	long run();
	LRESULT OnProcessMsg(WPARAM wParam,LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void OnSize(UINT nType, int cx, int cy);
	void OnMove(int x, int y);
	void OnDestroy();
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);
private:
	CCoreMsgClient m_msgClient;
	UINT m_uBrowserType;
	HWND m_hMsgClient;
	PROCESS_INFORMATION m_ProcessInfo;
	CUILoadingBar m_wndLoadingBar;
	BOOL m_bLoadBarShow ;
	CPoint m_LoadBarPoint;
	
};
