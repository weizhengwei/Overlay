#include "StdAfx.h"
#include "data/SNSManager.h"
#include "ui/OSBrowserDialog.h"
#include "data/DataPool.h"
#include "constant.h"
#include "data\UIString.h"
#include "toolkit.h"


IMPLEMENT_DYNAMIC(COSBrowserDialog, CUIWindow)

COSBrowserDialog::COSBrowserDialog(UINT id, CWnd* pParent,int nType)
:CUIWindow(id,pParent)
,m_uBrowserType(nType)
,m_bFullMode(FALSE)
,m_hBrowserWnd(NULL)
,m_bBrowserFocus(FALSE)
,m_bMouseIn(FALSE)
,m_psmMgr(NULL)
,m_bLoadBarShow(FALSE)
{
	m_defRect = CRect(0,0,0,0);
	m_defBrowserRect = CRect(0,0,0,0);
	m_LoadBarPoint = CPoint(70,50);
	m_pImage = GetSonicUI()->CreateImage();
	m_strCookieFolder = _T("cef_cookies");
}


COSBrowserDialog::~COSBrowserDialog(void)
{
	GetSonicUI()->DestroyObject(m_pImage);
}
BEGIN_MESSAGE_MAP(COSBrowserDialog,CUIWindow)
	ON_MESSAGE(WM_COREMESSAGE,   &COSBrowserDialog::OnProcessMsg)
	ON_MESSAGE(WM_BROWSER_DESTROY,          &COSBrowserDialog::OnBrowserDestory)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_MOVE()
	ON_WM_SETCURSOR()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()
