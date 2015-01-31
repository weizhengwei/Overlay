// PandoDisclaimerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ui/PandoDisclaimerDlg.h"
#include "data/UIString.h"
#include "data/UIConfig.h"
#include "data/DataPool.h"
#include "skin/skinscrollwnd.h"

// CPandoDisclaimerDlg dialog

IMPLEMENT_DYNAMIC(CPandoDisclaimerDlg, CDialog)

CPandoDisclaimerDlg::CPandoDisclaimerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPandoDisclaimerDlg::IDD, pParent)
{
	m_pBgBitmap = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CPandoDisclaimerDlg::~CPandoDisclaimerDlg()
{
	if (m_pBgBitmap != NULL)
	{
		delete m_pBgBitmap;
		m_pBgBitmap = NULL;
	}
}
HCURSOR CPandoDisclaimerDlg::OnQueryDragIcon()
{// The system calls this function to obtain the cursor to display while the user drags
	// the minimized window.
	return static_cast<HCURSOR>(m_hIcon);
}
void CPandoDisclaimerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISCLAIMER_TERMS, m_stcTerms);
	DDX_Control(pDX, IDC_DISCLAIMER_AGREEMENT, m_reAgreement);
	DDX_Control(pDX, IDC_DISCLAIMER_EXIT, m_btnExit);
	DDX_Control(pDX, IDC_DISCLAIMAER_MIN, m_btnMin);
	DDX_Control(pDX, IDC_DISCLAIMER_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DISCLAIMER_AGREE, m_btnAgree);
	DDX_Control(pDX, IDC_DISCLAIMER_TERMS1, m_stcTerms1);
	DDX_Control(pDX, IDC_DISCLAIMER_TERMS2, m_stcterms2);
}


BEGIN_MESSAGE_MAP(CPandoDisclaimerDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_DISCLAIMAER_MIN, &CPandoDisclaimerDlg::OnBnClickedDisclaimaerMin)
	ON_BN_CLICKED(IDC_DISCLAIMER_EXIT, &CPandoDisclaimerDlg::OnBnClickedDisclaimerExit)
	ON_BN_CLICKED(IDC_DISCLAIMER_CANCEL, &CPandoDisclaimerDlg::OnBnClickedDisclaimerCancel)
	ON_BN_CLICKED(IDC_DISCLAIMER_AGREE, &CPandoDisclaimerDlg::OnBnClickedDisclaimerAgree)
	ON_NOTIFY(EN_LINK, IDC_DISCLAIMER_AGREEMENT, &CPandoDisclaimerDlg::OnEnLinkDisclaimerAgreement)
END_MESSAGE_MAP()


// CPandoDisclaimerDlg message handlers
void CPandoDisclaimerDlg::OnBnClickedDisclaimaerMin()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_MINIMIZE);
}

void CPandoDisclaimerDlg::OnBnClickedDisclaimerExit()
{
	// TODO: Add your control notification handler code here
	UnskinWndScroll(&m_reAgreement);
	EndDialog(IDCANCEL) ;
}

void CPandoDisclaimerDlg::OnBnClickedDisclaimerCancel()
{
	// TODO: Add your control notification handler code here
	UnskinWndScroll(&m_reAgreement);
	EndDialog(IDCANCEL) ;
}

