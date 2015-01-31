#include "stdafx.h"
#include "skin/UICheck.h"

// CUICheck

IMPLEMENT_DYNAMIC(CUICheck, CWnd)

CUICheck::CUICheck():m_iState(0)
{
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	m_bSetCursor		= FALSE;
}

CUICheck::~CUICheck()
{
}


BEGIN_MESSAGE_MAP(CUICheck, CButton)
	ON_WM_MOUSEMOVE()
	//ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//ON_WM_SETCURSOR()
	ON_WM_DESTROY( )
	ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CUICheck::SetCheck( int nCheck)
{
	m_iState=nCheck;
	switch( m_iState)
	{
	case BST_CHECKED:
		this->SetButtonImage(m_tsFile[STATE_CHECKED].c_str( ),0);
		break;
	case BST_UNCHECKED:
		this->SetButtonImage(m_tsFile[STATE_UNCHECKED].c_str( ),0);
		break;
	default:
		break;
	}
	Invalidate();
	return;
}

void CUICheck::SetCheck(   )
{
	if(BST_UNCHECKED==m_iState)
	{
		m_iState=BST_CHECKED;
	}
	else
	{
		m_iState=BST_UNCHECKED;
	}
	Invalidate();
	return;
}

int CUICheck::GetCheck( )
{
	return m_iState;
}

void CUICheck::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(BST_UNCHECKED==m_iState)
	{
		this->SetCheck( );
		m_iState=BST_CHECKED;
	}
	else
	{
		this->SetCheck( );
		m_iState=BST_UNCHECKED;
	}
	Invalidate( );
	return;
}

void CUICheck::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

void CUICheck::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics    gDC(pDC->m_hDC);

	if( BST_UNCHECKED  == m_iState)
		gDC.DrawImage(m_pBitmap,0,0,0,0,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);	
	else
		gDC.DrawImage(m_pBitmap,0,0,0,m_pBitmap->GetHeight( )/2,m_pBitmap->GetWidth( ),m_pBitmap->GetHeight( )/2,UnitPixel);
	
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
		Color clr(m_clrText) ;
		SolidBrush     brush(Color(255,  m_clrText>>16, (m_clrText>>8)&0xff, m_clrText&0xff));
		StringFormat  gStrFormat;

		gStrFormat.SetAlignment( StringAlignmentNear);
		gStrFormat.SetLineAlignment( StringAlignmentCenter);

		gDC.DrawString(m_tsText.toWideString( ).c_str( ),m_tsText.length( ),&gFont,gRect,&gStrFormat,&brush );
	}	
	return;
}

void CUICheck::SetButtonImage(LPCTSTR lpszFileName,UINT nMask)
{
	HRESULT  hResult;
	IStream       *pIStream=NULL;
	LOGFONT   logFont;
	_tstring           szPath=lpszFileName;

//	m_pBitmap=Bitmap::FromFile(szPath.toWideString().c_str());
//	if(!m_pBitmap)
//		return ;

	hResult=SHCreateStreamOnFile( lpszFileName,STGM_READ| STGM_SHARE_DENY_NONE ,&pIStream);
	if(S_OK!=hResult)
	{
		return ;
	}

	m_pBitmap=new Bitmap(pIStream);
	if(!m_pBitmap)
		return ;
	pIStream->Release( );
	pIStream=NULL;



	m_nMask = nMask;
	
	// get button info..
	m_rectButton.left	= 0;
	m_rectButton.top	= 0;
	m_rectButton.right	= m_pBitmap->GetWidth();
	m_rectButton.bottom	= m_pBitmap->GetHeight()/2;
	////////////////////////////////////////////////////////////////


	// resize button size.
	// if button size is smaller than button image then we enlarge button size..
	CWnd *pWnd = this->GetParent();
	GetWindowRect(&m_rectButtonPos);
	pWnd->ScreenToClient(m_rectButtonPos);
//	m_rectButtonPos.right	= m_rectButtonPos.left + m_pBitmap->GetWidth();
	m_rectButtonPos.bottom	= m_rectButtonPos.top  + m_pBitmap->GetHeight();
	MoveWindow(m_rectButtonPos);
	UpdateWindow( );
	InvalidateRect( m_rectButtonPos);
	////////////////////////////////////////////////////////////////
}

void CUICheck::OnMouseMove(UINT nFlags, CPoint point) 
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
	
	CButton::OnMouseMove(nFlags, point);
}

LRESULT CUICheck::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	
	//Invalidate();
	return 0L;
}

BOOL CUICheck::PreTranslateMessage(MSG* pMsg) 
{
	// to showing a tooltip message, call RelayEvent if m_ToolTip has created.
	if( m_ToolTip.GetSafeHwnd() != NULL )
		m_ToolTip.RelayEvent(pMsg);
	
	return CButton::PreTranslateMessage(pMsg);
}

//BOOL CUICheck::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
//{
//	if( m_bSetCursor )
//	{
//		::SetCursor(m_hCursor);
//		return TRUE;
//	}
//	else
//		return CButton::OnSetCursor(pWnd,nHitTest,message);
//}

void CUICheck::SetCursor(HCURSOR hCursor)
{
	m_bSetCursor	= TRUE;
	m_hCursor		= hCursor;
}

BOOL CUICheck::OnEraseBkgnd(CDC* pDC) 
{
	// do not draw back ground. just return here, so we can see the background.
	// if you have curiosity about this, then remove ON_WM_ERASEBKGND() and test..
	return TRUE;
//	return CButton::OnEraseBkgnd(pDC) ;
}

void CUICheck::PreSubclassWindow()
{
 // SetButtonStyle(GetButtonStyle()|BS_OWNERDRAW);
  CButton::PreSubclassWindow();
}

void CUICheck::OnDestroy()
{
//	delete m_pBitmap;
	return CButton::OnDestroy();
}