BOOL COSBrowserDialog::OnInitDialog()
{
	CUIWindow::OnInitDialog();
	m_psmMgr = new CArcShareMemMgr<arcbrowser_sm_header>;
	_tstring strName;
	strName.Format(1,_T("arc%d"),m_hWnd);
	m_psmMgr->Create(strName.c_str(),2048 * 2048 * 4);
	m_msgClient.Initialize(m_hWnd,-1);

	m_wndLoadingBar.Create(IDD_DIALOG_POP,this);
	m_wndLoadingBar.ShowWindow(SW_HIDE) ;
	
	return TRUE;
}
void COSBrowserDialog::OnDestroy()
{
	
	m_msgClient.Uninitialize();
	m_psmMgr->Close();
	delete m_psmMgr;
	m_psmMgr = NULL;
	CUIWindow::OnDestroy();
	
}
void COSBrowserDialog::SendCoreMsg(core_msg_header * pHerader)
{
	m_msgClient.SendCoreMsg(m_hMsgClient,pHerader);
}
LRESULT COSBrowserDialog::OnProcessMsg(WPARAM wParam,LPARAM lParam)
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	if (!pHeader)
	{
		return 0L;
	}
	switch(pHeader->dwCmdId)
	{
	case CORE_MSG_BROWSER_INITCONNET:
		{
 			core_msg_initbaseinfo *pInitBaseInfo = (core_msg_initbaseinfo *)pHeader;
 			m_hBrowserWnd = pInitBaseInfo->hBrowserWnd;
			m_hMsgClient= pInitBaseInfo->hSelfWnd;
			pInitBaseInfo->rcDefBrowser = m_defRect;
			CRect rc;
			GetClientRect(&rc);
			//pInitBaseInfo->szBrowser.cx = rc.Width();
			//pInitBaseInfo->szBrowser.cy = rc.Height();
 			pInitBaseInfo->hSelfWnd = m_msgClient.GetSelfWindow();
			
 			lstrcpynA(pInitBaseInfo->csStartUrl,m_csStartUrl.toNarrowString().c_str(),1024);
 			pInitBaseInfo->dwMode = m_nMode;
			lstrcpynA(pInitBaseInfo->csAllGameUrl,m_strAllGameUrl.toNarrowString().c_str(),1024);
 			lstrcpynA(pInitBaseInfo->csCookieUrl,theUIString._GetStringFromId(_T("IDS_COOKIE_URL")).toNarrowString().c_str(),1024);
 			lstrcpynA(pInitBaseInfo->csSessionId,g_theSNSManager.GetSessionId().c_str(),128);
 			lstrcpynA(pInitBaseInfo->csWebsite_Pwrd_Oldnews,theUIString._GetStringFromId(_T("IDS_FILTER_WEBSITE_PWRD_OLDNEWS")).toNarrowString().c_str(),1024);
 			lstrcpynA(pInitBaseInfo->csWebsite_perfectworld,theUIString._GetStringFromId(_T("IDS_FILTER_WEBSITE_PERFECTWORLD")).toNarrowString().c_str(),1024);
			lstrcpynA(pInitBaseInfo->csCookieFolder,m_strCookieFolder.toNarrowString().c_str(),256);
			//GLOG(_T("CORE_MSG_BROWSER_INITCONNET"));
			
		}
		break;
	case CORE_MSG_NAVIGATE_E:
		{
			core_msg_navigate_e *pNavigate = (core_msg_navigate_e*)pHeader;
			m_csStartUrl = pNavigate->scUrl;
			m_nMode = pNavigate->dwMode;
		}
		break;
	case  CORE_MSG_CURSORCHANGE:
		{
			if (m_bMouseIn)
			{
				core_msg_cursorchange * pCursorChange = (core_msg_cursorchange *)pHeader;
				m_hBrowserCursor = LoadCursor(NULL,MAKEINTRESOURCE(pCursorChange->lCursorId));
				//::SetCursor(m_hBrowserCursor);
				///GLOG(_T("lCursorId = %d"),pCursorChange->lCursorId);
				SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)(LONG_PTR)m_hBrowserCursor);
				//SendMessage( WM_SETCURSOR, (WPARAM)m_hWnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
				
			}
			
		}
		break;
	case CORE_MSG_BROWSER_PAINT:
		{
			//core_msg_paint *pMsgPaint = (core_msg_paint*)pHeader;
			//InvalidateRect(&pMsgPaint->rect);
			//GLOG(_T("pMsgPaint->rect.left = %d pMsgPaint->rect.top =%d pMsgPaint->rect.right = %d pMsgPaint->rect.bottom = %d"));
		}
		break;
	case CORE_MSG_BROWSER_INFO:
		{
			core_msg_browserinfo* pbrowserInfo = (core_msg_browserinfo*)pHeader;
			if (pbrowserInfo->bLoading)
			{
				SetLoadBarRect();
				if (IsWindowVisible() && m_wndLoadingBar.GetSafeHwnd() && !IsIconic())
				{
					m_wndLoadingBar.ShowWindow(SW_SHOWNA);	
				}
				m_bLoadBarShow = TRUE;
			}
			else 
			{
				if (m_wndLoadingBar.GetSafeHwnd())
				{
					m_wndLoadingBar.ShowWindow(SW_HIDE);
				}
				
				m_bLoadBarShow = FALSE;
			}
		}
		break;
	default:
		break;

	}
	return 0L;
}
void COSBrowserDialog::SetLoadBarRect()
{
	if (m_wndLoadingBar.GetSafeHwnd())
	{
		CRect rcClient,rc;
		
		if (!IsIconic())
		{
			
				GetWindowRect(&rcClient);
				rcClient.DeflateRect(m_defRect);
				rcClient.DeflateRect(m_defBrowserRect);
				rc.left = rcClient.right - m_LoadBarPoint.x;
				rc.top = rcClient.top + m_LoadBarPoint.y;
				rc.right = rc.left + 30;
				rc.bottom = rc.top + 30;	
		}
		else
		{
			rc = CRect(0,0,0,0);
		}
		m_wndLoadingBar.MoveWindow(&rc);
	}
}
void COSBrowserDialog::SetLoadBarPoint(CPoint &pt)
{
	m_LoadBarPoint = pt;
}

