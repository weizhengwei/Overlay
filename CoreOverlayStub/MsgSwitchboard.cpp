#include "StdAfx.h"
#include "MsgSwitchboard.h"

CMsgSwitchboard g_MsgSwitchboard;

CMsgSwitchboard::CMsgSwitchboard(void)
{
}

CMsgSwitchboard::~CMsgSwitchboard(void)
{
}

LRESULT CMsgSwitchboard::OnCoreMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	core_msg_header * pHeader = (core_msg_header *)lParam;
	switch (pHeader->dwCmdId)
	{
	case CORE_MSG_CURSORCHANGE:
		{
			core_msg_cursorchange * pBody = (core_msg_cursorchange *)pHeader;
			if(pBody->lCursorId)
			{
	
				if(!g_CoreHook.m_hCurrCursor)
				{
					g_CoreHook.m_nCursorCount = g_CoreHook.SysShowCursor(TRUE) - 1;
					g_CoreHook.SysShowCursor(FALSE);
				}
				if(g_CoreHook.IsOverlayPoppingup())
				{
					if(!g_CoreHook.m_hCurrCursor)
					{
						while(g_CoreHook.SysShowCursor(TRUE) < 0);
					}
					g_CoreHook.m_hCurrCursor = LoadCursor(NULL, MAKEINTRESOURCE(pBody->lCursorId));
					::SetCursor(g_CoreHook.m_hCurrCursor);
				}
			}
			else
			{
				g_CoreHook.m_hCurrCursor = NULL;
				g_CoreHook.SysShowCursor(FALSE);

				int nCurCursorCount = g_CoreHook.SysShowCursor(TRUE) - 1;
				g_CoreHook.SysShowCursor(FALSE);
				int n = nCurCursorCount - g_CoreHook.m_nCursorCount;
				while(n > 0){
					g_CoreHook.SysShowCursor(FALSE);
					n--;
				}
				while(n < 0){
					g_CoreHook.SysShowCursor(TRUE);
					n++;
				}
			}
		}
		break;
	case CORE_MSG_OVERLAY_INIT:
		{
			core_msg_overlay_init * pBody = (core_msg_overlay_init *)pHeader;
			g_CoreHook.m_hOverlayMsgCenter = pBody->hMsgClient;
			GLOG(_T("CORE_MSG_OVERLAY_INIT:%x"), pBody->hMsgClient);
		}
		break;
	case CORE_MSG_TOKEN:
		{
			core_msg_token * pBody = (core_msg_token *)pHeader;
			if(pBody->dwClientId == g_CoreHook.GetGameID())
			{
				g_CoreHook.UpdateGameToken(pBody->csGameToken);
			}
			// launcher will query token in a single message thread, so here we notify the thread to exit
			PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
		}
		break;
	}
	return 0;
}

LRESULT CMsgSwitchboard::OnCoreUninit( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	g_CoreHook.UninitOverlay();
	return 0;
}

LRESULT CMsgSwitchboard::OnCoreGotoUrl( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled )
{
	core_msg_navigate msg;
	USES_CONVERSION;
	lstrcpynA(msg.szUrl, T2A(m_strCached), sizeof(msg.szUrl));
	g_CoreHook.m_MsgClient.SendCoreMsg(g_CoreHook.m_hOverlayMsgCenter, &msg);
	return 0;
}

LRESULT CMsgSwitchboard::OnCoreShowOverlay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	core_msg_showoverlay msg;
	msg.bShow = (BOOL)lParam;
	g_CoreHook.m_MsgClient.SendCoreMsg(g_CoreHook.m_hOverlayMsgCenter, &msg);
	return 0;
}