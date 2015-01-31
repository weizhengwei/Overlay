#pragma once


#include "skin/UIButton.h"
// CUICheck

class CUICheck : public CButton
{
	DECLARE_DYNAMIC(CUICheck)

public:
	CUICheck();
	virtual ~CUICheck();

	enum BUTTON_STATE
	{
		BUTTON_ENABLE = 0,
		BUTTON_HOVER,
		BUTTON_CLICK,
		BUTTON_DISABLE
	};


	enum 
	{		
		STATE_UNCHECKED=0,
		STATE_CHECKED,
		STATE_MAX
	};

	void   SetCheck(int nCheck );
	virtual void SetCheck(  );
	virtual int    GetCheck( );
	void			SetCursor(HCURSOR hCursor);// set cursor

	void			SetButtonImage(LPCTSTR lpszFileName,UINT nMakse);
	void SetTextColor(COLORREF clr = 0x616161) 
	{
		m_clrText = clr ;
	};

	void SetButtonText( LPCTSTR lpStr)
	{
		m_tsText=lpStr;
	};
	_tstring GetButtonText()
	{
		return m_tsText ;
	};
protected:
	_tstring   m_tsFile[STATE_MAX];
	int           m_iState;

	String::_tstring  m_tsText;
	COLORREF m_clrText ;

	Bitmap			*m_pBitmap;			// save image.
	UINT			   m_nMask;			// save mask
	CRect		    	m_rectButton;		// about button's position
	CRect			    m_rectButtonPos;	// about button's position
	BOOL			    m_bCursorOnButton; 	// cursor is on button or not.
	BOOL			   m_bHover;			// determine button state is hover or not.
	CToolTipCtrl	m_ToolTip;			// for tooltip message..	
	BOOL			     m_bSetCursor;
	HCURSOR			m_hCursor;			// set cursor	


protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	//afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

	
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PreSubclassWindow();

};


