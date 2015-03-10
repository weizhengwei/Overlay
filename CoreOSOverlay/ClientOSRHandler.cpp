#include "StdAfx.h"
#include "ClientOSRHandler.h"
#include "BrowserImpl.h"
#include "data//DataPool.h"


ClientOSRHandler::ClientOSRHandler(void)
:m_nWidth(0),m_nHeight(0)
{
	m_strTitle = _T("Loading...");
}

ClientOSRHandler::~ClientOSRHandler(void)
{
	
}

bool ClientOSRHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
									 CefRefPtr<CefFrame> frame,
									 const CefString& target_url,
									 const CefString& target_frame_name,
									 const CefPopupFeatures& popupFeatures,
									 CefWindowInfo& windowInfo,
									 CefRefPtr<CefClient>& client,
									 CefBrowserSettings& settings,
									 bool* no_javascript_access) 
{
	//OutputDebugStringA("OnBeforePopup");
	windowInfo.window_rendering_disabled = TRUE;
	CefRefPtr<ClientOSRHandler> ClientRHandler (new ClientOSRHandler());
	client = ClientRHandler;
	tagBrowserMsgData msgdata;
	msgdata.msgid = BROWSER_HANDLER_BEFORECREATE;
	msgdata.pHandler = ClientRHandler;
	msgdata.pParentHandler = this;
	::SendMessage(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
	return false;
}


void ClientOSRHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	REQUIRE_UI_THREAD();
	Lock();
	// Set the view size to match the plugin window size.
	browser->GetHost()->WasResized();
	m_Browser = browser;
	//g_BrowserDelegate.HandleAfterCreated(this,browser);
	tagBrowserMsgData msgdata;
	msgdata.browser = browser;
	msgdata.pHandler = this;
	msgdata.msgid = BROWSER_HANDLER_AFTERCREATE;
	::SendMessage(m_BrowserHwnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
	Unlock();
}



 void ClientOSRHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
								  bool isLoading,
								  bool canGoBack,
								  bool canGoForward)
{
	SetFocus(CBrowserImpl::GetInstance()->GetParent());
	tagBrowserMsgData msgdata;
	msgdata.pHandler = this;
	msgdata.loading = isLoading;
	msgdata.msgid = BROWSER_HANDLER_LOAD;
	::SendMessage(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
	if (!isLoading)
	{
		::SendMessage(m_BrowserHwnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
	}
	
}

void ClientOSRHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
							 CefRefPtr<CefFrame> frame,
							 const CefString& url)
{
	REQUIRE_UI_THREAD();

	tagBrowserMsgData msgdata;
	msgdata.pHandler = this;
	msgdata.msgid = BROWSER_HANDLER_ADDRESSCHANGE;
	if (url.size())
	{
		lstrcpyn(msgdata.szTitle,url.c_str(),512);
	}
	::SendMessage(m_BrowserHwnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
}

 void ClientOSRHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
						   const CefString& title) 
 {
	 REQUIRE_UI_THREAD();
	 tagBrowserMsgData msgdata;
	 msgdata.pHandler = this;
	 if (title.size() == 0)
	 {
		 return;
	 }
	 {
		 lstrcpyn(msgdata.szTitle,title.c_str(),512);
	 }
	 msgdata.msgid = BROWSER_HANDLER_TITLECHANGE;
	 Lock();
	 m_strTitle = title.c_str();
	 Unlock();
	 ::SendMessage(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
 }
 bool ClientOSRHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
	 const CefKeyEvent& event,
	 CefEventHandle os_event,
	 bool* is_keyboard_shortcut)
 {

	 if (event.character == VK_F12)
	 {
		 //browser->ShowDevTools();
		 int nDevTool = 0;
		nDevTool = GetPrivateProfileInt(INI_CFG_DEVTOOLS,INI_CFG_DEVTOOLS,nDevTool,theDataPool.GetUserProfilePath().c_str());
		 if (nDevTool == 1)
		 {
			 std::string devtools_url = browser->GetHost()->GetDevToolsURL(true);
			 browser->GetMainFrame()->ExecuteJavaScript(
				 "window.open('" +  devtools_url + "');", "about:blank", 0);
		 }

	 }
	 return false;
 }
bool ClientOSRHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
						 CefRect& rect) 
{
	REQUIRE_UI_THREAD();

	// The simulated screen and view rectangle are the same. This is necessary
	// for popup menus to be located and sized inside the view.
	rect.x = rect.y = 0;
	rect.width = m_nWidth;
	rect.height = m_nHeight;
	return true;
}
bool ClientOSRHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
						   CefRect& rect)
{
	REQUIRE_UI_THREAD();
	CRect rcWindow;
	::GetWindowRect(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),rcWindow);
	rect.x = rcWindow.left;
	rect.y = rcWindow.top;
	rect.height = rcWindow.Height();
	rect.width = rcWindow.Width();
	return true;
	//return GetViewRect(browser,rect);

}
void ClientOSRHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
						 bool show) 
{
	REQUIRE_UI_THREAD();
	// g_BrowserDelegate.HandlePopupChange(this,browser,show,rect);
	tagBrowserMsgData msgdata;
	msgdata.msgid = BROWSER_HANDLER_POPUPCHANGE;
	msgdata.pHandler = this;
	msgdata.show = show;
	msgdata.rect = popup_rect_;
	msgdata.browser = browser;
	::SendMessage(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
	if(!show) 
	{
		popup_rect_.Set(0,0,0,0);
	}
}
void ClientOSRHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
				 const CefRect& rect)
{
	if (rect.width > 0 && rect.height > 0)
	{
		popup_rect_ = rect;
	}
    if (popup_rect_.x < 0)
    {
       // OutputDebugStringA("resize popup_rect_.x = 0");
        popup_rect_.x = 0;
    }
    if (popup_rect_.y < 0)
    {
       // OutputDebugStringA("resize popup_rect_.y = 0");
        popup_rect_.y = 0;
    }	

	if (popup_rect_.x + popup_rect_.width > m_nWidth)
		popup_rect_.width = m_nWidth - popup_rect_.x;
	if (popup_rect_.y + popup_rect_.height > m_nHeight)
		popup_rect_.height = m_nHeight  - popup_rect_.y ;
}

