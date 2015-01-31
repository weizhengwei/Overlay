
#include "stdafx.h"
#include "constant.h"
#include "cefclient.h"
#include "cef/cef_runnable.h"
#include <string>
#include "BrowserImpl.h"
#include "data/DataPool.h"
#include "cef/cef_runnable.h"
#include <shlobj.h>
#include <userenv.h>
//#include "toolkit.h"
//#include "ISonicUI.h"
using namespace  std;
ClientHandler::ClientHandler()
:m_Browser(NULL),
m_bLoadingPage(false),
m_syncUsable(false, true),
m_uType(0),
m_hClientWnd(NULL),
m_dwMode(0),
m_nWidth(0),
m_nHeight(0)
{

}
ClientHandler::~ClientHandler()
{

}


bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
								  CefRefPtr<CefFrame> frame,
								  const CefString& target_url,
								  const CefString& target_frame_name,
								  const CefPopupFeatures& popupFeatures,
								  CefWindowInfo& windowInfo,
								  CefRefPtr<CefClient>& client,
								  CefBrowserSettings& settings,
								  bool* no_javascript_access) 
{

	_tstring szUrl = target_url.c_str();
	//GLOG(_T("szUrl = %s"),szUrl.c_str());
	if ((!popupFeatures.widthSet  || !popupFeatures.heightSet) && szUrl.Find(_T("devtools.html"))==string::npos)
	{
		windowInfo.height = 0 ;
		windowInfo.width  = 0 ;
		windowInfo.x = GetSystemMetrics(SM_CXSCREEN) + 100 ;
		windowInfo.y = GetSystemMetrics(SM_CYSCREEN) + 100 ;
		windowInfo.style=  WS_POPUP;//(createInfo.m_dwStyle)|~WS_VISIBLE ;
		m_bLoadingPage = false;
		if (szUrl.empty())
		{
			//m_NavLock.unlock();
			return true;
		}

		CUtility::NavURLWithDefaultBrowser(szUrl.c_str());	
		return true;	
	}
	else
	{
		//client = new ClientHandler();
		
		
		windowInfo.height = 600;
		windowInfo.width =  800;
		windowInfo.x= (GetSystemMetrics(SM_CXSCREEN) - windowInfo.width)/2 ;
		windowInfo.y = (GetSystemMetrics(SM_CYSCREEN) - windowInfo.height)/2 ;
        if (szUrl.Find(_T("facebook")) != string::npos)
        {
			windowInfo.height = 500;
			windowInfo.width =  300;
        }
	}

	return false;

}
void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	AutoLock lock_scope(this);
	if (!m_Browser.get())   
	{
		// We need to keep the main child window, but not popup windows
		m_Browser = browser;
		browser->GetHost()->WasResized();
		//GLOG(_T("m_nWidth = %d"),m_nWidth);
		//m_BrowserHwnd = browser->GetWindowHandle();
		if(m_Browser)
		{
			m_syncUsable.signal() ;
		}
		CBrowserImpl::GetInstance()->ActiveThreadWnd();		
	}
// 	if (browser->IsPopup() && browser->GetWindowHandle())
// 	{
// 		//CBrowserImpl::GetInstance()->AttachThreadToArc(browser->GetWindowHandle());
// 	}

}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
	REQUIRE_UI_THREAD();
	if (!browser->GetHost()->GetWindowHandle())
	{
		Thread::stop();
		m_Browser = NULL;
	}

}


void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame)
{
	if(!browser->IsPopup() && frame->IsMain())
	{
        //SetFocus(CBrowserImpl::GetInstance()->GetParent());

		core_msg_browserinfo browserinfo;
		browserinfo.bGet = FALSE;
		browserinfo.bLoading = TRUE;
		CBrowserImpl::GetInstance()->SendCoreMsg(&browserinfo);
	}

}


void  ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   int httpStatusCode)
{
	if(!browser->IsPopup() && frame->IsMain())
	{
		core_msg_browserinfo browserinfo;
		browserinfo.bGet = FALSE;
		browserinfo.bLoading = FALSE;
		CBrowserImpl::GetInstance()->SendCoreMsg(&browserinfo);
		m_bLoadingPage = false ; //set flag to indicate that page has been loaded      
	}
}

