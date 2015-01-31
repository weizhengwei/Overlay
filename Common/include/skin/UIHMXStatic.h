/** 
 * @file      UIHMXStatic.h
   @brief     The header file for customized static control.
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

#pragma once

#include "stdafx.h"
#include "tstring/tstring.h"
#include "data/UIConfig.h"

class CUIHMXStatic : public CStatic
{
// Construction
public:
	CUIHMXStatic();

	virtual BOOL PreTranslateMessage(MSG* pMsg);


public:
	bool SetToolTipText(const CString& sText, bool bActivate = true);
	bool ActivateToolTip(bool bEnable = true);
	bool GetTransparent();
	bool SetTransparent( bool bTransparent = true);
	bool SetFontRotation( int nAngle );
	bool SetFontBold(bool bBold = true);
	bool SetFontItalic( bool bItalic = true);
	bool SetFontHeight( int nHeight );
	bool SetFontFaceName( const CString& sFaceName );
	bool GetTextFont( LOGFONT* plgfnt);
	bool SetTextFont( LONG nHeight, bool bBold, bool bItalic, const CString& sFaceName );
	bool SetTextFont( const LOGFONT& lgfnt );
	bool GetTextClr(COLORREF &clr);
	bool SetTextClr( COLORREF clr );
	bool GetBkClr(COLORREF &clr);
	bool SetBkClr( COLORREF clr );
	virtual ~CUIHMXStatic();
	void EnableHyperLink(BOOL bEnable, LPCTSTR lpszLink, LPCTSTR lpszCmd = NULL) ;
	void EnableHyperLinkCallback(BOOL bEnable, HWND hNotifyWnd, LPCTSTR lpszCmd = NULL) ;
	bool SetWindowText(LPCTSTR lpszString) ;
	void SetExternalShell( bool bUse)
	{
		m_bUseShell=bUse;

	}

protected:
	bool InitToolTip();
	CToolTipCtrl m_tt;
	CFont m_fntText;
	COLORREF m_clrTextClr;
	COLORREF m_clrBkClr;
	CBrush m_brsBkGnd;
	CBrush m_brsHollow;
	bool m_bTransparent;

	bool m_bHyperLink ;
	CString m_sLink ;
	CString m_sCmd ;
	bool  m_bUseShell;

	HWND m_hNotifyWnd ; //notify window

	// Generated message map functions
protected:
	//{{AFX_MSG(CUIHMXStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText(WPARAM,LPARAM);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	//afx_msg void OnPaint() ;
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


//////////////////////////////////////////////////////////
class CUIHMXStaticForDash : public CUIHMXStatic
{
public:
	CUIHMXStaticForDash() 
	{
		m_bDash = false ;
		m_pItemBk = NULL ; 
	};

	void SetStaticDash(bool bDash, LPCTSTR lpszItemBkFile = NULL) 
	{ 
		m_bDash = bDash ;

		if (lpszItemBkFile != NULL)
		{
			CreateBitmap(lpszItemBkFile, &m_pItemBk) ;
		}
	} ;

	~CUIHMXStaticForDash() 
	{
		if (m_pItemBk)
		{
			DeleteBitmap(m_pItemBk) ;
		}
	} ;

private:
	bool m_bDash ;
	Bitmap* m_pItemBk ;

	BOOL CreateBitmap(LPCTSTR lpszFile, Bitmap** ppbpParam) ;
	BOOL DeleteBitmap(Bitmap* pbpParam) ; //destroy Bitmap object

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};
