#pragma once

#include "ClientOSRHandler.h"

// COverlayBrowserWnd dialog

class COverlayBrowserWnd : public CWindowImpl<COverlayBrowserWnd>
{
	
public:
	COverlayBrowserWnd(CefRefPtr<ClientOSRHandler> Handler);   // standard constructor
	virtual ~COverlayBrowserWnd();
	DECLARE_WND_CLASS(_T("COverlayBrowserWnd"))

	BEGIN_MSG_MAP_EX(COverlayBrowserWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnInputMessages)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnInputMessages)
		MESSAGE_HANDLER(WM_COREBROWSERMESSAGE,OnBrowserMsg)
		MESSAGE_HANDLER(WM_EDIT_VK_RETURN_KEYDOW,OnEditReturnKeyDown)
		END_MSG_MAP()

	void SetHandler(CefRefPtr<ClientOSRHandler> Handler){ m_BrowserHandler = Handler;}

protected:
	LRESULT OnInputMessages(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnKillFocus(CWindow wndFocus);
	void OnBackClick(ISonicBase *, LPVOID pReserve);
	void OnForwardClick(ISonicBase *, LPVOID pReserve);
	void OnRefreshClick(ISonicBase *, LPVOID pReserve);
	void OnDeleteClick(ISonicBase *, LPVOID pReserve);
	void OnHomeClick(ISonicBase *, LPVOID pReserve);
	void InitEvent();
	void BtnChange();
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnPaint(CDCHandle dc);
	void OnMouseLeave();

	LRESULT OnBrowserMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	LRESULT OnEditReturnKeyDown(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL PtInBrowserRect(CPoint pt);
	//BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void OnSize(UINT nType, CSize size);
	virtual void OnFinalMessage(HWND hWnd);
	void DrawEditBg(ISonicPaint * pPaint, LPVOID);
	void SetBrowserEvent(UINT message, WPARAM wParam,LPARAM lParam);
	static void CloseBrowser(ClientOSRHandler* pBrowserHandler);
public:
	CefRefPtr<ClientOSRHandler> m_BrowserHandler;
private:
	
	BOOL m_bBrowserFocus;
	WTL::CCoreEdit*	m_pCoreEdit;
	ISonicString *m_pForward;
	ISonicString *m_pBack;
	BOOL m_bMouseIn;
	ISonicString *m_pHome;

};
