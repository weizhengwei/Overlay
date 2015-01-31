// XListBox.cpp  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
// Notes on use:  
//     To use in an MFC project, first create a listbox using the standard 
//     dialog editor.  Be sure to mark the listbox as OWNERDRAW FIXED, and 
//     check the HAS STRINGS box.  Using Class Wizard, create a variable for
//     the listbox.  Finally, manually edit the dialog's .h file and replace 
//     CListBox with CXListBox, and #include XListBox.h.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "skin\XListBox.h"
#include "data\Clipboard.h"
#include "tstring/tstring.h"
#include "skin/BCMenu.h"

using namespace String;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4996)	// disable bogus deprecation warning

//=============================================================================
// set resource handle (in case used in DLL)
//=============================================================================
#ifdef _USRDLL
#define AFXMANAGESTATE AfxGetStaticModuleState
#else
#define AFXMANAGESTATE AfxGetAppModuleState
#endif

//=============================================================================	
// NOTE - following table must be kept in sync with ColorPickerCB.cpp
//=============================================================================	

static COLORREF ColorTable[17] = { RGB(  0,   0,   0),		// Black
RGB(0xAC, 0xAC, 0xAC),		// White
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
RGB(78,78,78),		// Fuschia
RGB(  78, 77, 77), //// Aqua
RGB(38,37,38)};	///Warning

//=============================================================================	
BEGIN_MESSAGE_MAP(CXListBox, CListBox)
	//=============================================================================	
	//{{AFX_MSG_MAP(CXListBox)
	ON_WM_SYSCOMMAND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_XLIST_COPY, OnEditCopy)
	ON_COMMAND(IDM_XLIST_SELECT_ALL, OnEditSelectAll)
	//	ON_WM_PAINT( )
	ON_WM_ERASEBKGND()
	ON_WM_MEASUREITEM_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=============================================================================	
CXListBox::CXListBox()
//=============================================================================	
{
	m_ColorWindow        = RGB( 28,27,30 ); //::GetSysColor(COLOR_WINDOW);
	m_ColorHighlight     = ::GetSysColor(COLOR_HIGHLIGHT);
	m_ColorWindowText    = ::GetSysColor(COLOR_WINDOWTEXT);
	m_ColorHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_ColorGutter        = RGB(245,245,245);
	m_ColorLineNo        = RGB(80,80,80);
	m_strLogFile         = _T("");
	m_bColor             = TRUE;
	m_cxExtent           = 0;
	m_nTabPosition       = 8;	// tab stops every 8 columns
	m_nSpaceWidth        = 7;
	m_nContextMenuId     = (UINT)-1;
	m_bLineNumbers       = FALSE;
	m_nGutterWidth       = 5;

	m_bTimpStamp=true;

	m_bScrolled=false;

	m_brBg.Detach();
	m_brBg.CreateSolidBrush(CXListBox::Gray);

	m_ftName.Detach();
	m_ftName.CreateFont(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));

	m_ftTime.Detach();
	m_ftTime.CreateFont(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));


	for (int i = 0; i < MAXTABSTOPS; i++)
		m_nTabStopPositions[i] = (i+1) * m_nTabPosition * m_nSpaceWidth;
}