void COSBrowserDialog::RunOnceOnStart()
{

}

long COSBrowserDialog::run()
{
	RunOnceOnStart();
	while (true)
	{
		if (::IsWindow(m_hWnd))
		{
			if (m_bFullMode)
			{
				::SendMessage(m_hWnd,WM_SETWEBPLAYERFULLSCREEN, 0, 0) ;
			}
			_tstring sBaseDir = theDataPool.GetBaseDir() + FILE_EXE_OSBROWSER;
			_tstring sCmd;
			sCmd.Format(5,_T(" \"%d\" \"%d\" \"%d\" \"%s\" \"%s\""),m_uBrowserType,m_hWnd,m_msgClient.GetSelfWindow(),m_csStartUrl.c_str(),theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str());
			STARTUPINFO si = {0};
			si.cb = sizeof(si);
			if (!::CreateProcess(sBaseDir.c_str(), (LPWSTR)sCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &m_ProcessInfo))
			{
				return 0;
			}
			Sleep(100);
		}
		else
		{
			return 0;
		}
		WaitForSingleObject(m_ProcessInfo.hProcess,INFINITE);
		CloseHandle(m_ProcessInfo.hProcess);
		CloseHandle(m_ProcessInfo.hThread);
		::SendMessage(m_hWnd,WM_BROWSER_DESTROY,0,0);
	}
}
LRESULT COSBrowserDialog::OnBrowserDestory(WPARAM wParam,LPARAM lParam)
{
	if (m_wndLoadingBar.GetSafeHwnd())
	{
		m_wndLoadingBar.ShowWindow(SW_HIDE);
	}
	return 0;
}
void COSBrowserDialog::OnPaint()
{
 	CPaintDC dc(this);
	
}
void COSBrowserDialog::DrawBrowser(ISonicPaint * pPaint,CRect &rcClient)
{
	
}
void COSBrowserDialog::DrawBrowser(HWND  hWnd,CRect &rcClient)
{
	BYTE * pData = GetSonicUI()->SkinFromHwnd(hWnd)->GetPaint()->GetBits();
	
	if (!m_psmMgr || !m_psmMgr->GetBits() || !pData)
	{
		return;
	}
	m_psmMgr->Lock();
	arcbrowser_sm_header * pHeader = (arcbrowser_sm_header *) m_psmMgr->GetHeader();
	CRect rc ,rect,rcWndClient,rcWebClient;
	GetClientRect(&rcWndClient);
	rc = rcWndClient;
	rc.DeflateRect(GetDefRect());
	rcWebClient = rc;
	if (!::IntersectRect(rect,rc,rcClient))
	{
		m_psmMgr->Unlock();
		return;
	}
	int nWidth,nHeight;
	nWidth = rect.Width();
	if (nWidth > pHeader->nWidth)
	{
		nWidth = pHeader->nWidth;
	}
	nHeight = rect.Height();
	if (nHeight > pHeader->nHeight)
	{
		nHeight = pHeader->nHeight;
	}
	BYTE * pWndData = pData + (rect.top * rcWndClient.Width() + rect.left)*4;
	BYTE * pWebData = (BYTE *)m_psmMgr->GetBits() + ((rect.top - m_defRect.top) * pHeader->nWidth + (rect.left - m_defRect.left))*4;
	BYTE * pA = NULL;
	for (int y = 0 ; y <  nHeight ; y++)
	{
		for (int x = 0 ; x < nWidth ; x++)
		{
			pA = pWebData + 3;
			if (*pA==0 && *pWebData !=0) //fix error for  fonts of youtube videos .
			{
				*pA = 0xff;
			}
			for (int i = 0 ; i < 3; i++)
			{
				(*pWndData) =((*pA) *(*pWebData) + (255 - *pA)*(*pWndData))/255;
				++pWndData;
				++pWebData;
			}
			++pWndData;
			++pWebData;
		}
		pWebData += (pHeader->nWidth  - nWidth) *4;
		pWndData += (rcWndClient.Width() - nWidth) *4;
	}
	m_psmMgr->Unlock();
	//GLOG(_T("rc.left = %d, rc.top = %d,rc.right = %d,rc.bottom = %d"),rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
}
void COSBrowserDialog::DrawBrowser(HDC hdc,CRect &rcClient)
{

	if (!m_psmMgr || !m_psmMgr->GetBits())
	{
		return;
	}

// 	static ISonicImage * pImg = NULL;
// 	if (!pImg)
// 	{
// 		pImg = GetSonicUI()->CreateImage();
// 	}
// 	pImg->Create(view_width, view_height, 0, TRUE);
// 	memcpy(pImg->GetBits(), view_buffer_,view_width* view_height *4);
// 	pImg->Draw(hdc,BROWSER_OFFSET_LEFT,BROWSER_OFFSET_TOP);
 	m_psmMgr->Lock();
	CRect rc ,rect;
	//GLOG(_T("rc.left = %d, rc.top = %d,rc.right = %d,rc.bottom = %d"),rcClient.left,rcClient.top,rcClient.right,rcClient.bottom);
	GetClientRect(&rc);
	rc.DeflateRect(GetDefRect());
	::IntersectRect(rect,rc,rcClient);

	arcbrowser_sm_header *pHeader = (arcbrowser_sm_header *)m_psmMgr->GetHeader();
	BITMAPINFOHEADER m_bmpHeader;
	m_bmpHeader.biSize = sizeof(m_bmpHeader);
	m_bmpHeader.biBitCount = 32;
	m_bmpHeader.biCompression = BI_RGB;
	m_bmpHeader.biWidth = pHeader->nWidth;
	m_bmpHeader.biHeight = -(pHeader->nHeight);
	m_bmpHeader.biPlanes = 1;
	SetDIBitsToDevice(hdc,rect.left,rect.top , rect.Width(),rect.Height(), rect.left - m_defRect.left,pHeader->nHeight + m_defRect.top - rect.bottom, 0, pHeader->nHeight,m_psmMgr->GetBits(), (const BITMAPINFO *)&m_bmpHeader, DIB_RGB_COLORS);
	m_psmMgr->Unlock();
	
}
LRESULT COSBrowserDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	//case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSCHAR:
	case WM_CHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		{
			OnMessage(message, wParam, lParam);
		}
		break;
	default:
		break;
	}
	return CUIWindow::WindowProc(message, wParam, lParam);
}
BOOL COSBrowserDialog::PreTranslateMessage(MSG* pMsg)
{
// 	switch(pMsg->message)
// 	{
// 	case WM_CHAR:
// 	case WM_SYSCHAR:
// 	case WM_KEYDOWN:
// 	case WM_SYSKEYDOWN:
// 		{
// 			//OnMessage(pMsg->message, pMsg->wParam, pMsg->lParam);	
// 			//return TRUE;
// 			int i = 0;
// 		}
// 	}
	if (pMsg->message == WM_XBUTTONUP)
	{
		pMsg->hwnd = m_hWnd;
	}
	if (pMsg->message == WM_INPUTLANGCHANGEREQUEST)
	{
		::DefWindowProc(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}
	return FALSE;
	return CUIWindow::PreTranslateMessage(pMsg);
}
BOOL COSBrowserDialog::SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam)
{
	//GLOG(_T("message= %d"),message);
	return::SendMessage(m_hBrowserWnd,message,wParam,lParam);
	//return SetCoreBrowserEvent(message,wParam,lParam);
	
	
}
BOOL COSBrowserDialog::SetCoreBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam)
{
	//GLOG(_T("message= %d"),message);
	core_msg_msgevent msgevent;
	msgevent.message = message;
	msgevent.wParam = wParam;
	msgevent.lParam = lParam;
	SendCoreMsg(&msgevent);
	return FALSE;
}
void COSBrowserDialog::SetDefRect(CRect rc)
{
	m_defRect = rc;
	core_msg_defrect defrect;
	defrect.rcdef = m_defRect;
	SendCoreMsg(&defrect);
// 	CRect rect;
// 	GetClientRect(&rect);
// 	SetBrowserEvent(WM_SIZE,WPARAM(rect.Width() - m_defRect.left - m_defRect.right),LPARAM(rect.Height() - m_defRect.top - m_defRect.bottom));
}
void COSBrowserDialog::SetDefBrowserRect(CRect rc)
{
	SetDefBrowserRect(rc.left,rc.top,rc.right,rc.bottom);
}
void COSBrowserDialog::SetDefBrowserRect(int l, int t, int r, int b)
{
	m_defBrowserRect.left = l;
	m_defBrowserRect.top = t;
	m_defBrowserRect.right = r;
	m_defBrowserRect.bottom = b;
}
void COSBrowserDialog::SetDefRect(int l, int t, int r, int b)
{
	m_defRect.left = l;
	m_defRect.top = t; 
	m_defRect.right = r; 
	m_defRect.bottom = b;
	core_msg_defrect defrect;
	defrect.rcdef = m_defRect;
	SendCoreMsg(&defrect);
// 	CRect rc;
// 	GetClientRect(&rc);
// 	SetBrowserEvent(WM_SIZE,WPARAM(rc.Width() - m_defRect.left - m_defRect.right),LPARAM(rc.Height() - m_defRect.top - m_defRect.bottom));
}
BOOL COSBrowserDialog::OnMessage(UINT message, WPARAM wParam, LPARAM lParam)
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
				pt.x -= m_defRect.left;
				pt.y -= m_defRect.top ;
				SetCapture();
				SetFocus();
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
				m_bBrowserFocus = TRUE;

			}
			else
			{
				SetBrowserEvent(WM_KILLFOCUS,0,0);
				m_bBrowserFocus = FALSE;
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	
		//case WM_MOUSELEAVE:
		//case WM_MOUSEWHEEL:
		{
			CPoint pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (PtInBrowserRect(pt))
			{
				pt.x -= m_defRect.left;
				pt.y -= m_defRect.top ;
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			if (::GetCapture() == m_hWnd && m_bBrowserFocus)
			{
				ReleaseCapture();
			}
			CPoint pt;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			///if (PtInBrowserRect(pt))
			{
				pt.x -= m_defRect.left;
				pt.y -= m_defRect.top ;
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
			}
		}
		break;
	
		break;
	case WM_MOUSEWHEEL:
		{
			
			CPoint pt;
			pt.x = LOSHORT(lParam) ;
			pt.y = HISHORT(lParam);
			//GLOG(_T("WM_MOUSEWHEELpt.x = %d pt.y = %d"),pt.x,pt.y);
			CPoint  point(pt);
			ScreenToClient(&point);
			if (PtInBrowserRect(point))
			{
				pt.x -= m_defRect.left;
				pt.y -= m_defRect.top ;
				ScreenToClient(&pt);
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
				//GLOG(_T("pt.x = %d pt.y = %d"),pt.x,pt.y);
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
				pt.x -= m_defRect.left;
				pt.y -= m_defRect.top ;
				SetBrowserEvent(message,wParam,MAKELPARAM(pt.x,pt.y));
				if (!m_bMouseIn)
				{
					m_bMouseIn = TRUE;
				}
				//GLOG(_T("WM_MOUSEMOVE ptx = %d, pt.y - %d"),pt.x,pt.y);
			}
			else if(m_bMouseIn)
			{
				//m_BrowserHandler->SetBrowserEvent(WM_MOUSELEAVE,0,0);
				m_bMouseIn = FALSE;
				//SetBrowserEvent(WM_MOUSELEAVE,0,0);
				//GLOG(_T("SetBrowserEvent(WM_MOUSELEAVE"));
				//COverlayImpl::GetInstance()->SendCursorMsg(32512);
				//::SetCursor(m_hBrowserCursor);
				SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)(LONG_PTR)LoadCursor(NULL,MAKEINTRESOURCE(32512)));
				SendMessage( WM_SETCURSOR, (WPARAM)m_hWnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if(m_bMouseIn)
			{
				//m_BrowserHandler->SetBrowserEvent(WM_MOUSELEAVE,0,0);
				m_bMouseIn = FALSE;
				//SetBrowserEvent(WM_MOUSELEAVE,0,0);
				//GLOG(_T("SetBrowserEventWM_MOUSELEAVE(WM_MOUSELEAVE"));
				SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)(LONG_PTR)LoadCursor(NULL,MAKEINTRESOURCE(32512)));
				SendMessage( WM_SETCURSOR, (WPARAM)m_hWnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
			}
		}
		break;
	case WM_KEYDOWN:	
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_IME_CHAR:
	case WM_KILLFOCUS:
		{
			//HWND whnd = ::GetFocus();
			SetBrowserEvent(message,wParam,lParam);
		}
		break;
	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		{
			//HWND whnd = ::GetFocus();
			if (m_bBrowserFocus)
			{
				SetBrowserEvent(message,wParam,lParam);
			}
			
		}
		break;
	}
	return FALSE;
}

