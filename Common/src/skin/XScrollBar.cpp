// XScrollBar.cpp  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// History
//     Version 1.2 - 2008 August 29
//     - Optimized bitmap loading
//     - Changed to CWnd
//     - Added hot state to arrows and thumb (no color)
//     - Added pressed state to arrows
//     - Focus shown by hot state of thumb
//     - Added CreateFromWindow() and CreateFromRect() functions
//     - Removed CreateFromStatic()
//     - Added GetScrollRange()
//     - Added keyboard input
//
//     Version 1.1 - 2004 September 21
//     - Update with Steve Mayfield's vertical scrollbar support
//
//     Version 1.0 - 2004 September 9
//     - Initial public release
//
// Acknowledgements:
//     Thanks to Greg Ellis for his CSkinHorizontalScrollbar class,
//     which I used as the starting point for CXScrollBar:
//         http://www.codeproject.com/listctrl/skinlist.asp
//
//     Thanks to Christian Rodemeyer for his CColor class:
//         http://www.codeproject.com/bitmap/ccolor.asp
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "skin/XScrollBar.h"
#include "skin/memdcScroll.h"
#include "skin/Color.h"

#if _MFC_VER < 0x700
#pragma optimize("g", off)	// necessary in VC 6.0 to suppress (release mode)
							//  "fatal error C1001: INTERNAL COMPILER ERROR"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#define TRACE __noop
#undef TRACERECT
#define TRACERECT __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)		// From WINUSER.H
#endif

//=============================================================================
// timer defines

#define TIMER_MOUSE_OVER_BUTTON		1	// mouse is over an arrow button, and
										// left button is down

#define TIMER_LBUTTON_PRESSED		2	// mouse is over an arrow button, and
										// left button has just been pressed

#define TIMER_MOUSE_OVER_THUMB		3	// mouse is over thumb

#define TIMER_MOUSE_HOVER_BUTTON	4	// mouse is over an arrow button

#define MOUSE_OVER_BUTTON_TIME		50

//=============================================================================
// color defines

#define THUMB_MASK_COLOR					RGB(0,0,1)
#define THUMB_GRIPPER_MASK_COLOR			RGB(0,0,2)
#define THUMB_LEFT_TRANSPARENT_MASK_COLOR	RGB(0,0,3)
#define THUMB_RIGHT_TRANSPARENT_MASK_COLOR	RGB(0,0,4)
#define THUMB_UP_TRANSPARENT_MASK_COLOR		THUMB_LEFT_TRANSPARENT_MASK_COLOR
#define THUMB_DOWN_TRANSPARENT_MASK_COLOR	THUMB_RIGHT_TRANSPARENT_MASK_COLOR
#define THUMB_GRIPPER_COLOR					RGB(91,91,91)	// dark gray
#define THUMB_HOVER_COLOR					RGB(51,51,51)	// orange
#define FRAME_COLOR							RGB(37,37,37)	// dark gray

//=============================================================================
static BOOL IsVista()
//=============================================================================
{
	BOOL rc = FALSE;

	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{
		if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
			(osvi.dwMajorVersion >= 6))
		{
			rc = TRUE;
		}
	}

	return rc;
}

//=============================================================================
BEGIN_MESSAGE_MAP(CXScrollBar, XSCROLLBAR_BASE_CLASS)
//=============================================================================
	//{{AFX_MSG_MAP(CXScrollBar)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
CXScrollBar::CXScrollBar()
//=============================================================================
{
	m_pParent              = NULL;
	m_bHorizontal          = TRUE;
	m_hCursor              = NULL;
	m_bMouseDown           = FALSE;
	m_bMouseOverArrowRight = FALSE;
	m_bMouseOverArrowLeft  = FALSE;
	m_bMouseOverArrowUp    = FALSE;
	m_bMouseOverArrowDown  = FALSE;
	m_bMouseDownArrowLeft  = FALSE;
	m_bMouseDownArrowRight = FALSE;
	m_bMouseDownArrowUp    = FALSE;
	m_bMouseDownArrowDown  = FALSE;
	m_bDragging            = FALSE;
	m_nPos                 = 0;
	m_nMinPos              = 0;
	m_nMaxPos              = 0;
	m_nRange               = 0;
	m_bChannelColor        = TRUE;
	m_bThumbColor          = FALSE;
	m_bThumbGripper        = TRUE;
	m_bThumbHover          = FALSE;
	m_rectThumb            = CRect(-1,-1,-1,-1);
	m_rectClient           = CRect(-1,-1,-1,-1);
	m_rectLeftArrow        = CRect(-1,-1,-1,-1);
	m_rectRightArrow       = CRect(-1,-1,-1,-1);
	m_rectUpArrow          = CRect(-1,-1,-1,-1);
	m_rectDownArrow        = CRect(-1,-1,-1,-1);
	m_nThumbLeft           = 25;
	m_nThumbTop            = 16;
	m_nBitmapWidth         = 13;
	m_nBitmapHeight        = 16;
	m_ThumbColor           = RGB(0,0,0);
	m_ThumbHoverColor      = THUMB_HOVER_COLOR;
	m_ArrowHotColor        = RGB(37,37,37);
	if (IsVista())
		m_ArrowHotColor    = RGB(37,37,37);

	m_nNotVScrolledCount = 0 ;
	m_nTotalCount        = 0 ;
	m_nItemHeight        = 0 ;

	m_ptMouseDown.x = m_ptMouseDown.y = 0 ;
}

//=============================================================================
CXScrollBar::~CXScrollBar()
//=============================================================================
{
	if (m_hCursor)
		DestroyCursor(m_hCursor);
	m_hCursor = NULL;

	if (m_bmpThumb.GetSafeHandle())
		m_bmpThumb.DeleteObject();

	if (m_bmpThumbHot.GetSafeHandle())
		m_bmpThumbHot.DeleteObject();

	if (m_bmpUpLeftArrow.GetSafeHandle())
		m_bmpUpLeftArrow.DeleteObject();

	if (m_bmpDownRightArrow.GetSafeHandle())
		m_bmpDownRightArrow.DeleteObject();
}