//=============================================================================	
CXListBox::~CXListBox()
//=============================================================================	
{
}
/*
LRESULT CXListBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
if(message==WM_VSCROLL||message==WM_HSCROLL)
{
WORD sbCode=LOWORD(wParam);
if(sbCode==SB_THUMBTRACK
||sbCode==SB_THUMBPOSITION)
{
SCROLLINFO siv={0};
siv.cbSize=sizeof(SCROLLINFO);
siv.fMask=SIF_ALL;
SCROLLINFO sih=siv;
int nPos=HIWORD(wParam);
CRect rcClient;
GetClientRect(&rcClient);
GetScrollInfo(SB_VERT,&siv);
GetScrollInfo(SB_HORZ,&sih);
SIZE sizeAll;
if(sih.nPage==0) 
sizeAll.cx=rcClient.right;
else
sizeAll.cx=rcClient.right*(sih.nMax+1)/sih.nPage ;
if(siv.nPage==0)
sizeAll.cy=rcClient.bottom;
else
sizeAll.cy=rcClient.bottom*(siv.nMax+1)/siv.nPage ;

SIZE size={0,0};
if(WM_VSCROLL==message)
{
size.cx=sizeAll.cx*sih.nPos/(sih.nMax+1);
size.cy=sizeAll.cy*(nPos-siv.nPos)/(siv.nMax+1);
}else
{
size.cx=sizeAll.cx*(nPos-sih.nPos)/(sih.nMax+1);
size.cy=sizeAll.cy*siv.nPos/(siv.nMax+1);
}
//			CListCtrl::Scroll(size);
return 1;
}
}
return CListBox::WindowProc(message, wParam, lParam);
}
*/

