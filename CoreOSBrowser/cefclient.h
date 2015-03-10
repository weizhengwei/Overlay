#ifndef _CEFCLEINT__H
#define _CEFCLEINT__H

#include "cef/cef_client.h"
#include "cef/cef_life_span_handler.h"
#include "cef/cef_load_handler.h"
#include "cef/cef_request_handler.h"
#include "cef/cef_display_handler.h"
#include "cef/cef_keyboard_handler.h"
#include "cef/cef_render_handler.h"
#include "cef/cef_app.h"

#include "tstring/tstring.h"
#include "thread/thread.h"
#include "CoreMsgBrowser2Svr.h"
#include "data/Utility.h"
#include "thread/Condition.h"
#include "thread/WQueue.h"
#include "util.h"

using namespace String;

class CBrowserHandler : public CefClient
	,public CefLifeSpanHandler
	,public CefLoadHandler
{
public:
	CBrowserHandler()
	{

	}
	~CBrowserHandler()
	{

	}
	// CefClient methods
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
		return this;
	}
	 void OnLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int httpStatusCode)
	{
		if (m_Browser && !browser->IsPopup() && frame->IsMain())
		{
			m_Browser->GetHost()->CloseBrowser(false);
		}
	}
	 void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	 {
		m_Browser = browser;
	 }

private:
	CefRefPtr<CefBrowser> m_Browser;
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(CBrowserHandler);
	// Include the default locking implementation.
	IMPLEMENT_LOCKING(CBrowserHandler);
};

class ClientHandler : public CefClient
	,public CefLifeSpanHandler
	,public CefLoadHandler
	,public CefRequestHandler
	,public CefDisplayHandler
	,public CefKeyboardHandler
	,public CefRenderHandler
	,public CefDownloadHandler
	,public Thread
{
	
public:
	ClientHandler();
    ~ClientHandler();

	// CefClient methods
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
		return this;
	}
	
	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler()OVERRIDE {
		return this;
	}

	// Called before a new popup window is created. The |parentBrowser| parameter
	// will point to the parent browser window. The |popupFeatures| parameter will
	// contain information about the style of popup window requested. Return false
	// to have the framework create the new popup window based on the parameters
	// in |windowInfo|. Return true to cancel creation of the popup window. By
	// default, a newly created popup window will have the same client and
	// settings as the parent window. To change the client for the new window
	// modify the object that |client| points to. To change the settings for the
	// new window modify the |settings| structure.
	///
	/*--cef(optional_param=url)--*/
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access) ;
	///
	// Called after a new window is created.
	///
	/*--cef()--*/
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	 virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) ;

	 ///
	 // Called when the loading state has changed. This callback will be executed
	 // twice -- once when loading is initiated either programmatically or by user
	 // action, and once when loading is terminated due to completion, cancellation
	 // of failure.
	 ///
	 /*--cef()--*/
	 virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
		 bool isLoading,
		 bool canGoBack,
		 bool canGoForward);
	


		///
		// Called on the UI thread before browser navigation. Return true to cancel
		// the navigation or false to allow the navigation to proceed.
		///
		/*--cef()--*/
	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool is_redirect);

		///
		// Called when a frame's address has changed.
		///
		/*--cef()--*/
		virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& url);

	///
	// Called when the page title changes.
	///
	/*--cef(optional_param=title)--*/
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title);

	virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
		const CefKeyEvent& event,
		CefEventHandle os_event,
		bool* is_keyboard_shortcut);
  // Called to retrieve the view rectangle which is relative to screen
  // coordinates. Return true if the rectangle was provided.
  ///
  /*--cef()--*/
  virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,CefRect& rect) ;

  ///
  // Called to retrieve the simulated screen rectangle. Return true if the
  // rectangle was provided.
  ///
  /*--cef()--*/
  virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
	  CefRect& rect) ;
  ///
  // Called to retrieve the translation from view coordinates to actual screen
  // coordinates. Return true if the screen coordinates were provided.
  ///
  /*--cef()--*/
  virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
	  int viewX,
	  int viewY,
	  int& screenX,
	  int& screenY){return false;}

  ///
  // Called when the browser wants to show or hide the popup widget. The popup
  // should be shown if |show| is true and hidden if |show| is false.
  ///
  /*--cef()--*/
  virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,
	  bool show) ;

  ///
  // Called when the browser wants to move or resize the popup widget. |rect|
  // contains the new location and size.
  ///
  /*--cef()--*/
  virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
	  const CefRect& rect) ;

  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
	  PaintElementType type,
	  const RectList& dirtyRects,
	  const void* buffer,
	  int width, int height);
 void OnCursorChange(CefRefPtr<CefBrowser> browser,
	 CefCursorHandle cursor);

 static void OpenFolder(const CefString& fileName);