bool ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
								   CefRefPtr<CefFrame> frame,
								   CefRefPtr<CefRequest> request,
								   bool is_redirect)
{
	m_NavLock.lock();
	_tstring szUrl = request->GetURL().ToString().c_str();
	//GLOG(_T("szUrl = %s"),szUrl.c_str());
	if (szUrl.empty())
	{
		m_bLoadingPage = false;
		m_NavLock.unlock();
		return true;
	}
	if (m_uType == TYPE_BASEBROWSER)
	{
		m_NavLock.unlock();
		return false;
	}
	core_msg_navigateurl msgNavigateUrl;
	lstrcpynA(msgNavigateUrl.scUrl,szUrl.toNarrowString().c_str(),2048);
	CBrowserImpl::GetInstance()->SendCoreMsg(&msgNavigateUrl);
	BOOL bUrlGameInfo = msgNavigateUrl.bUrlGameInfo;
	//BOOL bUrlInGameLis = msgNavigateUrl.bUrlInGameList;

	// is loading, not to fullscreen;
	if(!bUrlGameInfo)
	{
		m_bLoadingPage = true ; //set flag to indicate that page is loading
	}

	//popup window from internal link
	if (browser->IsPopup())
	{
		m_bLoadingPage = false ; //reset flag to indicate the page has been handled
		
		REQUIRE_UI_THREAD();  
		m_NavLock.unlock();
		return false;
	}

	if(szUrl.find(_T("mailto:"))!=string::npos)  
	{
		//email address;
		_tstring  szMail = _T("") ;
		szMail.Format(1,_T("mailto:%s"),szUrl.c_str());
		CUtility::NavURLWithDefaultBrowser(szUrl.c_str());
		m_bLoadingPage = false ;
		m_NavLock.unlock();
		return true;
	}

	m_NavLock.unlock();
	return false;
}


void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
									CefRefPtr<CefFrame> frame,
									const CefString& url)
{
	if (browser->IsPopup())
	{
		return;
	}
	core_msg_addresschange addresschange;
	USES_CONVERSION;
	m_NavLock.lock();
//	m_sCurUrl = url.c_str();
	m_NavLock.unlock();
	lstrcpynA(addresschange.scUrl,T2A(url.c_str()),2048);
	CBrowserImpl::GetInstance()->SendCoreMsg(&addresschange);
}


void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
								  const CefString& title)
{

}

long ClientHandler::run()
{
	while(true)
	{
		bool bRet = false ;
		BROWSER_INFO sInfo = m_wTaskQueue.leave();

		m_syncUsable.wait() ;

		m_NavLock.lock();
		_tstring szUrl = sInfo.sUrl ;

		m_Browser->StopLoad() ;
		//HideBrwoserWnd(sInfo.nMode);
		m_Browser->GetMainFrame()->LoadURL(CefString(szUrl.toNarrowString()));
		if (m_uType == TYPE_MAINBROWSER)
		{
			//				::PostMessage(GetParent(GetParent(m_BrowserHwnd)),WM_WEB_NAVIGATE_URL,0,sInfo.nMode);
			//m_sCurUrl.assign(szUrl.c_str()) ; // save current page url.
			core_msg_navigate_e msgNavigate;
			msgNavigate.dwMode = sInfo.nMode;
			lstrcpynA(msgNavigate.scUrl,szUrl.toNarrowString().c_str(),2048);
			CBrowserImpl::GetInstance()->SendCoreMsg(&msgNavigate);	

		}	

		m_NavLock.unlock();

	}

	return 0 ;
}

