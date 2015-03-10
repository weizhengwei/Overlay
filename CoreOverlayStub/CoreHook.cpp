#include "stdafx.h"
#include "CoreHook.h"
#include "MsgSwitchboard.h"
#include "dbghelp.h"
#include "constant.h"
#include "CDXWrapper.h"
#include "psapi.h"
#include <string>
#include "CoreHook_WinApi.h"
#include "CoreHook_DirectX.h"
#include "CoreHook_Imp.h"


#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")


CCoreHook g_CoreHook;

extern Hook g_Hook;
//extern DirectXRender g_Render;
extern FUNC_GetCursorPos g_pGetCursorPos;
extern FUNC_ShowCursor g_pShowCursor;


DWORD WINAPI InitializeHookDll(PVOID params)
{
    g_Hook.HookD3D();
    return 0;
}

// visit url
BOOL GotoUrlInOverlay(LPCTSTR szUrl)
{
	if(!g_CoreHook.GetGameID())
	{
		// not start from Arc, disable this function    
		return FALSE;
	}
	if(!IsWindow(g_CoreHook.m_hOverlayMsgCenter))
	{
		return FALSE;
	}
	g_MsgSwitchboard.m_strCached = szUrl;
	g_MsgSwitchboard.PostMessage(WM_COREGOTOURL);
	return TRUE;
}

BOOL ShowOverlay(BOOL bShow)
{
	if(!g_CoreHook.GetGameID())
	{
		// not start from Arc, disable this function
		return FALSE;
	}
	if(!IsWindow(g_CoreHook.m_hOverlayMsgCenter))
	{
		return FALSE;
	}
	g_MsgSwitchboard.PostMessage(WM_CORESHOWOVERLAY, 0, (LPARAM)bShow);
	return TRUE;
}


// implementations of CCoreHook
HHOOK CCoreHook::m_sMsgHook = NULL;

CCoreHook::CCoreHook(void)
{
	m_hArc = NULL;
	m_hGame = NULL;
	m_hOverlayMsgCenter = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_hInst = NULL;
	m_hCurrCursor = NULL;
	memset(m_szGameToken, 0, sizeof(m_szGameToken));

    m_pt.x = m_pt.y = -1;
    m_pDXRender = NULL;
}

CCoreHook::~CCoreHook(void)
{
}

BOOL CCoreHook::Init(HMODULE hInst)
{  
	m_hInst = hInst;
    //MessageBox(0,0,0,0);
	// ensure single instance
    TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, Tsizeof(szPath));
	TCHAR szInstallPath[MAX_PATH], szFileName[MAX_PATH];
	if(GetEnvironmentVariable(_T("ArcGameInstallPath"), szInstallPath, Tsizeof(szInstallPath)))
	{
		lstrcpy(szFileName, szPath);
		PathStripPath(szFileName);
		if(StrStrI(szPath, szInstallPath) || StrStrI(szFileName, _T("launcher")))
		{
			// we have to filter the string "launcher" here because RH is an exception that all its launcher processes start from system disk
			// only the processes started from the install directory create the named event for single instance
			CString strSingleTon;
			strSingleTon.Format(_T("arc_%d_%d"), GetArcProcessID(), GetGameID());
			CreateEvent(NULL, FALSE, FALSE, strSingleTon);
		}
	}

	// check if the overlay should work
	if(IsInGame())
	{
		// create an event to mark this process as a game, for Arc
		CString strGameEvent;
		strGameEvent.Format(_T("arc_%d_%d_game"), GetArcProcessID(), GetGameID());
		CreateEvent(NULL, FALSE, FALSE, strGameEvent);

		TCHAR szVal[64];
		BOOL bEnableOverlay = TRUE;
		if(GetEnvironmentVariable(_T("ArcEnableOverlay"), szVal, Tsizeof(szVal)))
		{
			bEnableOverlay = _ttoi(szVal);
		}
		if(!bEnableOverlay)
		{
			return FALSE;
		}
	}
	

    Hook::HookCreateProcess();
    //Hook::HookLoadLibrary();
	CreateThread(NULL, 0, InitializeHookDll, NULL, NULL, NULL);//spawn a thread that will start running when the dll init exits

	return TRUE;
}



