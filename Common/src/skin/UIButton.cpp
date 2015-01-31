#include "stdafx.h"
#include "skin/UIButton.h"

CUIButton::CUIButton():m_pBitmap(NULL)
{	
	m_bDisable			= FALSE;
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	m_bSetCursor		= FALSE;

	m_bHandOnMove       = TRUE ;

	m_colorText         = 0x1f1f1f ;

	m_bMultiState       = TRUE ;
}

CUIButton::~CUIButton()
{
	ReleaseResource();
}
void CUIButton::ReleaseResource()
{
	__try{
		if(NULL!=m_pBitmap)
		{
			delete m_pBitmap;
			m_pBitmap=NULL;
		}
	}__except(EXCEPTION_EXECUTE_HANDLER){
		m_pBitmap=NULL;
	}
}

BEGIN_MESSAGE_MAP(CUIButton, CButton)
	//{{AFX_MSG_MAP(CKbcButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY( )
	ON_MESSAGE(WM_MOUSEHOVER,OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKbcButton message handlers

void CUIButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	
	if (m_pBitmap == NULL)
	{
		return;
	}
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics    gDC(pDC->m_hDC);

	if( m_pBitmap->GetHeight( )>=30 && m_bMultiState)
	{
		if( m_bDisable == TRUE )
			gDC.DrawImage(m_pBitmap,0,0,0,0,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);	

		else
		{
			// click state
			if( lpDrawItemStruct->itemState & ODS_SELECTED )
				gDC.DrawImage(m_pBitmap,0,0,0,0,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);	

			// hover state
			else if ( m_bHover)
				gDC.DrawImage(m_pBitmap,0,0,0,m_pBitmap->GetHeight( )/2,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);	

			// enable state
			else
				gDC.DrawImage(m_pBitmap,0,0,0,0,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);
		}
	}
	else
	{
		gDC.DrawImage(m_pBitmap,0,0,0,0,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( ),UnitPixel);
	}
	
	if(m_tsText.length( )>0)
	{
		CRect   rect;
		LOGFONT   logFont;
		GetClientRect( &rect);	
		this->GetFont( )->GetLogFont(&logFont);
		_tcscpy(logFont.lfFaceName, _T("Tahoma")) ;

		Gdiplus::RectF   gRect(rect.left,rect.top,rect.Width( ),rect.Height() );
		PointF           pointF(5, 5);
		Font              gFont(lpDrawItemStruct->hDC,&logFont);
		SolidBrush     brush(Color(255, m_colorText>>16, (m_colorText>>8)&0x000000ff, (m_colorText&0x000000ff)));
		StringFormat  gStrFormat;

		gStrFormat.SetAlignment( StringAlignmentCenter);
		gStrFormat.SetLineAlignment( StringAlignmentCenter);
		gStrFormat.SetTrimming(StringTrimmingEllipsisCharacter);

		gDC.DrawString(m_tsText.toWideString( ).c_str( ),m_tsText.length( ),&gFont,gRect,&gStrFormat,&brush );
	}	
	ReleaseDC(pDC);
	return;
}

void CUIButton::SetButtonImage(LPCTSTR lpszFileName,UINT nMask)
{
	HRESULT  hResult;
	IStream       *pIStream=NULL;
	String::_tstring        szPath=lpszFileName;

	if(m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap=NULL;
	}
	m_pBitmap=new Bitmap(szPath.toWideString().c_str( ));
	if (m_pBitmap == NULL)
	{
		return;
	}
	m_nMask = nMask;
	
	// get button info..
	m_rectButton.left	= 0;
	m_rectButton.top	= 0;
	m_rectButton.right	= m_pBitmap->GetWidth();
	m_rectButton.bottom	= m_pBitmap->GetHeight();
	////////////////////////////////////////////////////////////////

	// resize button size.
	// if button size is smaller than button image then we enlarge button size.
	///add text size to button
	CWnd *pWnd = this->GetParent();
	GetWindowRect(&m_rectButtonPos);
	if (pWnd != NULL)
	{
		pWnd->ScreenToClient(m_rectButtonPos);
		m_rectButtonPos.right	= m_rectButtonPos.left + m_pBitmap->GetWidth();
		m_rectButtonPos.bottom	= m_rectButtonPos.top  + m_pBitmap->GetHeight();
		MoveWindow(m_rectButtonPos);
	}
	////////////////////////////////////////////////////////////////
}
                                                           

void CUIButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// we wanna get WM_MOUSEHOVER,WM_MOUSELEAVE message, so we call _TrackMouseEvent
	if( m_bCursorOnButton == FALSE )
	{
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme,sizeof(TRACKMOUSEEVENT));
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;
		m_bCursorOnButton = _TrackMouseEvent(&tme);
	}

	if (m_bHandOnMove)
	{
		//SetFocus( );
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND)));
	}
	
	return CButton::OnMouseMove(nFlags,point);
}


LRESULT CUIButton::OnMouseHover(WPARAM wparam, LPARAM lparam)
{
	m_bHover = TRUE;
//	CButton::OnMo
	Invalidate();
	return 0L;
}

LRESULT CUIButton::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;

	CRect    rect;

	GetClientRect(&rect);
	ClientToScreen(&rect);

	if (m_bHandOnMove)
	{
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));
	}
	ReleaseCapture();
	Invalidate();

//	GetParent()->ScreenToClient(&rect);
//	GetParent()->InvalidateRect(rect);
	return 0L;
}

void CUIButton::SetButtonEnable()
{
	m_bDisable = FALSE;	
	Invalidate();
}

void CUIButton::SetButtonDisable()
{
	m_bDisable = TRUE;	
	Invalidate();
}

void CUIButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// if button state is disable we do not call CButton::OnLButtonDown funtion..
	if( !m_bDisable )
		CButton::OnLButtonDown(nFlags, point);
}

BOOL CUIButton::PreTranslateMessage(MSG* pMsg) 
{
	// treat double click as simple click to avoid some issue when triggering popup window.
	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;

	// to showing a tooltip message, call RelayEvent if m_ToolTip has created.
	 if( m_ToolTip.GetSafeHwnd() != NULL&& pMsg->message==WM_MOUSEMOVE  )
	{
		m_ToolTip.RelayEvent(pMsg);
	}
	
	if (pMsg->message == WM_XBUTTONUP)
	{
		if (AfxGetApp()->GetMainWnd() != NULL)
		{
			::PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(),pMsg->message,pMsg->wParam,pMsg->lParam);
		}
	}
	
	return CButton::PreTranslateMessage(pMsg);
}

void CUIButton::SetToolTipText(LPCTSTR lpszText)
{
	if(NULL==m_ToolTip.GetSafeHwnd( ))
	{
		m_ToolTip.Create(this);
	}
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool(this,lpszText);
}

BOOL CUIButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if( m_bSetCursor )
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	else
		return CButton::OnSetCursor(pWnd,nHitTest,message);
}

void CUIButton::SetCursor(HCURSOR hCursor)
{
	m_bSetCursor	= TRUE;
	m_hCursor		= hCursor;
}

BOOL CUIButton::OnEraseBkgnd(CDC* pDC) 
{
	// do not draw back ground. just return here, so we can see the background.
	// if you have curiosity about this, then remove ON_WM_ERASEBKGND() and test..
	return TRUE;
}

BOOL CUIButton::DestroyWindow()
{
//	delete m_pBitmap;
	return CButton::DestroyWindow();
}

void CUIButton::OnDestroy( )
{
//	delete m_pBitmap;
	return CButton::OnDestroy( );
}