//=============================================================================
//
// CreateFromWindow
//
// Purpose:     Create the CXScrollBar control from placeholder window
//
// Parameters:  dwStyle    - the scroll bar’s style. Typically this will be
//                           SBS_HORZ | WS_CHILD | WS_VISIBLE | WS_TABSTOP.
//              pParentWnd - the scroll bar’s parent window, usually a CDialog
//                           object. It must not be NULL.
//              hWnd       - HWND of placeholder window (must already exist)
//              nId        - the resource id of the CXScrollBar control
//
// Returns:     BOOL       - TRUE = success
//
BOOL CXScrollBar::CreateFromWindow(DWORD dwStyle,
								   CWnd* pParentWnd,
								   HWND hWnd,
								   UINT nId)
{
	TRACE(_T("in CXScrollBar::CreateFromWindow\n"));

	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));
	ASSERT(::IsWindow(hWnd));

	CRect rect;
	::GetWindowRect(hWnd, &rect);
	pParentWnd->ScreenToClient(&rect);

	// hide placeholder window
	::ShowWindow(hWnd, SW_HIDE);

	return CreateFromRect(dwStyle, pParentWnd, rect, nId);
}

//=============================================================================
//
// CreateFromRect
//
// Purpose:     Create the CXScrollBar control from rect
//
// Parameters:  dwStyle    - the scroll bar’s style. Typically this will be
//                           SBS_HORZ | WS_CHILD | WS_VISIBLE | WS_TABSTOP.
//              pParentWnd - the scroll bar’s parent window, usually a CDialog
//                           object. It must not be NULL.
//              rect       - the size and position of the window, in client 
//                           coordinates of pParentWnd
//              nId        - the resource id of the CXScrollBar control
//
// Returns:     BOOL       - TRUE = success
//
BOOL CXScrollBar::CreateFromRect(DWORD dwStyle,
								 CWnd* pParentWnd,
								 CRect& rect,
								 UINT nId)
{
	TRACE(_T("in CXScrollBar::CreateFromRect\n"));

	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));

	m_pParent = pParentWnd;

	// load hand cursor
	SetDefaultCursor();

	m_bHorizontal = (dwStyle & SBS_VERT) ? FALSE : TRUE;

	BOOL bResult = 	XSCROLLBAR_BASE_CLASS::Create(
						AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, 0, 0, 0),
						_T(""), dwStyle, rect, pParentWnd, nId);

	if (bResult)
	{
		// we assume that width of thumb is same as width of arrows
		LoadThumbBitmap();

		if (m_bmpThumb.GetSafeHandle())
		{
			BITMAP bm;
			m_bmpThumb.GetBitmap(&bm);

			GetClientRect(&m_rectClient);

			m_nBitmapWidth = bm.bmWidth;
			if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
			{
				m_nBitmapHeight = m_rectClient.Height() * ((float)m_nNotVScrolledCount/m_nTotalCount) ;
			}
			else
			{
				m_nBitmapHeight = bm.bmHeight;
			}

			TRACE(_T("m_nBitmapWidth=%d  m_nBitmapHeight=%d\n"),
				m_nBitmapWidth, m_nBitmapHeight);

			TRACE(_T("m_rectClient:  %d, %d, %d, %d\n"),
				m_rectClient.left, m_rectClient.top, 
				m_rectClient.right, m_rectClient.bottom);

			m_rectLeftArrow = CRect(m_rectClient.left, m_rectClient.top,
				m_rectClient.left + m_nBitmapWidth, m_rectClient.bottom);

			m_rectRightArrow = CRect(m_rectClient.right - m_nBitmapWidth, m_rectClient.top,
				m_rectClient.right, m_rectClient.bottom);

			m_rectUpArrow = CRect(m_rectClient.left, m_rectClient.top, 
				m_rectClient.right, m_rectClient.top + 16);

			m_rectDownArrow = CRect(m_rectClient.left, m_rectClient.bottom - 16, 
				m_rectClient.right, m_rectClient.bottom);
		}
		else
		{
			TRACE(_T("ERROR - failed to load thumb bitmap\n"));
			ASSERT(FALSE);
		}

		// load arrow bitmaps

		VERIFY(m_bmpUpLeftArrow.LoadBitmap(m_bHorizontal ?
										   IDB_HORIZONTAL_SCROLLBAR_LEFTARROW :
										   IDB_VERTICAL_SCROLLBAR_UPARROW));

		VERIFY(m_bmpDownRightArrow.LoadBitmap(m_bHorizontal ?
											  IDB_HORIZONTAL_SCROLLBAR_RIGHTARROW :
											  IDB_VERTICAL_SCROLLBAR_DOWNARROW));
	}
	else
	{
		TRACE(_T("ERROR - failed to create CXScrollBar\n"));
		ASSERT(FALSE);
	}

	return bResult;
}

//=============================================================================
void CXScrollBar::OnPaint()
//=============================================================================
{
	CPaintDC dc(this); // device context for painting
	Draw(&dc);
}

//=============================================================================
void CXScrollBar::Draw(CDC *pDC /*= NULL*/)
//=============================================================================
{
	BOOL bRelease = FALSE;
	if (!pDC)
	{
		pDC = GetDC();
		bRelease = TRUE;
	}
	if (m_bHorizontal)
		DrawHorizontal(pDC);
	else
		DrawVertical(pDC);
	if (bRelease)
		ReleaseDC(pDC);
}