BOOL CXListBox::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN &&pMsg->wParam=='C' )
	{
		if(GetKeyState(VK_CONTROL)&0x8000)
		{
			OnEditCopy( );
		}
	}	
	return CListBox::PreTranslateMessage(pMsg);
}
//=============================================================================	
void CXListBox::MeasureItem(LPMEASUREITEMSTRUCT lpDIS)
//=============================================================================	
{
	_tstring  szStr;
	HGDIOBJ  hFont,hNewFont;
	int nLen = CListBox::GetTextLen(lpDIS->itemID);
	TCHAR *buf = new TCHAR [nLen + 16];
	ASSERT(buf);
	if (buf && (GetTextWithColor(lpDIS->itemID, buf) != LB_ERR))
	{
		int itext = int (buf[0] - 1);
		int iback = int (buf[1] - 1);
		szStr=&buf[2];
		if(szStr.empty( ) )
			szStr=_T(' ');

		CRect        rectCal,rectWnd;
		CPaintDC dc(this); 

		GetWindowRect(&rectWnd);
//		if(GetStyle( )|WS_VSCROLL)
//			rectWnd.right-=14;
		rectCal.SetRect(14,0,rectWnd.Width()-42,3500);
		hFont=dc.SelectObject(GetFont());
		dc.DrawText(szStr.c_str(),rectCal,DT_CALCRECT|DT_EDITCONTROL|DT_NOPREFIX|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
		if(0==lpDIS->itemID)
		{
			lpDIS->itemHeight=rectCal.Height()+18;
		}
		else if(Purple==iback || Teal==iback ||Yellow ==iback)   ///Set Chatter Iterm
		{
			lpDIS->itemHeight=rectCal.Height()+6;
		}
		else if(Warning==iback)
		{
			lpDIS->itemHeight=rectCal.Height()+18;
		}
		else                  ///Set Content Item;
		{
			lpDIS->itemHeight=rectCal.Height()+18;
		}
		hNewFont=dc.SelectObject(hFont);
//		DeleteObject(hNewFont);
	}
	return;
}

//=============================================================================	
int CXListBox::CompareItem(LPCOMPAREITEMSTRUCT)
//=============================================================================	
{

	return 0;
}

//=============================================================================	
void CXListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
//=============================================================================	
{
	HGDIOBJ    hFont,hObj;
	COLORREF oldtextcolor, oldbackgroundcolor;

	TRIVERTEX        m_vert[2];  
	GRADIENT_RECT    m_mesh; 
	CRect rectItem(lpDIS->rcItem);
	//rectItem.top=rectItem.top+8;
	//rectItem.bottom=rectItem.bottom+8;
	//lpDIS->rcItem.top+=8;
	//lpDIS->rcItem.bottom+=8;

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
	pDC->GetCharWidth((UINT) 'c', (UINT) 'c', &m_nAveCharWidth);

	hObj=pDC->SelectObject(GetFont());

	for (int i = 0; i < MAXTABSTOPS; i++)
		m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

	// draw focus rectangle when no items in listbox
	if (lpDIS->itemID == (UINT)-1)
	{
//		if (lpDIS->itemAction & ODA_FOCUS)
//			pDC->DrawFocusRect(&rectItem);
		return;
	}
	else
	{
		int selChange   = lpDIS->itemAction & ODA_SELECT;
		int focusChange = lpDIS->itemAction & ODA_FOCUS;
		int drawEntire  = lpDIS->itemAction & ODA_DRAWENTIRE;

		if (selChange || drawEntire)
		{
			BOOL sel = lpDIS->itemState & ODS_SELECTED;

			int nLen = CListBox::GetTextLen(lpDIS->itemID);
			if (nLen != LB_ERR)
			{
				TCHAR *buf = new TCHAR [nLen + 16];
				ASSERT(buf);
				if (buf && (GetTextWithColor(lpDIS->itemID, buf) != LB_ERR))
				{				
					
					CSize size;
					// set text color from first character in string -
					// NOTE: 1 was added to color index to avoid asserts by CString
					int itext = int (buf[0] - 1);

					// set background color from second character in string -
					// NOTE: 1 was added to color index to avoid asserts by CString
					int iback = int (buf[1] - 1);
					buf[0] = _T(' ');
					buf[1] = _T(' ');
					COLORREF textcolor = sel ? m_ColorHighlightText : ColorTable[itext];
					oldtextcolor = pDC->SetTextColor(textcolor);
					COLORREF backgroundcolor = sel ? m_ColorHighlight : ColorTable[iback];
					oldbackgroundcolor = pDC->SetBkColor(backgroundcolor);

					// fill the rectangle with the background color the fast way
//				pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rectItem, NULL, 0, NULL);
								
//					pDC->TabbedTextOut(rectItem.left+2, rectItem.top, &buf[2],
//						(int)_tcslen(&buf[2]), MAXTABSTOPS, (LPINT)m_nTabStopPositions, 0);

					///draw name and timestamp
					if(Purple==iback || Teal==iback ||Yellow ==iback) ///draw timestamps
					{
						switch(iback)
						{
						case Purple:							
							m_vert[0].Alpha  = 0x0000;  						  
							m_vert[0].Red    = 66<<8;
							m_vert[0].Green  = 40<<8;
							m_vert[0].Blue   = 76<<8;

							m_vert[1].Alpha  = 0x0000; 							
							m_vert[1].Red    =55<<8;								  
							m_vert[1].Green  =28<<8;  
							m_vert[1].Blue   = 64<<8;
							break;
						case Teal:
							m_vert[0].Alpha  = 0x0000;  						  
							m_vert[0].Red    = 72<<8;
							m_vert[0].Green  = 72<<8;
							m_vert[0].Blue   = 72<<8;

							m_vert[1].Alpha  = 0x0000; 							
							m_vert[1].Red    =60<<8;								  
							m_vert[1].Green  =60<<8;  
							m_vert[1].Blue   = 60<<8;
							break;
						case Yellow:
							m_vert[0].Alpha  = 0x0000;  						  
							m_vert[0].Red    = 96<<8;
							m_vert[0].Green  = 79<<8;
							m_vert[0].Blue   = 22<<8;

							m_vert[1].Alpha  = 0x0000; 							
							m_vert[1].Red    =84<<8;								  
							m_vert[1].Green  =67<<8;  
							m_vert[1].Blue   = 10<<8;
							break;
						}
						if(0==lpDIS->itemID)
						{
							m_vert[0].x      = rectItem.left+8;  
							m_vert[0].y      = rectItem.top+12;  
							m_vert[1].x      =rectItem.right-8;  
							m_vert[1].y      = rectItem.bottom;  
						}
						else
						{
							m_vert[0].x      = rectItem.left+8;  
							m_vert[0].y      = rectItem.top;  
							m_vert[1].x      =rectItem.right-8;  
							m_vert[1].y      = rectItem.bottom;  
						}
						m_mesh.UpperLeft=0;
						m_mesh.LowerRight=1;
						
						GradientFill( pDC->m_hDC, m_vert, 2, &m_mesh, 1, GRADIENT_FILL_RECT_V );  

	//				   pDC->FillSolidRect(rectItem.left+8,rectItem.top,rectItem.Width()-16,rectItem.Height(),backgroundcolor);	
						_tstring szStr=&buf[2];

						if( !m_bTimpStamp)    //Draw user name 
						{
						      HGDIOBJ   hNewFont;
							   hFont=pDC->SelectObject(m_ftName);
							   pDC->SetTextColor(ColorTable[itext]);
							   if(0==lpDIS->itemID)
								   	pDC->DrawText(szStr.Left(szStr.length()-5).c_str(),CRect(rectItem.left+14,rectItem.top+15,rectItem.right-12,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
							   else
								   	pDC->DrawText(szStr.Left(szStr.length()-5).c_str(),CRect(rectItem.left+14,rectItem.top+3,rectItem.right-12,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
	//						   hNewFont=pDC->SelectObject(hFont);
	//						   DeleteObject(hNewFont);

						}
						else          ///draw timestamp
						{
							HGDIOBJ   hNewFont,hTmpFont;
							hFont=pDC->SelectObject(m_ftName);
							pDC->SetTextColor(ColorTable[itext]);
							if(0==lpDIS->itemID)
							{
								pDC->DrawText(szStr.Left(szStr.length()-5).c_str(),CRect(rectItem.left+14,rectItem.top+15,rectItem.right-100,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
								hTmpFont=pDC->SelectObject(&m_ftTime);
	//							DeleteObject(hTmpFont);
								pDC->DrawText(szStr.Right(5).c_str(),CRect(rectItem.right-100,rectItem.top+15,rectItem.right-14,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_RIGHT|DT_VCENTER|DT_WORDBREAK);
							}
							else
							{
								pDC->DrawText(szStr.Left(szStr.length()-5).c_str(),CRect(rectItem.left+14,rectItem.top+3,rectItem.right-100,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
								hTmpFont=pDC->SelectObject(&m_ftTime);
	//							DeleteObject(hTmpFont);
								pDC->DrawText(szStr.Right(5).c_str(),CRect(rectItem.right-100,rectItem.top+3,rectItem.right-14,rectItem.bottom),DT_NOPREFIX|DT_EDITCONTROL|DT_RIGHT|DT_VCENTER|DT_WORDBREAK);
							}
							hNewFont=pDC->SelectObject(hFont);
//							DeleteObject(hNewFont);

						}
					}
					else if(Warning==iback)  ///Draw warning message
					{
						pDC->FillSolidRect(rectItem.left+8,rectItem.top,rectItem.Width()-16,rectItem.Height()-12,backgroundcolor);						
						pDC->SetTextColor(RGB(223,223,223));
						pDC->DrawText(&buf[2],CRect(rectItem.left+14,rectItem.top+2,rectItem.right-14,rectItem.bottom-12),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);

					}
					else  //draw chat content items
					{
						pDC->FillSolidRect(rectItem.left+8,rectItem.top+1,rectItem.Width()-16,rectItem.Height()-12,backgroundcolor);	
						 pDC->SetTextColor(ColorTable[itext]);
						pDC->DrawText(&buf[2],CRect(rectItem.left+14,rectItem.top+3,rectItem.right-12,rectItem.bottom-12),DT_NOPREFIX|DT_EDITCONTROL|DT_LEFT|DT_VCENTER|DT_WORDBREAK);
					}

					size = pDC->GetOutputTextExtent(&buf[2]);
					int nScrollBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
					size.cx += nScrollBarWidth;	// in case of vertical scrollbar

					int cxExtent = (size.cx > m_cxExtent) ? size.cx : m_cxExtent;

					if (cxExtent > m_cxExtent)
					{
						m_cxExtent = cxExtent;
						SetHorizontalExtent(m_cxExtent+(m_cxExtent/32));
					}
				}
				if (buf)
					delete [] buf;
			}
		}

//		if (focusChange || (drawEntire && (lpDIS->itemState & ODS_FOCUS)))
//			pDC->DrawFocusRect(&lpDIS->rcItem);
	}
	hFont=pDC->SelectObject(hObj);
//	DeleteObject(hFont);
}

//=============================================================================	
// GetTextWithColor - get text string with color bytes
int CXListBox::GetTextWithColor(int nIndex, LPTSTR lpszBuffer) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	ASSERT(lpszBuffer);
	lpszBuffer[0] = 0;
	return CListBox::GetText(nIndex, lpszBuffer);
}

//=============================================================================	
// GetTextWithColor - get text string with color bytes
void CXListBox::GetTextWithColor(int nIndex, CString& rString) const
//=============================================================================	
{
	rString.Empty();

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return;
	}

	CListBox::GetText(nIndex, rString);
}

//=============================================================================	
// GetText - for compatibility with CListBox (no color bytes)
int CXListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
//=============================================================================	
{
	ASSERT(lpszBuffer);
	if (lpszBuffer == NULL)
		return LB_ERR;

	lpszBuffer[0] = 0;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	int nRet = CListBox::GetText(nIndex, lpszBuffer);

	size_t n = _tcslen(lpszBuffer);
	if (n > 2)
		memcpy(&lpszBuffer[0], &lpszBuffer[2], (n-1)*sizeof(TCHAR));	// copy nul too

	return nRet;
}

//=============================================================================	
// GetText - for compatibility with CListBox (no color bytes)
void CXListBox::GetText(int nIndex, CString& rString) const
//=============================================================================	
{
	rString.Empty();

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return;
	}

	CString str = _T("");
	CListBox::GetText(nIndex, str);
	if ((!str.IsEmpty()) && (str.GetLength() > 2))
		rString = str.Mid(2);
}

//=============================================================================	
// GetTextLen - for compatibility with CListBox (no color bytes)
int CXListBox::GetTextLen(int nIndex) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	int n = CListBox::GetTextLen(nIndex);
	if (n != LB_ERR && n >= 2)
		n -= 2;
	return n;
}

//=============================================================================	
int CXListBox::SearchString(int nStartAfter, LPCTSTR lpszItem, BOOL bExact) const
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	// start the search after specified index
	int nIndex = nStartAfter + 1;

	int nCount = GetCount();
	if (nCount == LB_ERR)
		return LB_ERR;

	// convert string to search for to lower case
	CString strItem = lpszItem;
	strItem.MakeLower();
	int nItemSize = strItem.GetLength();

	CString strText = _T("");

	// search until end
	for ( ; nIndex < nCount; nIndex++)
	{
		GetText(nIndex, strText);
		strText.MakeLower();
		if (!bExact)
			strText = strText.Left(nItemSize);
		if (strText == strItem)
			return nIndex;
	}

	// if we started at beginning there is no more to do, search failed
	if (nStartAfter == -1)
		return LB_ERR;

	// search until we reach beginning index
	for (nIndex = 0; (nIndex <= nStartAfter) && (nIndex < nCount); nIndex++)
	{
		GetText(nIndex, strText);
		strText.MakeLower();
		if (!bExact)
			strText = strText.Left(nItemSize);
		if (strText == strItem)
			return nIndex;
	}

	return LB_ERR;
}

//=============================================================================	
int CXListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
//=============================================================================	
{
	return SearchString(nStartAfter, lpszItem, FALSE);
}

//=============================================================================	
int CXListBox::SelectString(int nStartAfter, LPCTSTR lpszItem)
//=============================================================================	
{
	int rc = SearchString(nStartAfter, lpszItem, FALSE);
	if (rc != LB_ERR)
		SetCurSel(rc);
	return rc;
}


//=============================================================================	
int CXListBox::FindStringExact(int nStartAfter, LPCTSTR lpszItem) const
//=============================================================================	
{
	return SearchString(nStartAfter, lpszItem, TRUE);
}

//=============================================================================	
// InsertString - override to add text color
int CXListBox::InsertString(int nIndex, 
							LPCTSTR lpszItem, 
							Color tc /*= Black*/, 
							Color bc /*= White*/)
							//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	CString s = lpszItem;

	if (!m_bColor)
	{
		tc = Black;			// to force black-only text
		bc = White;
	}

	UINT nColor = (UINT) tc;
	ASSERT(nColor < 16);
	if (nColor >= 16)
		tc = Black;

	// don't display \r or \n characters
	int i = 0;
	while ((i = s.FindOneOf(_T("\r\n"))) != -1)
		s.SetAt(i, ' ');

	// first character in string is color -- add 1 to color
	// to avoid asserts by CString class
	CString t = _T("");
	t += (char) (tc + 1);
	t += (char) (bc + 1);
	t += s;

	// try to insert the string into the listbox
	i = CListBox::InsertString(nIndex, t);

	return i;
}

//=============================================================================	
// AddString - override to add text color
int CXListBox::AddString(LPCTSTR lpszItem)
//=============================================================================	
{
	return AddLine(CXListBox::Black, CXListBox::White, lpszItem);
}

//=============================================================================	
int CXListBox::AddLine(Color tc, Color bc, LPCTSTR lpszLine,bool bAutoFill)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
		return LB_ERR;
	}

	CString s = lpszLine;

	if (!m_bColor)
	{
		tc = Black;			// to force black-only text
		bc = White;
	}

	UINT nColor = (UINT) tc;
	ASSERT(nColor < 16);
	if (nColor >= 16)
		tc = Black;

	// don't display \r or \n characters
	int i = 0;
	s.Replace(_T("\r\n"),_T(" "));
	s.Replace(_T("\n"),_T(" "));

/*	while ((i = s.Find(_T("\r\n"))) != -1)
	{
		s.SetAt(i, _T(' '));
	}*/

	// first character in string is color -- add 1 to color
	// to avoid asserts by CString class
	CString t = _T("");
	t += (TCHAR) (tc + 1);
	t += (TCHAR) (bc + 1);
	t += s;

	CPaintDC dc(this); 
	// try to add the string to the listbox
	i = CListBox::AddString(t);
	
	if (i == LB_ERRSPACE)
	{
		// will get LB_ERRSPACE if listbox is out of memory
		int n = GetCount();

		if (n == LB_ERR)
			return LB_ERR;

		if (n < 2)
			return LB_ERR;

		// try to delete some strings to free up some room --
		// don't spend too much time deleting strings, since
		// we might be getting a burst of messages
		n = (n < 20) ? (n-1) : 20;
		if (n <= 0)
			n = 1;

		SetRedraw(FALSE);
		for (i = 0; i < n; i++)
			DeleteString(0);

		i = CListBox::AddString(t);
		SetRedraw(TRUE);
		
	}

	if (i >= 0)
	{
		SetTopIndex(i);
	}
	return i;
}

//=============================================================================	
int _cdecl CXListBox::Printf(Color tc, Color bc, UINT nID, LPCTSTR lpszFmt, ...)
//=============================================================================	
{
	AFX_MANAGE_STATE(AFXMANAGESTATE())

		TCHAR buf[2048], fmt[1024];
	va_list marker;

	// load format string from string resource if
	// a resource ID was specified
	if (nID)
	{
		CString s = _T("");
		if (!s.LoadString(nID))
		{
			_stprintf(s.GetBufferSetLength(80), _T("Failed to load string resource %u"),
				nID);
			s.ReleaseBuffer(-1);
		}
		_tcsncpy(fmt, s, sizeof(fmt)/sizeof(TCHAR)-1);
	}
	else
	{
		// format string was passed as parameter
		_tcsncpy(fmt, lpszFmt, sizeof(fmt)/sizeof(TCHAR)-1);
	}
	fmt[sizeof(fmt)/sizeof(TCHAR)-1] = 0;

	// combine output string and variables
	va_start(marker, lpszFmt);
	_vsntprintf(buf, (sizeof(buf)/sizeof(TCHAR))-1, fmt, marker);
	va_end(marker);
	buf[sizeof(buf)/sizeof(TCHAR)-1] = 0;

	return AddLine(tc, bc, buf);
}

//=============================================================================	
void CXListBox::EnableColor (BOOL bEnable)
//=============================================================================	
{
	m_bColor = bEnable;
}

void CXListBox::EableTimeStamp(bool bEnable)
{
	m_bTimpStamp=bEnable;
}
//=============================================================================	
void CXListBox::SetTabPosition(int nSpacesPerTab)
//=============================================================================	
{
	ASSERT(nSpacesPerTab > 0 && nSpacesPerTab < 11);

	m_nTabPosition = nSpacesPerTab;

	CDC* pDC = GetDC();

	if (pDC)
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
		pDC->GetCharWidth((UINT) '9', (UINT) '9', &m_nAveCharWidth);

		for (int i = 0; i < MAXTABSTOPS; i++)
			m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

		ReleaseDC(pDC);
	}
}

//=============================================================================	
int CXListBox::GetVisibleLines()
//=============================================================================	
{
	int nCount = 0;

	CDC* pDC = GetDC();

	if (pDC)
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		int h = tm.tmHeight + tm.tmInternalLeading;
		ReleaseDC(pDC);

		CRect rect;
		GetClientRect(&rect);
		nCount = rect.Height() / h;
	}
	return nCount;
}

