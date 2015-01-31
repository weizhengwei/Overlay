#pragma once
#include "ArcShareMem.h"
#include "cefclient.h"
#include "CoreMsgOverlay2Browser.h"
#include "thread/thread.h"
#include "tstring/tstring.h"
#include "browser/BrowserImplbase.h"
#include <map>
using std::map;
using String::_tstring;

class CBrowserImpl : public CBrowserImplbase,
	public Thread
{
public:
	CBrowserImpl(void);
	~CBrowserImpl(void);
	
	static CBrowserImpl* GetInstance()
	{
		static CBrowserImpl Instance;
		return &Instance;
	}
	void  Init(HWND hParentWnd,HWND hMsgClient,HINSTANCE hInstance,CefRefPtr<CefApp> app);
	void  InitBaseInfo(HWND hwnd,int nCommandLineType,_tstring &strUrl);
	void  BrowserInit(HINSTANCE hInstance,CefRefPtr<CefApp> app);
	void  UnInit();

	
	BOOL SendCoreMsg(core_msg_header * pHeader);
	void SetInitBaseInfo(core_msg_initbaseinfo & initBaseInfo);
	LRESULT OnProcessMsg(WPARAM wParam,LPARAM lParam);
	void Navigate(_tstring tsURL,int nMode);
	void CreateBrowser(LPCTSTR szUrl);
	bool IsPageLoading(){if(m_cefHandler.get()) return m_cefHandler->IsPageLoading(); return false;};
	void HandlePopupChange(CefRefPtr<ClientHandler> BrowserHandler,CefRefPtr<CefBrowser> browser, bool show,
		const CefRect& rect);
	void HandlePaint(CefRefPtr<CefBrowser> browser,
		int type, const CefRect& dirtyRect,
		const void* buffer,int width, int height);
	CArcShareMemMgr<arcbrowser_sm_header> GetMenMgr(){return m_smMgr;}
	HWND GetHwnd(){return m_hWnd;}
    HWND GetParent(){return m_hParentWnd;}
	void SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam);
	void PopupShow(CefRefPtr<CefBrowser> browser,bool show) ;
	void ActiveThreadWnd();

	static void IOT_Del(const CefString& url,const CefString& cookie_name){
	 	CefCookieManager::GetGlobalManager()->DeleteCookies(url,cookie_name);
	};

	void DelCookies(const CefString& url,const CefString& cookie_name,const _tstring& strCookieFolder);


		
protected:
	void GoBack();
	void GoForward();
	void Refresh();
	virtual long run();
	
	void SetBuffer(int width, int height, bool view,const void* buffer);
	void CopyBuffer(int type, const CefRect dirtyRect,const void* buffer,int nWidth,int nHeight);
	BOOL IntersectCefRect(CRect &rcDst,CefRect rcRect1,CefRect rcRect2);
	
private:
	CefRefPtr<ClientHandler> m_cefHandler;

	HWND m_hWnd;
	HWND m_hMsgClient;
	HWND m_hParentWnd;
	CefRect m_cefPopupRect;
	unsigned char* popup_buffer_;
	int popup_buffer_size_;
	int view_width;
	int view_height;
	Mutex m_mBrowser;
	CCoreMsgClient m_msgClient;
	CRect m_defRect;
	CArcShareMemMgr<arcbrowser_sm_header> m_smMgr;
};