void CCoreHook::UnInit()
{
    // UNhook kernel32 functions
    //g_Hook.UnHookD3D();

    //Hook::UnHookLoadLibrary();

    Hook::UnHookUser32Method();

    Hook::UnHookCreateProcess();

}



void CCoreHook::UninitOverlay()
{
	GLOG(_T(""));   


//     if (NULL != m_pDXRender)
//     {
//         delete m_pDXRender;
//         m_pDXRender = NULL;
//     }
    m_pDXRender->Uninit();

}

LRESULT CCoreHook::GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    static bool first = true;
    if (first)
    {
        first = false;
        // in game process, create the message client in the same thread with game window to simplify the thread model
        _ThreadMsgClient(NULL);

        // first time to render overlay, here we already get the valid width and height of the surface, let's create the off-screen overlay process
        //dwRenderThreadId = GetCurrentThreadId();
        CreateThread(NULL, 0, _ThreadCreateAndMonitorOSOverlay, NULL, 0, NULL);
    }


	if(nCode >= 0)
	{
		LPMSG pMsg = (LPMSG)lParam;
		if(wParam == PM_REMOVE)
		{
			if(pMsg->message >= WM_NCMOUSEMOVE && pMsg->message <= WM_NCMBUTTONDBLCLK)
			{
				if(g_CoreHook.IsOverlayPoppingup())
				{
					pMsg->message = WM_NULL;
				}
			}
			if(pMsg->hwnd == g_CoreHook.m_hGame && pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
			{
				if(g_CoreHook.m_smMgr.GetHeader()->bHasImage)
				{
					// now is rendering overlay
					POINT pt = {LOSHORT(pMsg->lParam), HISHORT(pMsg->lParam)};
					if(pMsg->message == WM_MOUSEWHEEL)
					{
						ScreenToClient(pMsg->hwnd, &pt);
					}
					// we should test the alpha channel of pixel at the position of cursor to check if the cursor is now above some overlay windows
					// if the alpha is not 0, this mouse message should be relayed to overlay and eaten
					if(g_CoreHook.HitTestOverlayWindow(pt))
					{
						// some games don't have the class style of CS_DBLCLKS, so we have to emulate a double-click event
						if(!(GetClassLong(pMsg->hwnd, GCL_STYLE) & CS_DBLCLKS))
						{
							if(pMsg->message == WM_LBUTTONUP)
							{
								static DWORD dwLastClickTick = 0;
								DWORD dwNow = GetTickCount();
								if(dwNow - dwLastClickTick < 300)
								{
									pMsg->message = WM_LBUTTONDBLCLK;
									dwLastClickTick = 0;
								}
								else
								{
									dwLastClickTick = dwNow;
								}
							}   
						}
						::PostMessage(g_CoreHook.m_smMgr.GetHeader()->hOSPanel, pMsg->message, pMsg->wParam, pMsg->lParam);
						pMsg->message = WM_NULL;
					}
				}
			}
			if((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
				|| (pMsg->message >= WM_SYSKEYDOWN && pMsg->message <= WM_SYSDEADCHAR))
			{
				if(pMsg->message == WM_KEYDOWN && g_CoreHook.IsOverlayPoppingup())
				{
					TranslateMessage(pMsg); 
				}
				::SendMessage(g_CoreHook.m_smMgr.GetHeader()->hOSPanel, pMsg->message, pMsg->wParam, pMsg->lParam);
				if(g_CoreHook.IsOverlayPoppingup())
				{
                    // show overlay suc [12/30/2014 liuyu]
                    if (g_CoreHook.m_pt.x == -1 || g_CoreHook.m_pt.y == -1)
                    {
                        g_pGetCursorPos(&g_CoreHook.m_pt);
                    }
					pMsg->message = WM_NULL;
				}
                else if(g_CoreHook.m_pt.x != -1 && g_CoreHook.m_pt.y != -1)
                {
                    SetCursorPos(g_CoreHook.m_pt.x,g_CoreHook.m_pt.y);
                    g_CoreHook.m_pt.x = -1;g_CoreHook.m_pt.y = -1;  
                }
			}
			if(pMsg->message == WM_NULL)
			{
				return 0;
			}
		}
	}
	return CallNextHookEx(m_sMsgHook, nCode, wParam, lParam);
}

void CCoreHook::InitMsgHook()
{
	if(m_sMsgHook)
	{
		UnhookWindowsHookEx(m_sMsgHook);
		m_sMsgHook = NULL;
	}
	m_sMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, NULL, /*GetCurrentThreadId()*/GetWindowThreadProcessId(m_hGame, NULL));
    //GLOG(_T("SetWindowsHookEx: %d"), GetWindowThreadProcessId(m_hGame, NULL));

}



