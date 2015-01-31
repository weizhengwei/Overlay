//----------------------------------------------------------------------------
// N O L D U S   I N F O R M A T I O N   T E C H N O L O G Y   B . V .
//----------------------------------------------------------------------------
// Filename:      BitmapPickerCombo.h
// Project:       EthoVision
// Module:        BitmapPicker
// Programmer:    Anneke Sicherer-Roetman
// Version:       1.00
// Revision Date: 06-10-1999
//----------------------------------------------------------------------------
// Description:   Declaration of class CBitmapPickerCombo
//----------------------------------------------------------------------------
// Revision history:
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// Bugs: ........
//----------------------------------------------------------------------------
// @doc
//----------------------------------------------------------------------------

#if !defined(AFX_CBITMAPPICKERCOMBO_H__8AAE34F7_7B02_11D3_A615_0060085FE616__INCLUDED_)
#define AFX_CBITMAPPICKERCOMBO_H__8AAE34F7_7B02_11D3_A615_0060085FE616__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 

//----------------------------------------------------------------------------
// @class         CBitmapPickerCombo |
//                combobox to select bitmaps from
// @base          public | CComboBox
//----------------------------------------------------------------------------
// @prog 
// Anneke Sicherer-Roetman <nl>
// after: Icon Picker Combo Box by Joel Wahlberg <nl>
// http://www.codeguru.com/combobox/icon_combobox.shtml
// @revs 
// 06-10-1999 - First implementation
//----------------------------------------------------------------------------
// @ex Here's how to use this control in a dialog |
// 1. Include CBitmapPickerCombo.cpp and CBitmapPickerCombo.h in your project
// 2. In the resource editor create a regular combobox with at least the 
//    following styles: DropList, OwnerDraw Variable, Has Strings 
// 3. Create a control member variable for the combobox in VC's classwizard
// 4. Replace CComboBox with CBitmapPickerCombo in your dialog's .h file
// 5. In OnInitDialog use the AddBitmap or InsertBitmap member functions
//    to add bitmaps (and strings) to the combobox
//----------------------------------------------------------------------------
// @todo 
//----------------------------------------------------------------------------

#include "BitListBox.h"

class CBitmapPickerCombo : public CComboBox
{
  // @access Public Member Functions and Variables
public:

  // @cmember
  // constructor
  CBitmapPickerCombo();

  // @cmember,mfunc
  // destructor
  virtual ~CBitmapPickerCombo() {}

  // @cmember
  // adds bitmap (and string) item to combobox
  int AddBitmap(const CBitmap *bitmap, const TCHAR *string = NULL);

  // @cmember
  // adds bitmap (and string) item to combobox at specified index
  int InsertBitmap(int nIndex, const CBitmap *bitmap, const TCHAR *string = NULL);

  // @access Protected Member Functions and Variables
protected:

  // @cmember
  // Called by MFC when visual aspect of combo box changes 
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

  // @cmember
  // Called by MFC when combo box is created
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

  // @cmember,mfunc
  // strings cannot be added
  virtual int AddString(LPCTSTR lpszString) { return -1; }

  // @cmember,mfunc
  // strings cannot be added
  virtual int InsertString(int nIndex, LPCTSTR lpszString) { return -1; }

  // @cmember,mfunc
  // strings cannot be deleted
  virtual int DeleteString(int nIndex) { return -1; }

#ifdef _DEBUG
  // @cmember,mfunc
  // for assertion only
  virtual void PreSubclassWindow();
#endif

// @access Private Member Functions and Variables
private:

  int m_nItemWidth, m_nItemHeight; // @cmember size of items

  // @cmember
  // draws bitmap (and string) in item
  void OutputBitmap(LPDRAWITEMSTRUCT lpDIS, bool selected);

  // @cmember
  // sets overall item size
  void SetSize(int width, int height);
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg LRESULT OnCtlColorListBox(WPARAM wParam, LPARAM lParam);

	CFont m_fontText ;

	CBitListBox   m_listbox;
	CRect         m_listrc ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CBITMAPPICKERCOMBO_H__8AAE34F7_7B02_11D3_A615_0060085FE616__INCLUDED_)
