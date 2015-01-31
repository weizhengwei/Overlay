//----------------------------------------------------------------------------
// N O L D U S   I N F O R M A T I O N   T E C H N O L O G Y   B . V .
//----------------------------------------------------------------------------
// Filename:      BitmapPickerCombo.cpp
// Project:       EthoVision
// Module:        BitmapPicker
// Programmer:    Anneke Sicherer-Roetman
// Version:       1.00
// Revision Date: 06-10-1999
//----------------------------------------------------------------------------
// Description:   Definition of class CBitmapPickerCombo
//                See CBitmapPickerCombo.h
//----------------------------------------------------------------------------
// Acknowledgements: based on Joel Wahlberg's CIconComboBox
//                  (joel.wahlberg@enator.se)
//----------------------------------------------------------------------------
// Revision history:
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// Bugs: ........
//----------------------------------------------------------------------------
// @doc
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "skin/BitmapPickerCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
static LRESULT FAR PASCAL BitComboBoxListBoxProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	//TRACE("BitComboBoxListBoxProc 0x%.4X\n",nMsg);
	return CallWindowProc(m_pWndProc, hWnd, nMsg, wParam, lParam);
}
                   
//----------------------------------------------------------------------------
// Function DrawBitmap
// @func    draws bitmap at specified point in specified device context
// @rdesc   nothing
// @parm    const CBitmap | *bitmap | bitmap to draw
// @parm    const CDC     | *pDC    | device context to draw in
// @parm    const CPoint  | &point  | top left point of bitmap
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
static void DrawBitmap(const CBitmap *bitmap, const CDC *pDC, const CPoint &point)
{
  BITMAP bm; ((CBitmap*)bitmap)->GetBitmap(&bm);
  int w = bm.bmWidth; 
  int h = bm.bmHeight;
  CDC memDC; memDC.CreateCompatibleDC((CDC*)pDC);
  CBitmap *pBmp = memDC.SelectObject((CBitmap*)bitmap);
  ((CDC*)pDC)->BitBlt(point.x, point.y, w, h, &memDC, 0, 0, SRCCOPY);
  memDC.SelectObject(pBmp);
}