DWORD CCoreHook::GetGameID()
{
	TCHAR szGameId[64];
	if(!GetEnvironmentVariable(_T("CoreGameId"), szGameId, 64))
	{
		return 0;
	}
	return _ttoi(szGameId);
}

void CCoreHook::UpdateGameToken( char szGameToken[] )
{
	memcpy(m_szGameToken, szGameToken, sizeof(m_szGameToken));
}

void TrimCommandLineA(LPSTR pCommandLine, LPSTR sToken, LPSTR param)
{
	LPSTR pStartCmd, pEndCmd, pFindInToken;
	pStartCmd = StrStrA(pCommandLine, param);
	pFindInToken = StrStrA(sToken, param);
	if(pStartCmd && pFindInToken)
	{
		pEndCmd = pStartCmd + lstrlenA(param);
		while(pEndCmd)
		{
			if(pEndCmd[0] == ' ' || pEndCmd[0] == 0)
			{
				break;
			}
			pEndCmd++;
		}
		if(pEndCmd[0] == ' ')
		{
			lstrcpyA(pStartCmd, pEndCmd+1);
		}
		if(pEndCmd[0] == 0)
		{
			pStartCmd[0] = 0;
		}
	}
}

void TrimCommandLineW(LPWSTR pCommandLine, LPWSTR sToken, LPWSTR param)
{
	LPWSTR pStartCmd, pEndCmd, pFindInToken;
	pStartCmd = StrStrW(pCommandLine, param);
	pFindInToken = StrStrW(sToken, param);
	if(pStartCmd && pFindInToken)
	{
		pEndCmd = pStartCmd + lstrlenW(param);
		while(pEndCmd)
		{
			if(pEndCmd[0] == L' ' || pEndCmd[0] == 0)
			{
				break;
			}
			pEndCmd++;
		}
		if(pEndCmd[0] == L' ')
		{
			lstrcpyW(pStartCmd, pEndCmd+1);
		}
		if(pEndCmd[0] == 0)
		{
			pStartCmd[0] = 0;
		}
	}
}

void CCoreHook::TrimCommandLine( LPVOID pCommandLine, BOOL bIsWideChar /*= FALSE*/ )
{
	if(bIsWideChar)
	{
		USES_CONVERSION;
		LPWSTR pCmdLine = (LPWSTR)pCommandLine;
		if(pCmdLine[0])
		{
			LPWSTR pGameToken = A2W(m_szGameToken);
			LPWSTR pSearchStr = L"coreclient:1 arc:1";
			TrimCommandLineW(pCmdLine, pGameToken, pSearchStr);

			pSearchStr = L"user:";
			TrimCommandLineW(pCmdLine, pGameToken, pSearchStr);
			pSearchStr = L"token:";
			TrimCommandLineW(pCmdLine, pGameToken, pSearchStr);
		}
	}
	else
	{
		LPSTR pCmdLine = (LPSTR)pCommandLine;
		if(pCmdLine[0])
		{
			LPSTR pGameToken = m_szGameToken;
			LPSTR pSearchStr = "coreclient:1 arc:1";
			TrimCommandLineA(pCmdLine, pGameToken, pSearchStr);

			pSearchStr = "user:";
			TrimCommandLineA(pCmdLine, pGameToken, pSearchStr);
			pSearchStr = "token:";
			TrimCommandLineA(pCmdLine, pGameToken, pSearchStr);
		}
	}
}

