#pragma once

#include "im/TabListWnd.h"
#include "ClientOSRHandler.h"
// COverlayBrowserMainWnd dialog

class COverlayBrowserMainWnd : public CWindowImpl<COverlayBrowserMainWnd> ,public TabListImplBase
	                          
{

public:
	COverlayBrowserMainWnd();   // standard constructor
	virtual ~COverlayBrowserMainWnd();

	DECLARE_WND_CLASS(_T("COverlayBrowserMainWnd"))

	BEGIN_MSG_MAP_EX(COverlayMainWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_COREBROWSERMESSAGE,OnCoreBrowserMsg)
		MESSAGE_HANDLER(WM_COREBROWSEREVENTMESSAGE,OnCoreBrowserEvent)
		END_MSG_MAP()
public:
		CTabListWnd m_BrowserListWnd;
		 void SetSelTab (int nIndex,HWND hWnd);

protected:

	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
    void OnDestroy();
	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	
	LRESULT  OnCoreBrowserMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	LRESULT  OnCoreBrowserEvent(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	//BOOL PreTranslateMessage(MSG* pMsg);
	void OnTabChanged(ISonicBase *,LPVOID pReserve);
	void OnClose();
	void OnSize(UINT nType, CSize size);
	
	void ShowBrowserListWnd(BOOL bShow);
	void OnAddBrwoser(ISonicBase *,LPVOID);
	void DrawTab(ISonicPaint * pPaint, LPVOID);
	void OnShowList(ISonicBase *,LPVOID);
	void DeleteTab(ISonicBase *,LPVOID pReserve);
	void OnLoading(int nIndex,BOOL bLoading);
	void OnPageUp(ISonicBase *, LPVOID);
	void OnPageDown(ISonicBase* ,LPVOID);
	void DrawMainBg(ISonicPaint * pPaint, LPVOID);
	
private:
	ISonicString *m_pListbar;
	ISonicString *m_pBtnAdd;
	ISonicTab *m_pTab;
	ISonicString *m_pLoading;
	ISonicString *m_pPageUP;
	ISonicString *m_pPageDown;
};
