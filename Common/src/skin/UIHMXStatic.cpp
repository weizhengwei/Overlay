/** 
 * @file      HMXStatic.cpp
   @brief     The cpp file for customized static control.
   @author    
   @data      
   @version   01.00.00	
 */

/*

 */

/*
 * $LastChangedBy: 
 * $LastChangedDate: 
 * $Revision: 01.00.00
 * $Id: N/A
 */


/****************************************************************************/
/*                             User include                                 */
/****************************************************************************/

#include "skin/UIHMXStatic.h"
#include "skin/MemDC.h"

/****************************************************************************/
/*                        Class Method                                      */
/****************************************************************************/

CUIHMXStatic::CUIHMXStatic()
{
	m_bTransparent = true;
	m_clrBkClr = ::GetSysColor(COLOR_WINDOW);
	m_clrTextClr = ::GetSysColor(COLOR_WINDOWTEXT);
	m_brsBkGnd.Detach();
	m_brsHollow.Detach();
	m_brsBkGnd.CreateSolidBrush(m_clrBkClr);
	m_brsHollow.CreateStockObject(HOLLOW_BRUSH);

	m_bHyperLink = FALSE ;
	m_bUseShell=true;
	m_hNotifyWnd = NULL ;
}

CUIHMXStatic::~CUIHMXStatic()
{
	m_brsBkGnd.DeleteObject();
	m_brsHollow.DeleteObject();
	m_fntText.DeleteObject();
}


BEGIN_MESSAGE_MAP(CUIHMXStatic, CStatic)
	//{{AFX_MSG_MAP(CUIHMXStatic)
	//ON_WM_PAINT()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SETTEXT,OnSetText)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIHMXStatic message handlers

void CUIHMXStatic::EnableHyperLink(BOOL bEnable, LPCTSTR lpszLink, LPCTSTR lpszCmd)
{
	m_bHyperLink = bEnable ;
	m_sLink = lpszLink ;
	m_sCmd = lpszCmd ;
}

void CUIHMXStatic::EnableHyperLinkCallback(BOOL bEnable, HWND hNotifyWnd, LPCTSTR lpszCmd)
{
	m_bHyperLink = bEnable ;
	m_hNotifyWnd = hNotifyWnd ;
	m_sCmd = lpszCmd ;
}

void CUIHMXStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bUseShell)
	{
		return CStatic::OnLButtonDown( nFlags,point);
	}
	if (m_bHyperLink && !m_sLink.IsEmpty())
	{
		ShellExecute(0, _T("open"), m_sLink, m_sCmd, 0, SW_SHOWNORMAL);
	}
	if (m_bHyperLink && m_hNotifyWnd)
	{
		::SendMessage(m_hNotifyWnd, WM_GAME_LAUNCHPATCHER, 0, 0) ;
		//m_pItem->LaunchPatcher() ;
	}
	return CStatic::OnLButtonDown( nFlags,point);
}

LRESULT CUIHMXStatic::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_bHyperLink)
	{
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));	
	}
	return 0;
}

void CUIHMXStatic::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bHyperLink)
	{
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND)));		
	}
	CStatic::OnMouseMove(nFlags, point);
}


HBRUSH CUIHMXStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	// TODO: Change any attributes of the DC here
	if( m_bTransparent )
 		pDC->SetBkMode( TRANSPARENT );
	else {
		pDC->SetBkMode( OPAQUE );
		pDC->SetBkColor(m_clrBkClr);
	}
	pDC->SetTextColor(m_clrTextClr);

	// TODO: Return a non-NULL brush if the parent's handler should not be called
	if( m_bTransparent )
		return (HBRUSH)m_brsHollow;
	else
		return (HBRUSH)m_brsBkGnd;
}

BOOL CUIHMXStatic::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	//if( m_bTransparent )
	//	pDC->SetBkMode( TRANSPARENT );
	//else
		//pDC->SetBkColor(m_clrBkClr);
	return TRUE;

//	return CStatic::OnEraseBkgnd(pDC);
}

/////////////////////////////////////////////////////////////////////////////

/********************************************************************

	created:	2001/10/25
	in:			clr
	out:		none
	return:		always true;
	
	purpose:	set background color

*********************************************************************/