void ClientHandler::Navigate(_tstring tsURL,int nMode)
{
	//_tstring strPerfectworId = m_initBaseInfo.csWebsite_perfectworld;
	if(tsURL.find(_T("perfectworld."))==string::npos && tsURL.find(_T("arcgames.")) == string::npos && m_uType >= TYPE_MAINBROWSER)
	{
		CUtility::NavURLWithDefaultBrowser(tsURL.c_str());
		return;
	}
	BROWSER_INFO sInfo = {BROWSE_PAGE, tsURL,nMode};
	m_wTaskQueue.enter(sInfo) ;
}
void ClientHandler::SetInitBaseInfo(core_msg_initbaseinfo & initBaseInfo)
{
	memcpy(&m_initBaseInfo,&initBaseInfo,sizeof(core_msg_initbaseinfo));
	//SetCookie();

	m_uType = m_initBaseInfo.uType;
	_tstring strUrl = m_initBaseInfo.csStartUrl;
	//GLOG(strUrl.c_str());
	m_hClientWnd = m_initBaseInfo.hSelfWnd;
	//SetBrowserSize(PET_VIEW,m_initBaseInfo.szBrowser.cx,m_initBaseInfo.szBrowser.cy);
	SetCefCookie();
	_tstring strAllGame = m_initBaseInfo.csAllGameUrl;
	if (!strAllGame.empty())
	{
		//Navigate(strAllGame,0);
		//Sleep(200);
		NavigatePage(strAllGame);
	}
	//GLOG(strAllGame.c_str());

	if (!strUrl.empty())
	{
		Navigate(strUrl,m_initBaseInfo.dwMode);
	}
}
void ClientHandler::NavigatePage(_tstring &sURL)
{

	CefWindowInfo info;
	info.SetAsOffScreen(NULL);
	CefBrowserSettings browserDefaults;
	m_hBrowserHandler = new CBrowserHandler();
	CefBrowserHost::CreateBrowser(info, static_cast<CefRefPtr<CefClient> >(m_hBrowserHandler),sURL.toNarrowString().c_str(),browserDefaults,NULL);
}

bool ClientHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
								  const CefKeyEvent& event,
								  CefEventHandle os_event,
								  bool* is_keyboard_shortcut)
{
	if (event.type == KEYEVENT_RAWKEYDOWN && browser)
	{
		if (event.character == VK_F12)
		{
			int nDevTool = 0;
			nDevTool = GetPrivateProfileInt(INI_CFG_DEVTOOLS,INI_CFG_DEVTOOLS,nDevTool,theDataPool.GetUserProfilePath().c_str());
			if (nDevTool == 1)
			{
				std::string devtools_url = browser->GetHost()->GetDevToolsURL(true);
				browser->GetMainFrame()->ExecuteJavaScript(
					"window.open('" +  devtools_url + "');", "about:blank", 0);
			}
		}
		else if (event.character == VK_F5)
		{
			browser->ReloadIgnoreCache();
		}
	}

	REQUIRE_UI_THREAD();
	return false;
}
void ClientHandler::SetCefCookie()
{
	if (m_initBaseInfo.csSessionId[0] !='\0')
	{
		USES_CONVERSION;
		CBrowserImpl::GetInstance()->SetCefCookie(A2T(m_initBaseInfo.csSessionId),A2T(m_initBaseInfo.csCookieUrl),A2T(m_initBaseInfo.csCookieFolder));
	}
}

void  ClientHandler::ThreadStart()
{
	Thread::start() ;
}