//=============================================================================
void CXScrollBar::DrawHorizontal(CDC *pDC)
//=============================================================================
{
	TRACE(_T("in CXScrollBar::DrawHorizontal\n"));

	CMemDC memDC(pDC, &m_rectClient);

	CBrush brushFrame(FRAME_COLOR);
	CBrush brushHot(m_ArrowHotColor);

	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(pDC);

	// =====  draw left arrow  =====

	CBitmap* pOldBitmap = NULL;

	pOldBitmap = bitmapDC.SelectObject(&m_bmpUpLeftArrow);

	// NOTE:  thumb and arrow bitmaps are assumed to be same width and height

	CRect rectLeft(m_rectLeftArrow);
	if (m_bMouseDownArrowLeft)
		rectLeft.OffsetRect(1, 1);

	memDC.StretchBlt(rectLeft.left, rectLeft.top+1,
		rectLeft.Width(), rectLeft.Height()-1,
		&bitmapDC, 0, 0, m_nBitmapWidth, m_nBitmapHeight, SRCCOPY);

	if (m_bMouseOverArrowLeft)
	{
		CRect rect(m_rectLeftArrow);
		rect.DeflateRect(1, 1);
		memDC.FrameRect(&rect, &brushHot);
		rect.OffsetRect(1, 1);
		memDC.FrameRect(&rect, &brushHot);
	}
	else
	{
		memDC.FrameRect(&m_rectLeftArrow, &brushFrame);
	}

	int nChannelStart = m_rectClient.left + m_nBitmapWidth;
	int nChannelWidth = m_rectClient.Width() - 2*m_nBitmapWidth;

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;

	// =====  draw channel  =====

	// save new thumb position
	TRACE(_T("m_nThumbLeft=%d\n"), m_nThumbLeft);
	m_rectThumb.left   = m_rectClient.left + m_nThumbLeft;
	m_rectThumb.right  = m_rectThumb.left + m_nBitmapWidth;
	m_rectThumb.top    = m_rectClient.top;
	m_rectThumb.bottom = m_rectThumb.top + m_rectClient.Height();

	CBitmap bmpChannel;
	VERIFY(bmpChannel.LoadBitmap(IDB_HORIZONTAL_SCROLLBAR_CHANNEL));

	pOldBitmap = bitmapDC.SelectObject(&bmpChannel);

	CRect rectChannelRight(m_rectThumb.left + m_nBitmapWidth/2, m_rectClient.top,
		nChannelStart + nChannelWidth, m_rectClient.bottom);

	memDC.StretchBlt(rectChannelRight.left, rectChannelRight.top+1,
		rectChannelRight.Width(), rectChannelRight.Height()-1,
		&bitmapDC, 0, 0, 1, m_nBitmapHeight, SRCCOPY);

	if (m_bChannelColor && m_bThumbColor)
	{
		COLORREF rgb1, rgb2, rgb3;
		GetChannelColors(rgb1, rgb2, rgb3);

		BITMAP bm;
		bmpChannel.GetBitmap(&bm);

		// set highlight colors
		bitmapDC.SetPixel(0, 0, rgb1);
		bitmapDC.SetPixel(0, 1, rgb2);

		// set main color
		for (int y = 2; y < (bm.bmHeight); y++)
			bitmapDC.SetPixel(0, y, rgb3);
	}

	CRect rectChannelLeft(nChannelStart, m_rectClient.top,
		m_rectThumb.left + m_nBitmapWidth/2, m_rectClient.bottom);

	memDC.StretchBlt(rectChannelLeft.left, rectChannelLeft.top+1,
		rectChannelLeft.Width(), rectChannelLeft.Height()-1,
		&bitmapDC, 0, 0, 1, m_nBitmapHeight, SRCCOPY);

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	if (bmpChannel.GetSafeHandle())
		bmpChannel.DeleteObject();
	pOldBitmap = NULL;

	// =====  draw right arrow  =====

	pOldBitmap = bitmapDC.SelectObject(&m_bmpDownRightArrow);

	CRect rectRight(m_rectRightArrow);
	if (m_bMouseDownArrowRight)
		rectRight.OffsetRect(1, 1);

	memDC.StretchBlt(rectRight.left, rectRight.top+1,
		rectRight.Width(), rectRight.Height()-1,
		&bitmapDC, 0, 0, m_nBitmapWidth, m_nBitmapHeight, SRCCOPY);

	if (m_bMouseOverArrowRight)
	{
		CRect rect(m_rectRightArrow);
		rect.DeflateRect(0, 1, 1, 1);
		memDC.FrameRect(&rect, &brushHot);
		rect.OffsetRect(1, 1);
		memDC.FrameRect(&rect, &brushHot);
	}
	else
	{
		memDC.FrameRect(&m_rectRightArrow, &brushFrame);
	}

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;

	// If there is nothing to scroll then don't show the thumb
	if (m_nRange)
	{
		// =====  draw thumb  =====

		if (m_bThumbHover || (::GetFocus() == m_hWnd))
			pOldBitmap = bitmapDC.SelectObject(&m_bmpThumbHot);
		else
			pOldBitmap = bitmapDC.SelectObject(&m_bmpThumb);

		// fill in transparent channel color
		for (int x = 0; x < m_nBitmapWidth; x++)
		{
			for (int y = 0; y < m_nBitmapHeight; y++)
			{
				COLORREF rgb = bitmapDC.GetPixel(x, y);

				if (rgb == THUMB_LEFT_TRANSPARENT_MASK_COLOR)
					bitmapDC.SetPixel(x, y, memDC.GetPixel(nChannelStart, y));
				else if (rgb == THUMB_RIGHT_TRANSPARENT_MASK_COLOR)
					bitmapDC.SetPixel(x, y, 
						memDC.GetPixel(nChannelStart+nChannelWidth-1, y));
			}
		}

		memDC.StretchBlt(m_rectThumb.left, m_rectThumb.top,
			m_rectThumb.Width(), m_rectThumb.Height(),
			&bitmapDC, 0, 0, m_nBitmapWidth, m_nBitmapHeight, SRCCOPY);

		if (!m_bThumbColor)
		{
			if (m_bThumbHover || (::GetFocus() == m_hWnd))
			{
				CRect rect(m_rectThumb);
				rect.DeflateRect(1, 1, 1, 2);
				memDC.FrameRect(&rect, &brushHot);
				rect.DeflateRect(1, 1);
				memDC.FrameRect(&rect, &brushHot);
			}
		}

		if (pOldBitmap)
			bitmapDC.SelectObject(pOldBitmap);
		pOldBitmap = NULL;
	}
	else
	{
		m_rectThumb = CRect(-1,-1,-1,-1);
	}

	memDC.FrameRect(&m_rectClient, &brushFrame);
}

