
////////////////////////////////////////////////////////////////////////////
//	Copyright : Ashok Jaiswal 2004
//
//	Email :ashok@muvee.com
//
//	This code may be used in compiled form in any way you desire. This
//	file may be redistributed unmodified by any means PROVIDING it is 
//	not sold for profit without the authors written consent, and 
//	providing that this notice and the authors name is included.
//
//	This file is provided 'as is' with no expressed or implied warranty.
//	The author accepts no liability if it causes any damage to your computer.
//
//	Do expect bugs.
//	Please let me know of any bugs/mods/improvements.
//	and I will try to fix/incorporate them into this file.
//	Enjoy!
//
//	Description : Description goes here 
////////////////////////////////////////////////////////////////////////////
// BitComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "skin/BitComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int nListBoxTextLeft = 2;

static WNDPROC m_pWndProc = 0;

//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:23:46 PM
// Function name	: LRESULT FAR PASCAL BitComboBoxListBoxProc
// Description	    : The list box control in combo box is actually owned by window so we let all the messages
//						handled by windows.
// Return type		: extern "C" 
// Argument         : HWND hWnd
// Argument         : UINT nMsg
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
//----------------------------------------------------------------------
extern "C" LRESULT FAR PASCAL BitComboBoxListBoxProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	//TRACE("BitComboBoxListBoxProc 0x%.4X\n",nMsg);
	return CallWindowProc(m_pWndProc, hWnd, nMsg, wParam, lParam);
}
/////////////////////////////////////////////////////////////////////////////
// CBitComboBox


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:42 PM
// Function name	: CBitComboBox::CBitComboBox
// Description	    : Constructor
// Return type		: 
//----------------------------------------------------------------------
CBitComboBox::CBitComboBox()
{
	m_nIDLeft = m_nIDRight = m_nIDCenter = 0;
	m_nIDListLeft, m_nIDListRight, m_nIDListTop, m_nIDListBot = 0;
		
	m_colBackGroundHighLight = 0x1f1f1f ; 
	m_colTextHighLight = 0xffffff ;

	m_colTextNormal = 0xacacac ;
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:43 PM
// Function name	: CBitComboBox::~CBitComboBox
// Description	    : Destructor
// Return type		: 
//----------------------------------------------------------------------
CBitComboBox::~CBitComboBox()
{
}


BEGIN_MESSAGE_MAP(CBitComboBox, CComboBox)
	//{{AFX_MSG_MAP(CBitComboBox)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitComboBox message handlers

//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:38 PM
// Function name	: CBitComboBox::OnCtlColorListBox
// Description	    : It is called when list box item are painted
// Return type		: LRESULT 
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
//----------------------------------------------------------------------
LRESULT CBitComboBox::OnCtlColorListBox(WPARAM wParam, LPARAM lParam) 
{
	// Here we need to get a reference to the listbox of the combobox
	// (the dropdown part). We can do it using 
	//TRACE("OnCtlColorListBox()\n");
	if (this->m_listbox.m_hWnd == 0) {
		HWND hWnd = (HWND)lParam;

		if (hWnd != 0 && hWnd != m_hWnd) 
		{
			// Save the handle
			m_listbox.m_hWnd = hWnd;

			// Subclass ListBox
			m_pWndProc = (WNDPROC)GetWindowLong(m_listbox.m_hWnd, GWL_WNDPROC);
			SetWindowLong(m_listbox.m_hWnd, GWL_WNDPROC, (LONG)BitComboBoxListBoxProc);
		}
	}


	return DefWindowProc(WM_CTLCOLORLISTBOX, wParam, lParam);
}


#define SET_ZERO(X) memset((void *)&X,0,sizeof(X));

//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:36 PM
// Function name	: CBitComboBox::OnPaint
// Description	    : Main painting procedure. Skin the Combo box and skin the list box once its created.
// Return type		: void 
//----------------------------------------------------------------------
void CBitComboBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	RECT rc;
	CDC MemDC;

	GetClientRect(&rc);
	MemDC.CreateCompatibleDC(&dc);

	CBitmap bmpComboRight,bmpComboLeft, bmpComboCenter,bmpComboBot;
	BITMAP bitRight, bitLeft, bitCenter;
	SET_ZERO(bitRight); SET_ZERO(bitLeft); SET_ZERO(bitCenter);

	if( m_nIDRight )
	{
		bmpComboRight.LoadBitmap(m_nIDRight);
		bmpComboRight.GetBitmap(&bitRight);
		MemDC.SelectObject(bmpComboRight);
		dc.BitBlt(rc.right-bitRight.bmWidth+2,rc.top+2,bitRight.bmWidth,bitRight.bmHeight,&MemDC,0,0,SRCCOPY);
	}

	CString sz;
	GetWindowText(sz);

	//Select the font only if list box is created
	if( m_listbox )
		dc.SelectObject(m_listbox.GetFont());

	RECT rcRepaint;
	GetClientRect(&rcRepaint);
	rcRepaint.right = rcRepaint.right - bitRight.bmWidth+2;
	rcRepaint.top += 2; rcRepaint.bottom -= 2;
	dc.FillSolidRect(&rcRepaint,m_colBackGroundNormal);
	dc.SetTextColor(m_colTextNormal);
	LOGFONT lf ;
	memset(&lf, 0, sizeof(LOGFONT)) ;
	lf.lfHeight = 14;
	m_Font.CreateFontIndirect(&lf);

	CFont* def_font = dc.SelectObject(&m_Font);
	rcRepaint.left = rcRepaint.left + bitLeft.bmWidth +5;
	if(this->GetStyle()&CBS_DROPDOWNLIST==CBS_DROPDOWNLIST)
		dc.DrawText(sz,&rcRepaint,DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(def_font);
	m_Font.DeleteObject();

	/*
	** draw frame of combo box.
	*/
    CRect rectframe;
    GetWindowRect(rectframe);
    rectframe.right += 4 ;    
    ScreenToClient(rectframe);
    dc.Draw3dRect(rectframe, RGB(88, 88, 88), RGB(88, 88,88));

	if( m_listbox )
	{

		TRACE(_T("Painting the listbox\n"));
		CDC *pDC = m_listbox.GetDC();
		m_listbox.GetClientRect(&rc);

		CBrush pBrush;
		pBrush.CreateSolidBrush(m_colBackGroundNormal);

		rc.right += 2 ;
		m_listbox.GetDC()->FillRect(&rc,&pBrush);

		pDC->SetTextColor(m_colTextNormal);
		//CBrush brItemData;
		RECT rcItemData;
		CString szItemString;
		//brItemData.CreateSolidBrush(m_colBackGroundNormal);

		for(int i =0;i<m_listbox.GetCount();i++)
		{
			m_listbox.GetText(i,szItemString);
			m_listbox.GetItemRect(i,&rcItemData);

			pDC->SetBkMode(TRANSPARENT);

			//selected/unselected
			if(m_listbox.GetCurSel()==i)
			{
				pDC->SetTextColor(m_colTextHighLight);
				pDC->FillSolidRect(&rcItemData,m_colBackGroundHighLight);
			}
			else
			{
				pDC->SetTextColor(m_colTextNormal);
				pDC->FillSolidRect(&rcItemData,m_colBackGroundNormal);
			}
			rcItemData.left += bitLeft.bmWidth+2;
			pDC->SelectObject(m_listbox.GetFont());
			pDC->DrawText(szItemString,&rcItemData, DT_VCENTER | DT_SINGLELINE);
		}

		//avoid white border in windows7
		CRect listrc ;
		m_listbox.GetWindowRect(&listrc) ;
		ScreenToClient(&listrc);
		CRgn rectRgn ;
		rectRgn.CreateRectRgn(listrc.left+1, 1, listrc.right-1, listrc.bottom-25);
		m_listbox.SetWindowRgn(rectRgn,true);
		rectRgn.DeleteObject() ;
	}
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:31 PM
// Function name	: CBitComboBox::Create
// Description	    : Sets the combo box properties to owner draw and fix.
// Return type		: BOOL 
// Argument         : LPCTSTR lpszClassName
// Argument         : LPCTSTR lpszWindowName
// Argument         : DWORD dwStyle
// Argument         : const RECT& rect
// Argument         : CWnd* pParentWnd
// Argument         : UINT nID
// Argument         : CCreateContext* pContext
//----------------------------------------------------------------------
BOOL CBitComboBox::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	// Remove the CBS_SIMPLE and CBS_DROPDOWN styles and add the one I'm designed for
	dwStyle &= ~0xF;
	dwStyle |= CBS_DROPDOWNLIST;

	// Make sure to use the CBS_OWNERDRAWVARIABLE style
	dwStyle |= CBS_OWNERDRAWVARIABLE;

	// Use default strings. We need the itemdata to store the state of the lamps
	dwStyle |= CBS_HASSTRINGS;	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:26 PM
// Function name	: CBitComboBox::OnDestroy
// Description	    : Releases the sub classed control
// Return type		: void 
//----------------------------------------------------------------------
void CBitComboBox::OnDestroy() 
{
	if (m_listbox.GetSafeHwnd() != NULL)
		m_listbox.UnsubclassWindow();

	CComboBox::OnDestroy();	
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:24 PM
// Function name	: CBitComboBox::DrawItem
// Description	    : Draws each item selected and unselected
// Return type		: void 
// Argument         : LPDRAWITEMSTRUCT lpDrawItemStruct
//----------------------------------------------------------------------
void CBitComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	//TRACE("DrawItem()\n");
	if( lpDrawItemStruct->itemID == -1 )
		return;

	CDC			pDC;	
	pDC.Attach(lpDrawItemStruct->hDC);
	CBrush pBrush;
	CString		sItem;

	if(lpDrawItemStruct->itemState & ODS_FOCUS 
		|| lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		pBrush.CreateSolidBrush(m_colBackGroundHighLight);
		pDC.FillSolidRect(&lpDrawItemStruct->rcItem, m_colBackGroundHighLight/*&pBrush*/);
		pDC.SetTextColor(m_colTextHighLight);
		Invalidate() ; //this instruction is important to refresh the ui smoothly. 20101124 by shenhui
	}
	else
	{
		pBrush.CreateSolidBrush(m_colBackGroundNormal);
		pDC.FillSolidRect(&lpDrawItemStruct->rcItem, m_colBackGroundNormal/*&pBrush*/);
		pDC.SetTextColor(m_colTextNormal);
	}

	// Copy the text of the item to a string
	this->GetLBText(lpDrawItemStruct->itemID, sItem);
	pDC.SetBkMode(TRANSPARENT);

	// Draw the text after the images left postion
	lpDrawItemStruct->rcItem.left = nListBoxTextLeft+4;
	pDC.SelectObject(m_listbox.GetFont());
	pDC.DrawText(sItem, &lpDrawItemStruct->rcItem, DT_VCENTER | DT_SINGLELINE);
}

//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:20 PM
// Function name	: CBitComboBox::SetComboBitmap
// Description	    : To save the resource IDs of images for main combo box skin
// Return type		: void 
// Argument         : UINT nIDLeft
// Argument         : UINT nIDRight
// Argument         : UINT nIDCenter
//----------------------------------------------------------------------
void CBitComboBox::SetComboBitmap(UINT nIDLeft, UINT nIDRight, UINT nIDCenter)
{
	m_nIDCenter = nIDCenter; m_nIDLeft = nIDLeft; m_nIDRight = nIDRight;
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:18:08 PM
// Function name	: CBitComboBox::SetComboListBitmap
// Description	    : To save the resource IDs of images for list box skin
// Return type		: void 
// Argument         : UINT nIDListLeft
// Argument         : UINT nIDListRight
// Argument         : UINT nIDListTop
// Argument         : UINT nIDListBot
//----------------------------------------------------------------------
void CBitComboBox::SetComboListBitmap(UINT nIDListLeft, UINT nIDListRight, UINT nIDListTop, UINT nIDListBot)
{
	m_nIDListLeft = nIDListLeft; m_nIDListRight = nIDListRight, m_nIDListTop = nIDListTop, m_nIDListBot = nIDListBot;
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:13:57 PM
// Function name	: CBitComboBox::SetHighlightColor
// Description	    : To save the color values for text and background 
//						to draw the listbos in highlight position. Must be set before using the CBitComboBox
// Return type		: void 
// Argument         : COLORREF colBackGround
// Argument         : COLORREF colText
//----------------------------------------------------------------------
void CBitComboBox::SetHighlightColor(COLORREF colBackGround, COLORREF colText)
{
	m_colBackGroundHighLight = colBackGround; m_colTextHighLight = colText;
}


//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:13:54 PM
// Function name	: CBitComboBox::SetNormalPositionColor
// Description	    : To save the color values for text and background 
//						to draw the listbos in normal position. Must be set before using the CBitComboBox
// Return type		: void 
// Argument         : COLORREF colBackGround
// Argument         : COLORREF colText
//----------------------------------------------------------------------
void CBitComboBox::SetNormalPositionColor(COLORREF colBackGround, COLORREF colText)
{
	m_colBackGroundNormal = colBackGround; m_colTextNormal = colText;
}
