// UIMessageBoxYesNo.cpp : implementation file
//

#include "ui/UIMessageBoxYesNo.h"
#include "data/Utility.h"
#include "data/DataPool.h"
#include "data/UIString.h"
//#include "ui/PWPClient.h"

// CUIMessageBoxYesNo dialog

IMPLEMENT_DYNAMIC(CUIMessageBoxYesNo, CDialog)

CUIMessageBoxYesNo::CUIMessageBoxYesNo(CWnd* pParent /*=NULL*/)
	: CDialog(CUIMessageBoxYesNo::IDD, pParent)
{
	/*
	** default text is for Changing language.
	*/
	m_sTitle = theUIString._GetStringFromId(_T("IDS_POPDLG_LANG_TITLE")) ;
	m_sText  = theUIString._GetStringFromId(_T("IDS_POPDLG_LANG_TEXT")) ;
	m_sYes   = theUIString._GetStringFromId(_T("IDS_POPDLG_LANG_RESTART")) ;
	m_sNo    = theUIString._GetStringFromId(_T("IDS_POPDLG_LANG_CANCEL")) ;
	m_dwYesResId = CL_BTN_RESTART_DO;
	m_dwNoResId  = CL_BTN_RESTART_CANCEL;
}

CUIMessageBoxYesNo::CUIMessageBoxYesNo(CWnd* pParent, _tstring sTitle, _tstring sText, _tstring sYes, _tstring sNo)
	: CDialog(CUIMessageBoxYesNo::IDD, pParent)
{
	m_sTitle = sTitle ;
	m_sText  = sText ;
	m_sYes   = sYes ;
	m_sNo    = sNo ;
	m_dwYesResId = CL_BTN_RESTART_DO;
	m_dwNoResId  = CL_BTN_RESTART_CANCEL;
}
CUIMessageBoxYesNo::CUIMessageBoxYesNo(CWnd* pParent, _tstring sTitle, _tstring sText, _tstring sYes, _tstring sNo,DWORD dwYesResId,DWORD dwNoResId)
    : CDialog(CUIMessageBoxYesNo::IDD, pParent)
{
	m_sTitle = sTitle ;
	m_sText  = sText ;
	m_sYes   = sYes ;
	m_sNo    = sNo ;
	m_dwYesResId = dwYesResId;
	m_dwNoResId  = dwNoResId;
}

CUIMessageBoxYesNo::~CUIMessageBoxYesNo()
{
}

void CUIMessageBoxYesNo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_BUTTON_LOGOUT_DO,m_btnInstall);
	DDX_Control(pDX,IDC_BUTTON_LOGOUT_CANCEL,m_btnCancel);
	DDX_Control(pDX,IDC_BUTTON_LOGOUT_EXIT,m_btnExit);
}


BEGIN_MESSAGE_MAP(CUIMessageBoxYesNo, CDialog)
	ON_WM_CTLCOLOR( )
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT( )
	ON_WM_ERASEBKGND( )
	ON_WM_NCHITTEST( )
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT_DO, &CUIMessageBoxYesNo::OnBnClickedButtonDo)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT_CANCEL, &CUIMessageBoxYesNo::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT_EXIT, &CUIMessageBoxYesNo::OnBnClickedButtonExit)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CUIMessageBoxYesNo message handlers
BOOL CUIMessageBoxYesNo::OnInitDialog()
{
	CDialog::OnInitDialog( );

	m_ftButton.Detach( );
	m_ftButton.CreateFont(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));

	m_ftTitle.Detach( );
	m_ftTitle.CreateFont(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));

	m_ftText.Detach( );
	m_ftText.CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE,0,0,0,0,0,0, _T("Verdana"));

	m_brush.Detach();
	m_brush.CreateSolidBrush(RGB(51, 51, 51));	

	GetDlgItem(IDC_STATIC_LOGOUT_TITLE)->SetFont( &m_ftTitle);
	//GetDlgItem(IDC_STATIC_LOGOUT_MSG)->SetFont( &m_ftText);

	GetDlgItem(IDC_STATIC_LOGOUT_TITLE)->MoveWindow(50,9,300,20,false);
	//GetDlgItem(IDC_STATIC_LOGOUT_MSG)->MoveWindow(20,40,360,57,false);

	m_vecThemeInfo=theUIConfig.ExportThemesInfo( );
	m_vecSkinFiles=theUIConfig.ExportThemeSkin( );	

	THEMES_SKIN_FILE tsfParam ;
	tsfParam._tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[CL_BG_LOGOUT]._tsPath;
	CUtility::CreateBitmap(tsfParam, &m_pBgBitmap) ;

	///Set Window Rect;
	if (AfxGetApp()->GetMainWnd() && AfxGetApp()->GetMainWnd()->GetSafeHwnd())
	{
		AfxGetApp()->GetMainWnd( )->GetWindowRect(&m_rectWnd);
		m_rectWnd.left += abs(int(m_rectWnd.Width() - m_pBgBitmap->GetWidth()))/2;
		m_rectWnd.top  += abs(int(m_rectWnd.Height() - m_pBgBitmap->GetHeight()))/2 ;
		m_rectWnd.right=m_rectWnd.left + m_pBgBitmap->GetWidth( );
		m_rectWnd.bottom=m_rectWnd.top + m_pBgBitmap->GetHeight( );
		MoveWindow(m_rectWnd,1);
	}

	_tstring tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[m_dwYesResId]._tsPath; 
	m_btnInstall.SetFont(&m_ftButton) ;
	m_btnInstall.SetButtonText(m_sYes.c_str());
	m_btnInstall.SetButtonImage(tsPath.c_str(),RGB(255,0,255));
	m_btnInstall.MoveWindow(m_vecSkinFiles[m_dwYesResId]._rect.left,m_vecSkinFiles[m_dwYesResId]._rect.top,m_vecSkinFiles[m_dwYesResId]._rect.right,m_vecSkinFiles[m_dwYesResId]._rect.bottom);

	tsPath = theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[m_dwNoResId]._tsPath; 
	m_btnCancel.SetFont(&m_ftButton) ;
	m_btnCancel.SetButtonText(m_sNo.c_str());
	m_btnCancel.SetButtonImage(tsPath.c_str(),RGB(255,0,255));
	m_btnCancel.MoveWindow(m_vecSkinFiles[m_dwNoResId]._rect.left,m_vecSkinFiles[m_dwNoResId]._rect.top,m_vecSkinFiles[m_dwNoResId]._rect.right,m_vecSkinFiles[m_dwNoResId]._rect.bottom);

	tsPath=theDataPool.GetBaseDir( ) + m_vecThemeInfo[0]._tsFolder + m_vecSkinFiles[CL_BTN_MAIN_EXIT]._tsPath;
	m_btnExit.SetToolTipText(theUIString._GetStringFromId(_T("IDS_POPDLG_LANG_EXIT")).c_str());
	//m_btnExit.SetButtonImage(tsPath.c_str(),RGB(255,0,255));
	m_btnExit.LoadImage(tsPath.c_str());
	m_btnExit.MoveWindow(382,8,18,18);

	GetDlgItem(IDC_STATIC_LOGOUT_TITLE)->SetWindowText(m_sTitle.c_str());
	
	//GetDlgItem(IDC_STATIC_LOGOUT_MSG)->SetWindowText(m_sText.c_str());
	GetDlgItem(IDC_STATIC_LOGOUT_MSG)->ShowWindow(SW_HIDE) ;

	CRect rc;
	GetWindowRect(&rc);
	ScreenToClient(&rc);
	CRgn rectRgn ;
	rectRgn.CreateRectRgn(rc.left+4, rc.top+4, rc.right-4, rc.bottom-4);
	SetWindowRgn(rectRgn,true);
	rectRgn.DeleteObject() ;

	SetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE,GetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE)^0x80000);

	return true;
}