//=============================================================================
void CXScrollBar::DrawVertical(CDC *pDC)
//=============================================================================
{
	TRACE(_T("in CXScrollBar::DrawVertical\n"));

	CMemDC memDC(pDC, &m_rectClient);

	CBrush brushFrame(FRAME_COLOR);
	CBrush brushHot(m_ArrowHotColor);

	CDC bitmapDC;
	bitmapDC.CreateCompatibleDC(pDC);

	// =====  draw Up arrow  =====

	CBitmap* pOldBitmap = bitmapDC.SelectObject(&m_bmpUpLeftArrow);

	// NOTE:  thumb and arrow bitmaps are assumed to be same width and height

	CRect rectUp(m_rectUpArrow);
	if (m_bMouseDownArrowUp)
		rectUp.OffsetRect(0, 0);

	memDC.StretchBlt(rectUp.left, rectUp.top, 
		rectUp.Width(), rectUp.Height(), 
		&bitmapDC, 0, 0, m_nBitmapWidth, 16, SRCCOPY);

	if (m_bMouseOverArrowUp)
	{
		CRect rect(m_rectUpArrow);
		rect.DeflateRect(1, 0, 1, 1);
		memDC.FrameRect(&rect, &brushHot);
		rect.OffsetRect(0, 0);
		memDC.FrameRect(&rect, &brushHot);
	}
	else
	{
		memDC.FrameRect(&m_rectUpArrow, &brushFrame);
	}

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;

	int nChannelStart = m_rectClient.top + 16;
	int nChannelHeight = m_rectClient.Height() - 2*16;

	// =====  draw channel  =====

	// save new thumb position
	TRACE(_T("m_nThumbTop=%d\n"), m_nThumbTop);
	m_rectThumb.left   = m_rectClient.left;
	m_rectThumb.right  = m_rectThumb.left + m_rectClient.Width();
	m_rectThumb.top    = m_rectClient.top + m_nThumbTop;
	m_rectThumb.bottom = m_rectThumb.top + m_nBitmapHeight;

	CBitmap bmpChannel;
	VERIFY(bmpChannel.LoadBitmap(IDB_VERTICAL_SCROLLBAR_CHANNEL));
	pOldBitmap = bitmapDC.SelectObject(&bmpChannel);

	CRect rectChannelDown(m_rectClient.left, m_rectThumb.top + m_nBitmapHeight/2, 
		m_rectClient.right, nChannelStart + nChannelHeight);

	memDC.StretchBlt(rectChannelDown.left, rectChannelDown.top, 
		rectChannelDown.Width()-1, rectChannelDown.Height(), 
		&bitmapDC, 0, 0, m_nBitmapWidth, 1, SRCCOPY);

	if (m_bChannelColor && m_bThumbColor)
	{
		COLORREF rgb1, rgb2, rgb3;
		GetChannelColors(rgb1, rgb2, rgb3);

		BITMAP bm;
		bmpChannel.GetBitmap(&bm);

		// set highlight colors
		bitmapDC.SetPixel(0, 0, rgb1);
		bitmapDC.SetPixel(1, 0, rgb2);

		// set main color
		for (int x = 2; x < (bm.bmWidth); x++)
			bitmapDC.SetPixel(x, 0, rgb3);
	}

	CRect rectChannelUp(m_rectClient.left, nChannelStart, 
		m_rectClient.right, m_rectThumb.top + m_nBitmapHeight/2);

	memDC.StretchBlt(rectChannelUp.left, rectChannelUp.top, 
		rectChannelUp.Width(), rectChannelUp.Height(), 
		&bitmapDC, 0, 0, m_nBitmapWidth, 1, SRCCOPY);

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	if (bmpChannel.GetSafeHandle())
		bmpChannel.DeleteObject();
	pOldBitmap = NULL;

	// =====  draw down arrow  =====

	pOldBitmap = bitmapDC.SelectObject(&m_bmpDownRightArrow);

	CRect rectDown(m_rectDownArrow);
	if (m_bMouseDownArrowDown)
		rectDown.OffsetRect(0, 0);

	memDC.StretchBlt(rectDown.left, rectDown.top, 
		rectDown.Width(), rectDown.Height(), 
		&bitmapDC, 0, 0, m_nBitmapWidth, 16, SRCCOPY);

	if (m_bMouseOverArrowDown)
	{
		CRect rect(m_rectDownArrow);
		rect.DeflateRect(1, 0, 1, 1);
		memDC.FrameRect(&rect, &brushHot);
		rect.OffsetRect(0, 0);
		memDC.FrameRect(&rect, &brushHot);
	}
	else
	{
		memDC.FrameRect(&m_rectDownArrow, &brushFrame);
	}

	if (pOldBitmap)
		bitmapDC.SelectObject(pOldBitmap);
	pOldBitmap = NULL;

	// if there is nothing to scroll then don't show the thumb
	if (m_nRange)
	{
		// =====  draw thumb  =====

		if (m_bThumbHover || (::GetFocus() == m_hWnd))
			pOldBitmap = bitmapDC.SelectObject(&m_bmpThumbHot);
		else
			pOldBitmap = bitmapDC.SelectObject(&m_bmpThumb);

		// fill in transparent channel color
		for (int x = 0; x < m_nBitmapWidth; x++)
		{
			for (int y = 0; y < m_nBitmapHeight; y++)
			{
				COLORREF rgb = bitmapDC.GetPixel(x, y);

				if (rgb	== THUMB_UP_TRANSPARENT_MASK_COLOR)
					bitmapDC.SetPixel(x, y, memDC.GetPixel(x, nChannelStart));
				else if (rgb == THUMB_DOWN_TRANSPARENT_MASK_COLOR)
					bitmapDC.SetPixel(x, y, memDC.GetPixel(x, nChannelStart+nChannelHeight));
			}
		}

		memDC.StretchBlt(m_rectThumb.left, m_rectThumb.top, 
			m_rectThumb.Width(), m_rectThumb.Height(), 
			&bitmapDC, 0, 0, m_nBitmapWidth, 1, SRCCOPY);

		if (!m_bThumbColor)
		{
			if (m_bThumbHover || (::GetFocus() == m_hWnd))
			{
				CRect rect(m_rectThumb);
				rect.DeflateRect(0, 0);
				memDC.FrameRect(&rect, &brushHot);
				rect.DeflateRect(0,0);
				memDC.FrameRect(&rect, &brushHot);
			}
		}
		if (pOldBitmap)
			bitmapDC.SelectObject(pOldBitmap);
		pOldBitmap = NULL;
	}
	else
	{
		m_rectThumb = CRect(-1,-1,-1,-1);
	}

	memDC.FrameRect(&m_rectClient, &brushFrame);
}

