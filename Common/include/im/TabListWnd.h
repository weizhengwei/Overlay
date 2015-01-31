#pragma once
#include "atlbase.h"
#include "ui/wtl/atlapp.h"
#include "ui/wtl/atlcrack.h"
#include "ui/wtl/atlmisc.h"
#include "ui/CoreWidget.h"
#include "ui/ICoreUI.h"
#include "tstring/tstring.h"
using String::_tstring;
using WTL::CCoreListBox;

#define IDC_TAB_LIST 2005

#define  LISTMAXHEIGHT 254
#define  LISTWIDTH 90
#define  LISTITEMHEIGHT 23
// CTabListWnd dialog


class TabListImplBase
{
public:
	virtual void SetSelTab (int nIndex,HWND hwnd = NULL) = 0;
};
class CTabListWnd : public CWindowImpl<CTabListWnd>
{
public:
	
	CTabListWnd(TabListImplBase * pTabListImpl );   // standard constructor
	virtual ~CTabListWnd();

	DECLARE_WND_CLASS(_T("CTabListWnd"))

	BEGIN_MSG_MAP_EX(CTabListWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_ACTIVATE(OnActivate)
		MSG_WM_DESTROY(OnDestroy)
		//MSG_WM_SETFOCUS(OnSetFocus)
		COMMAND_HANDLER_EX(IDC_TAB_LIST,LBN_SELCHANGE,OnListSelChange)
		COMMAND_HANDLER_EX(IDC_TAB_LIST,LBN_KILLFOCUS,OnListKillFocus)
		//COMMAND_HANDLER_EX(IDC_TAB_LIST,EN_SETFOCUS,OnListSetFocus)
   END_MSG_MAP()

    void OnDestroy();
	void AddListItem(int nIndex,LPCTSTR lpszItem);
	CCoreListBox*GetCoreListBox(){return m_pListBox;}
	void SelectItem(int nIndex);
	void DeleteItem(int nIndex);
	void Clear();
	void SetItemText(int nIndex,LPCTSTR lpctstr);
	void SetListFocus(){m_pListBox->SendMessage(WM_SETFOCUS);/*m_pListBox->SetFocus();*/}
	void SetListBoxMargin(int nWidthMargin, int nHeightMargin){m_nListBoxWidthMargin = nWidthMargin;m_nListBoxHeightMargin = nHeightMargin;}
	void GetListBoxMargin(int &nWidthMargin,int &nHieghtMargin){nWidthMargin = m_nListBoxWidthMargin; nHieghtMargin = m_nListBoxHeightMargin;}
	void SetListItemHeight(int nHeight){m_nListItemHeight = nHeight;}
	int  GetListItemHeight(){return m_nListItemHeight;}
	void SetListItemTextWidth(int nWidth){m_nItemTextWidth = nWidth ;}
	int  GetListItemTextWidth(){return m_nItemTextWidth;}
	void SetFontFormat(_tstring &strFontFormat){ m_strFontFormat = strFontFormat;}
	void SetTextPoint(POINT & point){m_TextPoint = point;}


protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnListSelChange(UINT uNotifyCode, int nID, CWindow wndCtl);
	//LRESULT OnListSelChange(WPARAM,LPARAM);
	void OnSize(UINT nType, WTL::CSize size);
	void OnKillFocus(CWindow wndFocus);
	void OnSetFocus(CWindow wndOld);
    void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther);
	void OnListKillFocus(UINT uNotifyCode, int nID, CWindow wndCtl);
	CCoreListBox *m_pListBox;
private:
	
	TabListImplBase * m_pTabListImpl;
	int m_nListBoxWidthMargin;
	int m_nListBoxHeightMargin;
	int m_nListItemHeight;
	int m_nItemTextWidth;
	_tstring m_strFontFormat;
	POINT m_TextPoint;
};