void CCoreHook::ModifyCommandLine( LPVOID pCommandLine, BOOL bIsWideChar /*= FALSE*/ )
{

	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, _ThreadMsgClient, (LPVOID)(INT_PTR)TRUE, 0, &dwThreadId);

	// wait for CoreClient to get remote token
	if(WaitForSingleObject(hThread, 4000) == WAIT_OBJECT_0 && m_szGameToken[0])
	{
		TrimCommandLine(pCommandLine, bIsWideChar);
		USES_CONVERSION;
		if (bIsWideChar)
		{			
			LPWSTR pCmdLine = (LPWSTR)pCommandLine;
			lstrcatW(pCmdLine, L" ");
			lstrcatW(pCmdLine, A2W(m_szGameToken));
			//OutputDebugStringW(pCmdLine);
		}
		else
		{
			LPSTR pCmdLine = (LPSTR)pCommandLine;
			lstrcatA(pCmdLine, " ");
			lstrcatA(pCmdLine, m_szGameToken);
			//OutputDebugStringA(pCmdLine);
		}
	}
	else
	{
		GLOG(_T("Get Token Failed!!!!!!"));
	}
	memset(m_szGameToken, 0, sizeof(m_szGameToken));
}

DWORD WINAPI CCoreHook::_ThreadMsgClient( LPVOID pParam )
{
	BOOL bMsgLoop = (BOOL)(INT_PTR)pParam;
	// create message client and switchboard
	g_MsgSwitchboard.Create(HWND_MESSAGE);
	g_CoreHook.m_MsgClient.Initialize(g_MsgSwitchboard, g_CoreHook.GetGameID(), MSG_SERVER_MAIN);

	// notify the server to create or renew the hwnd binded to client id
	if(!g_CoreHook.m_MsgClient.Connect())
	{
		//g_CoreHook.m_MsgClient.Uninitialize();
	}
	if(bMsgLoop && g_CoreHook.m_MsgClient.IsValid())
	{
		// this loop will only run in launcher
		// query token
		core_msg_header pack = {0};
		pack.dwSize = sizeof(core_msg_header);
		pack.dwCmdId = CORE_MSG_QUERYTOKEN;
		g_CoreHook.m_MsgClient.SendCoreMsg(&pack);
		if(pack.dwRet)
		{
			// no need to wait for the remote token, the server won't send it any more
			return 0;
		}

		MSG msg;
		while(GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

void CCoreHook::RenderOverlay(DWORD dwVer, LPVOID pDeviceOrSwapChain)
{
    if( PrepareRender(dwVer, pDeviceOrSwapChain))  
        m_pDXRender->Render(RenderResized());
}



BOOL CCoreHook::IsInGame()
{
	static BOOL bIsInGame = -1;
	if(bIsInGame < 0)
	{
		TCHAR szPath[MAX_PATH];
		GetModuleFileName(NULL, szPath, MAX_PATH);
		PathStripPath(szPath);
		bIsInGame = (GetGameExeName().CompareNoCase(szPath) == 0);
	}
	return bIsInGame;
}

CString CCoreHook::GetGameExeName()
{
	TCHAR szExeName[128];
	if(!GetEnvironmentVariable(_T("ArcGameExeName"), szExeName, Tsizeof(szExeName)))
	{
		return _T("");
	}
	return szExeName;
}


DWORD CCoreHook::GetArcProcessID()
{
	TCHAR szGameId[64];
	if(!GetEnvironmentVariable(_T("ArcProcessId"), szGameId, 64))
	{
		return 0;
	}
	return _ttoi(szGameId);
}

DWORD WINAPI CCoreHook::_ThreadCreateAndMonitorOSOverlay( LPVOID pParam )
{
	while(TRUE)
	{
		{
			CArcAutoLock lock;
			g_CoreHook.m_smMgr.GetHeader()->bHasImage = FALSE;
			g_CoreHook.m_smMgr.GetHeader()->bUpdate = FALSE;
            g_CoreHook.m_smMgr.GetHeader()->bFullScreen = TRUE;
		}
		TCHAR szPath[MAX_PATH], szCmd[MAX_PATH];
		_stprintf(szCmd, _T("%d"), g_CoreHook.m_MsgClient.GetSelfWindow());
		GetModuleFileName(g_CoreHook.m_hInst, szPath, MAX_PATH);
		PathRemoveFileSpec(szPath);
		PathAppend(szPath, FILE_EXE_OSOVERLAY);
		STARTUPINFO si = {0};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {0};
		if(!CreateProcess(szPath, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			break;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return 0;
}

BOOL CCoreHook::IsOverlayPoppingup()
{
	return ::IsWindowVisible(m_smMgr.GetHeader()->hOSPanel);
}

BOOL CCoreHook::HitTestOverlayWindow( POINT pt )
{
	if(pt.x < 0 || pt.x >= m_nWidth)
	{
		return FALSE;
	}
	if(pt.y < 0 || pt.y >= m_nHeight)
	{
		return FALSE;
	}
	CArcAutoLock lock;
	LPDWORD pBuff = (LPDWORD)m_smMgr.GetBits();
	DWORD dwColor = *(pBuff + pt.y * m_nWidth + pt.x);
	if(dwColor & 0xff000000)
	{
		return TRUE;
	}
	return FALSE;
}

int CCoreHook::SysShowCursor( BOOL bShow )
{   OutputDebugStringA("-------------------");    
	if(!g_pShowCursor)
	{
		return 0;
	}
	return (*g_pShowCursor)(bShow);
}



bool CCoreHook::CreateSharedMem()
{
    if (m_smMgr.IsValid())
        return false;

    CString strShareMemName;
    strShareMemName.Format(_T("arc_%d_%d_sm"), GetGameID(), GetCurrentProcessId());
    if(!m_smMgr.Create(strShareMemName, 2048 * 2048 * 4))
        return false;

    SetEnvironmentVariable(_T("ShareMemName"), strShareMemName);
    return true;
}

bool CCoreHook::PrepareRender( DWORD dwVer, LPVOID pDeviceOrSwapChain )
{    
    if (NULL != m_pDXRender)
    {
        if (m_pDXRender->m_pDevice == pDeviceOrSwapChain)
        {
            return TRUE;
        }
        else
        {
            delete m_pDXRender;
            m_pDXRender = NULL;
            GLOG(_T("device has changed, delete render"));
        }
    }
   
    // first time render [3/2/2015 liuyu]
    m_pDXRender = DirectXRenderFactory::CreateDirectXRender(dwVer, pDeviceOrSwapChain);
    if (NULL == m_pDXRender)
    {
        GLOG(_T("CreateDirectXRender failed, dwVer:%d"), dwVer);
        return FALSE;
    }

    m_hGame = m_pDXRender->GetRenderHwnd();
  
    if(GetCurrentThreadId() != GetWindowThreadProcessId(m_hGame, NULL))
    {
        GLOG(_T("GetCurrentThreadId %d,  GetWindowThreadProcessId %d"), GetCurrentThreadId() ,GetWindowThreadProcessId(m_hGame, NULL));
    }

    // create share memory 
    if (!CreateSharedMem())
    {
        return FALSE;
    }

    // hook user32 functions
    Hook::HookUser32Method();

    // install message hook
    InitMsgHook();

    return TRUE;

}

bool CCoreHook::RenderResized()
{
    m_nWidth = m_pDXRender->GetRenderWidth();
    m_nHeight = m_pDXRender->GetRenderHeight();

    //to-do:
    //m_smMgr.GetHeader()->bFullScreen = TRUE/*!m_pDXRender->RenderWndWindowed()*/;

    bool bResize = false;
    // first check if the size of off-screen surface should be resized to fit the render target
    overlay_sm_header * pHeader = m_smMgr.GetHeader();
    if(m_nWidth != pHeader->nWidth || m_nHeight != pHeader->nHeight)
    {
        // size was changed, notify the off-screen overlay to resize and repaint
        CArcAutoLock lock;
        bResize = TRUE;
        pHeader->nWidth = m_nWidth;
        pHeader->nHeight = m_nHeight;
        pHeader->hGame = m_hGame;
        pHeader->uStartTime = (UINT)time(NULL);
        GLOG(_T("resize:width:%d, height:%d, fullscreen:%d"), m_nWidth, m_nHeight, pHeader->bFullScreen);

        // the size is changed, notify the off-screen overlay to change its size and repaint
        if(IsWindow(m_hOverlayMsgCenter))
        {
            core_msg_resize msg;
            msg.nWidth = m_nWidth;
            msg.nHeight = m_nHeight;
            m_MsgClient.SendCoreMsg(m_hOverlayMsgCenter, &msg);
        }
    }


    if(!IsWindow(pHeader->hOSPanel))
    {
        CArcAutoLock lock;
        pHeader->bHasImage = FALSE;
        pHeader->bUpdate = FALSE;
    }

    return bResize;
}