bool CUIHMXStatic::SetBkClr(COLORREF clr)
{
	m_clrBkClr = clr;
	m_brsBkGnd.DeleteObject();
	m_brsBkGnd.CreateSolidBrush(m_clrBkClr);
	Invalidate();

	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			none
	out:		clr
	return:		always true
	
	purpose:	return background color

*********************************************************************/

bool CUIHMXStatic::GetBkClr(COLORREF & clr)
{
	clr = m_clrBkClr;
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			clr
	out:		none
	return:		always true;
	
	purpose:	set Text color

*********************************************************************/

bool CUIHMXStatic::SetTextClr(COLORREF clr)
{
	m_clrTextClr = clr;
	Invalidate();

	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			none
	out:		clr
	return:		always true
	
	purpose:	get text color

*********************************************************************/

bool CUIHMXStatic::GetTextClr(COLORREF & clr)
{
	clr = m_clrTextClr;
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			nHeight, bBold, bItalic, sFaceName
	out:		none
	return:		always true
	
	purpose:	set the font for the control

*********************************************************************/

bool CUIHMXStatic::SetTextFont( LONG nHeight, bool bBold, bool bItalic, const CString& sFaceName )
{
	LOGFONT lgfnt;

	lgfnt.lfHeight			= -MulDiv(nHeight, GetDeviceCaps(GetDC()->m_hDC, LOGPIXELSY), 72);
	lgfnt.lfWidth			= 0;
	lgfnt.lfEscapement		= 0;
	lgfnt.lfOrientation		= 0;
	lgfnt.lfWeight			= bBold?FW_BOLD:FW_DONTCARE;
	lgfnt.lfItalic			= bItalic?TRUE:FALSE;
	lgfnt.lfUnderline		= FALSE;
	lgfnt.lfStrikeOut		= FALSE;
	lgfnt.lfCharSet			= DEFAULT_CHARSET;
	lgfnt.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	lgfnt.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lgfnt.lfQuality			= DEFAULT_QUALITY;
	lgfnt.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s( lgfnt.lfFaceName, sFaceName );

	return SetTextFont( lgfnt );

}

/********************************************************************

	created:	2001/10/25
	in:			lgFont
	out:		none
	return:		always true
	
	purpose:	set the font for the control

*********************************************************************/

bool CUIHMXStatic::SetTextFont( const LOGFONT& lgfnt )
{
	m_fntText.DeleteObject();
	m_fntText.CreateFontIndirect( &lgfnt );
	SetFont( &m_fntText, TRUE );

	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			none
	out:		lgFont
	return:		always true
	
	purpose:	get text font

*********************************************************************/

bool CUIHMXStatic::GetTextFont( LOGFONT* plgfnt)
{
	GetFont()->GetObject( sizeof(LOGFONT), (void*)plgfnt);
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			bTransparent
	out:		none
	return:		always true
	
	purpose:	set transparency

*********************************************************************/

bool CUIHMXStatic::SetTransparent( bool bTransparent /* = true */)
{
	m_bTransparent = bTransparent;
//	Invalidate();
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			none
	out:		none
	return:		transparency
	
	purpose:	get transparency

*********************************************************************/

bool CUIHMXStatic::GetTransparent()
{
	return m_bTransparent;
}

/********************************************************************

	created:	2001/10/25
	in:			bBold
	out:		none
	return:		always true
	
	purpose:	set font bold

*********************************************************************/

bool CUIHMXStatic::SetFontBold( bool bBold )
{
	LOGFONT lgfnt;
	
	GetTextFont( &lgfnt );
	lgfnt.lfWeight	= bBold?FW_BOLD:FW_DONTCARE; 
	
	SetTextFont( lgfnt );
	
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			bItalic
	out:		none
	return:		always true
	
	purpose:	set the font italic

*********************************************************************/

bool CUIHMXStatic::SetFontItalic( bool bItalic )
{
	LOGFONT lgfnt;
	
	GetTextFont( &lgfnt );
	lgfnt.lfItalic	= bItalic ? TRUE : FALSE; 
	
	SetTextFont( lgfnt );
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			nHeight
	out:		none
	return:		always true
	
	purpose:	set the font height

*********************************************************************/

bool CUIHMXStatic::SetFontHeight( int nHeight )
{
	LOGFONT lgfnt;
	
	GetTextFont( &lgfnt );
	lgfnt.lfHeight	= nHeight ;//MulDiv(nHeight, GetDeviceCaps(GetDC()->m_hDC, LOGPIXELSY), 72);    
	lgfnt.lfWidth	= 0; 
	
	SetTextFont( lgfnt );
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			sFaceName
	out:		none
	return:		always true
	
	purpose:	set the font face name

*********************************************************************/

bool CUIHMXStatic::SetFontFaceName( const CString& sFaceName )
{
	LOGFONT lgfnt;
	
	GetTextFont( &lgfnt );
	//wcscpy( lgfnt.lfFaceName, sFaceName );
	
	SetTextFont( lgfnt );
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			nAngle
	out:		none
	return:		always true
	
	purpose:	set the rotation of fonts
				works only with TT fonts

*********************************************************************/

bool CUIHMXStatic::SetFontRotation( int nAngle )
{
	LOGFONT lgfnt;
	
	GetTextFont( &lgfnt );
	lgfnt.lfEscapement	= long(nAngle)*10;
	lgfnt.lfOrientation	= long(nAngle)*10;
	SetTextFont( lgfnt );
	
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			none
	out:		none
	return:		always true
	
	purpose:	init tooltip

*********************************************************************/

bool CUIHMXStatic::InitToolTip()
{
	if (m_tt.m_hWnd == NULL) {
		m_tt.Create(this);
		m_tt.Activate(true);
		m_tt.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	}
	
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			sText, bActivate
	out:		none
	return:		always true
	
	purpose:	set tooltip text

*********************************************************************/

bool CUIHMXStatic::SetToolTipText(const CString& sText, bool bActivate)
{
	InitToolTip(); 
	
	// If there is no tooltip defined then add it
	if (m_tt.GetToolCount() == 0)
	{
		CRect rect; 
		GetClientRect(rect);
		m_tt.AddTool(this, sText, rect, 1);
	}
	
	m_tt.UpdateTipText(sText, this, 1);
	m_tt.Activate(bActivate);
	
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			bActivate
	out:		none
	return:		always true
	
	purpose:	activate/deactivate tooltip

*********************************************************************/

bool CUIHMXStatic::ActivateToolTip(bool bActivate)
{
	if (m_tt.GetToolCount() == 0)
		return false;
	
	// Activate tooltip
	m_tt.Activate(bActivate);
	
	return true;
}

/********************************************************************

	created:	2001/10/25
	in:			see CWnd::PretanslateMessage
	out:		see CWnd::PretanslateMessage
	return:		see CWnd::PretanslateMessage
	
	purpose:	let tooltip works

*********************************************************************/

BOOL CUIHMXStatic::PreTranslateMessage(MSG* pMsg) 
{
	// treat double click as simple click to avoid some issue when triggering popup window.
	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;

	InitToolTip();
	m_tt.RelayEvent(pMsg);
	
	return CStatic::PreTranslateMessage(pMsg);
} 

/********************************************************************

	created:	2009/04/10
	
	purpose:	avoid flicking of text when setwindowtext

*********************************************************************/
LRESULT CUIHMXStatic::OnSetText(WPARAM wParam,LPARAM lParam)
{
   LRESULT Result = Default();
   CRect Rect;
   GetWindowRect(&Rect);
   GetParent()->ScreenToClient(&Rect);
   GetParent()->InvalidateRect(&Rect);
   GetParent()->UpdateWindow();
   return Result;
}

bool CUIHMXStatic::SetWindowText(LPCTSTR lpszString)
{
	TCHAR szText[MAX_PATH] = {0} ;
	GetWindowText(szText, MAX_PATH) ;

	if (!_tcscmp(szText, lpszString))
	{
		return true ;
	}
	else
	{
		CWnd::SetWindowText(lpszString) ;
	}

	return true ;
}


BEGIN_MESSAGE_MAP(CUIHMXStaticForDash, CUIHMXStatic)
//	ON_WM_PAINT()
ON_WM_PAINT()
END_MESSAGE_MAP()

//void CUIHMXStaticForDash::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: Add your message handler code here
//	// Do not call CUIHMXStatic::OnPaint() for painting messages
//}

void CUIHMXStaticForDash::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CUIHMXStatic::OnPaint() for painting messages
	if (m_bDash)
	{
		if (m_pItemBk == NULL)
		{
			CRect rc ;
			GetClientRect(&rc) ;

			CPen pFrame(PS_DASH, 1, 0xaCaCaC) ;
			dc.SelectObject(&pFrame) ;
			dc.DrawFocusRect(&rc) ;	
			pFrame.DeleteObject() ;
		}
		else
		{
			Graphics gDC(dc.m_hDC) ;
			gDC.DrawImage(m_pItemBk, 0, 0, m_pItemBk->GetWidth(), m_pItemBk->GetHeight()) ;
		}
	}
}

BOOL CUIHMXStaticForDash::CreateBitmap(LPCTSTR lpszFile, Bitmap** ppbpParam)
{

	HRESULT  hResult;
	IStream  *pIStream=NULL;
	_tstring    tsPath=lpszFile;

	///set listitem background
	
	*ppbpParam = Bitmap::FromFile(tsPath.toWideString().c_str());
	if(!(*ppbpParam))
	{
		return FALSE ;
	}

	return TRUE ;
}

BOOL CUIHMXStaticForDash::DeleteBitmap(Bitmap* pbpParam)
{
	if (pbpParam != NULL)
	{
		delete pbpParam ;
	}

	return TRUE ;
}