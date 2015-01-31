#pragma once

#include "stdafx.h"
#include <vector>
#include "skin/UIButton.h"
#include "data/uiconfig.h"
#include "skin/GdipButton.h"

// CUIMessageBoxYesNo dialog

class CUIMessageBoxYesNo : public CDialog
{
	DECLARE_DYNAMIC(CUIMessageBoxYesNo)

public:
	CUIMessageBoxYesNo(CWnd* pParent = NULL);   // standard constructor
	CUIMessageBoxYesNo(CWnd* pParent, _tstring sTitle, _tstring sText, _tstring sYes, _tstring sNo) ;
	CUIMessageBoxYesNo(CWnd* pParent, _tstring sTitle, _tstring sText, _tstring sYes, _tstring sNo,DWORD dwYesResId,DWORD dwNoResId) ;
	virtual ~CUIMessageBoxYesNo();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOGOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	virtual HBRUSH    OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnPaint( );
	afx_msg BOOL      OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT   OnNcHitTest(CPoint pt);	

	static DWORD WINAPI FadeThreadProc(PVOID lpParam) ;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDo();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonExit();

private:
	CFont       m_ftTitle;
	CFont       m_ftText;
	CFont       m_ftButton;
	CBrush      m_brush;
	CRect       m_rectWnd;
	CRgn        m_rectRgn;

	std::vector<THEMES_INFO>  m_vecThemeInfo;
	std::vector<THEMES_SKIN_FILE> m_vecSkinFiles;

	Bitmap         *m_pBgBitmap;

	CUIButton  m_btnInstall ;
	CUIButton  m_btnCancel ;
	CGdipButton  m_btnExit ;

	_tstring m_sTitle ;//title displayed on message box
	_tstring m_sText ;//text displayed on message box
	_tstring m_sYes ;//text on Confirm button
	_tstring m_sNo ; //text on Cancel button
	DWORD    m_dwYesResId;//resource id on Confirm button.
	DWORD    m_dwNoResId; //resource id on Cancel button.

public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
