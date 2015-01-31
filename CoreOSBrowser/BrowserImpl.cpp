#include "StdAfx.h"
#include "BrowserImpl.h"
#include "data/DataPool.h"
#include "toolkit.h"
#include "cef/cef_runnable.h"
#include <afxwin.h>
#define FILE_CSS_SCROLLBAR	_T("scrollbar.css")
#define FOLDER_CEF_CACHE	_T("cef_cache")
#define	TEXT_USER_AGENT		"Mozilla/5.0 (Windows NT 5.1) AppleWebKit/534.30 (KHTML, like Gecko) Chrome/12.0.742.112 Safari/534.30 PWECoreClient/1.0"
#define TEXT_CSS_HEADER		"data:text/css;charset=utf-8;base64,"

CBrowserImpl::CBrowserImpl(void)
:popup_buffer_(NULL)
,m_hWnd(NULL)
,m_hMsgClient(NULL)
,popup_buffer_size_(0)
,view_width(0)
,view_height(0)
{
	m_defRect = CRect(0,0,0,0);
}

CBrowserImpl::~CBrowserImpl(void)
{
}
void CBrowserImpl::BrowserInit(HINSTANCE hInstance,CefRefPtr<CefApp> app)
{
	USES_CONVERSION;
	CefSettings  settings;
	settings.remote_debugging_port = 9086;
	CBrowserImplbase::BrowserInit(hInstance,A2T(_tstring(TEXT_USER_AGENT).toUTF8().c_str()),app.get(),settings);
}
void CBrowserImpl::Init(HWND hParentWnd,HWND hMsgClient,HINSTANCE hInstance,CefRefPtr<CefApp> app)
{
	m_hParentWnd = hParentWnd;
	m_hMsgClient = hMsgClient;
	Thread::start();
	_tstring strName;
	strName.Format(1,("arc%d"),hParentWnd);
	m_smMgr.Open(strName.c_str());
	BrowserInit(hInstance,app.get());
	CreateBrowser(NULL);
}
void CBrowserImpl::InitBaseInfo(HWND hwnd,int nCommandLineType,_tstring &strUrl)
{
	//::MessageBox(NULL,L"InitBaseInfo",L"InitBaseInfo",0);
	m_hWnd = hwnd;

	m_msgClient.Initialize(m_hWnd,-1);
	core_msg_initbaseinfo initBaseInfo;
	initBaseInfo.uType = nCommandLineType;
	initBaseInfo.hBrowserWnd = m_hWnd;
	initBaseInfo.hSelfWnd = m_msgClient.GetSelfWindow();
	lstrcpynA(initBaseInfo.csStartUrl,strUrl.toNarrowString().c_str(),1024);
	//GLOG(_T("initBaseInfo.hSelfWnd = %d,m_hMsgClient = %d"),initBaseInfo.hSelfWnd,m_hMsgClient);
	for (int i = 0 ; i < 5; i++)
	{
		if (SendCoreMsg(&initBaseInfo))
		{
			 break;
		}
	}
	SetInitBaseInfo(initBaseInfo);
}
void  CBrowserImpl::UnInit()
{
	m_msgClient.Uninitialize();
	CefShutdown() ;
}
void CBrowserImpl::CreateBrowser(LPCTSTR szUrl)
{

	CefWindowInfo windowInfo;
	windowInfo.SetAsOffScreen(m_hParentWnd);
	windowInfo.SetTransparentPainting(true);
	m_cefHandler = new ClientHandler();
	//	BrowserHandler->SetBrowserSize(PET_VIEW,sz.cx,sz.cy);
	CBrowserImplbase::CreateBrowser(szUrl,m_cefHandler,windowInfo);
	m_cefHandler->ThreadStart();
}
BOOL CBrowserImpl::SendCoreMsg(core_msg_header * pHeader)
{
	//core_msg_initbaseinfo * pInfo = m_cefHandler->GetInitBaseInfo();
	return m_msgClient.SendCoreMsg(m_hMsgClient,pHeader);
}
void CBrowserImpl::SetInitBaseInfo(core_msg_initbaseinfo & initBaseInfo)
{

	m_mBrowser.lock();
	m_defRect = initBaseInfo.rcDefBrowser;
	if (m_cefHandler)
	{
		CRect rect;
		::GetClientRect(m_hParentWnd,&rect);
		//GLOG(_T("rect.Width() = %d,rect.Height() = %d m_defRect.left = %d m_defRect.top = %d"),rect.Width() - m_defRect.left - m_defRect.right,rect.Height() - m_defRect.top - m_defRect.bottom,m_defRect.left,m_defRect.top);
	   SetBrowserEvent(WM_SIZE,WPARAM(rect.Width() - m_defRect.left - m_defRect.right),LPARAM(rect.Height() - m_defRect.top - m_defRect.bottom));
	}
	m_mBrowser.unlock();
	if (m_cefHandler)
	{
		m_cefHandler->SetInitBaseInfo(initBaseInfo);
	}
}
void CBrowserImpl::GoBack()
{
	if(m_cefHandler->GetBrowser().get())
	{
		if (m_cefHandler->GetBrowser()->CanGoBack())
		{
			m_cefHandler->GetBrowser()->GoBack() ;
		}
	}
}
void CBrowserImpl::GoForward()
{
	if(m_cefHandler->GetBrowser().get())
	{
		if (m_cefHandler->GetBrowser()->CanGoForward())
		{
			m_cefHandler->GetBrowser()->GoForward() ;
		}
	}
}
void CBrowserImpl::Refresh() 
{ 
	if(m_cefHandler->GetBrowser().get()) 
	{
		m_cefHandler->GetBrowser()->ReloadIgnoreCache();
	}
}


