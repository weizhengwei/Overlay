#pragma once
#include "CoreMsgOverlay2Svr.h"
#include "CoreMsgOverlay2Browser.h"
#include "ArcShareMem.h"
#include "Render/CoreHook_Render.h"

enum
{
	DX_UNKNOWN = 0,
	DX_D3D8,
	DX_D3D9,
	DX_D3D10,
	DX_D3D11,
	DX_D3D9EX,
};

class CCoreHook
{
public:

public:
	CCoreHook(void);
	~CCoreHook(void);

	// global functions
	BOOL Init(HMODULE hInst);
    void UnInit();
	void UninitOverlay();

    DWORD GetGameID();
    DWORD GetArcProcessID();
    CString GetGameExeName();

    bool PrepareRender(DWORD dwVer, LPVOID pDeviceOrSwapChain);
    bool RenderResized();
    void RenderOverlay(DWORD dwVer, LPVOID pDeviceOrSwapChain);
	BOOL IsOverlayPoppingup();

	BOOL IsInGame();
	HMODULE GetDllHModule(){ return m_hInst; }
	void UpdateGameToken(char szGameToken[]);
	void ModifyCommandLine(LPVOID pCommandLine, BOOL bIsWideChar = FALSE);
	void TrimCommandLine(LPVOID pCommandLine, BOOL bIsWideChar = FALSE);

	BOOL HitTestOverlayWindow(POINT pt);
	int SysShowCursor(BOOL bShow);
	void InitMsgHook();

    inline int GetDXRenderWidth()
    {
        return m_nWidth;
    }
    inline int GetDXRenderHeight()
    {
        return m_nHeight;
    }

    inline CCoreMsgClient * GetMsgClient()
    { 
        return &m_MsgClient; 
    }

	// public variables
	CCoreMsgClient m_MsgClient;
	CArcShareMemMgr<overlay_sm_header> m_smMgr;

	HWND m_hArc;
	HWND m_hGame;
	HWND m_hOverlayMsgCenter;
	int m_nWidth;
	int m_nHeight;
	HCURSOR m_hCurrCursor;
	int m_nCursorCount;
    POINT m_pt;

private:
    char m_szGameToken[512];
    HMODULE m_hInst;
    static HHOOK m_sMsgHook;
	static DWORD WINAPI _ThreadMsgClient(LPVOID pParam);
	static DWORD WINAPI _ThreadCreateAndMonitorOSOverlay(LPVOID pParam);
	static LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);


    bool CreateSharedMem();

public:
    DirectXRender* m_pDXRender;
};

extern CCoreHook g_CoreHook;