#include "StdAfx.h"
#include "BrowserImpl.h"
#include "CoreMsgOverlay2Browser.h"
#include "data/DataPool.h"
#include "OverlayImpl.h"
#include "cef/cef_runnable.h"
#define  TAB_WIDTH_MIN 110
#define  TAB_WIDTH_MAX 175
#define  PANEL_RIGHTMARGIN 100

CBrowserImpl::CBrowserImpl(void)
:m_hWnd(NULL)
,m_nCurrentIndex(-1)
,view_buffer_(NULL)
,view_buffer_size_()
,view_height(0)
,view_width(0)
,popup_buffer_(NULL)
,popup_buffer_size_(0)
,m_pTab(NULL)
,m_hBrowserMainWnd(NULL)
{
	m_rcDirtyRect = CRect(0,0,0,0);
}
CBrowserImpl::~CBrowserImpl(void)
{
}
int CBrowserImpl::AddBrowserWnd(COverlayBrowserWnd* pBrowserwnd)
{
	return 0;
}
int CBrowserImpl::DelBrowserWnd(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_veBrowserWnd.size())
	{
		return -1;
	}
	m_mBrowser.lock();
// 	if (m_pTab)
// 	{
// 		m_pTab->DeleteTab(nIndex);
// 		//GLOG(_T("m_pTab->DeleteTab nIndex = %d"),nIndex);
// 	}
	::SendMessage(m_hBrowserMainWnd,WM_COREBROWSEREVENTMESSAGE,MAKEWPARAM(0,nIndex),0);
	AdjustTabItmeSize(TRUE);
	m_veBrowserWnd[nIndex]->DestroyWindow();
	delete m_veBrowserWnd[nIndex];
	m_veBrowserWnd.erase(m_veBrowserWnd.begin() + nIndex);
	if (m_veBrowserWnd.empty())
	{
		m_nCurrentIndex = -1;
	}
	else if (nIndex == m_veBrowserWnd.size())
	{
		m_nCurrentIndex = m_veBrowserWnd.size() -1 ;
	}
	else
	{
		m_nCurrentIndex = nIndex;
	}
