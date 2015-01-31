// XScrollBar.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
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

#ifndef XSCROLLBAR_H
#define XSCROLLBAR_H

#define XSCROLLBAR_BASE_CLASS CWnd

//=============================================================================
class CXScrollBar : public XSCROLLBAR_BASE_CLASS
//=============================================================================
{
// Construction and creation
public:
	CXScrollBar();
	virtual ~CXScrollBar();
	BOOL CreateFromWindow(DWORD dwStyle,
						  CWnd* pParentWnd,
						  HWND hStatic,
						  UINT nId);
	BOOL CreateFromRect(DWORD dwStyle,
						CWnd* pParentWnd,
						CRect& rect,
						UINT nId);

// Attributes
public:
	CXScrollBar&	EnableChannelColor(BOOL bEnable) 
					{ m_bChannelColor = bEnable; return *this; }
	CXScrollBar&	EnableThumbColor(BOOL bEnable);
	CXScrollBar&	EnableThumbGripper(BOOL bGripper) 
					{ m_bThumbGripper = bGripper; return *this; }
	int				GetScrollPos() const 
					{ return m_nPos; }
	void			GetScrollRange(LPINT lpMinPos, LPINT lpMaxPos) const
					{ *lpMinPos = m_nMinPos; *lpMaxPos = m_nMaxPos; }
	CXScrollBar&	SetArrowHotColor(COLORREF cr)	
					{ m_ArrowHotColor = cr; return *this; }
	int				SetScrollPos(int nPos, BOOL bRedraw = TRUE);
	CXScrollBar&	SetScrollRange(int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);
	CXScrollBar&	SetThumbColor(COLORREF rgb);
	CXScrollBar&	SetThumbHoverColor(COLORREF rgb) 
					{ m_ThumbHoverColor = rgb; return *this; }

	void            SetVScrollItemParam(int nNotScrolledCount, int nTotalCount, int nItemHeight) ;

// Operations
public:
	void	ScrollLeft();
	void	ScrollRight();
	void	ScrollUp();
	void	ScrollDown();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXScrollBar)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWnd *		m_pParent;				// control parent 
	int			m_nThumbLeft;			// left margin of thumb
	int			m_nThumbTop;			// top margin of thumb
	int			m_nBitmapHeight;		// height of arrow and thumb bitmaps
	int			m_nBitmapWidth;			// width of arrow and thumb bitmaps
	int			m_nPos;					// current thumb position in scroll units
	int			m_nMinPos;				// minimum scrolling position
	int			m_nMaxPos;				// maximum scrolling position
	int			m_nRange;				// absolute value of max - min pos
	CRect		m_rectThumb;			// current rect for thumb
	CRect		m_rectClient;			// control client rect
	CRect		m_rectLeftArrow;		// left arrow rect
	CRect		m_rectRightArrow;		// right arrow rect
	CRect		m_rectUpArrow;			// up arrow rect
	CRect		m_rectDownArrow;		// down arrow rect
	CBitmap		m_bmpThumb;				// bitmap for thumb
	CBitmap		m_bmpThumbHot;			// bitmap for hot thumb
	CBitmap		m_bmpUpLeftArrow;		// bitmap for up/left arrow
	CBitmap		m_bmpDownRightArrow;	// bitmap for down/right arrow
    HCURSOR		m_hCursor;				// hand cursor
	COLORREF	m_ThumbColor;			// thumb color
	COLORREF	m_ThumbHoverColor;		// thumb color when mouse hovers
	COLORREF	m_ArrowHotColor;		// hover color
	BOOL		m_bThumbColor;			// TRUE = display thumb with color
	BOOL		m_bChannelColor;		// TRUE = display color in channel
	BOOL		m_bThumbGripper;		// TRUE = display thumb gripper
	BOOL		m_bThumbHover;			// TRUE = mouse is over thumb
	BOOL		m_bMouseOverArrowRight;	// TRUE = mouse over right arrow
	BOOL		m_bMouseOverArrowLeft;	// TRUE = mouse over left arrow
	BOOL		m_bMouseOverArrowUp;	// TRUE = mouse over up arrow
	BOOL		m_bMouseOverArrowDown;	// TRUE = mouse over down arrow
	BOOL		m_bMouseDownArrowRight;	// TRUE = mouse over right arrow & left button down
	BOOL		m_bMouseDownArrowLeft;	// TRUE = mouse over left arrow & left button down
	BOOL		m_bMouseDownArrowUp;	// TRUE = mouse over up arrow & left button down
	BOOL		m_bMouseDownArrowDown;	// TRUE = mouse over down arrow & left button down
	BOOL		m_bMouseDown;			// TRUE = mouse over thumb & left button down
	BOOL		m_bDragging;			// TRUE = thumb is being dragged
	BOOL		m_bHorizontal;			// TRUE = horizontal scroll bar
	BOOL		m_bThemed;				// TRUE = use visual themes

	int         m_nNotVScrolledCount ;
	int         m_nTotalCount ;
	int         m_nItemHeight ;

	CPoint      m_ptMouseDown ;

	void		ColorThumb(CDC *pDC, COLORREF rgbThumb);
	void		Draw(CDC *pDC = NULL);
	void		DrawHorizontal(CDC *pDC);
	void		DrawVertical(CDC *pDC);
	void		GetChannelColors(COLORREF& rgb1, COLORREF& rgb2, COLORREF& rgb3);
	void		LimitThumbPosition();
	void		LoadThumbBitmap();
	void		SetDefaultCursor();
	void		SetPositionFromThumb();
	void		UpdateThumbPosition();

	// Generated message map functions
protected:
	//{{AFX_MSG(CXScrollBar)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //XSCROLLBAR_H