void CPandoDisclaimerDlg::OnBnClickedDisclaimerAgree()
{
	// TODO: Add your control notification handler code here
	EndDialog(IDOK) ;
}
BOOL CPandoDisclaimerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);//Set big icon
	SetIcon(m_hIcon, FALSE);//Set small icon

    SetWindowText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_TITLE")).c_str()) ;
	ModifyStyle(0,WS_MINIMIZEBOX|WS_SYSMENU);
	ModifyStyleEx(0, WS_EX_APPWINDOW);	
	m_brInner.Detach();
	m_brInner.CreateSolidBrush(RGB(0x25, 0x25, 0x25));	
	// TODO:  Add extra initialization here

	m_vecThemeInfo=theUIConfig.ExportThemesInfo( );
	m_vecSkinFiles=theUIConfig.ExportThemeSkin( );
	///read background bitmap
	_tstring tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder+ m_vecSkinFiles[CL_BK_PANDODISCLAIMER]._tsPath;
	m_pBgBitmap=Bitmap::FromFile(tsPath.toWideString().c_str());
	if(!m_pBgBitmap)
	{
		return false;
	}

	///Set Window Rect
	GetWindowRect( &m_rectWnd);
	if (this->GetParent() == NULL)
	{
		int     nFullWidth=GetSystemMetrics(SM_CXSCREEN);    
		int     nFullHeight=GetSystemMetrics(SM_CYSCREEN);  
		m_rectWnd.MoveToXY((nFullWidth-m_pBgBitmap->GetWidth( ))/2,(nFullHeight-m_pBgBitmap->GetHeight( ))/2);
	}
	else
	{
		CRect rcParent;
		this->GetParent()->GetWindowRect(&rcParent);
		m_rectWnd.MoveToXY(rcParent.left+(rcParent.Width()-m_pBgBitmap->GetWidth( ))/2,rcParent.top+(rcParent.Height()-m_pBgBitmap->GetHeight( ))/2);
	}
	m_rectWnd.right=m_rectWnd.left + m_pBgBitmap->GetWidth( );
	m_rectWnd.bottom=m_rectWnd.top+m_pBgBitmap->GetHeight( );

	MoveWindow(m_rectWnd,1);

	CRect rc = m_rectWnd;
	ScreenToClient(&rc);
	CRgn rectRgn ;
	rectRgn.CreateRectRgn(rc.left+3, rc.top+3, rc.right-3, rc.bottom-3);
	SetWindowRgn(rectRgn,true);
	SetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE,GetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE)^0x80000);
    InitMyDlg();
	ShowWindow(SW_SHOW);
	UpdateWindow();
	BringWindowToTop();
	return TRUE;  // return TRUE unless you set the focus to a control
}
BOOL CPandoDisclaimerDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

HBRUSH CPandoDisclaimerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	UINT  nID;
	HBRUSH hbr;

	nID=pWnd->GetDlgCtrlID();
	switch(nID)
	{
	case IDC_DISCLAIMER_AGREEMENT:
		pDC->SetTextColor(0X959595);
		pDC->SetBkColor(RGB(0x25,0x25,0x25));
		hbr=m_brInner;
		break;
	case IDC_DISCLAIMER_TERMS1:
	case IDC_DISCLAIMER_TERMS:
	case IDC_DISCLAIMER_TERMS2:
		pDC->SetTextColor(0x959595);
		pDC->SetBkMode(TRANSPARENT);
		hbr=(HBRUSH)::GetStockObject(NULL_BRUSH);
		break;
	default:
		hbr=CDialog::OnCtlColor(pDC, pWnd, nCtlColor);;
		break;
	}
	return hbr;
}
void CPandoDisclaimerDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// TODO: Add your message handler code here
		CRect        rect;
		GetClientRect(&rect);
		Graphics     gDc(dc.m_hDC);
		if(m_pBgBitmap)
		{
			gDc.DrawImage(m_pBgBitmap,rect.left,rect.top,rect.right,rect.bottom);
		}
		
		CRect        rectInner;
		rectInner.SetRect(30,119,479,237);
		dc.Draw3dRect(rectInner,RGB(88, 88, 88), RGB(88, 88,88));

		// draw text.

		RectF rt(31.0f, 92.0f,rect.Width()-3,20);
		SolidBrush brush(Color(0xff,0x95,0x95,0x95));
		StringFormat fmat;
		fmat.SetAlignment(StringAlignmentNear);
		Font font(_T("Tahoma"),12,FontStyleRegular,UnitPixel);
		_tstring stext = theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_PROMPT"));
		gDc.DrawString(stext.toWideString().c_str(),-1,&font,rt,&fmat,&brush);
	}

}
LRESULT CPandoDisclaimerDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