// 	if (m_pTab)
// 	{
// 		m_pTab->SetSel(nIndex);
// 	}
	m_mBrowser.unlock();
	
	return 0;
}
int CBrowserImpl::GetBrowserWndIndex(CefRefPtr<ClientOSRHandler> Handler)
{
	for (int i = 0 ; i < m_veBrowserWnd.size() ; i++)
	{
		if (m_veBrowserWnd[i]->m_BrowserHandler == Handler)
		{
			return i;
		}
	}
	return -1;
}
void CBrowserImpl::HandlePaint(ClientOSRHandler* BrowserHandler,CefRefPtr<CefBrowser> browser,
							   int type, const CefRect& dirtyRect,
							   const void* buffer,int width, int height)
{
	m_mBrowser.lock();
	if (m_nCurrentIndex != GetBrowserWndIndex(BrowserHandler) || m_nCurrentIndex == -1)
	{
		m_mBrowser.unlock();
		return;
	}
// 	if (nWidth != m_nWidth || nHeight != m_nHeight)
// 	{
// 		m_mBrowser.unlock();
// 		return;
// 	}
	m_cefPopupRect = m_veBrowserWnd[m_nCurrentIndex]->m_BrowserHandler->GetPopupCefRect();
	if (type == PET_VIEW)
	{

		//SendCoreMsg(type,dirtyRect,buffer,nWidth ,nHeight);
		SetBuffer(width,height,true,buffer);
		CopyBuffer(type,dirtyRect,buffer,width ,height);
		::InvalidateRect(m_veBrowserWnd[m_nCurrentIndex]->m_hWnd,CRect(dirtyRect.x + BROWSER_OFFSET_LEFT ,dirtyRect.y + BROWSER_OFFSET_TOP ,dirtyRect.x + dirtyRect.width + BROWSER_OFFSET_LEFT,dirtyRect.y + dirtyRect.height + BROWSER_OFFSET_TOP),TRUE);
	}
	else if (type == PET_POPUP)
	{
		if (m_cefPopupRect.width > 0)
		{
			//SendCoreMsg(type,rect,buffer,rect.width ,rect.height);
			SetBuffer(m_cefPopupRect.width,m_cefPopupRect.height,false,buffer);
			CopyBuffer(type,m_cefPopupRect,buffer,m_cefPopupRect.width ,m_cefPopupRect.height);
			::InvalidateRect(m_veBrowserWnd[m_nCurrentIndex]->m_hWnd,CRect(m_cefPopupRect.x + BROWSER_OFFSET_LEFT ,m_cefPopupRect.y + BROWSER_OFFSET_TOP ,m_cefPopupRect.x + m_cefPopupRect.width + BROWSER_OFFSET_LEFT,m_cefPopupRect.y + m_cefPopupRect.height + BROWSER_OFFSET_TOP),TRUE);
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
		if (dst_size != view_buffer_size_) 
		{
			if (view_buffer_)
				delete [] view_buffer_;
			view_buffer_ = new char[dst_size];
			view_buffer_size_ = dst_size;
		}
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
	if (type == 0)
	{
		view_height = nHeight;
		view_width = nWidth;
		char *pData = view_buffer_ + (dirtyRect.y * view_width + dirtyRect.x)*4;
		char *pBuffer = ((char *)buffer + (dirtyRect.y * view_width + dirtyRect.x)*4);
		if (!pData || !pBuffer)
		{
			return;
		}
		for (int y = dirtyRect.y ; y < dirtyRect.y + dirtyRect.height ; y++)
		{
			memcpy(pData,pBuffer,dirtyRect.width * 4);
			pData += view_width *4;
			pBuffer +=  view_width *4;
		}
	}
	if(popup_buffer_)
	{
		CRect rc;
		if ( !IntersectCefRect(rc,dirtyRect,m_cefPopupRect))
		{
			return;
		}
		char *pData = view_buffer_ + (rc.top * view_width + rc.left)*4;
		char *pBuffer = (char *)popup_buffer_ + (((rc.top - m_cefPopupRect.y) * m_cefPopupRect.width) + rc.left - m_cefPopupRect.x) *4;
		if (!pData || !pBuffer)
		{
			return;
		}
		for (int y = rc.top ; y < rc.top+ rc.Height() ; y++)
		{
			memcpy(pData,pBuffer,rc.Width() * 4);
			pData += view_width *4;
			pBuffer +=  m_cefPopupRect.width *4;
		}
	}
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
void CBrowserImpl::GetOffSetRect(CRect &rc)
{
	rc.left += BROWSER_OFFSET_LEFT;
	rc.top += BROWSER_OFFSET_TOP;
}
int CBrowserImpl::InsertTab(int nIndex,HWND hwnd,LPCTSTR lpctstr,bool bActive)
{
	if (!m_pTab)
	{
		return 0;
	}
	_tstring strText;
	strText.Format(4,_T("/c=%x%s,linkh=%x/%s"),RGB(170,170,170),TABTILE_FONT,RGB(243,243,243),lpctstr);
	if (nIndex == -1 )
	{
		m_pTab->AddTab(hwnd,strText.c_str());
		nIndex = m_pTab->GetTabCount() -1;
	}
	else
	{
		m_pTab->AddTab(hwnd, strText.c_str(), nIndex);
		
	}
	::SendMessage(m_hBrowserMainWnd,WM_COREBROWSEREVENTMESSAGE,MAKEWPARAM(1,nIndex),(LPARAM)lpctstr);
	AdjustTabItmeSize();

	if (bActive)
	{
		m_pTab->SetSel(nIndex);
		m_pTab->SetTabVisible(nIndex);
	}
	return 0;
}
void CBrowserImpl::AdjustTabItmeSize(BOOL bDelete)
{
	if (!m_pTab)
	{
		return;
	}
	int nTabWidth  = m_pTab->GetWidth() - PANEL_RIGHTMARGIN;
	
	int nCount = m_pTab->GetTabCount();
	int nItemWidth = TAB_WIDTH_MAX;
	int nNum = nCount;
	if (bDelete)
	{
		--nNum;
	}
	if (nNum > 0)
	{
		nItemWidth = nTabWidth / nNum;
		if (TAB_WIDTH_MIN > nItemWidth )
		{
			nItemWidth = TAB_WIDTH_MIN;
		}
		else if (nItemWidth > TAB_WIDTH_MAX)
		{
			nItemWidth = TAB_WIDTH_MAX;
		}
	}
	for (int i=0; i < nCount ; i++)
	{
		m_pTab->SetTabWidth(i,nItemWidth);
	}
}

int CBrowserImpl::OnBrowserMsg(WPARAM wParam,LPARAM lParam)
{
	   LPBROWSERMSGDATA pMsgData = (LPBROWSERMSGDATA) lParam;
	   //OutputDebugStringA("CBrowserImpl::OnBrowserMsg\n");
	   if (!pMsgData)
	   {
		 //  OutputDebugStringA(" return 0;\n");
		   return 0;
	   }
	   switch(pMsgData->msgid)
	   {
	    case BROWSER_HANDLER_BEFORECREATE:
		   {
			 
			int nIndex = GetBrowserWndIndex(pMsgData->pParentHandler);
			CreateBrowserWnd(pMsgData->pHandler, ++nIndex);
			
		   }
		   break;
		case BROWSER_HANDLER_POPUPCHANGE:
			{
				HandlePopupChange(pMsgData->pHandler,pMsgData->browser,pMsgData->show,pMsgData->rect);
			}
			break;
		case BROWSER_HANDLER_TITLECHANGE:
			{
				if (m_pTab)
				{
					int nIndex = GetBrowserWndIndex(pMsgData->pHandler);
					if (nIndex != -1)
					{
					  SetTabText(nIndex);
					}
					
				}
			}
			break;
		case BROWSER_HANDLER_CURSORCHANGE:
			{
				unsigned long lCursorId = 32512;
				map<HCURSOR,unsigned long>::iterator it = m_CursorMap.find(pMsgData->hCursor);
				if ( it != m_CursorMap.end())
				{
					lCursorId = it->second;
				}
				COverlayImpl::GetInstance()->SendCursorMsg(lCursorId);	
			}
			break;
		case BROWSER_HANDLER_CLOSE:
			{
				//OutputDebugStringA("CBrowserImpl::OnBrowserMsg(\n");
				if (m_pTab)
				{
					m_pTab->DeleteTab(GetBrowserWndIndex(pMsgData->pHandler));
				}
				//DeleteTab
				//DelBrowserWnd(GetBrowserWndIndex(pMsgData->pHandler));
			}
			break;
		default:
			break;
	   }
	   return 0;
}
int CBrowserImpl::OnProcessMsg(core_msg_header * pHeader)
{
	if (!pHeader)
	{
		return -1;
	}
	switch(pHeader->dwCmdId)
	{
	case CORE_MSG_NAVIGATE:
		{
			core_msg_navigate *pNavigate = (core_msg_navigate*)pHeader;
			USES_CONVERSION;
			CreateBrowser(A2T(pNavigate->szUrl),-1);
		}
		break;
	default:
		break;
	}
	return 0;
}
void CBrowserImpl::HandlePopupChange(CefRefPtr<ClientOSRHandler> BrowserHandler,CefRefPtr<CefBrowser> browser, bool show,
										 const CefRect& rect)
{
	if (m_nCurrentIndex != GetBrowserWndIndex(BrowserHandler))
	{	
		return;
	}
	if (!show)
	{		if (popup_buffer_)
			delete [] popup_buffer_;
		popup_buffer_ = NULL;		popup_buffer_size_ = 0;		if (BrowserHandler)
		{
			BrowserHandler->Invalidate(NULL);
		}
	}	
}
void CBrowserImpl::GetBrowserClientRect(CRect &rcParent,CRect &rect)
{
	rect.left = rcParent.left + BROWSER_OFFSET_LEFT;
	rect.right = rcParent.right - BROWSER_OFFSET_RIGHT;
	rect.top = rcParent.top + BROWSER_OFFSET_TOP;
	rect.bottom = rcParent.bottom - BROWSER_OFFSET_BOTTOM;	
}
void CBrowserImpl::SetBrowserSize(HWND hWnd, int nIndex)
{
	CRect rcClient;
	CRect rc;
	::GetClientRect(hWnd,&rc);
	GetBrowserClientRect(rc,rcClient);
	SetBrowserSize(nIndex,rcClient.right - rcClient.left,rcClient.bottom - rcClient.top);
}
void CBrowserImpl::BrowserInit(HINSTANCE hInstance,CefRefPtr<CefApp> app)
{

	USES_CONVERSION;
	CefSettings settings;
	settings.single_process = true;
	settings.remote_debugging_port = 9087;
	CBrowserImplbase::BrowserInit(hInstance,A2T(_tstring("Mozilla/5.0 (Windows NT 5.1) AppleWebKit/534.30 (KHTML, like Gecko) Chrome/12.0.742.112 Safari/534.30 PWECoreOverlay/1.0").toUTF8().c_str()),app.get(),settings);
}
void CBrowserImpl::CreateBrowser(LPCTSTR szUrl,int nTabIndex,bool bActive)
{

	CefWindowInfo windowInfo;
	windowInfo.SetAsOffScreen(COverlayImpl::GetInstance()->GetMainWnd()->m_hWnd);
	CefRefPtr<ClientOSRHandler> BrowserHandler (new ClientOSRHandler());

	CBrowserImplbase::CreateBrowser(szUrl,BrowserHandler,windowInfo);


	CreateBrowserWnd(BrowserHandler,nTabIndex,bActive);	
}
COverlayBrowserWnd * CBrowserImpl::CreateBrowserWnd(ClientOSRHandler *pHandeler,int nIndex,bool bActive)
{
	if (nIndex < 0 || nIndex > m_veBrowserWnd.size())
	{
		nIndex = m_veBrowserWnd.size();
	}
 	COverlayBrowserWnd *pBrowserWnd = new COverlayBrowserWnd(pHandeler);
	pBrowserWnd->Create(m_hBrowserMainWnd,CRect(0,0,1,1),NULL,WS_CHILD | WS_CLIPCHILDREN);
	m_mBrowser.lock();
 	m_veBrowserWnd.insert(m_veBrowserWnd.begin() + nIndex,pBrowserWnd);
	m_mBrowser.unlock();
	//SetBrowserSize(pBrowserWnd->m_hWnd,nIndex);
 	if (pHandeler)
 	{
 		pHandeler->SetWnd(pBrowserWnd->m_hWnd);
 	}
 	if (bActive)
 	{
		m_mBrowser.lock();
 		m_nCurrentIndex = nIndex;
		m_mBrowser.unlock();
 	}
	InsertTab(nIndex,pBrowserWnd->m_hWnd,_T("Loading..."),bActive);
	return pBrowserWnd;
}
void CBrowserImpl::SetBrowserSize(int nIndex,int nwidth, int nheight)
{
	if (nheight < 0 || nwidth < 0)
	{
		return;
	}
	m_mBrowser.lock();
	m_nWidth = nwidth;
	m_nHeight = nheight;
	m_mBrowser.unlock();
	if (nIndex >= 0 && nIndex < m_veBrowserWnd.size())
	{
		m_veBrowserWnd[nIndex]->m_BrowserHandler->SetBrowserSize(PET_VIEW,nwidth,nheight);
	}
	
}
BOOL CBrowserImpl::ClientToBrowserRect(HWND hWnd,CRect &rcIn,CRect &rcOut)
{
	CRect rc ,rect;
	::GetClientRect(hWnd,&rc);
	GetBrowserClientRect(rc,rect);
	return ::IntersectRect(rcOut,rect,rcIn);
}

void CBrowserImpl::UnInit()
{
	ClearAllBrowser();
	///https://code.google.com/p/chromiumembedded/issues/detail?id=1179
	CefShutdown();

	GetCoreUI()->Uninitialize();
}

void CBrowserImpl::DrawBrowser(HDC hdc,CRect &rect )
{
	
 	if (view_buffer_ && hdc)
 	{
		m_mBrowser.lock();
		BITMAPINFOHEADER m_bmpHeader;
 		m_bmpHeader.biSize = sizeof(m_bmpHeader);
 		m_bmpHeader.biBitCount = 32;
 		m_bmpHeader.biCompression = BI_RGB;
 		m_bmpHeader.biWidth = view_width;
 		m_bmpHeader.biHeight = -(view_height);
 		m_bmpHeader.biPlanes = 1;
 		SetDIBitsToDevice(hdc,rect.left,rect.top , rect.Width(),rect.Height(), rect.left - BROWSER_OFFSET_LEFT,view_height + BROWSER_OFFSET_TOP - rect.bottom, 0, view_height,view_buffer_, (const BITMAPINFO *)&m_bmpHeader, DIB_RGB_COLORS);
		m_mBrowser.unlock();
 	}
}

BOOL CBrowserImpl::IsCurrentBrowser(COverlayBrowserWnd *pBrowserWnd)
{
	//m_mBrowser.lock();
	if (pBrowserWnd == NULL || m_nCurrentIndex < 0 || m_nCurrentIndex >= m_veBrowserWnd.size())
	{
		return FALSE;
	}
	if (pBrowserWnd == m_veBrowserWnd[m_nCurrentIndex])
	{
		return TRUE;
	}
	return FALSE;
	//m_mBrowser.unlock();
}
void CBrowserImpl::ClearAllBrowser()
{
	m_nCurrentIndex = -1;
	for (int i = 0 ; i < m_veBrowserWnd.size() ; i++)
	{
		DestroyWindow(m_veBrowserWnd[i]->m_hWnd) ;
		delete m_veBrowserWnd[i];
	}
	m_veBrowserWnd.clear();	
}
void CBrowserImpl::SetCurrentBrowser(int nIndex,BOOL bUpdate)
{

	m_mBrowser.lock();
	if (nIndex >= 0 && nIndex < m_veBrowserWnd.size())
	{
		m_nCurrentIndex = nIndex;
		m_mBrowser.unlock();
		//m_veBrowserWnd[nIndex]->SetFocus();
		CefRefPtr<CefBrowser>  browser = m_veBrowserWnd[nIndex]->m_BrowserHandler->GetBrowser();
		if (browser && bUpdate)
		{
			//browser->SendFocusEvent(true);
			m_veBrowserWnd[nIndex]->m_BrowserHandler->Invalidate(NULL);	
		}
	}
	else
	{
		m_nCurrentIndex = -1;
		m_mBrowser.unlock();
	}
	//
}
void CBrowserImpl::SetSelTab(int nIndex,HWND hWnd)
{
	if (!m_pTab)
	{
		return;
	}
	m_pTab->SetSel(nIndex);
	m_pTab->SetTabVisible(nIndex);
}
 BOOL CBrowserImpl::GetBrowserTitle(int nIndex,_tstring &strTitle)
{
	if (nIndex >=0 && nIndex < m_veBrowserWnd.size())
	{
		m_veBrowserWnd[nIndex]->m_BrowserHandler->GetTitle(strTitle);
		return TRUE;
	}
   return FALSE;
}
BOOL CBrowserImpl::SetTabText(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_veBrowserWnd.size())
	{
		ISonicAnimation *pItem = m_pTab->GetTabItem(nIndex);
		int nVal = 0;
		_tstring strText;
		GetBrowserTitle(nIndex,strText);
		if (pItem->GetAttr(_T("loading"),nVal))
		{
			_tstring strTextFormat;
			strTextFormat.Format(5,_T("/p=%d, frame_interval=30,align=1//c=%x%s,align=1,single_line=2, linkh=%x/  %s"),GetCoreUI()->GetImage(_T("ANI_LOADING"))->GetObjectId(),RGB(170,170,170),TABTILE_FONT,RGB(243,243,243),GetSonicUI()->HandleRawString(strText.c_str(),RSCT_DOUBLE));
			m_pTab->SetTabText(nIndex,strTextFormat.c_str());
		}
		else
		{
			_tstring strTextFormat;
			strTextFormat.Format(4,_T("/c=%x%s,align=1,single_line=2,linkh=%x/%s"),RGB(170,170,170),TABTILE_FONT,RGB(243,243,243),GetSonicUI()->HandleRawString(strText.c_str(),RSCT_DOUBLE));
			m_pTab->SetTabText(nIndex,strTextFormat.c_str());
		}	
		return TRUE;
	}
	return FALSE;
}
void CBrowserImpl::SetCookie()
{
	core_msg_cookie *pMsgCookie = COverlayImpl::GetInstance()->GetMsgCookie();
 	if (pMsgCookie->csSessionId[0] !='\0')
 	{
		CBrowserImplbase::SetCefCookie(pMsgCookie->csSessionId,pMsgCookie->csCookiePage,L"cef_cookies" );
	}

}
void CBrowserImpl::SetBillingCookie()
{
	core_msg_cookie *pMsgCookie = COverlayImpl::GetInstance()->GetMsgCookie();
	
	if ( pMsgCookie->csGameAbbrName[0] == '\0')
	{
		return;
	}
	_tstring scookieURL = pMsgCookie->csBillingPage;
	_tstring sDomin = scookieURL.Right(scookieURL.length() - scookieURL.find(_T(".")) - 1);
	sDomin = sDomin.Left(sDomin.find(_T("/")));
	CefCookie cookie;
	CefString(&cookie.name).FromASCII("game_abbr");

	CefString(&cookie.value).FromWString(wstring(pMsgCookie->csGameAbbrName));
	CefString(&cookie.path).FromASCII("/");
	CefString(&cookie.domain).FromASCII(sDomin.toNarrowString().c_str());

	CefPostTask(TID_IO, NewCefRunnableFunction(&CBrowserImpl::IOT_Set, CefString(scookieURL.toNarrowString().c_str()), cookie));
}
void CBrowserImpl::ActiveThreadWnd()
{
	overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
	if (pHeader)
	{
		CefPostTask(TID_UI,NewCefRunnableFunction(&CBrowserImplbase::ActiveThreadWindow,pHeader->hGame));
	}
}

HWND CBrowserImpl::GetParent()
{
    overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
    if (pHeader)
    {
        return pHeader->hGame;
    }

    return NULL;
}