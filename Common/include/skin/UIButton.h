#pragma once
#include <gdiplus.h>
#include <tstring/tstring.h>

///Êý¾ÝÀà
class CUIButton:public CButton
{
public:
	enum BUTTON_STATE
	{
		BUTTON_ENABLE = 0,
		BUTTON_HOVER,
		BUTTON_CLICK,
		BUTTON_DISABLE
	};

	void SetButtonText( LPCTSTR lpStr)
	{
		m_tsText=lpStr;
	};

	void SetButtonTextColor(COLORREF col)
	{
		m_colorText = col ;
	} ;

	
	void			SetButtonImage(LPCTSTR lpszFileName,UINT nMakse); // set button image..	
	void			SetButtonEnable();// make button enable/disable
	void			SetButtonDisable();	
	void			SetToolTipText(LPCTSTR lpszText);// set tooltip message	
	void			SetCursor(HCURSOR hCursor);// set cursor

	void            SetHandOnMove(BOOL bHandOnMove = TRUE)
	{
		m_bHandOnMove = bHandOnMove ;
	}
	 
	void   SetHover( bool bHover)
	{
		m_bHover=bHover;
	}

	bool GetHover()
	{
		return m_bHover;
	}

	void SetMultiState(BOOL bMultiState)
	{
		m_bMultiState = bMultiState ;
	}

protected:
	Bitmap			*m_pBitmap;			// save image.
	UINT			   m_nMask;			// save mask
	BOOL			   m_bDisable;			// enable or disable	
	CRect		    	m_rectButton;		// about button's position
	CRect			    m_rectButtonPos;	// about button's position
	BOOL			    m_bCursorOnButton; 	// cursor is on button or not.
	BOOL			    m_bHover;			// determine button state is hover or not.
	CToolTipCtrl	m_ToolTip;			// for tooltip message..	
	BOOL			    m_bSetCursor;
	HCURSOR		m_hCursor;			// set cursor	
	BOOL             m_bHandOnMove ;     //track whether mouse becomes a hand when moving on it.

	String::_tstring        m_tsText;

	COLORREF         m_colorText ;
	BOOL            m_bMultiState ;

	
// Construction
public:
	CUIButton();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKbcButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUIButton();
	virtual BOOL DestroyWindow();
	void ReleaseResource();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKbcButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy( );
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};