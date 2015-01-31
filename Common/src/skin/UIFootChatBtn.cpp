#include "stdafx.h"
#include "skin/UIFootChatBtn.h"

CUIFootChatBtn::CUIFootChatBtn()
{	
	m_bDisable			= FALSE;
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	m_bSetCursor		= FALSE;	
	m_colorText         = 0x1f1f1f ;
	m_pBitmap           =NULL;
}

CUIFootChatBtn::~CUIFootChatBtn()
{
	ReleaseResource();
}
void CUIFootChatBtn::ReleaseResource()
{
	__try{
		if(NULL!=m_pBitmap)
		{
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
	}__except(EXCEPTION_EXECUTE_HANDLER){
		m_pBitmap = NULL;
	}
}

BEGIN_MESSAGE_MAP(CUIFootChatBtn, CButton)
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

void CUIFootChatBtn::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	
	if (m_pBitmap == NULL)
	{
		return;
	}
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics    gDC(pDC->m_hDC);
	CRect   itemRect(lpDrawItemStruct->rcItem);

	gDC.DrawImage(m_pBitmap,Point(0,itemRect.top+(itemRect.Height( )-m_pBitmap->GetHeight( ))/2));
	
	if(!m_tsText.empty( ))
	{
		CRect   rect;
		LOGFONT   logFont;
		GetClientRect( &rect);	
		this->GetFont( )->GetLogFont(&logFont);

		Gdiplus::RectF   gRect(rect.left,rect.top,rect.Width( ),rect.Height() );
		gRect.X  += m_pBitmap->GetWidth() +1.0f;
		PointF           pointF(5, 5);
		Font              gFont(lpDrawItemStruct->hDC,&logFont);
		SolidBrush     brush(Color(255, m_colorText>>16, (m_colorText>>8)&0x000000ff, (m_colorText&0x000000ff)));
		StringFormat  gStrFormat;

		gStrFormat.SetAlignment( StringAlignmentNear);
		gStrFormat.SetLineAlignment( StringAlignmentCenter);

		gDC.DrawString(m_tsText.toWideString( ).c_str( ),m_tsText.length( ),&gFont,gRect,&gStrFormat,&brush );
	}	
	return;
}

void CUIFootChatBtn::SetButtonImage(LPCTSTR lpszFileName,UINT nMask)
{
	HRESULT  hResult;
	IStream       *pIStream=NULL;
	String::_tstring       szPath=lpszFileName;

	ReleaseResource();	
	m_pBitmap=new Bitmap(szPath.toWideString().c_str());
	if (NULL == m_pBitmap)
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
	pWnd->ScreenToClient(m_rectButtonPos);
	m_rectButtonPos.right	= m_rectButtonPos.left + m_pBitmap->GetWidth();
	m_rectButtonPos.bottom	= m_rectButtonPos.top  + m_pBitmap->GetHeight();
	MoveWindow(m_rectButtonPos);
	////////////////////////////////////////////////////////////////
}


void CUIFootChatBtn::OnMouseMove(UINT nFlags, CPoint point) 
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

	::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND)));

	CButton::OnMouseMove(nFlags, point);
}


LRESULT CUIFootChatBtn::OnMouseHover(WPARAM wparam, LPARAM lparam)
{
	m_bHover = TRUE;
	
	//Invalidate();
	return 0L;
}

LRESULT CUIFootChatBtn::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;

	::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));
	
	//Invalidate();
	return 0L;
}

void CUIFootChatBtn::SetButtonEnable()
{
	m_bDisable = FALSE;	
	Invalidate();
}

void CUIFootChatBtn::SetButtonDisable()
{
	m_bDisable = TRUE;	
	Invalidate();
}

void CUIFootChatBtn::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// if button state is disable we do not call CButton::OnLButtonDown funtion..
	if( !m_bDisable )
		CButton::OnLButtonDown(nFlags, point);
}

BOOL CUIFootChatBtn::PreTranslateMessage(MSG* pMsg) 
{
	// to showing a tooltip message, call RelayEvent if m_ToolTip has created.
	if( m_ToolTip.GetSafeHwnd() != NULL )
		m_ToolTip.RelayEvent(pMsg);
	
	return CButton::PreTranslateMessage(pMsg);
}

void CUIFootChatBtn::SetToolTipText(LPCTSTR lpszText)
{
	if(NULL==m_ToolTip.GetSafeHwnd( ))
	{
		m_ToolTip.Create(this);
		m_ToolTip.Activate(TRUE);
		m_ToolTip.AddTool(this,lpszText);
	}
	else
	{
		m_ToolTip.Activate(TRUE);
		m_ToolTip.UpdateTipText(lpszText,this);
	}
}

BOOL CUIFootChatBtn::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if( m_bSetCursor )
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	else
		return CButton::OnSetCursor(pWnd,nHitTest,message);
}

void CUIFootChatBtn::SetCursor(HCURSOR hCursor)
{
	m_bSetCursor	= TRUE;
	m_hCursor		= hCursor;
}

BOOL CUIFootChatBtn::OnEraseBkgnd(CDC* pDC) 
{
	// do not draw back ground. just return here, so we can see the background.
	// if you have curiosity about this, then remove ON_WM_ERASEBKGND() and test..
	return TRUE;
}
void CUIFootChatBtn::OnDestroy()
{
//	delete m_pBitmap;
	return CButton::OnDestroy();
}