//=============================================================================	
void CXListBox::ResetContent()
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		CListBox::ResetContent();

		m_cxExtent = 0;

		SetHorizontalExtent(m_cxExtent);
	}
}

//=============================================================================	
void CXListBox::SetFont(CFont *pFont, BOOL bRedraw)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		CListBox::SetFont(pFont, bRedraw);

		CDC* pDC = GetDC();

		if (pDC)
		{
			CFont *pOldFont = pDC->SelectObject(pFont);

			TEXTMETRIC tm;
			pDC->GetTextMetrics(&tm);
			int h = tm.tmHeight;
			//			SetItemHeight(0, h);
			SetItemHeight(0,25);

			pDC->SelectObject(pOldFont);

			pDC->GetCharWidth((UINT) ' ', (UINT) ' ', &m_nSpaceWidth);
			pDC->GetCharWidth((UINT) '9', (UINT) '9', &m_nAveCharWidth);

			for (int i = 0; i < MAXTABSTOPS; i++)
				m_nTabStopPositions[i] = (i+1) * m_nAveCharWidth * m_nTabPosition;

			ReleaseDC(pDC);
		}

		m_cxExtent = 0;
	}
}

//=============================================================================	
void CXListBox::OnLButtonDblClk(UINT nFlags, CPoint point)
//=============================================================================	
{
	CListBox::OnLButtonDblClk(nFlags, point);
}