//=============================================================================
void CXScrollBar::GetChannelColors(COLORREF& rgb1, 
								   COLORREF& rgb2, 
								   COLORREF& rgb3)
//=============================================================================
{
	CColor color;
	color.SetRGB(GetRValue(m_ThumbColor),
				 GetGValue(m_ThumbColor), 
				 GetBValue(m_ThumbColor));
	color.ToHLS();
	float fLuminance = color.GetLuminance();

	// use 80% L, 50% S for main color
	fLuminance = 0.8f;
	float fSaturation = color.GetSaturation();
	fSaturation = 0.5f * fSaturation;
	float fHue = color.GetHue();
	color.SetHLS(fHue, fLuminance, fSaturation);
	color.ToRGB();
	rgb3 = RGB(color.GetRed(), color.GetGreen(), color.GetBlue());

	// use .87 L for second highlight color
	fLuminance = .87f;
	color.SetHLS(fHue, fLuminance, fSaturation);
	color.ToRGB();
	rgb2 = RGB(color.GetRed(), color.GetGreen(), color.GetBlue());

	// use .92 L for first highlight color
	fLuminance = .92f;
	color.SetHLS(fHue, fLuminance, fSaturation);
	color.ToRGB();
	rgb1 = RGB(color.GetRed(), color.GetGreen(), color.GetBlue());
}

//=============================================================================
void CXScrollBar::OnLButtonDown(UINT nFlags, CPoint point)
//=============================================================================
{
	KillTimer(TIMER_MOUSE_OVER_BUTTON);
	KillTimer(TIMER_LBUTTON_PRESSED);
	SetCapture();
	SetFocus();

	if (m_bHorizontal)
	{
		m_bMouseDownArrowLeft = FALSE;
		m_bMouseDownArrowRight = FALSE;
		CRect rectThumb(m_nThumbLeft, 0, m_nThumbLeft + m_nBitmapWidth, 
			m_rectClient.Height());

		if (rectThumb.PtInRect(point))
		{
			m_bMouseDown = TRUE;
		}
		else if (m_rectRightArrow.PtInRect(point))
		{
			m_bMouseDownArrowRight = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 200, NULL);
		}
		else if (m_rectLeftArrow.PtInRect(point))
		{
			m_bMouseDownArrowLeft = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 200, NULL);
		}
		else	// button down in channel
		{
			m_nThumbLeft = point.x - m_nBitmapWidth / 2;
			SetPositionFromThumb();
			Draw();

			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
				m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos),
				(LPARAM)m_hWnd);
		}
		Invalidate();
	}
	else
	{
		CRect rectThumb(0, m_nThumbTop, m_rectClient.Width(), m_nThumbTop + m_nBitmapHeight);

		if (rectThumb.PtInRect(point))
		{
			m_bMouseDown = TRUE;
			m_ptMouseDown.x = point.x ;
			m_ptMouseDown.y = point.y ;
		}
		else if (m_rectDownArrow.PtInRect(point))
		{
			m_bMouseDownArrowDown = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 150, NULL);
		}
		else if (m_rectUpArrow.PtInRect(point))
		{
			m_bMouseDownArrowUp = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 150, NULL);
		}
		else	// button down in channel
		{
			m_nThumbTop = point.y - m_nBitmapHeight / 2;
			SetPositionFromThumb();
			Draw();

			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
				m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos), 
				(LPARAM)m_hWnd);
		}
		Invalidate();
	}

	XSCROLLBAR_BASE_CLASS::OnLButtonDown(nFlags, point);
}

//=============================================================================
void CXScrollBar::OnLButtonUp(UINT nFlags, CPoint point)
//=============================================================================
{
	UpdateThumbPosition();
	KillTimer(TIMER_MOUSE_OVER_BUTTON);
	KillTimer(TIMER_LBUTTON_PRESSED);
	ReleaseCapture();

	if (m_bHorizontal)
	{
		CRect rectThumb(m_nThumbLeft, 0, m_nThumbLeft + m_nBitmapWidth, 
			m_rectClient.Height());

		m_bMouseDownArrowLeft = FALSE;
		m_bMouseDownArrowRight = FALSE;

		if (m_rectLeftArrow.PtInRect(point))
		{
			ScrollLeft();
		}
		else if (m_rectRightArrow.PtInRect(point))
		{
			ScrollRight();
		}
		else if (rectThumb.PtInRect(point))
		{
			m_bThumbHover = TRUE;
			SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
		}
		Invalidate();
	}
	else
	{
		CRect rectThumb(0, m_nThumbTop, m_rectClient.Width(), 
			m_nThumbTop + m_nBitmapHeight);

		m_bMouseDownArrowUp = FALSE;
		m_bMouseDownArrowDown = FALSE;

		if (m_rectUpArrow.PtInRect(point))
		{
			ScrollUp();
		}
		else if (m_rectDownArrow.PtInRect(point))
		{
			ScrollDown();
		}
		else if (rectThumb.PtInRect(point))
		{
			m_bThumbHover = TRUE;
			SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
		}
		Invalidate();
	}

	m_bMouseDown = FALSE;
	m_bDragging = FALSE;

	XSCROLLBAR_BASE_CLASS::OnLButtonUp(nFlags, point);
}

//=============================================================================
void CXScrollBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
//=============================================================================
{
	OnLButtonDown(nFlags, point);
	//XSCROLLBAR_BASE_CLASS::OnLButtonDblClk(nFlags, point);
}

