// UIMessageBox.cpp : implementation file
//

#include "stdafx.h"
#include "ui/UIMessageBox.h"
#include "data/Utility.h"
#include "data/DataPool.h"
#include "data/UIString.h"
#include "skin/MemDCGdiplus.h"

// CUIMessageBox dialog

IMPLEMENT_DYNAMIC(CUIMessageBox, CDialog)

CUIMessageBox::CUIMessageBox(CWnd* pParent /*=NULL*/)
	: CDialog(CUIMessageBox::IDD, pParent)
{

}

CUIMessageBox::CUIMessageBox(CWnd* pParent, LPCTSTR lpszText, LPCTSTR lpszCaption)
	: CDialog(CUIMessageBox::IDD, pParent)
{
	m_sText = lpszText ;
	m_sCaption = lpszCaption ;

	std::vector<THEMES_INFO>  vecThemeInfo=theUIConfig.ExportThemesInfo( );
	std::vector<THEMES_SKIN_FILE> vecSkinFiles=theUIConfig.ExportThemeSkin( );
	
	vecSkinFiles[CL_BK_ERROR]._tsPath = theDataPool.GetBaseDir( ) + vecThemeInfo[0]._tsFolder + vecSkinFiles[CL_BK_ERROR]._tsPath;
	CUtility::CreateBitmap(vecSkinFiles[CL_BK_ERROR], &m_pbpBitmap) ;
}


CUIMessageBox::~CUIMessageBox()
{
	if (m_pbpBitmap)
	{
		CUtility::DeleteBitmap(m_pbpBitmap) ;
	}
}

void CUIMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose) ;
	DDX_Control(pDX, IDC_BUTTON_MIN, m_btnMinimize) ;

	DDX_Control(pDX, IDC_STATIC_ERROR, m_stcCaption) ;
}


BEGIN_MESSAGE_MAP(CUIMessageBox, CDialog)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CUIMessageBox::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CUIMessageBox::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_MIN, &CUIMessageBox::OnBnClickedButtonMin)
END_MESSAGE_MAP()


// CUIMessageBox message handlers

LRESULT CUIMessageBox::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect(&rc);		

    CRect rct(0,0,rc.right,28); // rearrange window only in this area
	CPoint pt=point;
	ScreenToClient(&pt);
	if ( rct.PtInRect(pt))
	{
		return HTCAPTION;
	}

    return CDialog::OnNcHitTest(point);
}

BOOL CUIMessageBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rect ;
	GetClientRect(&rect) ;

	std::vector<THEMES_INFO>  vecThemeInfo=theUIConfig.ExportThemesInfo( );
	std::vector<THEMES_SKIN_FILE> vecSkinFiles=theUIConfig.ExportThemeSkin( );

	m_ftButton.Detach( );
	m_ftButton.CreateFont(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));
	
	_tstring tsPath=theDataPool.GetBaseDir( ) + vecThemeInfo[0]._tsFolder + vecSkinFiles[CL_BTN_MAIN_EXIT]._tsPath;
	m_btnClose.LoadImage(tsPath.c_str()) ;
	m_btnClose.SetToolTipText(theUIString._GetStringFromId(_T("IDS_POPDLG_ERROR_EXIT")).c_str()) ;

	tsPath=theDataPool.GetBaseDir( ) + vecThemeInfo[0]._tsFolder + vecSkinFiles[CL_BTN_MAIN_MIN]._tsPath;
	m_btnMinimize.SetButtonImage(tsPath.c_str(), RGB(255,0,255)) ;
	m_btnMinimize.SetToolTipText(theUIString._GetStringFromId(_T("IDS_POPDLG_ERROR_MIN")).c_str()) ;

	tsPath=theDataPool.GetBaseDir( ) + vecThemeInfo[0]._tsFolder + vecSkinFiles[CL_BTN_LOGOUT_CANCEL]._tsPath;
	m_btnCancel.SetFont(&m_ftButton) ;
	m_btnCancel.SetButtonText(theUIString._GetStringFromId(_T("IDS_POPDLG_ERROR_CLOSE")).c_str()) ;
	m_btnCancel.SetButtonImage(tsPath.c_str(), RGB(255,0,255)) ;
	m_btnCancel.MoveWindow(rect.Width()/2-36, rect.Height()-40, 79, 25) ;

	m_brush.Detach();
	m_brush.CreateSolidBrush(RGB(51, 51, 51));

	m_stcCaption.SetTextClr(0xacacac) ;
	m_stcCaption.SetFontBold() ;
	m_stcCaption.SetFontFaceName(_T("Tahoma")) ;
	m_stcCaption.SetFontHeight(-12) ;
	m_stcCaption.SetWindowText(m_sCaption.c_str()) ;

	CRgn rectRgn ;
	rectRgn.CreateRectRgn(rect.left+4, rect.top+4, rect.right-4, rect.bottom-4);
	SetWindowRgn(rectRgn,true);
	rectRgn.DeleteObject() ;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CUIMessageBox::OnPaint()
{
	CPaintDC	dc(this);
	Graphics	gDc(dc.m_hDC);
	CRect		rect;

	GetClientRect(&rect);
	if (m_pbpBitmap != NULL)
	{
		gDc.DrawImage(m_pbpBitmap,0, 0, rect.Width(), rect.Height());
	}

	// draw text.
	RectF targetBox(15, 32, 370, 115), boundBox;
	SolidBrush brush(Color(0xff, 0x95, 0x95, 0x95));
	StringFormat fmat;
	fmat.SetAlignment(StringAlignmentCenter);
	Font font(_T("Tahoma"), 13, FontStyleRegular, UnitPixel);
	gDc.MeasureString(m_sText.c_str(), m_sText.length(), &font, targetBox, &fmat, &boundBox);
	// make the text drawed vertically centrally
	targetBox.Y += (targetBox.Height - boundBox.Height) / 2;
	gDc.DrawString(m_sText.c_str(), -1, &font, targetBox, &fmat, &brush);
}

HBRUSH CUIMessageBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	UINT  nID;
	HBRUSH hbr;

	nID=pWnd->GetDlgCtrlID();
	switch(nID)
	{
	case IDC_STATIC_ERROR:
		pDC->SetTextColor(0xacacac);
		pDC->SetBkMode(TRANSPARENT);
		hbr=(HBRUSH)::GetStockObject(NULL_BRUSH);
		break;
	case IDC_BUTTON_CLOSE:
	case IDC_BUTTON_CANCEL:
		hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor) ;
		break ;
	default:
		hbr=m_brush; ///make dilaog frame smoothly.
		break;
	}
	return hbr;
}

void CUIMessageBox::OnBnClickedButtonCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK() ;
}

void CUIMessageBox::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK() ;
}

void CUIMessageBox::OnBnClickedButtonMin()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_MINIMIZE) ;
}

BOOL CUIMessageBox::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	/*
	** get correct background bitmap based
	** on whether item is focused.
	*/
	CMemDCGdiplus pDevC(pDC, rect);
	Color clr(255,255,255) ;
	HBITMAP hBitmap ;
	m_pbpBitmap->GetHBITMAP(clr, &hBitmap) ;

	/*
	** select correct background for item
	** based on whether the item is focused.
	*/
	pDevC->SelectObject(hBitmap);

	/*
	** update new background of button
	*/
	m_btnClose.SetBkGnd(pDevC) ;

	return TRUE ;
}
