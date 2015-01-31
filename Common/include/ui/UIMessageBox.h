#pragma once

#include "skin/UIHMXStatic.h"
#include "skin/UIButton.h"
#include "skin/gdipbutton.h"

// CUIMessageBox dialog

class CUIMessageBox : public CDialog
{
	DECLARE_DYNAMIC(CUIMessageBox)

public:
	CUIMessageBox(CWnd* pParent = NULL);   // standard constructor
	CUIMessageBox(CWnd* pParent, LPCTSTR lpszText, LPCTSTR lpszCaption) ;
	virtual ~CUIMessageBox();

// Dialog Data
	enum { IDD = IDD_DIALOG_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	_tstring m_sText ;
	_tstring m_sCaption ;

	CUIButton m_btnCancel ;
	CGdipButton m_btnClose ;
	CUIButton m_btnMinimize ;

	CUIHMXStatic m_stcCaption ;
	Bitmap* m_pbpBitmap ;

	CBrush m_brush ;
	CFont m_ftButton ;

	//static DWORD WINAPI FadeThreadProc(PVOID lpParam) ;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonMin();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