void ClientHandler::SetBrowserSize(PaintElementType type, int width, int height)
{
	// Lock();
	m_mBrowser.lock();
	m_nHeight = height;
	m_nWidth = width;
	m_mBrowser.unlock();
	if (m_Browser)
	{
		m_Browser->GetHost()->WasResized();
	}
	// Unlock();
}
void ClientHandler::Invalidate(CefRect *rect)
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
		m_Browser->GetHost()->Invalidate(*rect, PET_VIEW);
	} 
}
void ClientHandler::OnPopupSize(CefRefPtr<CefBrowser> browser,
								   const CefRect& rect)
{
	if (rect.width > 0 && rect.height > 0)
	{
		popup_rect_ = rect;
	}
}
bool ClientHandler::GetViewRect(CefRefPtr<CefBrowser> browser,
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
bool ClientHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
									 CefRect& rect)
{
	REQUIRE_UI_THREAD();
	return GetViewRect(browser,rect);

}
void ClientHandler::OnPopupShow(CefRefPtr<CefBrowser> browser,
								   bool show) 
{
	REQUIRE_UI_THREAD();
	// g_BrowserDelegate.HandlePopupChange(this,browser,show,rect);
	CBrowserImpl::GetInstance()->PopupShow(browser,show);
	if(!show) 
	{
		popup_rect_.Set(0,0,0,0);
	}
}
void ClientHandler::OnPaint(CefRefPtr<CefBrowser> browser,
							PaintElementType type,
							const RectList& dirtyRects,
							const void* buffer,
							int width, int height)
{
	m_mBrowser.lock();

	if (type == PET_VIEW)
	{
		if (width != m_nWidth || height!= m_nHeight)
		{
			m_mBrowser.unlock();
			return;
		}
	}
	m_mBrowser.unlock();
	
	REQUIRE_UI_THREAD();
	CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
	for (; i != dirtyRects.end(); ++i) 
	{
		const CefRect& rect = *i;
		//GLOG(_T("rect.x = %d, rect.y = %d,rect.width = %d,rect.height= %d type = %d"),rect.x,rect.y,rect.width,rect.height,type);
		CBrowserImpl::GetInstance()->HandlePaint(browser,type,rect,buffer,width,height);
	}
	//::SendMessage(CBrowserImpl::GetInstance()->m_hBrowserMainWnd,WM_COREBROWSERMESSAGE,0,(LPARAM)&msgdata);
}
void ClientHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
									  CefCursorHandle cursor)
{
	REQUIRE_UI_THREAD();
	core_msg_cursorchange cursorchange;
	cursorchange.lCursorId = (long)cursor;
	//GLOG(_T("lCursorId = %d"),cursorchange.lCursorId);
	::SendMessage(CBrowserImpl::GetInstance()->GetHwnd(),WM_COREMESSAGE,0,(LPARAM)&cursorchange);
//	GLOG(_T("lCursorId = %d"),cursorchange.lCursorId);
}
void ClientHandler::NotifyDownloadComplete(const CefString& fileName)
{
	CefPostTask(TID_UI,NewCefRunnableFunction(&ClientHandler::OpenFolder,fileName));
	
}
void ClientHandler::OpenFolder(const CefString& fileName)
{
	TCHAR   szPath[MAX_PATH];
	TCHAR    szPath1[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE,
		NULL, 0, szPath))) 
	{
		return ;
	}
	std::wstring str(fileName.c_str());
	lstrcpynW(szPath1,str.c_str(),MAX_PATH);
	LPITEMIDLIST  pidl;
	LPCITEMIDLIST cpidl;
	LPCITEMIDLIST cpidl1;
	LPSHELLFOLDER pDesktopFolder;
	ULONG         chEaten;
	ULONG         dwAttributes;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{

		if (FAILED(pDesktopFolder->ParseDisplayName(NULL,0,szPath,&chEaten,&pidl,&dwAttributes)))
		{
			return ;
		}
		cpidl = pidl;

		if (FAILED(pDesktopFolder->ParseDisplayName(NULL,0,szPath1,&chEaten,&pidl,&dwAttributes)))
		{
			return ;
		}
		cpidl1 = pidl;
		HRESULT RE = CoInitialize(NULL);
		int re = SHOpenFolderAndSelectItems(cpidl,1,&cpidl1,NULL);
		pDesktopFolder->Release();
	}  
}
// Called if the download fails.
void ClientHandler::NotifyDownloadError(const CefString& fileName)
{

}
_tstring ClientHandler::GetUserProfilePath()
{
	if (!m_sUserProfilePath.empty())
	{
		return m_sUserProfilePath ;
	}

	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess() ,TOKEN_QUERY, &hToken))
	{
		return _T("") ;
	}

	TCHAR szUserProfilePath[MAX_PATH] = {0} ;
	DWORD dwLen = MAX_PATH ;
	if (!GetUserProfileDirectory(hToken, szUserProfilePath, &dwLen))
	{
		return _T("") ;
	}

	m_sUserProfilePath = szUserProfilePath ;
	m_sUserProfilePath.append(_T("\\")) ;
	m_sUserProfilePath.append(APP_PATH) ;
	CreateDirectory(m_sUserProfilePath.c_str(), NULL) ;
	m_sUserProfilePath.append(FILE_CFG_CLIENT) ;

	return m_sUserProfilePath ;
}