///DownloadListener
 // Called when the download is complete.
 virtual void NotifyDownloadComplete(const CefString& fileName);

 // Called if the download fails.
 virtual void NotifyDownloadError(const CefString& fileName);

 // Called before a download begins. |suggested_name| is the suggested name for
 // the download file. By default the download will be canceled. Execute
 // |callback| either asynchronously or in this method to continue the download
 // if desired. Do not keep a reference to |download_item| outside of this
 // method.
 ///
 /*--cef()--*/
 virtual void OnBeforeDownload(
	 CefRefPtr<CefBrowser> browser,
	 CefRefPtr<CefDownloadItem> download_item,
	 const CefString& suggested_name,
	 CefRefPtr<CefBeforeDownloadCallback> callback){};

 ///
 // Called when a download's status or progress information has been updated.
 // This may be called multiple times before and after OnBeforeDownload().
 // Execute |callback| either asynchronously or in this method to cancel the
 // download if desired. Do not keep a reference to |download_item| outside of
 // this method.
 ///
 /*--cef()--*/
 virtual void OnDownloadUpdated(
	 CefRefPtr<CefBrowser> browser,
	 CefRefPtr<CefDownloadItem> download_item,
	 CefRefPtr<CefDownloadItemCallback> callback) {}


public:
	CefRefPtr<CefBrowser> GetBrowser() { return m_Browser; }
//	CefWindowHandle GetBrowserHwnd() { return m_BrowserHwnd; }
	bool IsPageLoading() {return m_bLoadingPage ;} ;
	void Navigate(_tstring sURL,int nMode=1);

	void SetClientType(UINT uClientType){m_uType = uClientType;}
	DWORD GetClientType(){return m_uType;}
	void SetInitBaseInfo(core_msg_initbaseinfo & initBaseInfo);
	void SetCefCookie();
	core_msg_initbaseinfo * GetInitBaseInfo(){return &m_initBaseInfo;}


	//CBrowserHandler
	void NavigatePage(_tstring &sURL);
	void ThreadStart();
	void Invalidate(CefRect *rect);
	void SetBrowserSize(PaintElementType type, int width, int height);
	CefRect GetPopupCefRect(){ return popup_rect_;}
	

	//CRect * GetBrowserRect(){return &m_rcClient;}

public:
	enum
	{
		BROWSER_USABLE = 0,
		BROWSE_PAGE,
		BROWSER_MAX
	};
	typedef struct _BROWSER_INFO{
		int nType ;
		_tstring sUrl ;
		int nMode;
	}BROWSER_INFO,*PBROWSER_INFO;
protected:
	long run();
	bool CheckBrowserUsable();
	static DWORD WINAPI CheckBrowserUsableThread(LPVOID lp);
	static void IOT_Set(const CefString& url,const CefCookie& cookie){
		CefCookieManager::GetGlobalManager()->SetCookie(url,cookie);
	};
	_tstring GetUserProfilePath();
	_tstring m_sUserProfilePath;


	//void HideBrwoserWnd(int mode);

protected:
//	bool m_bExternalPopup ;
	bool m_bLoadingPage ; // indicate whether a page is loading or not.
	Condition m_syncUsable ;
	Mutex m_NavLock;
	WQueue<BROWSER_INFO> m_wTaskQueue ;
	//_tstring m_sCurUrl ;// save current page url in order for filtering some keywords in url.
	DWORD m_dwMode;
	CefRefPtr<CefBrowser> m_Browser;// The child browser window
	//CefWindowHandle m_BrowserHwnd;// The child browser window handle
//	ClientHandlerListener* m_pListener ;// The main frame window handle
	core_msg_initbaseinfo m_initBaseInfo;
	//DWORD m_dwClientId;
	UINT m_uType;
	HWND m_hClientWnd;
//	CRect m_rcClient; //browsermainwnd rect;

	//CBrowserHandler
private:
	CefRefPtr<CBrowserHandler> m_hBrowserHandler;
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientHandler);
	// Include the default locking implementation.
	IMPLEMENT_LOCKING(ClientHandler);
	int m_nHeight;
	int m_nWidth;
	CefRect popup_rect_;
	Mutex m_mBrowser;
};

#endif