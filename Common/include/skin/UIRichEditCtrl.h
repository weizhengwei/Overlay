#pragma once
#include "skin/XScrollBar.h"

// CUIRichEditCtrl

class CUIRichEditCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CUIRichEditCtrl)

public:
	enum 
	{
		SIZE_ICON=0,
		SIZE_WINDOW,
		SIZE_NORMAIL
	};

	enum MyColor { Black,  White, Maroon,  Green,
		Olive,  Navy,  Purple,  Teal,
		Silver, Gray,  Red,     Lime,
		Yellow, Blue,  Fuschia, Aqua ,Warning};

	CUIRichEditCtrl();
	virtual ~CUIRichEditCtrl();

	void InsertBitmap(LPCTSTR lpszFilePath,UINT uSize);
	void AddText(LPCTSTR lpszStr,CHARFORMAT2& charFormat);
	void AddText(LPCTSTR lpszStr );
	void AddText(LPCTSTR lpszStr,int nscrollPos);
	void AddLine(MyColor tc, MyColor bc, LPCTSTR lpszLine,bool bAutoFill=false);

//	void DeleteString( );
	void ResetContent( );
//	void UpdateCtrl() ;

protected:
	CBrush    m_brWnd;
//	CXScrollBar   *m_pSbsBar;
	CRect          m_rectWnd;

protected:
	DECLARE_MESSAGE_MAP()
	virtual  void OnRButtonDown(UINT nFlags, CPoint point );
	virtual  void OnLButtonDown(UINT nFlags,CPoint point );
	afx_msg void OnPaint( );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnUndo(  );
	afx_msg void OnCutString( );
	afx_msg void OnCopyString( );
	afx_msg void OnPasteString( );
	afx_msg void OnClearAll( );
	afx_msg void OnSelectAll( );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
/*	afx_msg void OnUpdate() ;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	*/
};