//=============================================================================
void CXScrollBar::OnMouseMove(UINT nFlags, CPoint point)
//=============================================================================
{
	BOOL bOldThumbHover = m_bThumbHover;
	m_bThumbHover = FALSE;

	if (m_rectThumb.PtInRect(point))
		m_bThumbHover = TRUE;

	if (m_bMouseDown)
		m_bDragging = TRUE;

	BOOL bOldHover = m_bMouseOverArrowRight |
					 m_bMouseOverArrowLeft  |
					 m_bMouseOverArrowUp    |
					 m_bMouseOverArrowDown;

	m_bMouseOverArrowRight = FALSE;
	m_bMouseOverArrowLeft  = FALSE;
	m_bMouseOverArrowUp    = FALSE;
	m_bMouseOverArrowDown  = FALSE;

	if (m_bHorizontal)
	{
		if (m_rectLeftArrow.PtInRect(point))
			m_bMouseOverArrowLeft = TRUE;
		else if (m_rectRightArrow.PtInRect(point))
			m_bMouseOverArrowRight = TRUE;
	}
	else
	{
		if (m_rectUpArrow.PtInRect(point))
			m_bMouseOverArrowUp = TRUE;
		else if (m_rectDownArrow.PtInRect(point))
			m_bMouseOverArrowDown = TRUE;
	}

	BOOL bNewHover = m_bMouseOverArrowRight |
					 m_bMouseOverArrowLeft  |
					 m_bMouseOverArrowUp    |
					 m_bMouseOverArrowDown;

	if (bNewHover)
		SetTimer(TIMER_MOUSE_HOVER_BUTTON, 80, NULL);

	if (bOldHover != bNewHover)
		Invalidate();

	if (m_bDragging)
	{
		if (m_bHorizontal)
		{
			m_nThumbLeft = point.x - m_nBitmapWidth / 2;

			SetPositionFromThumb();

			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
				m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos), 
					(LPARAM)m_hWnd);
		}
		else
		{
			if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
			{
				if ((m_ptMouseDown.x = point.x) && (m_ptMouseDown.y == point.y))
				{
					m_nThumbTop = m_rectThumb.top ;
				}
				else
				{
					m_nThumbTop = m_rectThumb.top + point.y - m_ptMouseDown.y ;
				}
			}
			else
			{
				m_nThumbTop = point.y - m_nBitmapHeight / 2;
			}

			if (m_nThumbTop <= 16)
			{
				m_nThumbTop = 16 ;
			}
			else
			{
				SetPositionFromThumb();
				if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
					m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos), 
					(LPARAM)m_hWnd);
			}
		}

		Draw();
	}

	if (bOldThumbHover != m_bThumbHover)
	{
		Invalidate();
		SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
	}

	XSCROLLBAR_BASE_CLASS::OnMouseMove(nFlags, point);
}

