// OverlayBrowserWnd.cpp : implementation file
//

#include "stdafx.h"

#include "OverlayBrowserWnd.h"
#include "BrowserImpl.h"
#include "OverlayImpl.h"
#include "cef/cef_runnable.h"



COverlayBrowserWnd::COverlayBrowserWnd(CefRefPtr<ClientOSRHandler> Handler /*= NULL*/)
:m_BrowserHandler(Handler)
,m_bBrowserFocus(TRUE)
,m_pCoreEdit(NULL)
,m_pBack(NULL)
,m_pForward(NULL)
,m_bMouseIn(FALSE)
,m_pHome(NULL)
{

}

COverlayBrowserWnd::~COverlayBrowserWnd()
{
	if (m_BrowserHandler && m_BrowserHandler->GetBrowser())
	{
		//m_BrowserHandler->GetBrowser()->GetHost()->CloseBrowser(false);
		CefPostTask(TID_UI,
			NewCefRunnableFunction(&COverlayBrowserWnd::CloseBrowser,m_BrowserHandler.get()));
		//m_BrowserHandler->Release();
	}	
	SONICUI_DESTROY(m_pHome);
}
void COverlayBrowserWnd::CloseBrowser(ClientOSRHandler* pBrowserHandler)
{
	if(pBrowserHandler)
	{
		if (pBrowserHandler->GetBrowser()->IsLoading())
		{
			 pBrowserHandler->GetBrowser()->StopLoad();
		}
	  pBrowserHandler->GetBrowser()->GetHost()->CloseBrowser(false);
	}
}
int COverlayBrowserWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_browserwnd"));
	m_pCoreEdit = GetCoreUI()->GetObjByID<WTL::CCoreEdit>(m_hWnd, _T("overlay_address_edit"));
	GetSonicUI()->SkinFromHwnd(m_hWnd)->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);
	InitEvent();
	return 0;
}
void COverlayBrowserWnd::OnFinalMessage(HWND hWnd)
{
	//delete this;  must after wnd destory; 
}

void COverlayBrowserWnd::OnPaint(CDCHandle dc)
{
	CPaintDC Paintdc(m_hWnd);
	CRect rc;
	CRect rcClient;
	GetClientRect(&rcClient);
	rc.left = 0;
	rc.top = BROWSER_OFFSET_TOP -1;
	rc.right = rcClient.right;
	rc.bottom = BROWSER_OFFSET_TOP;
	//rc.bottom = 44;
	GetSonicUI()->FillSolidRect(Paintdc.m_hDC,rc,ARGB(255,34,36,41));
	
	if (CBrowserImpl::GetInstance()->IsCurrentBrowser(this))
	{
		//GetUpdateRect()
		CRect rcClient ,rcBrowser;
		GetClipBox(Paintdc.m_hDC,&rcClient);
		//Glog(_T("rcClient.right = %d rcClient.bottom = %d"),rcClient.right,rcClient.bottom);
		if (CBrowserImpl::GetInstance()->ClientToBrowserRect(m_hWnd,rcClient,rcBrowser))
		{
			CBrowserImpl::GetInstance()->DrawBrowser(Paintdc.m_hDC,rcBrowser);
		}
		

		
	}
}

void COverlayBrowserWnd::InitEvent()
{
	ISonicPaint * pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	ISonicPaint * pPaintEditbg = GetSonicUI()->CreatePaint();
	CRect rcClient;
	GetClientRect(&rcClient);
	pPaintEditbg->Create(FALSE);
	pPaintEditbg->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&COverlayBrowserWnd::DrawEditBg);
	pPaint->AddObject(pPaintEditbg->GetObjectId(),140,8,TRUE);


	
	m_pBack = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("browser_osweb_back"));
	m_pBack->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayBrowserWnd::OnBackClick);
	m_pBack->Enable(FALSE);

	m_pForward = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("browser_osweb_forward"));
	m_pForward->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayBrowserWnd::OnForwardClick);
	m_pForward->Enable(FALSE);

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("browser_osweb_refresh"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayBrowserWnd::OnRefreshClick);
	/*GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("browser_osweb_delete"))->
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayBrowserWnd::OnDeleteClick);*/
	m_pHome = GetSonicUI()->CreateString();
	m_pHome->Format(_T("/a,p=%d/"),GetCoreUI()->GetImage(_T("BTN_OSWEB_HOME"))->GetObjectId());
	m_pHome->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayBrowserWnd::OnHomeClick);
	
		
	
}
void COverlayBrowserWnd::OnBackClick(ISonicBase *, LPVOID pReserve)
{
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser && browser->CanGoBack())
	{
		browser->GoBack();
	}
}
void COverlayBrowserWnd::OnForwardClick(ISonicBase *, LPVOID pReserve)
{
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser && browser->CanGoForward())
	{
		browser->GoForward();
	}
}
void COverlayBrowserWnd::OnRefreshClick(ISonicBase *, LPVOID pReserve)
{
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser)
	{
		browser->ReloadIgnoreCache();
	}
}
void COverlayBrowserWnd::OnDeleteClick(ISonicBase *, LPVOID pReserve)
{
	//CBrowserImpl::GetInstance()->DelBrowserWnd(CBrowserImpl::GetInstance()->GetBrowserWndIndex(m_BrowserHandler));
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser)
	{
		browser->StopLoad();
	}
}
void COverlayBrowserWnd::OnHomeClick(ISonicBase *, LPVOID pReserve)
{
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser)
	{
		browser->GetMainFrame()->LoadURL(_T("http://www.google.com "));
	}
}
void COverlayBrowserWnd::OnMouseLeave()
{
	if (m_bMouseIn)
	{
		//m_BrowserHandler->SetBrowserEvent(WM_MOUSELEAVE,wParam,lParam);
		//GLOG(_T("OnMouseLeave.....(WM_MOUSELEAVE"));
		m_bMouseIn = FALSE;
		COverlayImpl::GetInstance()->SendCursorMsg(32512);

	}
}
LRESULT COverlayBrowserWnd::OnInputMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     OnMessage(uMsg,wParam,lParam);
	return 0;
}
void COverlayBrowserWnd::OnKillFocus(CWindow wndFocus)
{
	OnMessage(WM_KILLFOCUS,0,0);
	SetMsgHandled(FALSE);
}


