#pragma once
#include <vector>
#include "CoreMsgOverlay2Svr.h"
#include "CoreMsgOverlay2Browser.h"
#include "ArcShareMem.h"

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
	typedef struct tagDXModuleInfo
	{
		HMODULE hMod;
		DWORD dwVer;
	}DXMODULE_INFO;
	typedef std::vector<DXMODULE_INFO> VEC_DXMODULE_INFO;
public:
	CCoreHook(void);
	~CCoreHook(void);

	// global functions
	CCoreMsgClient * GetMsgClient(){ return &m_MsgClient; }
	DWORD GetGameID();
	DWORD GetArcProcessID();
	CString GetGameExeName();
	int GetGameType();
	HWND GetGameWnd();
	void RenderOverlay(DWORD dwVer, LPVOID pDeviceOrSwapChain);
	BOOL Init(HMODULE hInst);
    BOOL UnInit();
	void UninitOverlay();
    void CheckAndHookDX();
	BOOL HookD3D(const DXMODULE_INFO * pInfo);
    void UnHookD3D();

	void HookIDirect3D(LPVOID pDirect3D, DWORD dwVer);
	void HookIDirect3DEx(LPVOID pDirect3D);
	void HookIDirect3DDevice(LPVOID pD3DDevice, DWORD dwVer);
	void HookIDXGISwapChain(LPVOID pSwapChain, DWORD dwVer);
	void HookIDXGIFactory(const GUID * riid, LPVOID pFactory);
	BOOL IsInGame();
	BOOL ExeIsWebBrowser(LPCTSTR szExeFileName);
	BOOL LoadOverlay(HWND hWnd, DWORD dwVer);
	HMODULE GetDllHModule(){ return m_hInst; }
	void UpdateGameToken(char szGameToken[]);
	void ModifyCommandLine(LPVOID pCommandLine, BOOL bIsWideChar = FALSE);
	void TrimCommandLine(LPVOID pCommandLine, BOOL bIsWideChar = FALSE);
	CString GetCreateProcessFileName(LPCTSTR szApplicationName, LPCTSTR szCommandLine);
    CString GetCreateProcessFileFullPath(LPCTSTR szApplicationName, LPCTSTR szCommandLine);
	BOOL IsOverlayPoppingup();
	BOOL HitTestOverlayWindow(POINT pt);
	int SysShowCursor(BOOL bShow);

    static WORD FileMachineType( LPCTSTR lpExecuteFile );
	// public variables
	CCoreMsgClient m_MsgClient;
	CArcShareMemMgr<overlay_sm_header> m_smMgr;

	LPVOID m_pOSTexture;
	LPVOID m_pSysmemTexture;
	LPVOID m_pStateBlock;
	HWND m_hArc;
	HWND m_hGame;
	HWND m_hOverlayMsgCenter;
	int m_nWidth;
	int m_nHeight;
	HCURSOR m_hCurrCursor;
	int m_nCursorCount;
    POINT m_pt;
private:
	static DWORD WINAPI _ThreadMsgClient(LPVOID pParam);
	static DWORD WINAPI _ThreadCreateAndMonitorOSOverlay(LPVOID pParam);
	char m_szGameToken[512];
	HMODULE m_hInst;
	VEC_DXMODULE_INFO m_vecHooked;
	static HHOOK m_sMsgHook;
	static LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void InitMsgHook();

    bool InitShimEngineHookFixHandler();
    void FixShimEngineHookIssues();
    void ReleaseShimEngineHookFixHandler();

    bool Get_Method_In_Ntdll();

    void Hook_Proc_With_Method_In_Ntdll();
    void UnHook_Proc_Hooked_By_Method_In_Ntdll();

    CString GetFileNameByFullPath(LPCTSTR lpFileFullPath);
};

extern CCoreHook g_CoreHook;