void CBrowserImpl::Navigate(_tstring tsURL,int nMode)
{
	if (m_cefHandler.get())
	{
		m_cefHandler->Navigate(tsURL,nMode);
	}
}
LRESULT CBrowserImpl::OnProcessMsg(WPARAM wParam,LPARAM lParam)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;
	}
	switch(pHeader->dwCmdId)
	{
	case CORE_MSG_NAVIGATE_E:
		{
			core_msg_navigate_e *pNavigate = (core_msg_navigate_e *)pHeader;
			Navigate(pNavigate->scUrl,pNavigate->dwMode);
			//USES_CONVERSION;
			//GLOG(A2T(pNavigate->scUrl));
		}
		break;
	case CORE_MSG_BROWSER_EVENT:
		{
			core_msg_browserevent *pBrowserEvent = (core_msg_browserevent*)pHeader;
			switch(pBrowserEvent->dwBrowserEventId)
			{
			case BROWSER_GOBACK:
				{
					GoBack();
				}
				break;
			case BROWSER_GOFORWORD:
				{
					GoForward();
				}
				break;
			case BROWSER_REFRESH:
				{
					Refresh();
				}
				break;
		
			default:
				break;
			}
		}
		break;

	case CORE_MSG_BROWSER_INFO :
		{
			core_msg_browserinfo *pBrowserInfo = (core_msg_browserinfo *)pHeader;
			if (IsPageLoading())
			{
				pBrowserInfo->bLoading = TRUE;
			}
		}
		break;
	case  CORE_MSG_BROWSER_SETCOOOKIE:
		{
			core_msg_setcookie *pSetCookie = (core_msg_setcookie *)pHeader;
			if (m_cefHandler)
			{
				core_msg_initbaseinfo * pInitBaseInfo = m_cefHandler->GetInitBaseInfo();
				USES_CONVERSION;
				lstrcpynA(pInitBaseInfo->csSessionId,pSetCookie->csSessionId,128);
				m_cefHandler->SetCefCookie();
			}
		}
		break;
	case CORE_MSG_BROWSER_DELCOOOKIE:
		{
			core_msg_delcookie * pDelCookie = (core_msg_delcookie *)pHeader;
			DelCookies(CefString(pDelCookie->scUrl),CefString(pDelCookie->scCookieName),_tstring(pDelCookie-> csCookieFolder));
		}
		break;
	case CORE_MSG_BROWSER_MSGEVENT:
		{
			if (m_cefHandler)
			{
				core_msg_msgevent *pMsgEvent = (core_msg_msgevent *)pHeader;
				SetBrowserEvent(pMsgEvent->message,pMsgEvent->wParam,pMsgEvent->lParam);
			}
		}
		break;
	case CORE_MSG_BROWSER_DEFRECT:
		{
			m_mBrowser.lock();
			core_msg_defrect *pDefRect = (core_msg_defrect*)pHeader;
			m_defRect = pDefRect->rcdef;
			if (m_cefHandler)
			{
				CRect rect;
				::GetClientRect(m_hParentWnd,&rect);
				SetBrowserEvent(WM_SIZE,WPARAM(rect.Width() - m_defRect.left - m_defRect.right),LPARAM(rect.Height() - m_defRect.top - m_defRect.bottom));
			}
			
			m_mBrowser.unlock();
		}
		break;
	case CORE_MSG_CURSORCHANGE:
		{
			core_msg_cursorchange * pCursorChange = (core_msg_cursorchange *)pHeader;
			core_msg_cursorchange cursorchange;
			map<HCURSOR,unsigned long>::iterator it = m_CursorMap.find((HCURSOR)pCursorChange->lCursorId);
			cursorchange.lCursorId = 32512;
			if (it != m_CursorMap.end())
			{
				cursorchange.lCursorId = it->second;
				//GLOG(_T("mcursorchange.lCursorId = %d"),cursorchange.lCursorId);
			}
			SendCoreMsg(&cursorchange);
		}
		break;
	default:
		break;
	}
	return 0L;
}