BOOL COverlayBrowserWnd::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			CPoint pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (PtInBrowserRect(pt))
			{
				pt.x -= BROWSER_OFFSET_LEFT;
				pt.y -= BROWSER_OFFSET_TOP ;
				::SendMessage(m_hWnd,WM_SETFOCUS,0,0);
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
				m_bBrowserFocus = TRUE;

			}
			else
			{
				if (m_BrowserHandler->GetBrowser())
				{
					m_BrowserHandler->GetBrowser()->GetHost()->SendFocusEvent(false);
				}
				m_bBrowserFocus = FALSE;
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		//case WM_MOUSELEAVE:
	//case WM_MOUSEWHEEL:
		{
			CPoint pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (PtInBrowserRect(pt))
			{
				pt.x -= BROWSER_OFFSET_LEFT;
				pt.y -= BROWSER_OFFSET_TOP ;
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			CPoint pt;
			pt.x = LOSHORT(lParam) ;
			pt.y = HISHORT(lParam);
			CPoint  point(pt);
			CRect rc;
			CArcShareMemMgr<overlay_sm_header> memMgr =  COverlayImpl::GetInstance()->GetMenMgr();
			if (memMgr.GetHeader())
			{
				::ScreenToClient(memMgr.GetHeader()->hGame,&point);
				::ClientToScreen(COverlayImpl::GetInstance()->GetMainWnd()->m_hWnd,&point);
			}
			ScreenToClient(&point);
			if (PtInBrowserRect(point))
			{
				point.x -= BROWSER_OFFSET_LEFT;
				point.y -= BROWSER_OFFSET_TOP ;
				SetBrowserEvent(message,wParam,MAKELPARAM(point.x,point.y));
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			CPoint pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			//GLOG(_T("WM_MOUSEMOVE pt.x = %d pt.y=%d"),pt.x,pt.y);
			
			if (PtInBrowserRect(pt))
			{
				pt.x -= BROWSER_OFFSET_LEFT;
				pt.y -= BROWSER_OFFSET_TOP ;
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
				m_bMouseIn = TRUE;
			}
			else if(m_bMouseIn)
			{
				//m_BrowserHandler->SetBrowserEvent(WM_MOUSELEAVE,0,0);
				m_bMouseIn = FALSE;
				//GLOG(_T("SetBrowserEvent(WM_MOUSELEAVE"));
				COverlayImpl::GetInstance()->SendCursorMsg(32512);
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
		//	GLOG(_T("SetBrowserEvent(WM_MOUSELEAVE"));
// 			if (m_bMouseIn)
// 			{
// 				//m_BrowserHandler->SetBrowserEvent(WM_MOUSELEAVE,wParam,lParam);
// 				GLOG(_T("SetBrowserEvent(WM_MOUSELEAVE"));
// 				m_bMouseIn = FALSE;
// 				COverlayImpl::GetInstance()->SendCursorMsg(32512);
// 				
// 			}
			
		}
		break;
	case WM_KEYDOWN:
// 		{
// 			if (wParam == VK_RETURN)
// 			{
// 				CString strText;
// 				::GetWindowText(m_pCoreEdit->m_hWnd,strText.GetBuffer(::GetWindowTextLength(m_pCoreEdit->m_hWnd) + 1),::GetWindowTextLength(m_pCoreEdit->m_hWnd) + 1);
// 				m_BrowserHandler->GetBrowser()->GetMainFrame()->LoadURL(strText.GetBuffer(strText.GetLength()));
// 				strText.ReleaseBuffer();
// 				return TRUE;
// 			}		
// 		}
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_IME_CHAR:
		{
			if (m_bBrowserFocus)
			{
				SetBrowserEvent(message,wParam,lParam);
			}

		}
		break;
	case WM_KILLFOCUS:
		{
			SetBrowserEvent(message,wParam,lParam);
		}
		break;
	}
	return FALSE;
}
void COverlayBrowserWnd::SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam)
{
	if (m_BrowserHandler)
	{
		CBrowserImpl::GetInstance()->SetBrowserEvent(m_BrowserHandler->GetBrowser(),message,wParam,lParam);
	}

}
LRESULT COverlayBrowserWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return 1;
}
BOOL COverlayBrowserWnd::PtInBrowserRect(CPoint pt)
{
	 CRect rc;
	 GetClientRect(&rc);
	 rc.left += BROWSER_OFFSET_LEFT;
	 rc.right -= BROWSER_OFFSET_RIGHT;
	 rc.top += BROWSER_OFFSET_TOP;
	 rc.bottom -= BROWSER_OFFSET_BOTTOM;
	return  PtInRect(&rc,pt);
}
void COverlayBrowserWnd::BtnChange()
{
	CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
	if (browser)
	{
		if (browser->CanGoBack())
		{
			m_pBack->Enable(TRUE);
		}
		else
		{
			m_pBack->Enable(FALSE);
		}
		if (browser->CanGoForward())
		{
			m_pForward->Enable(TRUE);
		}
		else
		{
			m_pForward->Enable(FALSE);
		}	
	}
}
LRESULT COverlayBrowserWnd::OnEditReturnKeyDown(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
	CString strText;    ::GetWindowText(m_pCoreEdit->m_hWnd,strText.GetBuffer(::GetWindowTextLength(m_pCoreEdit->m_hWnd) + 1),::GetWindowTextLength(m_pCoreEdit->m_hWnd) + 1);   CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();   if (browser)
   {
	   browser->GetMainFrame()->LoadURL(strText.GetBuffer(strText.GetLength()));
   }	    strText.ReleaseBuffer();		
	return 0;
}
LRESULT COverlayBrowserWnd::OnBrowserMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
	LPBROWSERMSGDATA pMsgData = (LPBROWSERMSGDATA) lParam;
	if (!pMsgData)
	{
		return 0;
	}
	switch(pMsgData->msgid)
	{
	case BROWSER_HANDLER_AFTERCREATE:
		{
			CefRefPtr<CefBrowser> browser = m_BrowserHandler->GetBrowser();
			if (browser)
			{
				browser->GetHost()->SendFocusEvent(true);
			}
		}
		break;
	case BROWSER_HANDLER_ADDRESSCHANGE:
		{
			m_pCoreEdit->SetWindowText(pMsgData->szTitle);
			//GLOG(_T("pMsgData->szTitle = %s"),pMsgData->szTitle);
		}
		break;
	case BROWSER_HANDLER_CURSORCHANGE:
		{
			if (m_bMouseIn)
			{
				CBrowserImpl::GetInstance()->OnBrowserMsg(wParam,lParam);
			}
		}
		break;
	case BROWSER_HANDLER_LOAD:
		{
			if (!pMsgData->loading)
			{
				BtnChange();
			}
		}
		break;
	default:
		break;
	}
	return 1;
}
void COverlayBrowserWnd::OnSize(UINT nType, CSize size)
{
	if (m_BrowserHandler)
	{
		CBrowserImpl::GetInstance()->SetBrowserSize(m_hWnd,CBrowserImpl::GetInstance()->GetBrowserWndIndex(m_BrowserHandler));
	}
	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_pCoreEdit)
	{
		m_pCoreEdit->SetWindowPos(NULL,0,0,rcClient.Width() - 160,20,SWP_NOMOVE);
	}
	
}
void COverlayBrowserWnd::DrawEditBg(ISonicPaint * pPaint, LPVOID)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	ISonicImage *pIamge = GetCoreUI()->GetImage(_T("BG_BROWSER_EDIT"));
	DRAW_PARAM dp;
	memset(&dp, 0, sizeof(dp));
	dp.dwMask = DP_TILE;
	dp.cx = rcClient.Width() - 145;
	pIamge->Draw(pPaint->GetCurrentPaint()->hdc, 100, 8, &dp);
	if (m_pHome)
	{
		m_pHome->TextOut(pPaint->GetCurrentPaint()->hdc,rcClient.right - 40,2,m_hWnd);
	}
}
// COverlayBrowserWnd message handlers
