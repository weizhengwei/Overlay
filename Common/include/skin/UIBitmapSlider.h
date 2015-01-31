#pragma once
// BitmapSlider.h : header file
//

/////////////////////////////////////////////////////////////////////////////
//
// CUIBitmapSlider v1.5
//
// It's free for everywhere - 16/September/2003 - Joon-ho Ryu
//
/////////////////////////////////////////////////////////////////////////////

#include "memdc.h" // "Flicker Free Drawing In MFC" by Keith Rule

class CUIBitmapSlider : public CStatic
{
// Construction
public:
	CUIBitmapSlider();

// Implementation
public:

	void GetRange( int &nMin, int &nMax ) { nMin = m_nMin; nMax = m_nMax; };
	int GetRangeMax() { return m_nMax; };
	int GetRangeMin() { return m_nMin; };

	int GetPos() { return m_nPos; };

	void SetRange(int nMin, int nMax, BOOL bRedraw=FALSE);
	void SetRangeMin(int nMin, BOOL bRedraw = FALSE);
	void SetRangeMax(int nMax, BOOL bRedraw = FALSE);

	void SetPos(int nPos);

	BOOL SetBitmapChannel(LPCTSTR lpszChannelFilePath, LPCTSTR lpszActiveFilePath = NULL);
	///To enable tooltip, we should first relay event in pretranslatemessage routine, then create a tooltip
	///in the end, create the control with SS_NOTIFY style.
	BOOL SetToolTipText(LPCTSTR lpszText, BOOL bActivate = TRUE); 
	BOOL ActivateToolTip(BOOL bEnable = TRUE);

	virtual ~CUIBitmapSlider();

protected :
	int m_nMax, m_nMin, m_nPos;

	Bitmap* m_pbmChannel, *m_pbmActive ;
	CToolTipCtrl m_ttTooltip ;

	//{{AFX_MSG(CUIBitmapSlider)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG

	BOOL InitToolTip();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