void CPandoDisclaimerDlg::InitMyDlg()
{
	_tstring tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder+ m_vecSkinFiles[CL_BTN_PANDO_CANCEL]._tsPath;
	ftBtn.Detach( );
	ftBtn.CreateFont(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));

	m_btnCancel.SetFont( &ftBtn);
	m_btnCancel.SetButtonImage( tsPath.c_str( ),RGB(0,0,0));
	m_btnCancel.SetButtonText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_CANCEL")).c_str());
	m_btnCancel.MoveWindow(m_vecSkinFiles[CL_BTN_PANDO_CANCEL]._rect.left,m_vecSkinFiles[CL_BTN_PANDO_CANCEL]._rect.top,m_vecSkinFiles[CL_BTN_PANDO_CANCEL]._rect.right,m_vecSkinFiles[CL_BTN_PANDO_CANCEL]._rect.bottom);

	tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder+ m_vecSkinFiles[CL_BTN_PANDO_AGREE]._tsPath;
	m_btnAgree.SetFont( &ftBtn);
	m_btnAgree.SetButtonImage( tsPath.c_str( ),RGB(255,0,255));
	m_btnAgree.SetButtonText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_AGREE")).c_str());
	m_btnAgree.MoveWindow(m_vecSkinFiles[CL_BTN_PANDO_AGREE]._rect.left,m_vecSkinFiles[CL_BTN_PANDO_AGREE]._rect.top,m_vecSkinFiles[CL_BTN_PANDO_AGREE]._rect.right,m_vecSkinFiles[CL_BTN_PANDO_AGREE]._rect.bottom);
	//DeleteObject(ftBtn);

	tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[CL_BTN_PANDO_EXIT]._tsPath; 
	m_btnExit.LoadImage(tsPath.c_str()) ;
	m_btnExit.MoveWindow(m_vecSkinFiles[CL_BTN_PANDO_EXIT]._rect.left,m_vecSkinFiles[CL_BTN_PANDO_EXIT]._rect.top,m_vecSkinFiles[CL_BTN_PANDO_EXIT]._rect.right,m_vecSkinFiles[CL_BTN_PANDO_EXIT]._rect.bottom);
	m_btnExit.SetToolTipText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_EXIT")).c_str());

	// Fix Bug 0148931, remove the MIN button
	m_btnMin.ShowWindow(SW_HIDE);
	/*tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[CL_BTN_PANDO_MIN]._tsPath; 
	m_btnMin.LoadImage(tsPath.c_str()) ;
	m_btnMin.MoveWindow(m_vecSkinFiles[CL_BTN_PANDO_MIN]._rect.left,m_vecSkinFiles[CL_BTN_PANDO_MIN]._rect.top,m_vecSkinFiles[CL_BTN_PANDO_MIN]._rect.right,m_vecSkinFiles[CL_BTN_PANDO_MIN]._rect.bottom);
	m_btnMin.SetToolTipText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_MIN")).c_str());*/

	stcft.Detach( );
	stcft.CreateFont(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));
	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&stcft);

	_tstring tsStr = theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_TERMS1")) + _T(" ");
	GetDlgItem(IDC_DISCLAIMER_TERMS1)->SetFont(&stcft);
	GetDlgItem( IDC_DISCLAIMER_TERMS1)->SetWindowText( tsStr.c_str( ) );
    SIZE sizeTerms1 = pDC->GetTextExtent(tsStr.c_str());

	tsStr = theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_TERMS")) + _T(" ");
	GetDlgItem(IDC_DISCLAIMER_TERMS)->SetFont(&stcft);
	GetDlgItem( IDC_DISCLAIMER_TERMS)->SetWindowText( tsStr.c_str( ) );
	m_stcTerms.SetColours(0x959595,0x959595,0x959595);
    SIZE sizeTerms = pDC->GetTextExtent(tsStr.c_str());
	tsStr = theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_TERMS_URL"));
    m_stcTerms.SetURL(tsStr.c_str());
	tsStr = theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_TERMS2"));
	GetDlgItem(IDC_DISCLAIMER_TERMS2)->SetFont(&stcft);
	GetDlgItem( IDC_DISCLAIMER_TERMS2)->SetWindowText( tsStr.c_str( ) );
	SIZE sizeTerms2 = pDC->GetTextExtent(tsStr.c_str());

	GetDlgItem(IDC_DISCLAIMER_TERMS1)->MoveWindow(m_vecSkinFiles[CL_STC_PANDO_COMMENT_PREFIX]._rect.left,m_vecSkinFiles[CL_STC_PANDO_COMMENT_PREFIX]._rect.top,m_vecSkinFiles[CL_STC_PANDO_COMMENT_PREFIX]._rect.right,m_vecSkinFiles[CL_STC_PANDO_COMMENT_PREFIX]._rect.bottom,0);
	GetDlgItem(IDC_DISCLAIMER_TERMS)->MoveWindow(m_vecSkinFiles[CL_STC_PANDO_COMMENT_URL]._rect.left,m_vecSkinFiles[CL_STC_PANDO_COMMENT_URL]._rect.top,m_vecSkinFiles[CL_STC_PANDO_COMMENT_URL]._rect.right,m_vecSkinFiles[CL_STC_PANDO_COMMENT_URL]._rect.bottom,0);
	GetDlgItem(IDC_DISCLAIMER_TERMS2)->MoveWindow(m_vecSkinFiles[CL_STC_PANDO_COMMENT_SUBFIX]._rect.left,m_vecSkinFiles[CL_STC_PANDO_COMMENT_SUBFIX]._rect.top,m_vecSkinFiles[CL_STC_PANDO_COMMENT_SUBFIX]._rect.right,m_vecSkinFiles[CL_STC_PANDO_COMMENT_SUBFIX]._rect.bottom,0);	
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);
   

	CHARFORMAT2 chFormat;
	ZeroMemory(&chFormat,sizeof(CHARFORMAT2));
	chFormat.cbSize = sizeof(CHARFORMAT2);
	chFormat.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE |CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE;
	chFormat.dwEffects = 0;
	chFormat.yHeight = 14*14;
	chFormat.crTextColor = RGB(0x95,0x95,0x95) ; 
	_tcscpy(chFormat.szFaceName ,_T("Tahoma"));

	m_reAgreement.SetDefaultCharFormat( chFormat );
	m_reAgreement.SetBackgroundColor(false,RGB(0x25,0x25,0x25));
	
	m_reAgreement.MoveWindow( 31,120,447,116,false);
	m_bitmapScroll.LoadBitmap(IDB_BITMAP_SCROLLBAR);
	SkinWndScroll(&m_reAgreement,(HBITMAP)m_bitmapScroll);
	
	DWORD Mask = m_reAgreement.GetEventMask();
	Mask = Mask|ENM_LINK|ENM_MOUSEEVENTS|ENM_SCROLLEVENTS|ENM_KEYEVENTS;
	m_reAgreement.SetEventMask(Mask); 
	::SendMessage(m_reAgreement.m_hWnd, EM_AUTOURLDETECT, true, 0);

	/*
	** display unicode text in richedit20A control.
	*/
	SETTEXTEX st;
    st.codepage=1200;
    st.flags=GT_DEFAULT;
	m_reAgreement.SendMessage(EM_SETTEXTEX,(WPARAM)&st,(LPARAM)theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_AGREEMENT")).c_str());
	//m_reAgreement.AddText(theUIString._GetStringFromId(_T("IDS_POPDLG_PANDO_AGREEMENT")).c_str() ,SB_TOP);
}
void CPandoDisclaimerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
    UnskinWndScroll(&m_reAgreement);
	CDialog::OnClose();
}
DWORD CPandoDisclaimerDlg::FadeThreadProc(PVOID lpParam)
{
	// TODO: Add your message handler code here and/or call default
	CPandoDisclaimerDlg* pItem = (CPandoDisclaimerDlg*)lpParam ;

	int i = 1 ;
	while(TRUE)	
	{
		if (i <= 255)
		{
			pItem->SetLayeredWindowAttributes(0,i,LWA_ALPHA);
		}
		else
		{
			pItem->SetLayeredWindowAttributes(0,255,LWA_ALPHA);
			break ;
		}
		i = i + 10;
		Sleep(15) ;
	}
	return 0 ;
}
void CPandoDisclaimerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow)
	{
		CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(CPandoDisclaimerDlg::FadeThreadProc), this, 0, NULL));
	}
}

void CPandoDisclaimerDlg::OnEnLinkDisclaimerAgreement(NMHDR *pNMHDR, LRESULT *pResult)
{
	ENLINK *pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	switch( pEnLink->msg )
	{
	case WM_LBUTTONDOWN:
		{
			CString strURL ;
			CHARRANGE crCharRange ;
			CRichEditCtrl * pTempEdit;
			pTempEdit = (CRichEditCtrl*)CRichEditCtrl::FromHandle(pEnLink->nmhdr.hwndFrom);
			pTempEdit->GetSel( crCharRange ) ;
			pTempEdit->SetSel( pEnLink->chrg ) ;
			strURL = pTempEdit->GetSelText() ;
			pTempEdit->SetSel( crCharRange ) ;
			CWaitCursor WaitCursor ;
			ShellExecute( this->GetSafeHwnd(), _T( "open" ),strURL, NULL, NULL, SW_SHOWNORMAL ) ;
			*pResult = 1 ;
		}
		break ;

	case WM_LBUTTONUP:
		{
			*pResult = 1 ;
		}
		break ;
	default:
		break ;
	}
}
