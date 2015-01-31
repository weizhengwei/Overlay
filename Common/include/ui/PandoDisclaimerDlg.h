#pragma once
#include "resource.h"
#include "skin/UIHMXStatic.h"
#include "skin/HyperLink.h"
#include "skin/UIButton.h"
#include "skin/gdipbutton.h"
#include "skin/UIRichEditCtrl.h"

// CPandoDisclaimerDlg dialog

class CPandoDisclaimerDlg : public CDialog
{
	DECLARE_DYNAMIC(CPandoDisclaimerDlg)

public:
	CPandoDisclaimerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPandoDisclaimerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DISCLAIMER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
private:
	CHyperLink m_stcTerms;
	CUIHMXStatic m_stcTerms1;
	CUIHMXStatic m_stcterms2;
	CUIRichEditCtrl m_reAgreement;
	CGdipButton m_btnExit;
	CGdipButton m_btnMin;
	CUIButton m_btnCancel;
	CUIButton m_btnAgree;
private:
	Bitmap       *m_pBgBitmap;
	CBitmap     m_bitmapScroll;
	CRect      m_rectWnd;
	std::vector<THEMES_INFO>  m_vecThemeInfo;
	std::vector<THEMES_SKIN_FILE> m_vecSkinFiles;
	CFont ftBtn;
	CFont stcft;
	CBrush      m_brInner;
	_tstring    m_szLoop;
	HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedDisclaimaerMin();
	afx_msg void OnBnClickedDisclaimerExit();
	afx_msg void OnBnClickedDisclaimerCancel();
	afx_msg void OnBnClickedDisclaimerAgree();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnLinkDisclaimerAgreement(NMHDR *pNMHDR, LRESULT *pResult);
public:
	void InitMyDlg();
	static DWORD WINAPI FadeThreadProc(PVOID lpParam) ;
	

};