void ClientOSRHandler:: OnPaint(CefRefPtr<CefBrowser> browser,
								PaintElementType type,
								const RectList& dirtyRects,
								const void* buffer,
								int width, int height) 
{
	REQUIRE_UI_THREAD();
	CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
	for (; i != dirtyRects.end(); ++i) 
	{
		const CefRect& rect = *i;
		//GLOG(_T("rect.x = %d, rect.y = %d,rect.width = %d,rect.height= %d type = %d"),rect.x,rect.y,rect.width,rect.height,type);
		CBrowserImpl::GetInstance()->HandlePaint(this,browser,type,rect,buffer,width,height);
	}
	//::SendMessage(CBrowserImpl::GetInstance()->m_hBrowserMainWnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
}
void ClientOSRHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
							CefCursorHandle cursor)
{
	REQUIRE_UI_THREAD();
	tagBrowserMsgData msgdata;
	msgdata.msgid = BROWSER_HANDLER_CURSORCHANGE;
	msgdata.hCursor = cursor;
	msgdata.pHandler = this;

	::SendMessage(m_BrowserHwnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
}

void ClientOSRHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
	REQUIRE_UI_THREAD();
	//if(m_BrowserHwnd == browser->GetWindowHandle())
	{
		// Free the browser pointer so that the browser can be destroyed
		m_Browser = NULL;
		if (IsWindow(m_BrowserHwnd))
		{
			int nIndex = CBrowserImpl::GetInstance()->GetBrowserWndIndex(this);
			if (nIndex != -1)
			{
			//	CBrowserImpl::GetInstance()->DelBrowserWnd(nIndex);
				//OutputDebugStringA("OnBeforeClose\n");
				tagBrowserMsgData msgdata;
				msgdata.msgid = BROWSER_HANDLER_CLOSE;
				msgdata.pHandler = this;
				::SendMessage(CBrowserImpl::GetInstance()->GetBrwoserMainWnd(),WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
				
			}
		}
		
	}
}

 void ClientOSRHandler::SetBrowserSize(PaintElementType type, int width, int height)
 {
	 m_nHeight = height;
	 m_nWidth = width;
	 if (m_Browser)
	 {
		 m_Browser->GetHost()->WasResized();
	 }
	
 }
 void ClientOSRHandler::Invalidate(CefRect *rect)
 {
	 if (m_Browser)
	 {
		 CefRect rc;
		 if (rect == NULL)
		 {
			 rect = &rc;
			 rc.x = 0;
			 rc.y = 0;
			 rc.width = m_nWidth;
			 rc.height = m_nHeight;
		 }
		  m_Browser->GetHost()->Invalidate(*rect,PET_VIEW);
	 } 
 }