//=============================================================================
void CXScrollBar::OnTimer(UINT nIDEvent)
//=============================================================================
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (nIDEvent == TIMER_MOUSE_HOVER_BUTTON)
	{
		m_bMouseOverArrowRight = FALSE;
		m_bMouseOverArrowLeft  = FALSE;
		m_bMouseOverArrowUp    = FALSE;
		m_bMouseOverArrowDown  = FALSE;

		if (m_bHorizontal)
		{
			if (m_rectLeftArrow.PtInRect(point))
				m_bMouseOverArrowLeft = TRUE;
			else if (m_rectRightArrow.PtInRect(point))
				m_bMouseOverArrowRight = TRUE;
		}
		else
		{
			if (m_rectUpArrow.PtInRect(point))
				m_bMouseOverArrowUp = TRUE;
			else if (m_rectDownArrow.PtInRect(point))
				m_bMouseOverArrowDown = TRUE;
		}

		if (!m_bMouseOverArrowLeft  &&
			!m_bMouseOverArrowRight && 
			!m_bMouseOverArrowUp    &&
			!m_bMouseOverArrowDown)
		{
			KillTimer(nIDEvent);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_MOUSE_OVER_BUTTON)	// mouse is in an arrow button,
													// and left button is down
	{
		if (m_bMouseDownArrowLeft)
			ScrollLeft();
		if (m_bMouseDownArrowRight)
			ScrollRight();
		if (m_bMouseDownArrowUp)
			ScrollUp();
		if (m_bMouseDownArrowDown)
			ScrollDown();

		if (!rect.PtInRect(point))
		{
			m_bMouseDownArrowLeft  = FALSE;
			m_bMouseDownArrowRight = FALSE;
			m_bMouseDownArrowUp    = FALSE;
			m_bMouseDownArrowDown  = FALSE;
		}
		if (!m_bMouseDownArrowLeft  &&
			!m_bMouseDownArrowRight && 
			!m_bMouseDownArrowUp    &&
			!m_bMouseDownArrowDown)
		{
			KillTimer(nIDEvent);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_LBUTTON_PRESSED)	// mouse is in an arrow button,
												// and left button has just been pressed
	{
		KillTimer(nIDEvent);

		if (m_bMouseDownArrowLeft  || 
			m_bMouseDownArrowRight || 
			m_bMouseDownArrowUp    || 
			m_bMouseDownArrowDown)
		{
			// debounce left click
			SetTimer(TIMER_MOUSE_OVER_BUTTON, MOUSE_OVER_BUTTON_TIME, NULL);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_MOUSE_OVER_THUMB)	// mouse is over thumb
	{
		if (!m_rectThumb.PtInRect(point))
		{
			// no longer over thumb, restore thumb color
			m_bThumbHover = FALSE;
			KillTimer(nIDEvent);
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			Invalidate();
		}
	}

	XSCROLLBAR_BASE_CLASS::OnTimer(nIDEvent);
}

//=============================================================================
void CXScrollBar::ScrollLeft()
//=============================================================================
{
	if (m_nPos > 0)
		m_nPos--;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

//=============================================================================
void CXScrollBar::ScrollRight()
//=============================================================================
{
	if (m_nPos < m_nRange)
		m_nPos++;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

//=============================================================================
void CXScrollBar::ScrollUp()
//=============================================================================
{
	if (m_nPos > 0)
		m_nPos--;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

//=============================================================================
void CXScrollBar::ScrollDown()
//=============================================================================
{
	if (m_nPos < m_nRange)
		m_nPos++;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

//=============================================================================
void CXScrollBar::SetPositionFromThumb()
//=============================================================================
{
	double dPixels, dMax, dInterval, dPos;

	LimitThumbPosition();
	dMax = m_nRange;

	if (m_bHorizontal)
	{
		dPixels   = m_rectClient.Width() - 3*m_nBitmapWidth;
		dInterval = dMax / dPixels;
		dPos      = dInterval * (m_nThumbLeft - m_nBitmapWidth);
	}
	else
	{
		if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
		{
			dPixels   = m_rectClient.Height() - m_nBitmapHeight - 2*16;
		}
		else
		{
			dPixels   = m_rectClient.Height() - 3*m_nBitmapHeight;
		}
		
		if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
		{
			dInterval = dMax / dPixels;
			dPos      = dInterval * (m_nThumbTop-16) ;
		}
		else
		{
			dInterval = dMax / dPixels;
			dPos      = dInterval * (m_nThumbTop - m_nBitmapHeight);
		}
	}

	m_nPos = (int) (dPos + 0.5);
	if (m_nPos < 0)
		m_nPos = 0;
	if (m_nPos > m_nRange)
		m_nPos = m_nRange;
}

//=============================================================================
void CXScrollBar::UpdateThumbPosition()
//=============================================================================
{
	double dPixels, dMax, dInterval, dPos;

	dMax = m_nRange;
	dPos = m_nPos;

	if (m_bHorizontal)
	{
		dPixels   = m_rectClient.Width() - 3*m_nBitmapWidth;
		dInterval = dPixels / dMax;
		double dThumbLeft = dPos * dInterval + 0.5;
		m_nThumbLeft = m_nBitmapWidth + (int)dThumbLeft;
	}
	else
	{
		if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
		{
			dPixels = m_rectClient.Height() - m_nBitmapHeight ;
		}
		else
		{
			dPixels = m_rectClient.Height() - 3*20;
		}
		dInterval = dPixels / dMax;
		double dThumbTop = dPos * dInterval + 0.5;
		m_nThumbTop = 16+ (int)dThumbTop;
	}

	LimitThumbPosition();

	Draw();
}

//=============================================================================
void CXScrollBar::LimitThumbPosition()
//=============================================================================
{
	if (m_bHorizontal)
	{
		if ((m_nThumbLeft + m_nBitmapWidth) > (m_rectClient.Width() - m_nBitmapWidth))
			m_nThumbLeft = m_rectClient.Width() - 2*m_nBitmapWidth;

		if (m_nThumbLeft < (m_rectClient.left + m_nBitmapWidth))
			m_nThumbLeft = m_rectClient.left + m_nBitmapWidth;
	}
	else
	{
		if ((m_nThumbTop + m_nBitmapHeight) > (m_rectClient.Height() - 16))
		{
			if ((m_nNotVScrolledCount != 0) && (m_nTotalCount != 0) && (m_nItemHeight != 0))
			{
				m_nThumbTop = m_rectClient.Height() - 16 - m_nBitmapHeight ;
			}
			else
			{
				m_nThumbTop = m_rectClient.Height() - 2*16;
			}
		}

		if (m_nThumbTop < (m_rectClient.top + 16))
			m_nThumbTop = m_rectClient.top + 16;
	}
}

//=============================================================================
CXScrollBar& CXScrollBar::SetScrollRange(int nMinPos,
										 int nMaxPos,
										 BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_nMinPos = nMinPos;
	m_nMaxPos = nMaxPos;
	if (m_nMinPos < m_nMaxPos)
		m_nRange = m_nMaxPos - m_nMinPos;
	else
		m_nRange = m_nMinPos - m_nMaxPos;

	if (bRedraw)
		Invalidate();

	return *this;
}

//=============================================================================
int CXScrollBar::SetScrollPos(int nPos, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	TRACE(_T("in CXScrollBar::SetScrollPos: nPos=%d\n"), nPos);
	int nOldPos = m_nPos;

	m_nPos = nPos;

	UpdateThumbPosition();

	if (bRedraw)
		Invalidate();

	return nOldPos;
}

//=============================================================================
BOOL CXScrollBar::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
//=============================================================================
{
	if (m_bThumbHover && m_hCursor)
		::SetCursor(m_hCursor);
	else
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	return TRUE;
}

//=============================================================================
void CXScrollBar::SetDefaultCursor()
//=============================================================================
{
	if (m_hCursor == NULL)				// No cursor handle - try to load one
	{
		// First try to load the Win98 / Windows 2000 hand cursor

		TRACE(_T("loading from IDC_HAND\n"));
		m_hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW/*IDC_HAND*/);

		if (m_hCursor == NULL)			// Still no cursor handle -
										// load the WinHelp hand cursor
		{
			// The following appeared in Paul DiLascia's Jan 1998 MSJ articles.
			// It loads a "hand" cursor from the winhlp32.exe module.

			TRACE(_T("loading from winhlp32\n"));

			// Get the windows directory
			CString strWndDir;
			GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
			strWndDir.ReleaseBuffer();

			strWndDir += _T("\\winhlp32.exe");

			// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
			HMODULE hModule = LoadLibrary(strWndDir);
			if (hModule)
			{
				HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
				if (hHandCursor)
					m_hCursor = (HCURSOR)CopyIcon(hHandCursor);
				FreeLibrary(hModule);
			}
		}
	}
}

//=============================================================================
CXScrollBar& CXScrollBar::SetThumbColor(COLORREF rgb) 
//=============================================================================
{ 
	m_ThumbColor = rgb;
	m_bThumbColor = (m_ThumbColor == CLR_INVALID) ? FALSE : TRUE;
	LoadThumbBitmap();
	return *this;
}

//=============================================================================
CXScrollBar& CXScrollBar::EnableThumbColor(BOOL bEnable) 
//=============================================================================
{ 
	m_bThumbColor = bEnable; 
	LoadThumbBitmap();
	return *this;
}

//=============================================================================
void CXScrollBar::ColorThumb(CDC *pDC, COLORREF rgbThumb)
//=============================================================================
{
	COLORREF rgbPrev = 0;

	// add desired hot color to thumb
	for (int x = 0; x < m_nBitmapWidth; x++)
	{
		for (int y = 0; y < m_nBitmapHeight; y++)
		{
			COLORREF rgb = pDC->GetPixel(x, y);

			if (m_bThumbColor && (rgb == THUMB_MASK_COLOR))
			{
				pDC->SetPixel(x, y, rgbThumb);
			}
			else if (rgb == THUMB_GRIPPER_MASK_COLOR)
			{
				if (m_bThumbGripper)
					pDC->SetPixel(x, y, THUMB_GRIPPER_COLOR);
				else
					pDC->SetPixel(x, y, rgbPrev);
			}

			rgbPrev = rgb;
		}
	}
}

//=============================================================================
void CXScrollBar::LoadThumbBitmap()
//=============================================================================
{
	if (m_bmpThumb.GetSafeHandle())
		m_bmpThumb.DeleteObject();

	if (m_bmpThumbHot.GetSafeHandle())
		m_bmpThumbHot.DeleteObject();

	if (m_bThumbColor)
	{
		VERIFY(m_bmpThumb.LoadBitmap(m_bHorizontal ? 
									 IDB_HORIZONTAL_SCROLLBAR_THUMB : 
									 IDB_VERTICAL_SCROLLBAR_THUMB));
		VERIFY(m_bmpThumbHot.LoadBitmap(m_bHorizontal ? 
									 IDB_HORIZONTAL_SCROLLBAR_THUMB : 
									 IDB_VERTICAL_SCROLLBAR_THUMB));

		CClientDC dc(this);

		CDC bitmapDC;
		bitmapDC.CreateCompatibleDC(&dc);

		CBitmap *pOldBitmap = bitmapDC.SelectObject(&m_bmpThumbHot);

		// add desired hot color to thumb
		ColorThumb(&bitmapDC, m_ThumbHoverColor);

		bitmapDC.SelectObject(&m_bmpThumb);

		// add desired cold color to thumb
		ColorThumb(&bitmapDC, m_ThumbColor);

		bitmapDC.SelectObject(pOldBitmap);
	}
	else
	{
		VERIFY(m_bmpThumb.LoadBitmap(m_bHorizontal ? 
									 IDB_HORIZONTAL_SCROLLBAR_THUMB_NO_COLOR :
									 IDB_VERTICAL_SCROLLBAR_THUMB_NO_COLOR));
		VERIFY(m_bmpThumbHot.LoadBitmap(m_bHorizontal ? 
									 IDB_HORIZONTAL_SCROLLBAR_THUMB_NO_COLOR :
									 IDB_VERTICAL_SCROLLBAR_THUMB_NO_COLOR));
	}
}

//=============================================================================
BOOL CXScrollBar::OnEraseBkgnd(CDC* /*pDC*/) 
//=============================================================================
{
	return TRUE;	
	//return XSCROLLBAR_BASE_CLASS::OnEraseBkgnd(pDC);
}

//=============================================================================
BOOL CXScrollBar::PreTranslateMessage(MSG* pMsg) 
//=============================================================================
{
	//=========================================================================
	// WM_KEYDOWN
	//=========================================================================
	if (pMsg->message == WM_KEYDOWN)
	{
		TRACE(_T("WM_KEYDOWN: lParam=0x%X\n"), pMsg->lParam);

		if (m_bHorizontal)
		{
			//=================================================================
			// VK_RIGHT or VK_LEFT or VK_HOME or VK_END
			//=================================================================
			if ((pMsg->wParam == VK_RIGHT) || 
				(pMsg->wParam == VK_LEFT) ||
				(pMsg->wParam == VK_HOME) ||
				(pMsg->wParam == VK_END))
			{
				UpdateThumbPosition();

				if (pMsg->wParam == VK_RIGHT)
				{
					ScrollRight();
				}
				else if (pMsg->wParam == VK_LEFT)
				{
					ScrollLeft();
				}
				else if (pMsg->wParam == VK_HOME)
				{
					m_nPos = 0;
					ScrollLeft();
				}
				else if (pMsg->wParam == VK_END)
				{
					m_nPos = m_nRange;
					ScrollRight();
				}
				return 1;
			}
		}
		else
		{
			// vertical

			//=================================================================
			// VK_DOWN or VK_UP or VK_HOME or VK_END
			//=================================================================
			if ((pMsg->wParam == VK_DOWN) || 
				(pMsg->wParam == VK_UP) ||
				(pMsg->wParam == VK_HOME) ||
				(pMsg->wParam == VK_END))
			{
				UpdateThumbPosition();

				if (pMsg->wParam == VK_DOWN)
				{
					ScrollDown();
				}
				else if (pMsg->wParam == VK_UP)
				{
					ScrollUp();
				}
				else if (pMsg->wParam == VK_HOME)
				{
					m_nPos = 0;
					ScrollUp();
				}
				else if (pMsg->wParam == VK_END)
				{
					m_nPos = m_nRange;
					ScrollDown();
				}
				return 1;
			}
		}
	}

	return XSCROLLBAR_BASE_CLASS::PreTranslateMessage(pMsg);
}

//=============================================================================
void CXScrollBar::OnSize(UINT nType, int cx, int cy) 
//=============================================================================
{
	XSCROLLBAR_BASE_CLASS::OnSize(nType, cx, cy);
	
	if (m_hWnd)
	{
		GetClientRect(&m_rectClient);
		UpdateThumbPosition();
	}
}

//=============================================================================
void CXScrollBar::OnSetFocus(CWnd* pOldWnd) 
//=============================================================================
{
	XSCROLLBAR_BASE_CLASS::OnSetFocus(pOldWnd);
	Invalidate();	
}

//=============================================================================
void CXScrollBar::OnKillFocus(CWnd* pNewWnd) 
//=============================================================================
{
	XSCROLLBAR_BASE_CLASS::OnKillFocus(pNewWnd);
	Invalidate();	
}

void CXScrollBar::SetVScrollItemParam(int nNotScrolledCount, int nTotalCount, int nItemHeight)
{
	if (nTotalCount < nNotScrolledCount)
	{
		return ;
	}

	m_nNotVScrolledCount = nNotScrolledCount ;
	m_nTotalCount        = nTotalCount ;
	m_nItemHeight        = nItemHeight ;
}