void CBrowserImpl::HandlePopupChange(CefRefPtr<ClientHandler> BrowserHandler,CefRefPtr<CefBrowser> browser, bool show,
									 const CefRect& rect)
{
// 	if (m_nCurrentIndex != GetBrowserWndIndex(BrowserHandler))
// 	{	
// 		return;
// 	}
	if (!show)
	{		if (m_cefHandler)
		{
			m_cefHandler->Invalidate(NULL);
		}
	}	
}
void CBrowserImpl::HandlePaint(CefRefPtr<CefBrowser> browser,
							   int type, const CefRect& dirtyRect,
							   const void* buffer,int width, int height)
{
	m_mBrowser.lock();
	
	m_cefPopupRect = m_cefHandler->GetPopupCefRect();
	if (type == PET_VIEW)
	{
		CopyBuffer(type,dirtyRect,buffer,width ,height);
		::InvalidateRect(m_hParentWnd,CRect(dirtyRect.x + m_defRect.left ,dirtyRect.y + m_defRect.top ,dirtyRect.x + dirtyRect.width + m_defRect.left ,dirtyRect.y + dirtyRect.height + m_defRect.top),TRUE);
// 		core_msg_paint msgPaint;
// 		msgPaint.rect.left = dirtyRect.x + m_defRect.left;
// 		msgPaint.rect.top = dirtyRect.y + m_defRect.top;
// 		msgPaint.rect.right = dirtyRect.x + dirtyRect.width + m_defRect.left;
// 		msgPaint.rect.bottom = dirtyRect.y + dirtyRect.height + m_defRect.top;
 		//GLOG(_T("PET_VIEWrect.left = %d,rect.top = %d,rect.right = %d, rect.bottom = %d m_defRect.top = %d"),dirtyRect.x + m_defRect.left,dirtyRect.y + m_defRect.top,dirtyRect.x + dirtyRect.width + m_defRect.left,dirtyRect.y + dirtyRect.height + m_defRect.top,m_defRect.top);
// 		SendCoreMsg(&msgPaint);
	}
	else if (type == PET_POPUP)
	{

		if (m_cefPopupRect.width > 0)
		{
			//SendCoreMsg(type,rect,buffer,rect.width ,rect.height);
			//SetBuffer(m_cefPopupRect.width,m_cefPopupRect.height,false,buffer);
			SetBuffer(m_cefPopupRect.width,m_cefPopupRect.height,false,buffer);
			CopyBuffer(type,m_cefPopupRect,buffer,m_cefPopupRect.width ,m_cefPopupRect.height);
			::InvalidateRect(m_hParentWnd,CRect(m_cefPopupRect.x + m_defRect.left  ,m_cefPopupRect.y + m_defRect.top  ,m_cefPopupRect.x + m_cefPopupRect.width + m_defRect.left ,m_cefPopupRect.y + m_cefPopupRect.height + m_defRect.top),TRUE);
		//	GLOG(_T("PET_POPUP rect.left = %d,rect.top = %d,rect.right = %d, rect.bottom = %d m_defRect.top = %d"),m_cefPopupRect.x + m_defRect.left,m_cefPopupRect.y + m_defRect.top,m_cefPopupRect.x + m_cefPopupRect.width + m_defRect.left,m_cefPopupRect.y + m_cefPopupRect.height + m_defRect.top,m_defRect.top);
// 			core_msg_paint msgPaint;
// 			msgPaint.rect.left = dirtyRect.x + m_defRect.left;
// 			msgPaint.rect.top = dirtyRect.y + m_defRect.top;
// 			msgPaint.rect.right = dirtyRect.x + dirtyRect.width + m_defRect.left;
// 			msgPaint.rect.bottom = dirtyRect.y + dirtyRect.height + m_defRect.top;
// 			SendCoreMsg(&msgPaint);
		}
	}
	m_mBrowser.unlock();
}
void CBrowserImpl::SetBuffer(int width, int height, bool view,const void* buffer)
{
	int dst_size = width * height * 4;

	// Allocate a new buffer if necesary.
	if (view) 
	{
// 		if (dst_size != view_buffer_size_) 
// 		{
// 			if (view_buffer_)
// 				delete [] view_buffer_;
// 			view_buffer_ = new char[dst_size];
// 			view_buffer_size_ = dst_size;
// 		}
		//memcpy(view_buffer_,buffer,view_buffer_size_);
	} 
	else 
	{
		if (dst_size != popup_buffer_size_) 
		{
			if (popup_buffer_)
				delete [] popup_buffer_;
			popup_buffer_ = new unsigned char[dst_size];
			popup_buffer_size_ = dst_size;
		}
		memcpy(popup_buffer_,buffer,popup_buffer_size_);
	}
}
void CBrowserImpl::CopyBuffer(int type, const CefRect dirtyRect,const void* buffer,int nWidth,int nHeight)
{

	m_smMgr.Lock();
	arcbrowser_sm_header * pHeader = (arcbrowser_sm_header *) m_smMgr.GetHeader();
	if (!m_smMgr.GetBits())
	{
		m_smMgr.Unlock();
		return;
	}
	
	if (type == PET_VIEW)
	{

		if (view_height != nHeight)
		{
			view_height = nHeight;
			pHeader->nHeight = nHeight;
		}
		if (view_width != nWidth)
		{
			view_width = nWidth;
			pHeader->nWidth = nWidth;
		}
		if (nWidth * nHeight * 2 >  view_width * view_height)
		{
			memcpy((char *)m_smMgr.GetBits(),buffer ,view_height * view_width * 4);

		}
		else
		{
			char *pData = (char *)m_smMgr.GetBits() + (dirtyRect.y * view_width + dirtyRect.x)*4;
			char *pBuffer = ((char *)buffer + (dirtyRect.y * view_width + dirtyRect.x)*4);
			if (!pData || !pBuffer)
			{
				m_smMgr.Unlock();
				return;
			}
			for (int y = dirtyRect.y ; y < dirtyRect.y + dirtyRect.height ; y++)
			{
				memcpy(pData,pBuffer,dirtyRect.width * 4);
				pData += view_width *4;
				pBuffer +=  view_width *4;
			}
		}
	}
	else if(type == PET_POPUP)
	{
		memcpy(popup_buffer_,buffer,popup_buffer_size_);
	}
	if(popup_buffer_)
	{
		//GLOG(_T("m_cefPopupRect.left = %d, m_cefPopupRect.top = %d ,m_cefPopupRect.right = %d, m_cefPopupRect.bottom = %d"),m_cefPopupRect.x,m_cefPopupRect.y,m_cefPopupRect.x + m_cefPopupRect.width,m_cefPopupRect.y + m_cefPopupRect.height);
		CRect rc;
		if ( !IntersectCefRect(rc,dirtyRect,m_cefPopupRect))
		{
			m_smMgr.Unlock();
			return;
		}
		char *pData = (char *)m_smMgr.GetBits() + (rc.top * view_width + rc.left)*4;
	
		char *pBuffer = (char *)popup_buffer_ + (((rc.top - m_cefPopupRect.y) * m_cefPopupRect.width) + rc.left - m_cefPopupRect.x) *4;
		if (!pData || !pBuffer)
		{
			m_smMgr.Unlock();
			return;
		}
		for (int y = rc.top ; y < rc.top+ rc.Height() ; y++)
		{
			memcpy(pData,pBuffer,rc.Width() * 4);
			pData += view_width *4;
			pBuffer +=  m_cefPopupRect.width *4;
		}
		//GLOG(_T("rc.left = %d, rc.top = %d ,rc.right = %d, rc.bottom = %d"),rc.left,rc.top,rc.right,rc.bottom);
	}
	m_smMgr.Unlock();
}
void CBrowserImpl::PopupShow(CefRefPtr<CefBrowser> browser,bool show)
{
	m_mBrowser.lock();
	if (!show)
	{		if (popup_buffer_)
			delete [] popup_buffer_;
		popup_buffer_ = NULL;		popup_buffer_size_ = 0;		if (m_cefHandler)
		{
			m_cefHandler->Invalidate(NULL);
		}
	}
	m_mBrowser.unlock();
}
BOOL CBrowserImpl::IntersectCefRect(CRect &rcDst,CefRect rcRect1,CefRect rcRect2)
{
	RECT rc1,rc2;

	rc1.left = rcRect1.x;
	rc1.right = rcRect1.x +rcRect1.width;
	rc1.top = rcRect1.y;
	rc1.bottom = rcRect1.y + rcRect1.height;

	rc2.left = rcRect2.x;
	rc2.right = rcRect2.x +rcRect2.width;
	rc2.top = rcRect2.y;
	rc2.bottom = rcRect2.y + rcRect2.height;

	if (!::IntersectRect(&rcDst,&rc1,&rc2))
	{
		return FALSE;
	}
	return TRUE;
}
long CBrowserImpl::run()
{

	while (true)
	{
		Sleep(2000);
		if (!IsWindow(m_hParentWnd))
		{
			::PostMessage(m_hWnd,WM_CLOSE,0,0);
			Sleep(2000);
			TerminateProcess(GetCurrentProcess(),0);
			return 0;
		}
	}
	return 0;
}
void CBrowserImpl::SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam)
{
	if (message == WM_SIZE)
	{
		if (m_cefHandler)
		{
			m_cefHandler->SetBrowserSize(PET_VIEW,int(wParam),int(lParam));
		}
		return;
	}
	CBrowserImplbase::SetBrowserEvent(m_cefHandler->GetBrowser(),message,wParam,lParam);
}
void CBrowserImpl::ActiveThreadWnd()
{
	CefPostTask(TID_UI,NewCefRunnableFunction(&CBrowserImpl::ActiveThreadWindow,m_hParentWnd));
}
void CBrowserImpl::DelCookies(const CefString& url,const CefString& cookie_name,const _tstring& strCookieFolder)
{
	if (!strCookieFolder.empty())
	{
		_tstring scookieCache = theDataPool.GetBaseDir() + strCookieFolder ;
		CefCookieManager::GetGlobalManager()->SetStoragePath(CefString(scookieCache.toUTF8().c_str()),true);
	}
   CefPostTask(TID_IO, NewCefRunnableFunction(&CBrowserImpl::IOT_Del,url, cookie_name));

   if (!strCookieFolder.empty() && m_cefHandler)
   {
		m_cefHandler->SetCefCookie();
   }
}

