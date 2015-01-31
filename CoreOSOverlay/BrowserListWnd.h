#pragma once
using WTL::CCoreListBox;
#define IDC_LISTBOX_OVERLAY_PAGELIST 2005

#define  LISTMAXHEIGHT 400
#define  LISTWIDTH 176
#define  LISTITEMHEIGHT 15
// CBrowserListWnd dialog


class CBrowserListWnd : public CWindowImpl<CBrowserListWnd>
{
	

public:
	
	CBrowserListWnd();   // standard constructor
	virtual ~CBrowserListWnd();

	DECLARE_WND_CLASS(_T("CBrowserListWnd"))

	BEGIN_MSG_MAP_EX(COverlayMainWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_ACTIVATE(OnActivate)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_HANDLER_EX(IDC_LISTBOX_OVERLAY_PAGELIST,LBN_SELCHANGE,OnListSelChange)
	//	NOTIFY_HANDLER(IDC_LISTBOX_OVERLAY_PAGELIST,LVN_ITEMCHANGED,OnListSelChange)
   END_MSG_MAP()

    void OnDestroy();
	void AddListItem(int nIndex,LPCTSTR lpszItem);
	CCoreListBox*GetCoreListBox(){return m_pListBox;}
	void SelectItem(int nIndex);
	void DeleteItem(int nIndex);
	void SetItemText(int nIndex,LPCTSTR lpctstr);
	void SetListFocus(){m_pListBox->SendMessage(WM_SETFOCUS);/*m_pListBox->SetFocus();*/}
	
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnListSelChange(UINT uNotifyCode, int nID, CWindow wndCtl);
	//LRESULT OnListSelChange(WPARAM,LPARAM);
	void OnSize(UINT nType, CSize size);
	void OnKillFocus(CWindow wndFocus);
    void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther);
	void DrawItem(ISonicPaint * pPaint, LPVOID) ;
private:
	CCoreListBox *m_pListBox;
	ISonicString* m_pSelect;
};