void CUIMessageBoxYesNo::OnPaint( )
{
	CPaintDC	dc(this);
	Graphics	gDc(dc.m_hDC);
	CRect		rect;

	GetClientRect(&rect);
	if(m_pBgBitmap)
	{
		gDc.DrawImage(m_pBgBitmap, 0, 0, m_pBgBitmap->GetWidth(), m_pBgBitmap->GetHeight());
	}

	// draw text.
	RectF targetBox(22, 32, 356, 68), boundBox;
	SolidBrush brush(Color(0xff, 0xac, 0xac, 0xac));
	StringFormat fmat;
	fmat.SetAlignment(StringAlignmentCenter);
	Font font(_T("Verdana"), 12, FontStyleRegular, UnitPixel);
	gDc.MeasureString(m_sText.c_str(), m_sText.length(), &font, targetBox, &fmat, &boundBox);
	// make the text drawed vertically centrally
	targetBox.Y += (targetBox.Height - boundBox.Height) / 2;
	gDc.DrawString(m_sText.c_str(), -1, &font, targetBox, &fmat, &brush);

	return;
}

BOOL CUIMessageBoxYesNo::OnEraseBkgnd(CDC* pDC)
{

	return true; 
}

HBRUSH  CUIMessageBoxYesNo::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	UINT  nID;
	HBRUSH hbr;

	nID=pWnd->GetDlgCtrlID();
	switch(nID)
	{
	//case IDC_STATIC_LOGOUT_MSG:
	case IDC_STATIC_LOGOUT_TITLE:
		pDC->SetTextColor(0xacacac);
		pDC->SetBkMode(TRANSPARENT);
		hbr=(HBRUSH)::GetStockObject(NULL_BRUSH);
		break;
	case IDC_BUTTON_LOGOUT_EXIT:
		hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor) ;
		break ;
	default:
		hbr=m_brush;
		break;
	}
	return hbr;
}


LRESULT CUIMessageBoxYesNo::OnNcHitTest(CPoint pt)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(pt) ? HTCAPTION : CDialog::OnNcHitTest(pt);
}

void CUIMessageBoxYesNo::OnBnClickedButtonDo()
{
	EndDialog(IDOK) ;
}

void CUIMessageBoxYesNo::OnBnClickedButtonCancel()
{
	EndDialog(IDCANCEL);
}

void CUIMessageBoxYesNo::OnBnClickedButtonExit()
{
	EndDialog(IDCANCEL);
}

DWORD CUIMessageBoxYesNo::FadeThreadProc(PVOID lpParam)
{
	// TODO: Add your message handler code here and/or call default
	CUIMessageBoxYesNo* pItem = (CUIMessageBoxYesNo*)lpParam ;

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
			pItem->Invalidate() ;
			break ;
		}
		i = i + 10;
		Sleep(15) ;
	}
	return 0 ;
}

void CUIMessageBoxYesNo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow)
	{
		HANDLE hFadeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(CUIMessageBoxYesNo::FadeThreadProc), this, 0, NULL) ;
		if (hFadeThread != NULL)
		{
			CloseHandle(hFadeThread) ;
		}
	}
}
