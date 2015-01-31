#pragma once
#include "corehook.h"

#define WM_COREUNINIT				WM_USER + 0x428
#define WM_COREGOTOURL				WM_USER + 0x429
#define WM_CORESHOWOVERLAY			WM_USER + 0x42a

class CMsgSwitchboard : public CWindowImpl<CMsgSwitchboard>
{
public:
	DECLARE_WND_CLASS(_T("Arcgamemsgswitchboard"))
	CMsgSwitchboard(void);
	~CMsgSwitchboard(void);
	BEGIN_MSG_MAP(CMsgSwitchboard)
		MESSAGE_HANDLER(WM_COREMESSAGE, OnCoreMessage)
		MESSAGE_HANDLER(WM_COREUNINIT, OnCoreUninit)
		MESSAGE_HANDLER(WM_COREGOTOURL, OnCoreGotoUrl)
		MESSAGE_HANDLER(WM_CORESHOWOVERLAY, OnCoreShowOverlay)
	END_MSG_MAP()

	LRESULT OnCoreMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnCoreUninit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnCoreGotoUrl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnCoreShowOverlay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
public:
	CString m_strCached;
};

extern CMsgSwitchboard g_MsgSwitchboard;