//=============================================================================	
void CXListBox::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
//=============================================================================	
{
	AFX_MANAGE_STATE(AFXMANAGESTATE())
	
	BCMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(0, IDM_XLIST_COPY, _T("&Copy(&C)\tCtrl+C "));
	menu.AppendMenu(0, IDM_XLIST_SELECT_ALL,  _T("Select All"));

	BCMenu::bRemoveMenuBolder = TRUE ; //change style of popup menu

	menu.TrackPopupMenu(TPM_RIGHTBUTTON,point.x, point.y, this, NULL);
	menu.DestroyMenu();

	BCMenu::bRemoveMenuBolder = TRUE ; //change style of popup menu
}

//=============================================================================	
void CXListBox::OnEditCopy()
//=============================================================================	
{
	CString str = _T("");

	int nCount = GetCount();
	int nSel = 0;

	for (int i = 0; i < nCount; i++)
	{
		if (GetSel(i) > 0)
		{
			CString s = _T("");
			GetText(i, s);
			if (!s.IsEmpty())
			{
				nSel++;
				s.TrimLeft(_T("\r\n"));
				s.TrimRight(_T("\r\n"));
				if (s.Find(_T('\n')) == -1)
					s += _T("\n");
				s.Replace(_T("\t"), _T(" "));
				str += s;
			}
		}
	}

	if (!str.IsEmpty())
		CClipboard::SetText(str);
}

