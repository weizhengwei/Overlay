#pragma once

#include "tstring/tstring.h"
#include "OverlayBrowserWnd.h"
#include "thread/thread.h"
#include "CoreMsgOverlay2Browser.h"
#include "browser/BrowserImplbase.h"
#include <vector>
#include <map>
using std::vector;
using String::_tstring;
using std::map;
const int BROWSER_OFFSET_LEFT =0 ;
const int BROWSER_OFFSET_TOP = 43;
const int BROWSER_OFFSET_RIGHT = 0;
const int BROWSER_OFFSET_BOTTOM = 0;	

#define  TABTILE_FONT _T(",font, font_height=13,font_face='Open Sans'")

class CBrowserImpl : public CBrowserImplbase
{
public:
	CBrowserImpl(void);
	~CBrowserImpl(void);
	static CBrowserImpl* GetInstance()
	{
		static CBrowserImpl Instance;
		return &Instance;
	}
	int AddBrowserWnd(COverlayBrowserWnd* pBrowserwnd);
	int DelBrowserWnd(int nIndex);
	int OnBrowserMsg(WPARAM wParam,LPARAM lParam);
	void SetWnd(HWND hwnd){m_hWnd = hwnd;}
	int GetBrowserWndIndex(CefRefPtr<ClientOSRHandler> Handler);
	void HandlePaint(ClientOSRHandler* BrowserHandler,CefRefPtr<CefBrowser> browser,
		int type, const CefRect& dirtyRect,
		const void* buffer,int width, int height);
	COverlayBrowserWnd * CreateBrowserWnd(ClientOSRHandler *pHandeler,int nIndex,bool bActive = true);
	void SetTab(ISonicTab *pTabs){m_pTab = pTabs;}
    void SetBrowserMainWnd(HWND hWnd){m_hBrowserMainWnd = hWnd;}
	void BrowserInit(HINSTANCE hInstance,CefRefPtr<CefApp> app);
	void  UnInit();
	void CreateBrowser(LPCTSTR szUrl,int nTabIndex,bool bActive = true);
	void DrawBrowser(HDC hdc,CRect &rect);
	BOOL IsCurrentBrowser(COverlayBrowserWnd *pBrowserWnd);
	void SetBrowserSize(int nIndex,int nwidth, int nheight);
	void SetCurrentBrowser(int nIndex,BOOL bUpdate);
	void GetBrowserClientRect(CRect &rcParent,CRect &rect);
	void ClearAllBrowser();
	BOOL IntersectCefRect(CRect &rcDst,CefRect rcRect1,CefRect rcRect2);
	void GetOffSetRect(CRect &rc);
	void SetBrowserSize(HWND hWnd, int nIndex);
	HWND GetHWnd(){return m_hWnd;}
	HWND GetBrwoserMainWnd(){return m_hBrowserMainWnd;}
	void SetSelTab(int nIndex,HWND hWnd = NULL);
	BOOL ClientToBrowserRect(HWND hWnd,CRect &rcIn,CRect &rcOut);
	int InsertTab(int nIndex,HWND hwnd,LPCTSTR lpctstr,bool bActive = true);
	void AdjustTabItmeSize(BOOL bDelete = FALSE);
	BOOL GetBrowserTitle(int nIndex,_tstring &strTitle);
	BOOL SetTabText(int nIndex);
	int OnProcessMsg(core_msg_header * pHeader);
	void SetCookie();
	void SetBillingCookie();
	CRect *GetMainBrowserWndRect(){return &m_rcWnd;}
	void ActiveThreadWnd();
    HWND GetParent();
protected:
	void SetBuffer(int width, int height, bool view,const void* buffer);
	void CopyBuffer(int type, const CefRect dirtyRect,const void* buffer,int nWidth,int nHeight);
	void HandlePopupChange(CefRefPtr<ClientOSRHandler> BrowserHandler,CefRefPtr<CefBrowser> browser, bool show,
		const CefRect& rect);
	static void IOT_Set(const CefString& url,const CefCookie& cookie){
		CefCookieManager::GetGlobalManager()->SetCookie(url,cookie);
	};
	

	
private:
	typedef vector<COverlayBrowserWnd*> VEBROWSERWND;
	typedef VEBROWSERWND::iterator VEBROWSERWNDiter;

	VEBROWSERWND m_veBrowserWnd;
	int m_nCurrentIndex;
	int m_nWidth;
	int m_nHeight;
	HWND m_hWnd;
	HWND m_hBrowserMainWnd;

	_tstring m_strEventName;
	int view_height;
	int view_width;
	unsigned char* popup_buffer_;
	int popup_buffer_size_;
	CefRect m_cefPopupRect;
    char* view_buffer_;
	int view_buffer_size_;
	CefRect popup_rect_;
	ISonicTab*	m_pTab;
	Mutex m_mBrowser;
	CRect m_rcDirtyRect;
	CRect m_rcWnd;
};