//----------------------------------------------------------------------------
// Function DrawBitmap
// @func    draws bitmap centered in specified rectangle in specified device context
// @rdesc   nothing
// @parm    const CBitmap | *bitmap | bitmap to draw
// @parm    const CDC     | *pDC    | device context to draw in
// @parm    const CRect   | &rect   | rectangle to center in
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
static void DrawBitmap(const CBitmap *bitmap, const CDC *pDC, const CRect &rect)
{
  BITMAP bm; ((CBitmap*)bitmap)->GetBitmap(&bm);
  int w = bm.bmWidth; 
  int h = bm.bmHeight;
  CPoint point;
  point.x = rect.left + ((rect.right - rect.left) / 2) - (w / 2);
  point.y = rect.top + ((rect.bottom - rect.top) / 2) - (h / 2);
  DrawBitmap(bitmap, pDC, point);
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::CBitmapPickerCombo
// @mfunc   constructor
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
CBitmapPickerCombo::CBitmapPickerCombo():
  CComboBox(),
    m_nItemWidth(0),
    m_nItemHeight(0),
	m_listrc(0,0,0,0)
{
	m_fontText.Detach( );
	m_fontText.CreateFont(-11, 0, 0, 0, FW_REGULAR, FALSE, FALSE,0,0,0,0,0,0, _T("Tahoma"));
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::AddBitmap
// @mfunc   adds bitmap (and string) item to combobox
// @rdesc   index of item (-1 on failure) (int)
// @parm    const CBitmap | *bitmap | bitmap to add
// @parm    const char    | *string | string to add (default NULL)
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
int CBitmapPickerCombo::AddBitmap(const CBitmap *bitmap, const TCHAR *string)
{
  return InsertBitmap(GetCount(), bitmap, string);
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::InsertBitmap
// @mfunc   adds bitmap (and string) item to combobox at specified index
// @rdesc   index of item (-1 on failure) (int)
// @parm    int           | nIndex  | index at which to insert
// @parm    const CBitmap | *bitmap | bitmap to add
// @parm    const char    | *string | string to add (default NULL)
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman <nl>
// after: Icon Picker Combo Box by Joel Wahlberg <nl>
// http://www.codeguru.com/combobox/icon_combobox.shtml
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
int CBitmapPickerCombo::InsertBitmap(int nIndex, const CBitmap *bitmap, const TCHAR *string)
{
  int n = CComboBox::InsertString(nIndex, string ? string : _T(""));
  if (n != CB_ERR && n != CB_ERRSPACE) {
    SetItemData(n, (DWORD)bitmap);
    BITMAP bm; ((CBitmap*)bitmap)->GetBitmap(&bm);
    SetSize(bm.bmWidth, bm.bmHeight+5);
  }
  return n;
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::MeasureItem
// @mfunc   Called by MFC when combo box is created
// @rdesc   nothing
// @parm    LPMEASUREITEMSTRUCT | lpMIS | standard parameter
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
void CBitmapPickerCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{ 
  lpMIS->itemWidth = (m_nItemWidth + 2);
  lpMIS->itemHeight = (m_nItemHeight + 2);
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::DrawItem
// @mfunc   Called by MFC when visual aspect of combo box changes 
// @rdesc   nothing
// @parm    LPDRAWITEMSTRUCT | lpDIS | standard parameter
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman <nl>
// after: Icon Picker Combo Box by Joel Wahlberg <nl>
// http://www.codeguru.com/combobox/icon_combobox.shtml
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
void CBitmapPickerCombo::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
  CDC* pDC = CDC::FromHandle(lpDIS->hDC);

  if (!IsWindowEnabled()) {
    CBrush brDisabled(RGB(33,33,33)); // light gray
    CBrush* pOldBrush = pDC->SelectObject(&brDisabled);
    CPen penDisabled(PS_SOLID, 1, RGB(33,33,33));
    CPen* pOldPen = pDC->SelectObject(&penDisabled);
	OutputBitmap(lpDIS, false);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
    return;
  }

  // Selected
  if ((lpDIS->itemState & ODS_SELECTED) 
    && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
    CBrush brHighlight(RGB(55,55,55)); 
    CBrush* pOldBrush = pDC->SelectObject(&brHighlight);
    CPen penHighlight(PS_SOLID, 1, RGB(55,55,55));
    CPen* pOldPen = pDC->SelectObject(&penHighlight);
    pDC->Rectangle(&lpDIS->rcItem);
    pDC->SetBkColor(RGB(55,55,55));
    pDC->SetTextColor(RGB(223,223,223));
	lpDIS->rcItem.left += 1 ;
	OutputBitmap(lpDIS, true);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
  }

  // De-Selected
  if (!(lpDIS->itemState & ODS_SELECTED) 
    && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
    CBrush brWindow(RGB(33,33,33)); 
    CBrush* pOldBrush = pDC->SelectObject(&brWindow);
    CPen penHighlight(PS_SOLID, 1, RGB(33,33,33));
    CPen* pOldPen = pDC->SelectObject(&penHighlight);
    pDC->Rectangle(&lpDIS->rcItem);
    pDC->SetBkColor(RGB(33,33,33));
    pDC->SetTextColor(RGB(223,223,223));
	lpDIS->rcItem.left += 1 ;
    OutputBitmap(lpDIS, false);
    pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
  }

  // Focus
  if (lpDIS->itemAction & ODA_FOCUS) 
    pDC->DrawFocusRect(&lpDIS->rcItem);
}

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::OutputBitmap
// @mfunc   draws bitmap (and string) in item
// @rdesc   nothing
// @parm    LPDRAWITEMSTRUCT | lpDIS    | item data
// @parm    bool             | selected | is the item selected?
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman <nl>
// after: Icon Picker Combo Box by Joel Wahlberg <nl>
// http://www.codeguru.com/combobox/icon_combobox.shtml
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
void CBitmapPickerCombo::OutputBitmap(LPDRAWITEMSTRUCT lpDIS, bool selected)
{
  const CBitmap *bitmap = (const CBitmap*)(lpDIS->itemData);
  if (bitmap && bitmap != (const CBitmap *)(0xffffffff)) {
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    CString string; 
    if (lpDIS->itemID != -1) 
      GetLBText(lpDIS->itemID, string); 
    if (string.IsEmpty()) 
		DrawBitmap(bitmap, pDC, lpDIS->rcItem);
    else {
      CPoint point;
      point.x = lpDIS->rcItem.left + 2;
      point.y = lpDIS->rcItem.top + ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) / 2) - (m_nItemHeight / 2); 
      DrawBitmap(bitmap, pDC, point);
      CRect rcText(lpDIS->rcItem); 
	  rcText.top -= 3 ;
	  rcText.bottom -= 3 ;
      rcText.DeflateRect(m_nItemWidth + 4, 0, 0, 0);
      pDC->DrawText(string, rcText, DT_SINGLELINE |DT_VCENTER ); 
    }
  }
}               

//----------------------------------------------------------------------------
// Function CBitmapPickerCombo::SetSize
// @mfunc   sets overall item size
// @rdesc   nothing
// @parm    int | width  | width of item
// @parm    int | height | height of item
// @xref    <c CBitmapPickerCombo>
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------
void CBitmapPickerCombo::SetSize(int width, int height)
{
  if (width > m_nItemWidth)
    m_nItemWidth = width;
  if (height > m_nItemHeight)
    m_nItemHeight = height;
  for (int i = -1; i < GetCount(); i++) 
    SetItemHeight(i, m_nItemHeight + 6);
}

//----------------------------------------------------------------------------

#ifdef _DEBUG
void CBitmapPickerCombo::PreSubclassWindow() 
{
  CComboBox::PreSubclassWindow();

  // ensure some styles are set
  // modifying style here has NO effect!?!
  ASSERT(GetStyle() & CBS_DROPDOWNLIST);
  ASSERT(GetStyle() & CBS_OWNERDRAWVARIABLE);
  ASSERT(GetStyle() & CBS_HASSTRINGS);
}
#endif

//----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CBitmapPickerCombo, CComboBox)
	ON_WM_PAINT()
	ON_MESSAGE(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
END_MESSAGE_MAP()

//----------------------------------------------------------------------
// Author			: Ashok Jaiswal
// Date Time		: 3/23/2004 5:03:38 PM
// Function name	: CBitComboBox::OnCtlColorListBox
// Description	    : It is called when list box item are painted
// Return type		: LRESULT 
// Argument         : WPARAM wParam
// Argument         : LPARAM lParam
//----------------------------------------------------------------------
LRESULT CBitmapPickerCombo::OnCtlColorListBox(WPARAM wParam, LPARAM lParam) 
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

void CBitmapPickerCombo::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RECT rc;
	CDC MemDC;

	GetClientRect(&rc);
	MemDC.CreateCompatibleDC(&dc);

	// TODO: Add your message handler code here
    CRect rectframe;
    GetWindowRect(rectframe);
    rectframe.right += 4 ;  
	//rectframe.bottom -= 2 ;
    ScreenToClient(rectframe);
    dc.Draw3dRect(rectframe, RGB(88, 88, 88), RGB(88, 88,88));

	int nIndex = GetCurSel() ;
	const CBitmap *bitmap = (const CBitmap*)GetItemData(nIndex) ;

	CRect rcItem ;
	rcItem.left = rcItem.top = 1 ;
	rcItem.right = 80; rcItem.bottom = 27 ;
	static int nEnterCount ;
	if (bitmap && bitmap != (const CBitmap *)(0xffffffff)) 
	{
		CString string; 
		if (nIndex != -1) 
		{
			GetLBText(nIndex, string); 
		}

		CBrush brHighlight(RGB(55,55,55)); 
		CBrush* pOldBrush = dc.SelectObject(&brHighlight);
		CPen penHighlight(PS_SOLID, 1, RGB(55,55,55));
		CPen* pOldPen = dc.SelectObject(&penHighlight);
		CFont* pOldFont = dc.SelectObject(&m_fontText) ;
		rcItem.right += 3 ;
		dc.Rectangle(rcItem);
		dc.SetBkColor(RGB(55,55,55));
		dc.SetTextColor(RGB(223,223,223));
		if (!string.IsEmpty())
		{
			CPoint point;
			point.x = rcItem.left + 2 + 1;
			point.y = rcItem.top + ((rcItem.bottom - rcItem.top) / 2) - 7; 
			DrawBitmap(bitmap, &dc, point);
			CRect rcText ;
			rcText.left = point.x + 18;
			rcText.top  = point.y - 4;
			rcText.right = rectframe.right ;
			rcText.bottom = point.y + 14 ;
			//rcText.DeflateRect(16 + 4, 0, 0, 0);
			dc.DrawText(string, rcText, DT_SINGLELINE |DT_VCENTER ); 
		}
		dc.SelectObject(pOldBrush);
		dc.SelectObject(pOldPen);
	}

	// movedown here to avoid partially covered
	CBitmap bmpComboRight;
	BITMAP bitRight  ;
	memset((void *)&bitRight,0,sizeof(BITMAP));

	bmpComboRight.LoadBitmap(IDB_BITMAP4);
	bmpComboRight.GetBitmap(&bitRight);
	MemDC.SelectObject(bmpComboRight);
	//dc.BitBlt(rc.right-bitRight.bmWidth+2,rc.top+4,bitRight.bmWidth+4,bitRight.bmHeight+2,&MemDC,0,1,SRCCOPY);
	dc.StretchBlt(rc.right-bitRight.bmWidth+3,rc.top+2,bitRight.bmWidth,bitRight.bmHeight+2,
			&MemDC, 0, 0, bitRight.bmWidth, bitRight.bmHeight, SRCCOPY);

	if( m_listbox )
	{
		//m_listbox.GetWindowRect(&m_listrc) ;
		//m_listrc.right += 2 ;
		//m_listbox.MoveWindow(&m_listrc) ;

		//avoid white border in windows7
		CRect listrc ;
		m_listbox.GetWindowRect(&listrc) ;
		m_listbox.ScreenToClient(&listrc);
		CRgn rectRgn ;
		rectRgn.CreateRectRgn(listrc.left+2, listrc.top+2, listrc.right, listrc.bottom);
		m_listbox.SetWindowRgn(rectRgn,true);
		rectRgn.DeleteObject() ;
	}

}