//=============================================================================	
void CXListBox::OnEditClear()
//=============================================================================	
{
	ResetContent();
}

//=============================================================================	
void CXListBox::OnEditSelectAll()
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		SelItemRange(TRUE, 0, GetCount()-1);
	}
}

void CXListBox::OnPaint( )
{
	CPaintDC dc(this); 
	return;	
}

//=============================================================================	
BOOL CXListBox::OnEraseBkgnd(CDC* pDC) 
//=============================================================================	
{
	if (m_bLineNumbers)
	{
		CFont *pOldFont = NULL;
		CFont *pFont = GetFont();
		if (pFont == NULL)
			pFont = GetParent()->GetFont();
		if (pFont)
			pOldFont =pDC->SelectObject(pFont);

		CRect rect;
		GetClientRect(&rect);
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
		// fill the background with the window color the fast way
		pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

		CString strLineNo = _T("");
		for (int i = 0; i < m_nGutterWidth; i++)
			strLineNo += _T('9');
		if (strLineNo.IsEmpty())
			strLineNo = _T('9');
		CSize size = pDC->GetTextExtent(strLineNo);
		CRect rectGutter(rect);
		rectGutter.right = rectGutter.left + size.cx + 6;
		pDC->SetBkColor(m_ColorGutter);

		// fill the gutter with the gutter color the fast way
		pDC->ExtTextOut(0, 0, ETO_OPAQUE, &rectGutter, NULL, 0, NULL);
		if (pOldFont)
			pDC->SelectObject(pOldFont);
		return true;
	}
	else
	{
		CRect    iRect;
		GetClientRect(&iRect);
		pDC->FillRect(&iRect,&m_brBg);
		//		pDC->SetBkColor(Maroon);
		return true;///CListBox::OnEraseBkgnd(pDC);
	}
}

