#pragma once
#include "thread/thread.h"
#include "CoreMsgBrowser2Svr.h"
#include "CoreMsgOverlay2Browser.h"
#include "ui/UIWindow.h"
#include "tstring/tstring.h"
#include "ArcShareMem.h"
#include "ui/UILoadingBar.h"
using String::_tstring;
class COSBrowserDialog : public CUIWindow
	,public Thread
{
	DECLARE_DYNAMIC(COSBrowserDialog)
public:
	COSBrowserDialog(UINT id, CWnd* pParent = NULL,int nType = 0);
	~COSBrowserDialog(void);
	CCoreMsgClient * GetMsgClient(){return &m_msgClient;}
	void SendCoreMsg(core_msg_header * pHerader);
	void SetStartUrl(_tstring &strUrl){m_csStartUrl = strUrl;}

	//相对窗口偏移（左 上 右 下）
	void SetDefRect(CRect rc);
	void SetDefRect(int l, int t, int r, int b);

	//浏览器消息遮挡偏移
	void SetDefBrowserRect(CRect rc);
	void SetDefBrowserRect(int l, int t, int r, int b);

	CRect GetDefRect(){return m_defRect;}
	//msg to wnd
	virtual BOOL SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam);
	//msg to core msg
	virtual BOOL SetCoreBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam);
	virtual void DrawBrowser(HDC hdc,CRect &rcClient);
	virtual void DrawBrowser(HWND hWnd,CRect &rcClient);
	virtual void DrawBrowser(ISonicPaint * pPaint,CRect &rcClient);
	BOOL PtInBrowserRect(CPoint pt);
	virtual BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void SetLoadBarRect();
	virtual void Navigate(_tstring tsURL,int nMode = 1);
	virtual void RunOnceOnStart();
	//pt.x 相对父窗口右边向左偏移，pt.y 相对父窗口上边向下内偏移
	void SetLoadBarPoint(CPoint &pt);
	//must before  start
	void SetCookieFolder(_tstring & strCookieFolder){m_strCookieFolder = strCookieFolder;}

	void StartOSbrowser();
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bFullMode;
	HWND m_hBrowserWnd;
	_tstring m_csStartUrl;
	_tstring m_strAllGameUrl;
	int      m_nMode;
	HCURSOR m_hBrowserCursor;
	//must before  start
	_tstring m_strCookieFolder;
	Mutex m_lockBrowser;
	PROCESS_INFORMATION m_ProcessInfo;

protected:
	long run();
	LRESULT OnProcessMsg(WPARAM wParam,LPARAM lParam);
	LRESULT OnBrowserDestory(WPARAM wParam,LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void    OnPaint();
	void OnDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	void OnSize(UINT nType, int cx, int cy);
	void OnMove(int x, int y);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);
private:
	CCoreMsgClient m_msgClient;
	UINT m_uBrowserType;
	HWND m_hMsgClient;
	CRect m_defRect;
	CRect m_defBrowserRect;
	CArcShareMemMgr<arcbrowser_sm_header>  *m_psmMgr;
	HANDLE m_Event;
	_tstring m_strEventName;
	BOOL m_bBrowserFocus;
	BOOL m_bMouseIn;
	ISonicImage *m_pImage;
	CUILoadingBar m_wndLoadingBar;
	BOOL m_bLoadBarShow;
	CPoint m_LoadBarPoint;
	
	
};
