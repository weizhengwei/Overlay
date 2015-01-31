#include "stdafx.h"
#include <richedit.h>
#include <richole.h>
#include "skin/BCMenu.h"
#include "data/UIString.h"
#include "skin/UIRichEditCtrl.h"

// CUIRichEditCtrl

static COLORREF ColorTable[17] = { RGB(  0,   0,   0),		// Black
RGB(255, 255, 255),		// White
RGB(28,27,30),		// Maroon
RGB(  0, 128,   0),		// Green
RGB(128, 128,   0),		// Olive
RGB(  0,   0, 128),		// Navy
RGB(58,32,68),		// Purple
RGB(  59,59,59),		// Teal
RGB(64,64,64),		// Silver
RGB(38,37,39),		// Gray
RGB(255,   0,   0),		// Red
RGB(  0, 255,   0),		// Lime
RGB(84,67,10),		// Yellow
RGB(  0,   0, 255),		// Blue
RGB(255,   0, 255),		// Fuschia
RGB(  0, 255, 255), //// Aqua
RGB(0X80, 0X15, 0X15)};	///Warning

IMPLEMENT_DYNAMIC(CUIRichEditCtrl, CRichEditCtrl)

CUIRichEditCtrl::CUIRichEditCtrl()
{
	m_brWnd.Detach();
	m_brWnd.CreateSolidBrush(RGB( 28,27,30));
}

CUIRichEditCtrl::~CUIRichEditCtrl()
{
	m_brWnd.DeleteObject( );
}


BEGIN_MESSAGE_MAP(CUIRichEditCtrl, CRichEditCtrl)
	ON_WM_PAINT( )
	ON_WM_CREATE( )
	ON_WM_ERASEBKGND( )
	ON_WM_RBUTTONDOWN()
//	ON_WM_LBUTTONDOWN( )
	ON_COMMAND(IDM_RICH_UNDO, OnUndo)
	ON_COMMAND(IDM_RICH_CUT, OnCutString)
	ON_COMMAND(IDM_RICH_COPY,OnCopyString)
	ON_COMMAND(IDM_RICH_PASTE, OnPasteString)
	ON_COMMAND(IDM_RICH_CLEAR, OnClearAll)	
	ON_COMMAND(IDM_RICH_SELALL, OnSelectAll)	
//	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
//	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

int CUIRichEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRichEditCtrl::OnCreate(lpCreateStruct);

	GetClientRect(&m_rectWnd);
//	m_pSbsBar=new CXScrollBar;

/*	m_pSbsBar->CreateFromRect( SBS_VERT | SBS_RIGHTALIGN | WS_CHILD | WS_VISIBLE, this,
		CRect(m_rectWnd.right-18, m_rectWnd.top, m_rectWnd.right, m_rectWnd.bottom), 
		1189);
	m_pSbsBar->EnableThumbColor(false);
	m_pSbsBar->EnableChannelColor(false);*/

//	m_pSbsBar->EnableScrollBar(SB_VERT,ESB_ENABLE_BOTH );
//	m_pSbsBar->SetScrollRange(0,m_nItemCount);

	return  0;
}