//=============================================================================	
CXListBox::Color CXListBox::GetBackgroundColor(int nIndex) const
//=============================================================================	
{
	Color bc = White;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get background color from second character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int iback = int (buf[1] - 1);
				bc = (Color) iback;
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}

	return bc;
}

//=============================================================================	
CXListBox::Color CXListBox::GetTextColor(int nIndex) const
//=============================================================================	
{
	Color tc = Black;

	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// set text color from first character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int itext = int (buf[0] - 1);
				tc = (Color) itext;
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}

	return tc;
}

//=============================================================================	
void CXListBox::SetBackgroundColor(int nIndex, Color bc)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get text color from first character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int tc = (int) buf[0];

				// delete old string
				CListBox::DeleteString(nIndex);

				CString s = &buf[2];
				CString t = _T("");
				t += (char) tc;
				t += (char) (bc + 1);	// add 1 to color to avoid asserts by CString class
				t += s;
				CListBox::InsertString(nIndex, t);
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}
}

//=============================================================================	
void CXListBox::SetTextColor(int nIndex, Color tc)
//=============================================================================	
{
	if (!::IsWindow(m_hWnd))
	{
		ASSERT(FALSE);
	}
	else
	{
		int nLen = CListBox::GetTextLen(nIndex);
		if (nLen != LB_ERR)
		{
			TCHAR *buf = new TCHAR [nLen + 16];
			ASSERT(buf);
			if (buf && (GetTextWithColor(nIndex, buf) != LB_ERR))
			{
				// get background color from second character in string -
				// NOTE: 1 was added to color index to avoid asserts by CString
				int bc = (int) buf[1];

				// delete old string
				CListBox::DeleteString(nIndex);

				CString s = &buf[2];
				CString t = _T("");
				t += (char) (tc + 1);	// add 1 to color to avoid asserts by CString class
				t += (char) bc;
				t += s;
				CListBox::InsertString(nIndex, t);
			}
			if (buf)
				delete [] buf;
			buf = NULL;
		}
	}
}