BOOL COSBrowserDialog::PtInBrowserRect(CPoint pt)
{
	CRect rc;
	GetClientRect(&rc);
    rc.DeflateRect(m_defRect);
	rc.DeflateRect(m_defBrowserRect);
	return  PtInRect(&rc,pt);
}
void COSBrowserDialog::OnSetFocus(CWnd* pOldWnd)
{
	//CUIWindow::OnSetFocus(pOldWnd);
}
void COSBrowserDialog::OnSize(UINT nType, int cx, int cy)
{
	CUIWindow::OnSize(nType,cx,cy);
	if (!IsIconic())
	{
		CRect rc;
		GetClientRect(&rc);
		SetCoreBrowserEvent(WM_SIZE,WPARAM(rc.Width() - m_defRect.left - m_defRect.right),LPARAM(rc.Height() - m_defRect.top - m_defRect.bottom));
		SetLoadBarRect();
		if (m_bLoadBarShow && m_wndLoadingBar.GetSafeHwnd() && !m_wndLoadingBar.IsWindowVisible())
		{
			m_wndLoadingBar.ShowWindow(SW_SHOWNA);
		}
	}
}
BOOL COSBrowserDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
 	if(m_bMouseIn)
 	{
		SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)(LONG_PTR)m_hBrowserCursor);
 	}
	
	return CWnd::OnSetCursor(pWnd,nHitTest,message);
}
void COSBrowserDialog::OnMove(int x, int y)
{
	CUIWindow::OnMove(x,y);
	SetLoadBarRect();	 
}
void COSBrowserDialog::Navigate(_tstring tsURL,int nMode)
{
	if (m_hBrowserWnd)
	{
		core_msg_navigate_e msgNavigate;
		msgNavigate.dwMode = nMode;
		lstrcpynA(msgNavigate.scUrl,tsURL.toNarrowString().c_str(),2048);
		SendCoreMsg(&msgNavigate);
	}
		m_csStartUrl = tsURL;
		m_nMode = nMode;
}
void COSBrowserDialog::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CUIWindow::OnWindowPosChanged(lpwndpos);
	 if (lpwndpos && (lpwndpos->flags&SWP_HIDEWINDOW))
	{
		if (m_wndLoadingBar.GetSafeHwnd())
		{
			m_wndLoadingBar.ShowWindow(SW_HIDE);
		}
		
	}
	else if (lpwndpos && (lpwndpos->flags &SWP_SHOWWINDOW) && m_bLoadBarShow)
	 {
		 if (m_wndLoadingBar.GetSafeHwnd())
		 {
			 m_wndLoadingBar.ShowWindow(SW_SHOWNA);
		 }
		
	 }

}