void CUIRichEditCtrl::OnRButtonDown(UINT nFlags, CPoint point )
{
	SetFocus();
	//创建一个弹出式菜单
	BCMenu popmenu;
	popmenu.CreatePopupMenu();
	//Fix Bug 0148932: the menu items cut, paste, clear all and undo are not needed when CUIRichEditCtrl is read only.
	_tstring sCmd;
	if(GetWindowLong(GetSafeHwnd(), GWL_STYLE) & ES_READONLY)
	{
		popmenu.AppendMenu(0, IDM_RICH_SELALL, (TCHAR*)theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_SELECTALL")).c_str());
		sCmd = _T("&") ;
		sCmd.append(theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_COPY")).c_str()) ;
		sCmd.append(_T("(&X)\tCtrl+C ")) ;
		popmenu.AppendMenu(0, IDM_RICH_COPY, (TCHAR*)sCmd.c_str());
	}
	else
	{
		popmenu.AppendMenu(0, IDM_RICH_SELALL, (TCHAR*)theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_SELECTALL")).c_str());
		_tstring sCmd = _T("&") ;
		sCmd.append(theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_CUT")).c_str()) ;
		sCmd.append(_T("(&X)\tCtrl+X ")) ;
		popmenu.AppendMenu(0, IDM_RICH_CUT, (TCHAR*)sCmd.c_str());
		sCmd = _T("&") ;
		sCmd.append(theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_COPY")).c_str()) ;
		sCmd.append(_T("(&X)\tCtrl+C ")) ;
		popmenu.AppendMenu(0, IDM_RICH_COPY, (TCHAR*)sCmd.c_str());
		sCmd = _T("&") ;
		sCmd.append(theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_PASTE")).c_str()) ;
		sCmd.append(_T("(&X)\tCtrl+V ")) ;
		popmenu.AppendMenu(0, IDM_RICH_PASTE, (TCHAR*)sCmd.c_str());
		popmenu.AppendMenu(0, IDM_RICH_CLEAR, (TCHAR*)theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_CLEARALL")).c_str());
		popmenu.AppendMenu(0, IDM_RICH_UNDO, (TCHAR*)theUIString._GetStringFromId(_T("IDS_DIALOG_CHAT_MENU_UNDO")).c_str());
	}
	
	//初始化菜单项
/*	UINT nUndo=(CanUndo() ? 0 : MF_GRAYED );
	popmenu.EnableMenuItem(IDM_RICH_UNDO, MF_BYCOMMAND|nUndo);
	
	UINT nSel=((GetSelectionType()!=SEL_EMPTY) ? 0 : MF_GRAYED) ;
	popmenu.EnableMenuItem(IDM_RICH_CUT, MF_BYCOMMAND|nSel);
	popmenu.EnableMenuItem(IDM_RICH_COPY, MF_BYCOMMAND|nSel);
	popmenu.EnableMenuItem(IDM_RICH_CLEAR, MF_BYCOMMAND);
	popmenu.EnableMenuItem(IDM_RICH_SELALL, MF_BYCOMMAND);
	
	UINT nPaste=(CanPaste() ? 0 : MF_GRAYED) ;
	popmenu.EnableMenuItem(IDM_RICH_PASTE, MF_BYCOMMAND|nPaste);*/
	
	BCMenu::bRemoveMenuBolder = TRUE ; //change style of popup menu

	//显示菜单
	CPoint pt;
	GetCursorPos(&pt);
	popmenu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this);
	popmenu.DestroyMenu();

	BCMenu::bRemoveMenuBolder = FALSE ;
}

void CUIRichEditCtrl::OnLButtonDown(UINT nFlags,CPoint point )
{
	CRichEditCtrl::OnLButtonDown(nFlags,point);
	GetParent()->PostMessage(WM_LBUTTONDOWN,0,0);
}

void CUIRichEditCtrl::InsertBitmap(LPCTSTR lpszFilePath,UINT uSize)
{
	HRESULT            hResult;
	IStream                *pIStream=NULL;
	_tstring                 tsPath=lpszFilePath;

	Bitmap *pBitmap=Bitmap::FromFile(tsPath.toWideString().c_str());
	if(!pBitmap)
	{
		return ;
	}
	HBITMAP   hBitmap=NULL;
	pBitmap->GetHBITMAP(Color(0,0,0,0),&hBitmap);

	STGMEDIUM stgm;
	stgm.tymed = TYMED_GDI;    // Storage medium = HBITMAP handle
	stgm.hBitmap = hBitmap;
	stgm.pUnkForRelease = NULL; // Use ReleaseStgMedium
	
	FORMATETC fm;
	fm.cfFormat = CF_BITMAP;    // Clipboard format = CF_BITMAP
	fm.ptd = NULL;       // Target Device = Screen
	fm.dwAspect = DVASPECT_CONTENT;   // Level of detail = Full content
	fm.lindex = -1;       // Index = Not applicaple
	fm.tymed = TYMED_GDI;  
	
	////创建输入数据源
	IStorage *pStorage; 
	
	///分配内存
	LPLOCKBYTES lpLockBytes = NULL;
	SCODE sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		AfxThrowOleException(sc);
	ASSERT(lpLockBytes != NULL);
	
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
		STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &pStorage);
	if (sc != S_OK)
	{
		VERIFY(lpLockBytes->Release( ) == 0);
		lpLockBytes = NULL;
		AfxThrowOleException(sc);
	}
	ASSERT(pStorage != NULL);
	
	COleDataSource *pDataSource = new COleDataSource;
	pDataSource->CacheData(CF_BITMAP, &stgm);
	LPDATAOBJECT lpDataObject = 
		(LPDATAOBJECT)pDataSource->GetInterface(&IID_IDataObject);
	
	///获取RichEdit的OLEClientSite
	LPOLECLIENTSITE lpClientSite;
	this->GetIRichEditOle( )->GetClientSite( &lpClientSite );
	
	
	///创建OLE对象
	IOleObject *pOleObject;
	sc = OleCreateStaticFromData(lpDataObject,IID_IOleObject,OLERENDER_FORMAT,
		&fm,lpClientSite,pStorage,(void **)&pOleObject);
	if(sc!=S_OK)
		AfxThrowOleException(sc);
	
	///插入OLE对象
	
	REOBJECT reobject;
	ZeroMemory(&reobject, sizeof(REOBJECT));
	reobject.cbStruct = sizeof(REOBJECT);
	
	CLSID clsid;
	sc = pOleObject->GetUserClassID(&clsid);
	if (sc != S_OK)
		AfxThrowOleException(sc);
	
	reobject.clsid = clsid;
	reobject.cp = REO_CP_SELECTION;
	reobject.dvaspect = DVASPECT_CONTENT;
	reobject.poleobj = pOleObject;
	reobject.polesite = lpClientSite;
	reobject.pstg = pStorage;
	
	HRESULT hr = this->GetIRichEditOle()->InsertObject( &reobject );
}

void CUIRichEditCtrl::AddText(LPCTSTR lpszStr,CHARFORMAT2& charFormat)
{	
	UINT  uTextLength = GetWindowTextLength();
	this->SetSel(uTextLength ,uTextLength );
	this->SetSelectionCharFormat(charFormat);
	this->ReplaceSel(lpszStr);

	this->PostMessage( WM_VSCROLL,  SB_BOTTOM,0);
	return;
}

void CUIRichEditCtrl::AddText(LPCTSTR lpszStr)
{
	AddText(lpszStr,SB_BOTTOM);

}
void CUIRichEditCtrl::AddText(LPCTSTR lpszStr,int nscrollPos)
{
	UINT  uTextLength = GetWindowTextLength();
	this->SetSel(uTextLength ,uTextLength );
	this->ReplaceSel(lpszStr);
	this->PostMessage( WM_VSCROLL,  nscrollPos,0);
}
void CUIRichEditCtrl::ResetContent()
{
	this->SetSel(0,-1);
	this->Clear();
}

void CUIRichEditCtrl::AddLine( MyColor tc, MyColor bc, LPCTSTR lpszLine,bool bAutoFill)
{
    _tstring   szText;
	_tstring   szStr=lpszLine;
	UINT  nLine=0;
	UINT  nLen=_tcslen(lpszLine);
	DWORD  dwTotal;
	
	UINT  uTextLength = GetTextLength();
	this->SetSel(uTextLength ,uTextLength );
	this->ReplaceSel(_T("\r\n"));

	CHARFORMAT2 cf2; 
	ZeroMemory(&cf2, sizeof(CHARFORMAT2));
	cf2.cbSize = sizeof(CHARFORMAT2);
	cf2.dwMask = CFM_BACKCOLOR|CFM_BOLD | CFM_COLOR | CFM_FACE |
		CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE;;
	cf2.crBackColor=ColorTable[bc]; 
	cf2.dwEffects = 0;
	cf2.yHeight = 15*15;
	cf2.crTextColor =ColorTable[tc]; 
	this->SetSelectionCharFormat(cf2);

	uTextLength = GetTextLength();
	this->SetSel(uTextLength-1 ,uTextLength );
	this->ReplaceSel(lpszLine);	

	dwTotal=GetTextLength( );
	
	if(bAutoFill)
	{
//		SetSel(uTextLength-5 ,dwTotal +1);
//		szText=GetSelText();
		SetReadOnly(false);
		nLine=GetLineCount( );
		SetSel(dwTotal,dwTotal);
		while(GetLineCount() == nLine)
		{
			ReplaceSel(_T("                                                      \t"));
			dwTotal=GetTextLength();
			SetSel(dwTotal-5,dwTotal);
		}

		///remove the last line
		int nStart=LineIndex(GetLineCount()-1);
		SetSel(nStart,-1);
		ReplaceSel(_T(""));
		SetReadOnly(true);
	}


	this->PostMessage( WM_VSCROLL,  SB_BOTTOM,0);

/*
	nLine=this->GetFirstVisibleLine( );
	if(nLine>0)
	{		
		this->LineScroll(-nLine);
	}
	*/
	this->SetSel(0,0);
	UpdateData( );
	return;
}

void CUIRichEditCtrl::OnUndo(  )
{
	Undo( );
	return;
}

void CUIRichEditCtrl::OnCutString( )
{
	if(ECO_READONLY&GetOptions())
	{
		return;
	}
	ReplaceSel(TEXT(""));
	Copy( );
	return;

}

void CUIRichEditCtrl::OnCopyString( )
{
	Copy();
	return;
}

void CUIRichEditCtrl::OnPasteString( )
{
	int nLine=0;
	UINT  uTextStart=0;
	UINT  uTextEnd=0;
	CHARFORMAT2 charFormat;
	if(ECO_READONLY&GetOptions())
	{
		return;
	}

	this->GetDefaultCharFormat(charFormat);
	uTextStart= GetWindowTextLength();
	Paste( );
	uTextEnd= GetWindowTextLength();
	this->SetSel(uTextStart,uTextEnd);
	this->SetSelectionCharFormat(charFormat);
	this->PostMessage( WM_VSCROLL,  SB_BOTTOM,0);
	return;
}

void CUIRichEditCtrl::OnClearAll( )
{
	if(ECO_READONLY&GetOptions())
	{
		return;
	}
	SetSel(0L,-1L);
	ReplaceSel(NULL);
	return;
}

void CUIRichEditCtrl::OnSelectAll( )
{
	SetSel(0L,-1L);
	return;
}

void CUIRichEditCtrl::OnPaint( )
{	
/*	CPaintDC dc(this);
	CRect rect;
	GetWindowRect(rect);
	rect.left -= 1;
	rect.top -= 1;
	rect.right += 2;    
	rect.bottom += 2;

	ScreenToClient(rect);
	dc.Draw3dRect(rect, RGB(88, 88, 88), RGB(88, 88,88));
	Invalidate();
	Default();*/
	return  CRichEditCtrl::OnPaint();
}

BOOL CUIRichEditCtrl::OnEraseBkgnd(CDC* pDC)
{
	CRect        rect;
	CPaintDC  dc(this);
	GetClientRect(&rect);

	dc.FillRect( &rect,&m_brWnd);
	return true;
}

/*
void CUIRichEditCtrl::OnUpdate() 
{
    UpdateCtrl();
}


void CUIRichEditCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    UpdateCtrl();
    
    CRichEditCtrl::OnLButtonDown(nFlags, point);
}


void CUIRichEditCtrl::UpdateCtrl()
{
    CWnd* pParent = GetParent();
    CRect rect;
    
    GetWindowRect(rect);
    pParent->ScreenToClient(rect);
    rect.DeflateRect(2, 2);
    
    pParent->InvalidateRect(rect, FALSE);    
}*/