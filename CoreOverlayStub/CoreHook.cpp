#include "stdafx.h"
#include "CoreHook.h"
#include "MsgSwitchboard.h"
#include "dbghelp.h"
#include "constant.h"
#include "CDXWrapper.h"
#include "SSEFunc.h"
#pragma comment(lib, "dbghelp.lib")
#include "psapi.h"
#pragma comment(lib, "psapi.lib")
#include <winternl.h>
#include <string>
#include <TlHelp32.h>

#include "detours\detours.h"

#define FAKE_CMDLINE_LENGTH		4096

BYTE _byCreateSwapChainCode[] = {0x8b, 0x07, 0x8d, 0x4d, 0x0c, 0x51};
BYTE _byCreateSwapChainCode1[] = {0x8d, 0x7c, 0x24, 0x20, 0x89};
BYTE _byCreateSwapChainCode2[] = {0x8d, 0x74, 0x24, 0x18, 0x89};
BYTE _byCreateSwapChainCode3[] = {0x89, 0x74, 0x24, 0x28, 0x89};

GUID IID_ID3D10Texture2D = {0x6f15aaf2,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};
GUID IID_ID3D11Device = {0xdb6f6ddb,0xac77,0x4e88,{0x82,0x53,0x81,0x9d,0xf9,0xbb,0xf1,0x40}};
GUID IID_IDXGIFactory1 = {0x770aae78,0xf26f,0x4dba,{0xa8,0x29,0x25,0x3c,0x83,0xd1,0xb3,0x87}};
GUID IID_IDXGIFactory2 = {0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};

LPCSTR g_strShaderCode = "Texture2D txDiffuse : register( t0 );	\n\
						 SamplerState samPoint : register( s0 );	\n\
						 struct VS_INPUT\n\
						 {\n\
						 float4 Pos : POSITION;\n\
						 float2 Tex : TEXCOORD;\n\
						 };\n\
						 struct PS_INPUT\n\
						 {\n\
						 float4 Pos : SV_POSITION;\n\
						 float2 Tex : TEXCOORD;\n\
						 };\n\
						 PS_INPUT VS( VS_INPUT input )\n\
						 {\n\
						 PS_INPUT output = (PS_INPUT)0;\n\
						 output.Pos = input.Pos;\n\
						 output.Tex = input.Tex;\n\
						 return output;	\n\
						 }\n\
						 float4 PS( PS_INPUT input) : SV_Target0\n\
						 {\n\
						 return txDiffuse.Sample( samPoint, input.Tex );\n\
						 }";
LPVOID g_pVertexShader = NULL;
LPVOID g_pPixelShader = NULL;
LPVOID g_pVertexLayout = NULL;
LPVOID g_pVertexBuffer = NULL;
LPVOID g_pTextureRV = NULL;
LPVOID g_pSamplerPoint = NULL;
LPVOID g_pBlendState = NULL;
LPVOID g_pDepthStencilState = NULL;
LPVOID g_pRasterizerState = NULL;
LPVOID g_pRenderTargetView = NULL;
LPVOID g_pDepthStencilView = NULL;

CCoreHook g_CoreHook;
DWORD g_dwDXVer = 0;
DWORD g_dwCustomRefCount = 0;

PDETOUR_CREATE_PROCESS_ROUTINEA g_pCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pCreateProcessW = NULL;
//////////////////////////////////////////////////////////////////////////
//
HMODULE GetDxModule()
{
	HMODULE hModule = NULL;
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
	if (INVALID_HANDLE_VALUE != hSnapShot)
	{ 
		MODULEENTRY32* moduleInfo = new MODULEENTRY32;
		moduleInfo->dwSize = sizeof(MODULEENTRY32);

		while (Module32Next(hSnapShot, moduleInfo) != FALSE)
		{
			std::wstring s = moduleInfo->szModule;
			if(-1 != s.find(_T("d3dx11_")))
			{
				hModule = moduleInfo->hModule;
				break;
			}

		}

		CloseHandle(hSnapShot);
		delete moduleInfo;
	}

	if (hModule == NULL)
		hModule = ::LoadLibrary(_T("d3dx11_43.dll"));
	return hModule;

}
typedef HRESULT (WINAPI * FUNC_D3DX11CompileFromMemory)(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, void* pDefines, void* pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void * pPump, void** ppShader, void** ppErrorMsgs, HRESULT* pHResult);
HRESULT WINAPI D3DX11CompileFromMemory(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, void* pDefines, void* pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void * pPump, void** ppShader, void** ppErrorMsgs, HRESULT* pHResult)
{
	static FUNC_D3DX11CompileFromMemory pFunc = NULL;
	if(!pFunc)
	{
		HMODULE hMod = GetDxModule();
		pFunc = (FUNC_D3DX11CompileFromMemory)GetProcAddress(hMod, "D3DX11CompileFromMemory");
		if(!pFunc)
		{
			pFunc = (FUNC_D3DX11CompileFromMemory)-1;
		}
	}
	if(pFunc == (FUNC_D3DX11CompileFromMemory)-1)
	{
		return -1;
	}
	return (*pFunc)(pSrcData, SrcDataLen, pFileName, pDefines, pInclude, pFunctionName, pProfile, Flags1, Flags2, pPump, ppShader, ppErrorMsgs, pHResult);
}
//////////////////////////////////////////////////////////////////////////
// export functions
BOOL WINAPI CoreCreateProcessWithDll(LPCSTR lpApplicationName, LPSTR lpCommandLine, BOOL bInheritHandles, DWORD dwCreationFlags, LPCSTR lpCurrentDirectory, LPPROCESS_INFORMATION ppi)
{
	char szPath[MAX_PATH];
	GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFOA);
	if(ppi)
	{
		return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, NULL, NULL, bInheritHandles, dwCreationFlags, NULL, lpCurrentDirectory, &si, ppi, szPath, g_pCreateProcessA);
	}
	else
	{
		PROCESS_INFORMATION pi;
		if(DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, NULL, NULL, bInheritHandles, dwCreationFlags, NULL, lpCurrentDirectory, &si, &pi, szPath, g_pCreateProcessA))
		{
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return TRUE;
		}
	}
	return FALSE;
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

//////////////////////////////////////////////////////////////////////////
// detoured functions
// switch off the complier optimization to assure standard function stack.
#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
// win32 hooked APIs
BOOL WINAPI MyCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
						  BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
						  LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = g_CoreHook.GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

	if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(A2T(lpApplicationName), A2T(lpCommandLine)))))
	{
		return (*g_pCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	else
	{
		if(g_CoreHook.ExeIsWebBrowser(strExeName))
		{
			return (*g_pCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		}
		char szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
		if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
		{
			// is creating game, modify command line for bypass login
			lstrcpynA(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
			lpCommandLine = szCmdLine;
			g_CoreHook.ModifyCommandLine(lpCommandLine);
		}
		char szPath[MAX_PATH];
		GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
		return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pCreateProcessA);
	}
}

BOOL WINAPI MyCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
						   BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPWSTR lpCurrentDirectory, 
						   LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    CString strExeName = g_CoreHook.GetCreateProcessFileName(lpApplicationName, lpCommandLine);

	if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(lpApplicationName, lpCommandLine))))
	{
		return (*g_pCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	else
	{
		if(g_CoreHook.ExeIsWebBrowser(strExeName))
		{
			return (*g_pCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
		}
		WCHAR szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
		if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
		{
			// is creating game, modify command line for bypass login
			lstrcpynW(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
			lpCommandLine = szCmdLine;
			g_CoreHook.ModifyCommandLine(lpCommandLine, TRUE);
		}
		char szPath[MAX_PATH];
		GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
		return DetourCreateProcessWithDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
			lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pCreateProcessW);
	}
}


// begin :vista and above, handle shim engine see line: 623 [10/24/2014 liuyu]

PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrCreateProcessW = NULL;

BOOL WINAPI MyLdrCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
                             LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = g_CoreHook.GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

    if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(A2T(lpApplicationName), A2T(lpCommandLine)))))
    {
        return (*g_pLdrCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
        if(g_CoreHook.ExeIsWebBrowser(strExeName))
        {
            return (*g_pLdrCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
        char szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
        if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
        {
            // is creating game, modify command line for bypass login
            lstrcpynA(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
            lpCommandLine = szCmdLine;
            g_CoreHook.ModifyCommandLine(lpCommandLine);
        }
        char szPath[MAX_PATH];
        GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
        return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pLdrCreateProcessA);
    }
}

BOOL WINAPI MyLdrCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPWSTR lpCurrentDirectory, 
                             LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    CString strExeName = g_CoreHook.GetCreateProcessFileName(lpApplicationName, lpCommandLine);

    if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(lpApplicationName, lpCommandLine))))
    {
        return (*g_pLdrCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
        if(g_CoreHook.ExeIsWebBrowser(strExeName))
        {
            return (*g_pLdrCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
        WCHAR szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
        if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
        {
            // is creating game, modify command line for bypass login
            lstrcpynW(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
            lpCommandLine = szCmdLine;
            g_CoreHook.ModifyCommandLine(lpCommandLine, TRUE);
        }
        char szPath[MAX_PATH];
        GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
        return DetourCreateProcessWithDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pLdrCreateProcessW);
    }
}

typedef NTSTATUS (NTAPI* FUNC_LdrGetProcedureAddress)(
    HMODULE              ModuleHandle,
    PANSI_STRING         FunctionName OPTIONAL,
    WORD                 Oridinal OPTIONAL,
    PVOID               *FunctionAddress );

FUNC_LdrGetProcedureAddress g_pLdrGetProcedureAddress = NULL;

typedef NTSTATUS (NTAPI* FUNC_LdrGetProcedureAddressForCaller)(
    HMODULE              ModuleHandle,
    PANSI_STRING         FunctionName OPTIONAL,
    WORD                 Oridinal OPTIONAL,
    PVOID               *FunctionAddress, 
    BOOL                 bValue,
    PVOID               *CallbackAddress);

FUNC_LdrGetProcedureAddressForCaller g_pLdrGetProcedureAddressForCaller = NULL;

PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrForCallerCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrForCallerCreateProcessW = NULL;
BOOL WINAPI MyLdrForCallerCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
                                LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = g_CoreHook.GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

    if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(A2T(lpApplicationName), A2T(lpCommandLine)))))
    {
        return (*g_pLdrForCallerCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
        if(g_CoreHook.ExeIsWebBrowser(strExeName))
        {
            return (*g_pLdrForCallerCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
        char szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
        if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
        {
            // is creating game, modify command line for bypass login
            lstrcpynA(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
            lpCommandLine = szCmdLine;
            g_CoreHook.ModifyCommandLine(lpCommandLine);
        }
        char szPath[MAX_PATH];
        GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
        return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pLdrForCallerCreateProcessA);
    }
}

BOOL WINAPI MyLdrForCallerCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPWSTR lpCurrentDirectory, 
                                LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    CString strExeName = g_CoreHook.GetCreateProcessFileName(lpApplicationName, lpCommandLine);

    if(g_CoreHook.IsInGame() || (IMAGE_FILE_MACHINE_AMD64 == g_CoreHook.FileMachineType(g_CoreHook.GetCreateProcessFileFullPath(lpApplicationName, lpCommandLine))))
    {
        return (*g_pLdrForCallerCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
        if(g_CoreHook.ExeIsWebBrowser(strExeName))
        {
            return (*g_pLdrForCallerCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
        WCHAR szCmdLine[FAKE_CMDLINE_LENGTH] = {0};
        if(g_CoreHook.GetGameExeName().CompareNoCase(strExeName) == 0)
        {
            // is creating game, modify command line for bypass login
            lstrcpynW(szCmdLine, lpCommandLine, FAKE_CMDLINE_LENGTH);
            lpCommandLine = szCmdLine;
            g_CoreHook.ModifyCommandLine(lpCommandLine, TRUE);
        }
        char szPath[MAX_PATH];
        GetModuleFileNameA(g_CoreHook.GetDllHModule(), szPath, MAX_PATH);
        return DetourCreateProcessWithDllW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
            lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szPath, g_pLdrForCallerCreateProcessW);
    }
}

typedef VOID (NTAPI *FUNC_RtlInitAnsiString)( PANSI_STRING DestinationString, PCSZ SourceString );
FUNC_RtlInitAnsiString g_pRtlInitAnsiString = NULL;


// end [10/24/2014 liuyu]

typedef BOOL (WINAPI * FUNC_GetCursorPos)(LPPOINT lpPoint);
FUNC_GetCursorPos g_pGetCursorPos = NULL;
BOOL WINAPI MyGetCursorPos(LPPOINT lpPoint)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
//   		if(lpPoint)
//   		{
//   			lpPoint->x = 0;
//   			lpPoint->y = 0;
//   		}
        lpPoint->x = g_CoreHook.m_pt.x;
        lpPoint->y = g_CoreHook.m_pt.y;
        return TRUE;
	}
	return (*g_pGetCursorPos)(lpPoint);
}

typedef BOOL (WINAPI * FUNC_SetCursorPos)(int X, int Y);
FUNC_SetCursorPos g_pSetCursorPos = NULL;
BOOL WINAPI MySetCursorPos(int X, int Y)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
		return TRUE;
	}
	return (*g_pSetCursorPos)(X, Y);
}

typedef int (WINAPI * FUNC_ShowCursor)(BOOL bShow);
FUNC_ShowCursor g_pShowCursor = NULL;
int WINAPI MyShowCursor(BOOL bShow)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
		if(bShow)
			g_CoreHook.m_nCursorCount++;
		else
			g_CoreHook.m_nCursorCount--;

		return g_CoreHook.m_nCursorCount;
	}
	return (*g_pShowCursor)(bShow);
}

typedef SHORT (WINAPI * FUNC_GetAsyncKeyState)(int vKey);
FUNC_GetAsyncKeyState g_pGetAsyncKeyState = NULL;
SHORT WINAPI MyGetAsyncKeyState(int vKey)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
		return 0;
	}
	return (*g_pGetAsyncKeyState)(vKey);
}

typedef UINT (WINAPI * FUNC_GetRawInputBuffer)(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
FUNC_GetRawInputBuffer g_pGetRawInputBuffer = NULL;
UINT WINAPI MyGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
		return -1;
	}
	return (*g_pGetRawInputBuffer)(pData, pcbSize, cbSizeHeader);
}

typedef UINT (WINAPI * FUNC_GetRawInputData)(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
FUNC_GetRawInputData g_pGetRawInputData = NULL;
UINT WINAPI MyGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	if(g_CoreHook.IsOverlayPoppingup())
	{
		return -1;
	}
	return (*g_pGetRawInputData)(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
}

typedef HMODULE (WINAPI * FUNC_LoadLibraryExW)(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags);
FUNC_LoadLibraryExW g_pLoadLibraryExW = NULL;
HMODULE WINAPI MyLoadLibraryExW(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE hMod = (*g_pLoadLibraryExW)(lpFileName, hFile, dwFlags);
	if(lpFileName)
	{
		g_CoreHook.CheckAndHookDX();
	}	
	
	return hMod;
}

typedef HCURSOR (WINAPI * FUNC_SetCursor)(HCURSOR hCursor);
FUNC_SetCursor g_pSetCursor = NULL;
HCURSOR WINAPI MySetCursor(HCURSOR hCursor)
{
	if(g_CoreHook.IsOverlayPoppingup() && g_CoreHook.m_hCurrCursor)
	{
		hCursor = g_CoreHook.m_hCurrCursor;
	}
	return (*g_pSetCursor)(hCursor);
}

//////////////////////////////////////////////////////////////////////////
// dx hooked APIs
typedef HRESULT (WINAPI * FUNC_CreateSwapChain)(LPVOID pDXGIFactory, LPVOID pDeivce, LPVOID pSwapChainDesc, LPVOID * ppSwapChain);
FUNC_CreateSwapChain g_pCreateSwapChain = NULL;
HRESULT WINAPI MyCreateSwapChain(LPVOID pDXGIFactory, LPVOID pDeivce, LPVOID pSwapChainDesc, LPVOID * ppSwapChain)
{
	HRESULT hRet = (*g_pCreateSwapChain)(pDXGIFactory, pDeivce, pSwapChainDesc, ppSwapChain);
	if(hRet == S_OK && *ppSwapChain)
	{
        g_dwDXVer = DX_D3D11;
		HWND hWnd = *(HWND *)((PBYTE)pSwapChainDesc + 0x2c);
		if(g_CoreHook.LoadOverlay(hWnd, DX_D3D11))
		{
			g_CoreHook.HookIDXGISwapChain(*ppSwapChain, DX_D3D11);
		}
	}
	return hRet;
}

typedef HRESULT (WINAPI * FUNC_CreateSwapChainForHWND)(LPVOID pDXGIFactory, LPVOID pDevice, HWND hWnd, LPVOID pSwapChainDesc, LPVOID pSwapChainFullScreenDesc, LPVOID * ppDXGIOutput, LPVOID * ppSwapChain);
FUNC_CreateSwapChainForHWND g_pCreateSwapChainForHWND = NULL;
HRESULT WINAPI MyCreateSwapChainForHWND(LPVOID pDXGIFactory, LPVOID pDevice, HWND hWnd, LPVOID pSwapChainDesc, LPVOID pSwapChainFullScreenDesc, LPVOID * ppDXGIOutput, LPVOID * ppSwapChain)
{
	HRESULT hRet = (*g_pCreateSwapChainForHWND)(pDXGIFactory, pDevice, hWnd, pSwapChainDesc, pSwapChainFullScreenDesc, ppDXGIOutput, ppSwapChain);
	if(hRet == S_OK && *ppSwapChain)
	{
        g_dwDXVer = DX_D3D11;
		if(g_CoreHook.LoadOverlay(hWnd, DX_D3D11))
		{
			g_CoreHook.HookIDXGISwapChain(*ppSwapChain, DX_D3D11);
		}
	}
	return hRet;
}

typedef HRESULT (WINAPI * FUNC_CreateDXGIFactory)(LPVOID riid, LPVOID * ppFactory);
FUNC_CreateDXGIFactory g_pCreateDXGIFactory = NULL;
HRESULT WINAPI MyCreateDXGIFactory(LPVOID riid, LPVOID * ppFactory)
{
	HRESULT hRet = (*g_pCreateDXGIFactory)(riid, ppFactory);
	if(hRet == S_OK && *ppFactory)
	{
		g_CoreHook.HookIDXGIFactory((const GUID *)riid, *ppFactory);
	}
	return hRet;
}

typedef HRESULT (WINAPI * FUNC_CreateDXGIFactory1)(LPVOID riid, LPVOID * ppFactory);
FUNC_CreateDXGIFactory g_pCreateDXGIFactory1 = NULL;
HRESULT WINAPI MyCreateDXGIFactory1(LPVOID riid, LPVOID * ppFactory)
{
	HRESULT hRet = (*g_pCreateDXGIFactory1)(riid, ppFactory);
	if(hRet == S_OK && *ppFactory)
	{
		g_CoreHook.HookIDXGIFactory((const GUID *)riid, *ppFactory);
	}
	return hRet;
}

typedef LPVOID (WINAPI * FUNC_Direct3DCreate9)(UINT SDKVersion);
FUNC_Direct3DCreate9 g_pDirect3DCreate9 = NULL;
LPVOID WINAPI MyDirect3DCreate9(UINT SDKVersion)
{
	LPVOID pRet = (*g_pDirect3DCreate9)(SDKVersion);
	if(pRet)
	{
		g_CoreHook.HookIDirect3D(pRet, DX_D3D9);
	}
	return pRet;
}

typedef HRESULT (WINAPI * FUNC_Direct3DCreate9Ex)(UINT SDKVersion, LPVOID * ppD3D);
FUNC_Direct3DCreate9Ex g_pDirect3DCreate9Ex = NULL;
HRESULT WINAPI MyDirect3DCreate9Ex(UINT SDKVersion, LPVOID * ppD3D)
{
	HRESULT hRet = (*g_pDirect3DCreate9Ex)(SDKVersion, ppD3D);
	if(hRet == S_OK && *ppD3D)
	{
		g_CoreHook.HookIDirect3DEx(*ppD3D);
	}
	return hRet;
}

typedef LPVOID (WINAPI * FUNC_Direct3DCreate8)(UINT SDKVersion);
FUNC_Direct3DCreate8 g_pDirect3DCreate8 = NULL;
LPVOID WINAPI MyDirect3DCreate8(UINT SDKVersion)
{
	LPVOID pRet = (*g_pDirect3DCreate8)(SDKVersion);
	if(pRet)
	{
		g_CoreHook.HookIDirect3D(pRet, DX_D3D8);
	}
	return pRet;
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_CreateDevice)(LPVOID pThis, UINT uFlag, int nDeviceType, HWND hWnd, DWORD dwBehaviorFlags, LPVOID PresentPamameters, LPVOID * pDevice);
FUNC_CreateDevice g_pCreateDevice = NULL;
HRESULT STDMETHODCALLTYPE MyCreateDevice(LPVOID pThis, UINT uRsv, int nDeviceType, HWND hWnd, DWORD dwBehaviorFlags, LPVOID PresentPamameters, LPVOID * pDevice)
{
	HRESULT hRet = (*g_pCreateDevice)(pThis, uRsv, nDeviceType, hWnd, dwBehaviorFlags, PresentPamameters, pDevice);
	// retrieve the current Direct3D device
	if(hRet == S_OK && nDeviceType == 1/*D3DDEVTYPE_HAL*/)
	{
		DWORD dwVer = g_dwDXVer;
		if(g_CoreHook.LoadOverlay(hWnd, dwVer))
		{
			g_CoreHook.m_nWidth = *(int *)PresentPamameters;
			g_CoreHook.m_nHeight = *((int *)PresentPamameters + 1);
			if(dwVer == DX_D3D8)
			{
				g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentPamameters + 7);
			}
			else
			{
				g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentPamameters + 8);
			}
			GLOG(_T("Device:%x, Width:%d, Height:%d, bFullScreen:%d"), *pDevice, g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
			g_CoreHook.HookIDirect3DDevice(*pDevice, dwVer);
		}
	}
	return hRet;
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_CreateDeviceEx)(LPVOID pThis, UINT, int nDeviceType, HWND hWnd, DWORD dwBehaviorFlags, LPVOID PresentPamameters, LPVOID, LPVOID * pDevice);
FUNC_CreateDeviceEx g_pCreateDeviceEx = NULL;
HRESULT STDMETHODCALLTYPE MyCreateDeviceEx(LPVOID pThis, UINT uRsv, int nDeviceType, HWND hWnd, DWORD dwBehaviorFlags, LPVOID PresentPamameters, LPVOID pRsv, LPVOID * pDevice)
{
	HRESULT hRet = (*g_pCreateDeviceEx)(pThis, uRsv, nDeviceType, hWnd, dwBehaviorFlags, PresentPamameters, pRsv, pDevice);
	// retrieve the current Direct3D device
	if(hRet == S_OK && nDeviceType == 1/*D3DDEVTYPE_HAL*/)
	{
		DWORD dwVer = g_dwDXVer;
		if(g_CoreHook.LoadOverlay(hWnd, dwVer))
		{
			g_CoreHook.m_nWidth = *(int *)PresentPamameters;
			g_CoreHook.m_nHeight = *((int *)PresentPamameters + 1);
			g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentPamameters + 8);
			GLOG(_T("Device:%x, Width:%d, Height:%d, bFullScreen:%d"), *pDevice, g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
			g_CoreHook.HookIDirect3DDevice(*pDevice, dwVer);
		}
	}
	return hRet;
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_Present)(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect, HWND hDestWindowOverride, LPVOID pRsv);
FUNC_Present g_pPresent = NULL;
HRESULT STDMETHODCALLTYPE MyPresent(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
									HWND hDestWindowOverride, LPVOID pRsv)
{
	__try
	{
		g_CoreHook.RenderOverlay(g_dwDXVer, pThis);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}	
	return (*g_pPresent)(pThis, pSourceRect,pDestRect, hDestWindowOverride, pRsv);
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_SwapChainPresent)(LPVOID pThis, UINT SyncInterval, UINT Flags);
FUNC_SwapChainPresent g_pSwapChainPresent = NULL;
HRESULT STDMETHODCALLTYPE MySwapChainPresent(LPVOID pThis, UINT SyncInterval, UINT Flags)
{
	__try
	{
		g_CoreHook.RenderOverlay(g_dwDXVer, pThis);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}	
	return (*g_pSwapChainPresent)(pThis, SyncInterval, Flags);
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_Reset)(LPVOID pThis, LPVOID PresentParameters);
FUNC_Reset g_pReset = NULL;
HRESULT STDMETHODCALLTYPE MyReset(LPVOID pThis, LPVOID PresentParameters)
{
	DWORD dwVer = g_dwDXVer;
	if(dwVer == DX_D3D9 || dwVer == DX_D3D9EX)
	{
		g_CoreHook.UninitOverlay();
	}
	HRESULT hRet = (*g_pReset)(pThis, PresentParameters);
	g_CoreHook.m_nWidth = *(int *)PresentParameters;
	g_CoreHook.m_nHeight = *((int *)PresentParameters + 1);
	if(dwVer == DX_D3D8)
	{
		g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 7);
	}
	else
	{
		g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 8);
	}
	GLOG(_T("Width:%d, Height:%d, bFullScreen:%d"), g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
	return hRet;
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_PresentEx)(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect, HWND hDestWindowOverride, LPVOID, DWORD);
FUNC_PresentEx g_pPresentEx = NULL;
HRESULT STDMETHODCALLTYPE MyPresentEx(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
									  HWND hDestWindowOverride, LPVOID pRsv, DWORD dwRsv)
{
	__try
	{
		g_CoreHook.RenderOverlay(g_dwDXVer, pThis);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

	}	
	return (*g_pPresentEx)(pThis, pSourceRect, pDestRect, hDestWindowOverride, pRsv, dwRsv);
}

typedef HRESULT (STDMETHODCALLTYPE * FUNC_ResetEx)(LPVOID pThis, LPVOID PresentParameters, LPVOID);
FUNC_ResetEx g_pResetEx = NULL;
HRESULT STDMETHODCALLTYPE MyResetEx(LPVOID pThis, LPVOID PresentParameters, LPVOID pRsv)
{
	DWORD dwVer = g_dwDXVer;
	g_CoreHook.UninitOverlay();
	HRESULT hRet = (*g_pResetEx)(pThis, PresentParameters, pRsv);
	g_CoreHook.m_nWidth = *(int *)PresentParameters;
	g_CoreHook.m_nHeight = *((int *)PresentParameters + 1);
	g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 8);
	GLOG(_T("Width:%d, Height:%d, bFullScreen:%d"), g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
	return hRet;
}

//////////////////////////////////////////////////////////////////////////
// maintain a reference count to uninitialize timely
typedef ULONG (STDMETHODCALLTYPE * FUNC_AddRef)(LPVOID pThis);
FUNC_AddRef g_pAddRef = NULL;
ULONG STDMETHODCALLTYPE MyAddRef(LPVOID pThis)
{
	return (*g_pAddRef)(pThis);
}

typedef ULONG (STDMETHODCALLTYPE * FUNC_Release)(LPVOID pThis);
FUNC_Release g_pRelease = NULL;
ULONG STDMETHODCALLTYPE MyRelease(LPVOID pThis)
{
	ULONG uLong = (*g_pRelease)(pThis);
	static BOOL bOnce = FALSE;
	if(uLong == g_dwCustomRefCount && g_CoreHook.m_pOSTexture && !bOnce)
	{
		// final releasing device
		bOnce = TRUE;
 		g_MsgSwitchboard.SendMessage(WM_COREUNINIT);
 		uLong = 0;
	}
	return uLong;
}

typedef ULONG (STDMETHODCALLTYPE * FUNC_DX9SwapChainPresent)(LPVOID pThis, const RECT* pSourceRect, const RECT* pDestRect, 
                                                             HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags );
FUNC_DX9SwapChainPresent g_pDx9SwapChainPresent = NULL;
ULONG STDMETHODCALLTYPE MyDX9SwapChainPresent(LPVOID pThis, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    LPVOID pDevice = NULL;
    CDXSwapChian9::GetDevice(pThis, &pDevice);
    if(pDevice)
    {
        g_CoreHook.RenderOverlay(g_dwDXVer, pDevice);
        CUnknown::Release(pDevice);
    }
    return (*g_pDx9SwapChainPresent)(pThis, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}



#pragma optimize("", on)
//////////////////////////////////////////////////////////////////////////
// implementations of CCoreHook
HHOOK CCoreHook::m_sMsgHook = NULL;

CCoreHook::CCoreHook(void)
{
	m_pOSTexture = NULL;
	m_pSysmemTexture = NULL;
	m_pStateBlock = NULL;
	m_hArc = NULL;
	m_hGame = NULL;
	m_hOverlayMsgCenter = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_hInst = NULL;
	m_hCurrCursor = NULL;
	memset(m_szGameToken, 0, sizeof(m_szGameToken));

    m_pt.x = m_pt.y = -1;
}

CCoreHook::~CCoreHook(void)
{
}

BOOL CCoreHook::Init(HMODULE hInst)
{  
	m_hInst = hInst;
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathStripPath(szPath);
	if(lstrcmpi(szPath, _T("Arc.exe")) == 0 || lstrcmpi(szPath, _T("DetourTest.exe")) == 0)
	{
		// in CoreClient
		// add the path of current module to the tail of environment variable "PATH"
		DWORD dwPathSize = GetEnvironmentVariable(_T("PATH"), NULL, 0);
		dwPathSize += MAX_PATH;
		TCHAR * szEnvPath = new TCHAR[dwPathSize];
		if(GetEnvironmentVariable(_T("PATH"), szEnvPath, dwPathSize))
		{		
			TCHAR szTemp[MAX_PATH];
			GetModuleFileName(hInst, szTemp, MAX_PATH);
			PathRemoveFileSpec(szTemp);
			lstrcat(szEnvPath, _T(";"));
			lstrcat(szEnvPath, szTemp);
			SetEnvironmentVariable(_T("PATH"), szEnvPath);
		}
		delete []szEnvPath;
		return TRUE;
	}

	if(!g_CoreHook.GetGameID())
	{
		// not start from arc, overlay won't work any more
		return FALSE;
	}

	// ensure single instance
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
			strSingleTon.Format(_T("arc_%d_%d"), g_CoreHook.GetArcProcessID(), g_CoreHook.GetGameID());
			CreateEvent(NULL, FALSE, FALSE, strSingleTon);
		}
	}

	// check if the overlay should work
	if(g_CoreHook.IsInGame())
	{
		// create an event to mark this process as a game, for Arc
		CString strGameEvent;
		strGameEvent.Format(_T("arc_%d_%d_game"), g_CoreHook.GetArcProcessID(), g_CoreHook.GetGameID());
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

	if(g_CoreHook.IsInGame()&&g_CoreHook.GetGameID() == 1001)
	{
		// if the game is the blacklight(id = 1001),disable overlay
		//return FALSE;
	}
	
	// hook kernel32 functions
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	HMODULE hMod = GetModuleHandle(_T("kernel32.dll"));
	if(hMod)
	{
		// on Win7 64, sometimes you cannot get the real function address by explicitly passing the function to a variable, like LPVOID pFunc = CreateProcessW,
		// so we have to retrieve this real address from the export directory in the PE header of kernel32(in memory)


        // [10/24/2014 liuyu]
        // On vista and above , windows shim engine will hook GetProcAddress, so when we call GetProcAddress to get address of CreateProcess, it will give us a fake
        // address which located in aclayers.dll named like NS_EVELTER_APIHOOK_CreateProcess, sometimes it works fine, cause some games will use the fake address,
        // but sometimes there are some games calling the real address in kernel32 ,so we have to use LdrGetProcedureAddress to get the real address
#ifndef _WIN64
 		g_pCreateProcessA = (PDETOUR_CREATE_PROCESS_ROUTINEA)GetProcAddress(hMod, "CreateProcessA");
 		DetourAttach(&(PVOID&)g_pCreateProcessA, &MyCreateProcessA);
 		g_pCreateProcessW = (PDETOUR_CREATE_PROCESS_ROUTINEW)GetProcAddress(hMod, "CreateProcessW");
 		DetourAttach(&(PVOID&)g_pCreateProcessW, &MyCreateProcessW);

        if (InitShimEngineHookFixHandler())
        {
            FixShimEngineHookIssues();
        }
        else
        {
            OutputDebugString(_T("InitShimEngineHookFixHandler failed"));
        }
#endif
		HMODULE hModBase = GetModuleHandle(_T("kernelbase.dll"));
		if(hModBase)
		{
			// win7
			hMod = hModBase;
		}
		g_pLoadLibraryExW = (FUNC_LoadLibraryExW)GetProcAddress(hMod, "LoadLibraryExW");
		DetourAttach(&(PVOID&)g_pLoadLibraryExW, &MyLoadLibraryExW);
	}
	DetourTransactionCommit();

    CheckAndHookDX();

	return TRUE;
}



BOOL CCoreHook::UnInit()
{
    // UNhook kernel32 functions
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    UnHookD3D();

    if (NULL != g_pLoadLibraryExW)
    {
        DetourDetach(&(PVOID&)g_pLoadLibraryExW, &MyLoadLibraryExW);
    }

    if (NULL != g_pCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pCreateProcessA, &MyCreateProcessA);
    }
    if (NULL != g_pCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pCreateProcessW, &MyCreateProcessW);
    }

    ReleaseShimEngineHookFixHandler();

	DetourTransactionCommit();
    return TRUE;
}

BOOL CCoreHook::HookD3D( const DXMODULE_INFO * pInfo )
{
	if(!pInfo || !pInfo->hMod)
	{
		return FALSE;
	}
	for(VEC_DXMODULE_INFO::iterator it = m_vecHooked.begin(); it != m_vecHooked.end(); it++)
	{
		if(pInfo->hMod == it->hMod)
		{
			// already hooked
			return TRUE;
		}
	}
	m_vecHooked.push_back(*pInfo);


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	if(pInfo->dwVer == DX_D3D8)
	{
		g_pDirect3DCreate8 = (FUNC_Direct3DCreate8)GetProcAddress(pInfo->hMod, "Direct3DCreate8");
		DetourAttach(&(PVOID&)g_pDirect3DCreate8, &MyDirect3DCreate8);
	}
	else if(pInfo->dwVer == DX_D3D9)
	{
		g_pDirect3DCreate9 = (FUNC_Direct3DCreate9)GetProcAddress(pInfo->hMod, "Direct3DCreate9");
		DetourAttach(&(PVOID&)g_pDirect3DCreate9, &MyDirect3DCreate9);
		g_pDirect3DCreate9Ex = (FUNC_Direct3DCreate9Ex)GetProcAddress(pInfo->hMod, "Direct3DCreate9Ex");
		if(g_pDirect3DCreate9Ex)
		{
			DetourAttach(&(PVOID&)g_pDirect3DCreate9Ex, &MyDirect3DCreate9Ex);
		}
	}
	else if(pInfo->dwVer == DX_D3D11)
	{
// 		PBYTE pBaseAddr = (PBYTE)GetModuleHandle(_T("dxgi.dll"));
// 		if(pBaseAddr)
// 		{
// 			// win7 32
// 			int nOffset = 0x25;
// 			PBYTE pStartAddr = (PBYTE)ScanCode(pBaseAddr + 0x20000, 0x15000, _byCreateSwapChainCode, sizeof(_byCreateSwapChainCode));
// 			if(!pStartAddr)
// 			{
// 				// win7 64
// 				nOffset = 0x2d;
// 				pStartAddr = (PBYTE)ScanCode(pBaseAddr + 0x14000, 0x15000, _byCreateSwapChainCode1, sizeof(_byCreateSwapChainCode1));
// 			}
// 			if(!pStartAddr)
// 			{
// 				// win8 64
// 				nOffset = 0x2f;
// 				pStartAddr = (PBYTE)ScanCode(pBaseAddr + 0xe000, 0x15000, _byCreateSwapChainCode2, sizeof(_byCreateSwapChainCode2));
// 			}
// 			if(!pStartAddr)
// 			{
// 				// win8.1 64
// 				nOffset = 0x33;
// 				pStartAddr = (PBYTE)ScanCode(pBaseAddr + 0x14000, 0x15000, _byCreateSwapChainCode3, sizeof(_byCreateSwapChainCode3));
// 			}
// 			if(pStartAddr)
// 			{
// 				g_pCreateSwapChain = (FUNC_CreateSwapChain)(pStartAddr - nOffset);
// 				DetourAttach(&(PVOID&)g_pCreateSwapChain, &MyCreateSwapChain);
// 			}
// 		}
		HMODULE hMod = GetModuleHandle(_T("dxgi.dll"));
		g_pCreateDXGIFactory = (FUNC_CreateDXGIFactory)GetProcAddress(hMod, "CreateDXGIFactory");
		DetourAttach(&(PVOID&)g_pCreateDXGIFactory, &MyCreateDXGIFactory);
		g_pCreateDXGIFactory1 = (FUNC_CreateDXGIFactory1)GetProcAddress(hMod, "CreateDXGIFactory1");
		DetourAttach(&(PVOID&)g_pCreateDXGIFactory1, &MyCreateDXGIFactory1);
	}
	DetourTransactionCommit();

	return TRUE;
}

void CCoreHook::UnHookD3D()
{
    if(NULL != g_pDirect3DCreate8)
    {
        DetourDetach(&(PVOID&)g_pDirect3DCreate8, &MyDirect3DCreate8);
    }

    if(NULL != g_pDirect3DCreate9)
    {
        DetourDetach(&(PVOID&)g_pDirect3DCreate9, &MyDirect3DCreate9);
    }

    if(NULL != g_pDirect3DCreate9Ex)
    {
        DetourDetach(&(PVOID&)g_pDirect3DCreate9Ex, &MyDirect3DCreate9Ex);
    }

    if(NULL != g_pCreateSwapChain)
    {
        DetourDetach(&(PVOID&)g_pCreateSwapChain, &MyCreateSwapChain);
    }    
}

void CCoreHook::HookIDirect3D( LPVOID pDirect3D, DWORD dwVer )
{
    // already hooked, do not proceed
    // calling of detourattach will be failed at the second time,
    // there will be a circle when g_pCreateDevice was called[9/29/2014 user]
    if (g_pCreateDevice)
    {
        return;
    }

	DWORD dwIndex;
	switch(dwVer)
	{
	case DX_D3D8:
		dwIndex = 15;
		break;
	case DX_D3D9:
		dwIndex = 16;
		break;
	default:                                                         
		dwIndex = 0;
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_dwDXVer = dwVer;
	g_pCreateDevice = (FUNC_CreateDevice)GetVirtualCall(pDirect3D, dwIndex);
	DetourAttach(&(PVOID&)g_pCreateDevice, &MyCreateDevice);
	DetourTransactionCommit();
}


void CCoreHook::HookIDirect3DEx( LPVOID pDirect3D )
{
    if (g_pCreateDeviceEx)
    {
        return;
    }

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_dwDXVer = DX_D3D9EX;
	g_pCreateDeviceEx = (FUNC_CreateDeviceEx)GetVirtualCall(pDirect3D, 20);
	DetourAttach(&(PVOID&)g_pCreateDeviceEx, &MyCreateDeviceEx);
	DetourTransactionCommit();
}

void CCoreHook::HookIDirect3DDevice( LPVOID pD3DDevice, DWORD dwVer )
{
	DWORD dwPresentOffset = 0;
	DWORD dwResetOffset = 0;
	DWORD dwPresentExOffset = 0;
	DWORD dwResetExOffset = 0;
	switch(dwVer)
	{
	case DX_D3D8:
		dwPresentOffset = 15;
		dwResetOffset = 14;
		break;
	case DX_D3D9EX:
		dwPresentExOffset = 121;
		dwResetExOffset = 132;
	case DX_D3D9:
		dwPresentOffset = 17;
		dwResetOffset = 16;
		break;
	default:
		return;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());


//begin

	g_pPresent = (FUNC_Present)GetVirtualCall(pD3DDevice, dwPresentOffset);;
	DetourAttach(&(PVOID&)g_pPresent, &MyPresent);
	g_pReset = (FUNC_Reset)GetVirtualCall(pD3DDevice, dwResetOffset);
	DetourAttach(&(PVOID&)g_pReset, &MyReset);
	if(dwPresentExOffset)
	{
		g_pPresentEx = (FUNC_PresentEx)GetVirtualCall(pD3DDevice, dwPresentExOffset);
		DetourAttach(&(PVOID&)g_pPresentEx, &MyPresentEx);
	}
	if(dwResetExOffset)
	{
		g_pResetEx = (FUNC_ResetEx)GetVirtualCall(pD3DDevice, dwResetExOffset);
		DetourAttach(&(PVOID&)g_pResetEx, &MyResetEx);
	}

//end

    if(dwVer == DX_D3D9EX)
    {
        //ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pD3DDevice, dwPresentExOffset), &MyPresentEx, TRUE, dwVer);
        //ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pD3DDevice, dwResetExOffset), &MyResetEx, TRUE, dwVer);
        g_pPresentEx = (FUNC_PresentEx)GetVirtualCall(pD3DDevice, dwPresentExOffset);
        DetourAttach(&(PVOID&)g_pPresentEx, &MyPresentEx);
        g_pResetEx = (FUNC_ResetEx)GetVirtualCall(pD3DDevice, dwResetExOffset);
        DetourAttach(&(PVOID&)g_pResetEx, &MyResetEx);
    }
    else
    {
        if(dwVer == DX_D3D8)
        {
            //ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pD3DDevice, dwPresentOffset), &MyPresent, TRUE, dwVer);
            g_pPresent = (FUNC_Present)GetVirtualCall(pD3DDevice, dwPresentOffset);;
            DetourAttach(&(PVOID&)g_pPresent, &MyPresent);
        }
        else
        {
            LPVOID pSwapChain;
            CDXDevice9::GetSwapChain(pD3DDevice, 0, &pSwapChain);
            if(pSwapChain)
            {
                /*ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pSwapChain, 0xc), &MyDX9SwapChainPresent, TRUE, dwVer);*/
                g_pDx9SwapChainPresent = (FUNC_DX9SwapChainPresent)GetVirtualCall(pSwapChain, 0x3);
                DetourAttach(&(PVOID&)g_pDx9SwapChainPresent, &MyDX9SwapChainPresent);
                CUnknown::Release(pSwapChain);
            }
            if(!IsXP())
            {
                // implementation of d3d9 is different in xp and systems above. in xp, IDirect3DDevice::Present will call IDirect3DSwapChain9::Present internally,
                // so it's enough for us to hook IDirect3DSwapChain9::Present only. but in win7, you have to hook both of them.
                /*ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pD3DDevice, dwPresentOffset), &MyPresent, TRUE, dwVer);*/
                g_pPresent = (FUNC_Present)GetVirtualCall(pD3DDevice, dwPresentOffset);;
                DetourAttach(&(PVOID&)g_pPresent, &MyPresent);
            }

        }		
        /*ReplaceFuncAndCopy(CComObjectHooker::GetInterface(pD3DDevice, dwResetOffset), &MyReset, TRUE, dwVer);*/
        g_pReset = (FUNC_Reset)GetVirtualCall(pD3DDevice, dwResetOffset);
        DetourAttach(&(LPVOID&)g_pReset, &MyReset);
    }


	switch(dwVer)
	{
	case DX_D3D8:
		g_dwCustomRefCount = 1;
		break;
	case DX_D3D9:
		g_dwCustomRefCount = 2;
		break;
	case DX_D3D9EX:
		g_dwCustomRefCount = 3;
		break;
	default:
		g_dwCustomRefCount = 0;
		break;
	}
	g_pRelease = (FUNC_Release)GetVirtualCall(pD3DDevice, 2);
	DetourAttach(&(PVOID&)g_pRelease, &MyRelease);
	DetourTransactionCommit();
}

void CCoreHook::HookIDXGISwapChain( LPVOID pSwapChain, DWORD dwVer )
{
	DWORD dwPresentOffset; 
	switch(dwVer)
	{
	case DX_D3D11:
		dwPresentOffset = 8;
		break;
	default:
		break;
	}
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pSwapChainPresent = (FUNC_SwapChainPresent)GetVirtualCall(pSwapChain, dwPresentOffset);
	DetourAttach(&(PVOID&)g_pSwapChainPresent, &MySwapChainPresent);
	g_pRelease = (FUNC_Release)GetVirtualCall(pSwapChain, 2);
	DetourAttach(&(PVOID&)g_pRelease, &MyRelease);
	DetourTransactionCommit();

}

void CCoreHook::UninitOverlay()
{
	GLOG(_T(""));
	if(m_pOSTexture)
	{
		CUnknown::Release(m_pOSTexture);
		m_pOSTexture = NULL;
	}
	if(m_pSysmemTexture)
	{
		CUnknown::Release(m_pSysmemTexture);
		m_pSysmemTexture = NULL;
	}
	if(m_pStateBlock)
	{
		CUnknown::Release(m_pStateBlock);
		m_pStateBlock = NULL;
	}
    if(g_pVertexShader)
    {
        CUnknown::Release(g_pVertexShader);
        g_pVertexShader = NULL;
    }
    if(g_pPixelShader)
    {
        CUnknown::Release(g_pPixelShader);
        g_pPixelShader = NULL;
    }
    if(g_pVertexBuffer)
    {
        CUnknown::Release((g_pVertexBuffer));
        g_pVertexBuffer = NULL;
    }
    if(g_pVertexLayout)
    {
        CUnknown::Release(g_pVertexLayout);
        g_pVertexLayout = NULL;
    }
    if(g_pTextureRV)
    {
        CUnknown::Release(g_pTextureRV);
        g_pTextureRV = NULL;
    }
    if(g_pSamplerPoint)
    {
        CUnknown::Release(g_pSamplerPoint);
        g_pSamplerPoint = NULL;
    }
    if(g_pBlendState)
    {
        CUnknown::Release(g_pBlendState);
        g_pBlendState = NULL;
    }
    if(g_pDepthStencilState)
    {
        CUnknown::Release(g_pDepthStencilState);
        g_pDepthStencilState = NULL;
    }
    if(g_pRasterizerState)
    {
        CUnknown::Release(g_pRasterizerState);
        g_pRasterizerState = NULL;
    }
}

LRESULT CCoreHook::GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
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
	m_sMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, NULL, GetCurrentThreadId());
}

BOOL CCoreHook::LoadOverlay(HWND hWnd, DWORD dwVer)
{
	GLOG(_T("TryLoadOverlay"));
	if(!IsWindow(hWnd))
	{
		return FALSE;
	}
	if(GetCurrentThreadId() != GetWindowThreadProcessId(hWnd, NULL))
	{
		// this function must be called within the same thread with game window
		return FALSE;
	}
	if(m_smMgr.IsValid())
	{
		// here is the second time enter this function
		if(!IsWindow(m_hGame))
		{
			// the previous windows is invalid, which means the device created in the first time is not the real device,
			// we should record this new one instead
			overlay_sm_header * pHeader = m_smMgr.GetHeader();
			pHeader->hGame = hWnd;
			m_hGame = hWnd;
		}
		return FALSE;
	}
	// create share memory
	CString strShareMemName;
	strShareMemName.Format(_T("arc_%d_sm"), GetGameID());
	if(!m_smMgr.Create(strShareMemName, 2048 * 2048 * 4))
	{
		return FALSE;
	}
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// not the first time 
		return FALSE;
	}
	GLOG(_T("----------------------------------------------------------------------------------------"));
	GLOG(_T("GameWnd:%x, DX:%d, ThreadId:%d"), hWnd, dwVer, GetCurrentThreadId());
	overlay_sm_header * pHeader = m_smMgr.GetHeader();
	pHeader->hGame = hWnd;
	m_hGame = hWnd;
	pHeader->uStartTime = (UINT)time(NULL);

	// in game process, create the message client in the same thread with game window to simplify the thread model
	_ThreadMsgClient(NULL);

	//////////////////////////////////////////////////////////////////////////
	// initialization after overlay is loaded
	// hook user32 functions
	HMODULE hMod = GetModuleHandle(_T("user32"));
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pSetCursor = (FUNC_SetCursor)GetProcAddress(hMod, "SetCursor");
	DetourAttach(&(PVOID&)g_pSetCursor, &MySetCursor);
	g_pGetRawInputBuffer = (FUNC_GetRawInputBuffer)GetProcAddress(hMod, "GetRawInputBuffer");
	DetourAttach(&(PVOID&)g_pGetRawInputBuffer, &MyGetRawInputBuffer);
 	g_pGetRawInputData = (FUNC_GetRawInputData)GetProcAddress(hMod, "GetRawInputData");
	DetourAttach(&(PVOID&)g_pGetRawInputData, &MyGetRawInputData);
	g_pGetAsyncKeyState = (FUNC_GetAsyncKeyState)GetProcAddress(hMod, "GetAsyncKeyState");
	DetourAttach(&(PVOID&)g_pGetAsyncKeyState, &MyGetAsyncKeyState);
	g_pGetCursorPos = (FUNC_GetCursorPos)GetProcAddress(hMod, "GetCursorPos");
	DetourAttach(&(PVOID&)g_pGetCursorPos, &MyGetCursorPos);
	g_pSetCursorPos = (FUNC_SetCursorPos)GetProcAddress(hMod, "SetCursorPos");
	DetourAttach(&(PVOID&)g_pSetCursorPos, &MySetCursorPos);
	g_pShowCursor = (FUNC_ShowCursor)GetProcAddress(hMod, "ShowCursor");
	DetourAttach(&(PVOID&)g_pShowCursor, &MyShowCursor);
	DetourTransactionCommit();

	// install message hook
	InitMsgHook();
	return TRUE;
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
	/*if(bIsWideChar)
	{
		LPWSTR pCmdLine = (LPWSTR)pCommandLine;
		OutputDebugStringW(pCmdLine);
	}
	else
	{
		LPSTR pCmdLine = (LPSTR)pCommandLine;
		OutputDebugStringA(pCmdLine);
	}*/

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
			//if(pCmdLine[0])
			//{
			//	LPWSTR pUser = StrStrW(pCmdLine, L"user:");
			//	if(pUser)
			//	{
			//		*pUser = 0;
			//	}
			//	else
			//	{
			//		lstrcatW(pCmdLine, L" ");
			//	}
			//}
			lstrcatW(pCmdLine, L" ");
			lstrcatW(pCmdLine, A2W(m_szGameToken));
			//OutputDebugStringW(pCmdLine);
		}
		else
		{
			LPSTR pCmdLine = (LPSTR)pCommandLine;
			//if(pCmdLine[0])
			//{
			//	LPSTR pUser = StrStrA(pCmdLine, "user:");
			//	if(pUser)
			//	{
			//		*pUser = 0;
			//	}
			//	else
			//	{
			//		lstrcatA(pCmdLine, " ");
			//	} 
			//}
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
	if(!g_CoreHook.m_MsgClient.Connect(TRUE))
	{
		// fail to renew the binded hwnd, do uninitialization
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
	BOOL bResize = FALSE;
	DWORD dwFormat = 0;
	if(dwVer == DX_D3D11)
	{
		DXGI_SWAP_CHAIN_DESC desc;
		if(CDXSwapChian::GetDesc(pDeviceOrSwapChain, &desc) == S_OK)
		{
			m_nWidth = desc.BufferDesc.Width;
			m_nHeight = desc.BufferDesc.Height;
			m_smMgr.GetHeader()->bFullScreen = !desc.Windowed;
			dwFormat = desc.BufferDesc.Format;
		}
	}
	if(m_nWidth == 0 || m_nHeight == 0)
	{
		return;
	}

	// first check if the size of off-screen surface should be resized to fit the render target
	overlay_sm_header * pHeader = m_smMgr.GetHeader();
	if(m_nWidth != pHeader->nWidth || m_nHeight != pHeader->nHeight)
	{
		// size was changed, notify the off-screen overlay to resize and repaint
		CArcAutoLock lock;
		bResize = TRUE;
		pHeader->nWidth = m_nWidth;
		pHeader->nHeight = m_nHeight;
		GLOG(_T("resize:width:%d, height:%d, fullscreen:%d, format:%d"), m_nWidth, m_nHeight, pHeader->bFullScreen, dwFormat);
	}

	if(bResize)
	{
		// the size is changed, notify the off-screen overlay to change its size and repaint
		if(IsWindow(m_hOverlayMsgCenter))
		{
			core_msg_resize msg;
			msg.nWidth = m_nWidth;
			msg.nHeight = m_nHeight;
			m_MsgClient.SendCoreMsg(m_hOverlayMsgCenter, &msg);
		}
	}

	static DWORD dwRenderThreadId = 0;
	if(!dwRenderThreadId)
	{
		// first time to render overlay, here we already get the valid width and height of the surface, let's create the off-screen overlay process
		dwRenderThreadId = GetCurrentThreadId();
		DWORD dwThreadId;
		CreateThread(NULL, 0, _ThreadCreateAndMonitorOSOverlay, NULL, 0, &dwThreadId);
		GLOG(_T("First Time Render:ThreadId:%d"), dwRenderThreadId);
	}

	if(!IsWindow(pHeader->hOSPanel))
	{
		CArcAutoLock lock;
		pHeader->bHasImage = FALSE;
		pHeader->bUpdate = FALSE;
	}

	if(dwVer == DX_D3D9 || dwVer == DX_D3D9EX)
	{	
		LPVOID pDevice = pDeviceOrSwapChain;
		if(bResize || !m_pOSTexture)
		{
			// create or recreate texture with a new size
			if(m_pOSTexture)
			{
				CUnknown::Release(m_pOSTexture);
			}			
			CDXDevice9::CreateTexture(pDevice, m_nWidth, m_nHeight, 1, 0, 21/*D3DFMT_A8R8G8B8*/, (dwVer == DX_D3D9EX ? 0/*D3DPOOL_DEFAULT*/ : 1/*D3DPOOL_MANAGED*/),
				&m_pOSTexture, NULL);
			if(dwVer == DX_D3D9EX)
			{
				if(m_pSysmemTexture)
				{
					CUnknown::Release(m_pSysmemTexture);
				}
				CDXDevice9::CreateTexture(pDevice, m_nWidth, m_nHeight, 1, 0, 21/*D3DFMT_A8R8G8B8*/, 2/*D3DPOOL_SYSTEMMEM*/, &m_pSysmemTexture, NULL);
			}
			CArcAutoLock lock;
			pHeader->bUpdate = TRUE;
			::SetRect(&pHeader->rtDirty, 0, 0, pHeader->nWidth, pHeader->nHeight);
		}
		if(!m_pStateBlock)
		{
			CDXDevice9::CreateStateBlock(pDevice, 1, &m_pStateBlock);
		}

		// render overlay
		if(m_pOSTexture)
		{
			CArcAutoLock lock;
			LPVOID pLockTexture = dwVer == DX_D3D9 ? m_pOSTexture : m_pSysmemTexture;
			if(pLockTexture)
			{
				D3DLOCKED_RECT lr;
				if(m_smMgr.GetHeader()->bUpdate)
				{
					// update texture with dirty region if needed
					if(CDXTexture9::LockRect(pLockTexture, 0, &lr, NULL, 0x2000/*D3DLOCK_DISCARD*/) == S_OK)
					{
						CRect rtDirty = m_smMgr.GetHeader()->rtDirty;
						rtDirty.InflateRect(2, 2, 2, 2);
						CRect rtAll(0, 0, m_smMgr.GetHeader()->nWidth, m_smMgr.GetHeader()->nHeight);
						rtDirty.IntersectRect(rtDirty, rtAll);
						if(rtDirty.left >= 0 && rtDirty.top >= 0 && rtDirty.right <= m_nWidth && rtDirty.bottom <= m_nHeight)
						{
							for(int i = rtDirty.top; i < rtDirty.bottom; i++)
							{
								memcpy((LPBYTE)lr.pBits + (i * m_nWidth + rtDirty.left) * 4, m_smMgr.GetBits() + (i * m_nWidth + rtDirty.left) * 4, rtDirty.Width() * 4);
							}
						}
						CDXTexture9::UnlockRect(pLockTexture, 0);
						m_smMgr.GetHeader()->bUpdate = FALSE;
						::SetRect(&m_smMgr.GetHeader()->rtDirty, 0, 0, 0, 0);
					}
					if(dwVer == DX_D3D9EX)
					{
						CDXDevice9::UpdateTexture(pDevice, m_pSysmemTexture, m_pOSTexture);
					}
				}
			}
			if(m_smMgr.GetHeader()->bHasImage)
			{
				// not fully transparent, render it
				struct MYVERTEX
				{
					float x, y, z, rhw;
					float tu, tv;
				};
				MYVERTEX vert[4] = 
				{
					{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
					{m_nWidth - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
					{-0.5f, m_nHeight - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
					{m_nWidth - 0.5f, m_nHeight - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f}
				};
				if(CDXDevice9::BeginSence(pDevice) == S_OK && m_pStateBlock)
				{
					CDXStateBlock9::Capture(m_pStateBlock);
					CDXDevice9::SetPixelShader(pDevice, NULL);
					CDXDevice9::SetVertextShader(pDevice, NULL);
					CDXDevice9::SetTexture(pDevice, 0, m_pOSTexture);

                    LPVOID pSurfaceOld = NULL;
                    LPVOID pSurfaceNew = NULL;

                    D3D9_D3DSURFACE_DESC desc = {0};
                    CDXDevice9::GetRenderTarget(pDevice, 0, &pSurfaceOld);
                    if (pSurfaceOld)
                        CDXSurface9::GetDesc(pSurfaceOld, &desc);

                    if (m_nWidth != desc.Width)
                    {
                        CDXDevice9::GetBackBuffer(pDevice, 0, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurfaceNew);
                        CDXDevice9::SetRenderTarget(pDevice, 0, pSurfaceNew);
                    }

					CDXDevice9::SetRenderState(pDevice, 27/*D3DRS_ALPHABLENDENABLE*/, TRUE);
					CDXDevice9::SetRenderState(pDevice, 19/*D3DRS_SRCBLEND*/, 5/*D3DBLEND_SRCALPHA*/);
					CDXDevice9::SetRenderState(pDevice, 20/*D3DRS_DESTBLEND*/, 6/*D3DBLEND_INVSRCALPHA*/);
					CDXDevice9::SetRenderState(pDevice, 174/*D3DRS_SCISSORTESTENABLE*/, FALSE);
					CDXDevice9::SetRenderState(pDevice, 15/*D3DRS_ALPHATESTENABLE*/, FALSE);
					CDXDevice9::SetRenderState(pDevice, 22/*D3DRS_CULLMODE*/, 1/*D3DCULL_NONE*/);
					CDXDevice9::SetRenderState(pDevice, 168/*D3DRS_COLORWRITEENABLE*/, 0xf);
					CDXDevice9::SetRenderState(pDevice, 137/*D3DRS_LIGHTING*/, FALSE);
					CDXDevice9::SetRenderState(pDevice, 8/*D3DRS_FILLMODE*/, 3/*D3DFILL_SOLID*/);
					CDXDevice9::SetRenderState(pDevice, 206/*D3DRS_SEPARATEALPHABLENDENABLE*/, FALSE);
					CDXDevice9::SetRenderState(pDevice, 194/*D3DRS_SRGBWRITEENABLE*/, FALSE);
 					CDXDevice9::SetRenderState(pDevice, 14/*D3DRS_ZWRITEENABLE*/, FALSE);
 					CDXDevice9::SetRenderState(pDevice, 7/*D3DRS_ZENABLE*/, FALSE);
					CDXDevice9::SetTextureStageState(pDevice, 0, 1/*D3DTSS_COLOROP*/, 4/*D3DTOP_MODULATE*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 2/*D3DTSS_COLORARG1*/, 2/*D3DTA_TEXTURE*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 3/*D3DTSS_COLORARG2*/, 1/*D3DTA_CURRENT*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 4/*D3DTSS_ALPHAOP*/, 2/*D3DTOP_SELECTARG1*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 5/*D3DTSS_ALPHAARG1*/, 2/*D3DTA_TEXTURE*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 6/*D3DTSS_ALPHAARG2*/, 1/*D3DTA_CURRENT*/);
					CDXDevice9::SetTextureStageState(pDevice, 0, 11/*D3DTSS_TEXCOORDINDEX*/, 0);
					CDXDevice9::SetSamplerState(pDevice, 0, 11/*D3DSAMP_SRGBTEXTURE*/, 0);
					CDXDevice9::SetFVF(pDevice, 0x104/*D3DFVF_XYZRHW | D3DFVF_TEX1*/);
					CDXDevice9::DrawPrimitiveUp(pDevice, 5/*D3DPT_TRIANGLESTRIP*/, 2, vert, sizeof(MYVERTEX));
  
                    if (pSurfaceOld)
                        CUnknown::Release(pSurfaceOld);

                    if (pSurfaceNew)
                    {
                        CDXDevice9::SetRenderTarget(pDevice, 0, pSurfaceOld);
                        CUnknown::Release(pSurfaceNew);
                    }
  
					CDXDevice9::SetTexture(pDevice, 0, NULL);
					CDXStateBlock9::Apply(m_pStateBlock);
					CDXDevice9::EndScene(pDevice);
				}
			}
		}
	}
	else if(dwVer == DX_D3D8)
	{
		LPVOID pDevice = pDeviceOrSwapChain;
		if(bResize || !m_pOSTexture)
		{
			// create or recreate texture with a new size
			if(m_pOSTexture)
			{
				CUnknown::Release(m_pOSTexture);
			}
			CDXDevice8::CreateTexture(pDevice, m_nWidth, m_nHeight, 1, 0, 21/*D3DFMT_A8R8G8B8*/, 1/*D3DPOOL_MANAGED*/, &m_pOSTexture);
		}

		// render overlay
		if(m_pOSTexture)
		{
			CArcAutoLock lock;
			D3DLOCKED_RECT lr;
			if(m_smMgr.GetHeader()->bUpdate)
			{
				// update texture with dirty region if needed
				if(CDXTexture8::LockRect(m_pOSTexture, 0, &lr, NULL, 0x2000/*D3DLOCK_DISCARD*/) == S_OK)
				{
					CRect rtDirty = m_smMgr.GetHeader()->rtDirty;
					if(rtDirty.left >= 0 && rtDirty.top >= 0 && rtDirty.right <= m_nWidth && rtDirty.bottom <= m_nHeight)
					{
						for(int i = rtDirty.top; i < rtDirty.bottom; i++)
						{
							memcpy((LPBYTE)lr.pBits + (i * m_nWidth + rtDirty.left) * 4, m_smMgr.GetBits() + (i * m_nWidth + rtDirty.left) * 4, rtDirty.Width() * 4);
						}
					}
					CDXTexture8::UnlockRect(m_pOSTexture, 0);
					m_smMgr.GetHeader()->bUpdate = FALSE;
					::SetRect(&m_smMgr.GetHeader()->rtDirty, 0, 0, 0, 0);
				}
			}
			if(m_smMgr.GetHeader()->bHasImage)
			{
				// not fully transparent, render it
				struct MYVERTEX
				{
					float x, y, z, rhw;
					float tu, tv;
				};
				MYVERTEX vert[4] = 
				{
					{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
					{m_nWidth - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
					{-0.5f, m_nHeight - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
					{m_nWidth - 0.5f, m_nHeight - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f}
				};
				if(CDXDevice8::BeginSence(pDevice) == S_OK)
				{
					CDXDevice8::SetRenderState(pDevice, 27/*D3DRS_ALPHABLENDENABLE*/, TRUE);
					CDXDevice8::SetRenderState(pDevice, 19/*D3DRS_SRCBLEND*/, 5/*D3DBLEND_SRCALPHA*/);
					CDXDevice8::SetRenderState(pDevice, 20/*D3DRS_DESTBLEND*/, 6/*D3DBLEND_INVSRCALPHA*/);
					CDXDevice8::SetTexture(pDevice, 0, m_pOSTexture);
					CDXDevice8::SetVertexShader(pDevice, 0x104/*D3DFVF_XYZRHW | D3DFVF_TEX1*/);
					CDXDevice8::DrawPrimitiveUp(pDevice, 5/*D3DPT_TRIANGLESTRIP*/, 2, vert, sizeof(MYVERTEX));
					CDXDevice8::SetTexture(pDevice, 0, NULL);
					CDXDevice8::EndScene(pDevice);
				}				
			}
		}
	}
	else if(dwVer == DX_D3D11)
	{
		LPVOID pSwapChain = pDeviceOrSwapChain;
		LPVOID pDevice = NULL;
		CDXSwapChian::GetDevice(pSwapChain, IID_ID3D11Device, &pDevice);
		if(pDevice)
		{
			LPVOID pDeviceContext = NULL;
			CDXDevice11::GetImmediateContext(pDevice, &pDeviceContext);
			if(pDeviceContext)
			{
				if(!g_pVertexShader)
				{
					// initialize d3d11 objects
					LPVOID pVSBlob = NULL;
					LPVOID pPSBlob = NULL;
					HRESULT hr;
					do 
					{
						hr = D3DX11CompileFromMemory( g_strShaderCode, strlen(g_strShaderCode), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlob, NULL, NULL);
						if(hr != S_OK)
						{
							break;
						}
						hr = CDXDevice11::CreateVertexShader(pDevice, (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pVSBlob), CDXBlob::GetBufferSize(pVSBlob), NULL, &g_pVertexShader);
						if(hr != S_OK)
						{
							break;
						}
						D3D11_INPUT_ELEMENT_DESC layout[] =
						{
							{ "POSITION", 0, 6/*DXGI_FORMAT_R32G32B32_FLOAT*/, 0, 0, 0/*D3D11_INPUT_PER_VERTEX_DATA*/, 0 },
							{ "TEXCOORD", 0, 16/*DXGI_FORMAT_R32G32_FLOAT*/, 0,12/* DWORD(-1)*//*D3D11_APPEND_ALIGNED_ELEMENT*/, 0/*D3D11_INPUT_PER_VERTEX_DATA*/, 0 },
						};
						UINT numElements = 2;
						// Create the input layout
						hr = CDXDevice11::CreateInputLayout( pDevice, layout, ARRAYSIZE(layout), (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pVSBlob), 
							CDXBlob::GetBufferSize(pVSBlob), &g_pVertexLayout );
						if(hr != S_OK)
						{
							break;
						}

						hr = D3DX11CompileFromMemory( g_strShaderCode, strlen(g_strShaderCode), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob, NULL, NULL);
						if(hr != S_OK)
						{
							break;
						}
						// Create the pixel shader
						hr = CDXDevice11::CreatePixelShader(pDevice, (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pPSBlob), CDXBlob::GetBufferSize(pPSBlob), NULL, &g_pPixelShader);
						if(hr != S_OK)
						{
							break;
						}
						SimpleVertex vertices[] =
						{
// 							{-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
// 							{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
// 							{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
// 							{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
// 							{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
// 							{1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
                            { 1.0f,  1.0f, 1.0f, 1.0f, 0.0f },
                            { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f },		
                            {-1.0f, -1.0f, 1.0f, 0.0f, 1.0f },

                            {-1.0f, -1.0f, 1.0f, 0.0f, 1.0f },
                            {-1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
                            { 1.0f,  1.0f, 1.0f, 1.0f, 0.0f },		
						};


						D3D11_BUFFER_DESC bd;
						ZeroMemory( &bd, sizeof(bd) );
						bd.Usage = 1/*0*//*D3D11_USAGE_DEFAULT*/;
						bd.ByteWidth = sizeof( SimpleVertex ) * 6;
						bd.BindFlags = 1/*D3D11_BIND_VERTEX_BUFFER*/;
						bd.CPUAccessFlags = 0;
						D3D11_SUBRESOURCE_DATA InitData;
						ZeroMemory( &InitData, sizeof(InitData) );
						InitData.pSysMem = vertices;
						hr = CDXDevice11::CreateBuffer(pDevice, &bd, &InitData, &g_pVertexBuffer);
						if(hr != S_OK)
						{
							break;
						}
						D3D11_SAMPLER_DESC sampDesc;
						ZeroMemory( &sampDesc, sizeof(sampDesc) );
						sampDesc.Filter = 0x15/*D3D11_FILTER_MIN_MAG_MIP_LINEAR*/;
						sampDesc.AddressU = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
						sampDesc.AddressV = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
						sampDesc.AddressW = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
						sampDesc.ComparisonFunc = 1/*D3D11_COMPARISON_NEVER*/;
						sampDesc.MinLOD = 0;
						sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
						hr = CDXDevice11::CreateSamplerState(pDevice, &sampDesc, &g_pSamplerPoint);
						if(hr != S_OK)
						{
							break;
						}
						D3D11_RASTERIZER_DESC rasterizerDesc;
						ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
						rasterizerDesc.CullMode = 1/*3*//*D3D11_CULL_BACK*/;
						rasterizerDesc.FillMode = 3/*D3D11_FILL_SOLID*/;
						hr = CDXDevice11::CreateRasterizerState(pDevice, &rasterizerDesc, &g_pRasterizerState);
						if(hr != S_OK)
						{
							break;
						}


                        D3D11_DEPTH_STENCIL_DESC dsDesc;		
                        ZeroMemory(&dsDesc, sizeof(dsDesc));
                        //dsDesc.DepthEnable = false;
                        hr = CDXDevice11::CreateDepthStencilState(pDevice, &dsDesc, &g_pDepthStencilState);
                        if(hr != S_OK)
                        {
                            break;
                        }



// 						D3D11_BLEND_DESC blendDesc;
// 						ZeroMemory(&blendDesc, sizeof(blendDesc));
// 						blendDesc.RenderTarget[0].BlendEnable = true;
// 						blendDesc.RenderTarget[0].BlendOp = 1/*D3D11_BLEND_OP_ADD*/;
// 						blendDesc.RenderTarget[0].BlendOpAlpha = 1/*D3D11_BLEND_OP_ADD*/;
// 						blendDesc.RenderTarget[0].RenderTargetWriteMask = 0xf/*D3D11_COLOR_WRITE_ENABLE_ALL*/;
// 						blendDesc.RenderTarget[0].SrcBlend = 5/*D3D11_BLEND_SRC_ALPHA*/;
// 						blendDesc.RenderTarget[0].SrcBlendAlpha = 2/*D3D11_BLEND_ONE*/;
// 						blendDesc.RenderTarget[0].DestBlend = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;
// 						blendDesc.RenderTarget[0].DestBlendAlpha = 1/*D3D11_BLEND_ZERO*/;
// 						blendDesc.RenderTarget[1].BlendEnable = false;

                        D3D11_BLEND_DESC blend;

                        D3D11_RENDER_TARGET_BLEND_DESC rtbd;
                        ZeroMemory( &rtbd, sizeof(rtbd) );
                        ZeroMemory( &blend, sizeof(blend) );


                        rtbd.BlendEnable			 = true;
                        rtbd.SrcBlend				 = 5/*D3D11_BLEND_SRC_ALPHA*/;	
                        rtbd.DestBlend				 = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;	
                        rtbd.BlendOp				 = 1/*D3D11_BLEND_OP_ADD*/;
                        rtbd.SrcBlendAlpha			 = 5/*D3D11_BLEND_SRC_ALPHA*/;
                        rtbd.DestBlendAlpha			 = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;
                        rtbd.BlendOpAlpha			 = 1/*D3D11_BLEND_OP_ADD*/;
                        rtbd.RenderTargetWriteMask	 = 0xf/*D3D11_COLOR_WRITE_ENABLE_ALL*/;

                        blend.AlphaToCoverageEnable=false;
                        blend.IndependentBlendEnable=false; //true;
                        blend.RenderTarget[0]=rtbd;


						hr = CDXDevice11::CreateBlendState(pDevice, &blend, &g_pBlendState);
						if(hr != S_OK)
						{
							break;
						}

					} while (FALSE);
					if(pVSBlob)
					{
						CUnknown::Release(pVSBlob);
					}
					if(pPSBlob)
					{
						CUnknown::Release(pPSBlob);
					}
				}
				if(bResize || !m_pOSTexture) 
				{
					if(m_pOSTexture)
					{
						CUnknown::Release(m_pOSTexture);
					}
					D3D11_TEXTURE2D_DESC td = {0};
					td.Width = m_nWidth;
					td.Height = m_nHeight;
					td.Format = 87;
					td.ArraySize = 1;
					td.Count = 1;
					td.MipLevels = 1;
					td.Usage = 2/*D3D11_USAGE_DYNAMIC*/;
                    td.BindFlags = 8/*D3D11_BIND_SHADER_RESOURCE*/;
					td.CPUAccessFlags = 0x10000/*D3D11_CPU_ACCESS_WRITE*/;
					CDXDevice11::CreateTexture2D(pDevice, &td, NULL, &m_pOSTexture);
					if(m_pOSTexture)
					{
                        if(g_pTextureRV)
                        {
                            CUnknown::Release(g_pTextureRV);
                        }
						D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {0};
						srvd.Format = td.Format;
						srvd.ViewDimension = 4/*D3D11_SRV_DIMENSION_TEXTURE2D*/;
						srvd.MipLevels = td.MipLevels;
						CDXDevice11::CreateShaderResourceView(pDevice, m_pOSTexture, &srvd, &g_pTextureRV);
					}
				}

				// render overlay
				if(m_pOSTexture)
				{
					if(m_smMgr.GetHeader()->bHasImage)
					{
                        D3D11_MAPPED_SUBRESOURCE data = {0};
						if(CDXDeviceContext11::Map(pDeviceContext, m_pOSTexture, 0, 4/*D3D11_MAP_WRITE_DISCARD*/, 0, &data) == S_OK)
						{
							CArcAutoLock lock;
							int nLen = m_nWidth * m_nHeight;
                            if(data.RowPitch == m_nWidth * 4)
                            {
                                memcpy(data.pData, m_smMgr.GetBits(), nLen * 4);
                            }
                            else
                            {
                                LPDWORD pSrc = (LPDWORD)m_smMgr.GetBits();
                                LPDWORD pDest = (LPDWORD)data.pData;
							    for(int i = 0; i < nLen; i++)
							    {
								    // ARGB
								    *pDest++ = *pSrc++;
								    if((i + 1) % m_nWidth == 0)
								    {
									    pDest += data.RowPitch / 4 - m_nWidth;
								    }
							    }
                            }
							CDXDeviceContext11::Unmap(pDeviceContext, m_pOSTexture, 0);
						}

                        //create a rendertarget
                        LPVOID pBackBuffer = NULL;
                        HRESULT hr = CDXSwapChian::GetBuffer(pSwapChain,  0, IID_ID3D10Texture2D, ( LPVOID* )&pBackBuffer );
                        if( FAILED( hr ) )
                            return;

                        hr = CDXDevice11::CreateRenderTargetView(pDevice, pBackBuffer, NULL, &g_pRenderTargetView );
                        CUnknown::Release(pBackBuffer);



                        //                         LPVOID ExtraRenderTargetTexture = NULL;
                        //                         LPVOID ExtraRenderTarget = NULL;
                        //                         D3D11_TEXTURE2D_DESC texdesc;
                        //                         texdesc.ArraySize=1;
                        //                         texdesc.BindFlags=/*D3D11_BIND_RENDER_TARGET*/0x20L | /*D3D11_BIND_SHADER_RESOURCE*/0x8L;
                        //                         texdesc.CPUAccessFlags=0;
                        //                         texdesc.Format=/*DXGI_FORMAT_R8G8B8A8_UNORM*/28;
                        //                         texdesc.Height=m_nHeight;
                        //                         texdesc.Width=m_nWidth;
                        //                         texdesc.MipLevels=1;
                        //                         texdesc.MiscFlags=0;
                        //                         texdesc.Count=1;
                        //                         texdesc.Quality=0;
                        //                         texdesc.Usage=/*D3D11_USAGE_DEFAULT*/0;
                        // 
                        //                         if (SUCCEEDED(CDXDevice11::CreateTexture2D(pDevice, &texdesc, NULL, &ExtraRenderTargetTexture)))
                        //                         {
                        //                             CDXDevice11::CreateRenderTargetView(pDevice, ExtraRenderTargetTexture, NULL,  &ExtraRenderTarget);
                        //                         }		


                        // Create depth stencil texture

                        LPVOID pDepthStencil = NULL;
                        D3D11_TEXTURE2D_DESC descDepth;
                        ZeroMemory( &descDepth, sizeof(descDepth) );
                        descDepth.Width = m_nWidth;
                        descDepth.Height = m_nHeight;
                        descDepth.MipLevels = 1;
                        descDepth.ArraySize = 1;
                        descDepth.Format = 45/*DXGI_FORMAT_D24_UNORM_S8_UINT*/;
                        descDepth.Count = 1;
                        descDepth.Quality = 0;
                        descDepth.Usage = 0/*D3D11_USAGE_DEFAULT*/;
                        descDepth.BindFlags = 0x40L/*D3D11_BIND_DEPTH_STENCIL*/;
                        descDepth.CPUAccessFlags = 0;
                        descDepth.MiscFlags = 0;
                        hr = CDXDevice11::CreateTexture2D(pDevice, &descDepth, NULL, &pDepthStencil );
                        if( FAILED( hr ) )
                            return;

                        // Create the depth stencil view
                        LPVOID pDepthStencilView = NULL;
                        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
                        ZeroMemory( &descDSV, sizeof(descDSV) );
                        descDSV.Format = descDepth.Format;
                        descDSV.ViewDimension = 3/*D3D11_DSV_DIMENSION_TEXTURE2D*/;
                        descDSV.Texture2D.MipSlice = 0;
                        hr = CDXDevice11::CreateDepthStencilView(pDevice,  pDepthStencil, &descDSV, &g_pDepthStencilView );
                        if( FAILED( hr ) )
                            return;


                        LPVOID pOldGeometryShader = NULL;
                        CDXDeviceContext11::GSGetShader(pDeviceContext, &pOldGeometryShader, NULL, NULL);
                        LPVOID pOldVertexShader = NULL;
                        CDXDeviceContext11::VSGetShader(pDeviceContext, &pOldVertexShader, NULL, NULL);
                        LPVOID pOldSampler = NULL;
                        CDXDeviceContext11::PSGetSamplers(pDeviceContext, 0, 1, &pOldSampler);


                        LPVOID pOldLayout = NULL;
                        CDXDeviceContext11::IAGetInputLayout(pDeviceContext, &pOldLayout);
                        LPVOID pOldVertextBuffers = NULL;
                        UINT uOldSride = 0;
                        UINT uOldOffset = 0;
                        CDXDeviceContext11::IAGetVertexBuffers(pDeviceContext, 0, 1, &pOldVertextBuffers, &uOldSride, &uOldOffset);
                        UINT uOldPrimitiveType = 0;
                        CDXDeviceContext11::IAGetPrimitiveTopology(pDeviceContext, &uOldPrimitiveType);

                        LPVOID pOldPixelShader = NULL;
                        CDXDeviceContext11::PSGetShader(pDeviceContext, &pOldPixelShader, NULL, NULL);




                        LPVOID oldRenderTarget = NULL;
                        LPVOID oldDepthStencilView = NULL;
                        CDXDeviceContext11::OMGetRenderTargets(pDeviceContext, 1, &oldRenderTarget, &oldDepthStencilView);


                        LPVOID pOldBlendState = NULL;
                        float fOldFactor[4] = {0.0f};
                        UINT uOldSampleMask = 0;
                        CDXDeviceContext11::OMGetBlendState(pDeviceContext, &pOldBlendState, fOldFactor, &uOldSampleMask);
                        LPVOID pOldDepthStencilState = NULL;
                        UINT uOldDepthRef = 0;
                        CDXDeviceContext11::OMGetDepthStencilState(pDeviceContext, &pOldDepthStencilState, &uOldDepthRef);


                        LPVOID pOldRSState = NULL;
                        CDXDeviceContext11::RSGetState(pDeviceContext, &pOldRSState);


                        UINT oldviewports=0;
                        D3D11_VIEWPORT viewports[16];
                        CDXDeviceContext11::RSGetViewports(pDeviceContext, &oldviewports, NULL);
                        CDXDeviceContext11::RSGetViewports(pDeviceContext, &oldviewports, viewports);


                        /*************************************************************************************/
                        CDXDeviceContext11::GSSetShader(pDeviceContext, NULL,  NULL, 0);
                        CDXDeviceContext11::VSSetShader(pDeviceContext, g_pVertexShader, NULL, 0);
                        CDXDeviceContext11::PSSetSamplers(pDeviceContext, 0, 1, &g_pSamplerPoint);
                        D3D11_VIEWPORT vp;
                        vp.Width = (float)m_nWidth;
                        vp.Height = (float)m_nHeight;
                        vp.MinDepth = 0.0f;
                        vp.MaxDepth = 1.0f;
                        vp.TopLeftX = 0;
                        vp.TopLeftY = 0;
                        CDXDeviceContext11::RSSetViewports(pDeviceContext, 1, &vp );

                        CDXDeviceContext11::OMSetRenderTargetsAndUnorderedAccessViews(pDeviceContext, 0,NULL,NULL,0,0,NULL,NULL);
                        CDXDeviceContext11::OMSetRenderTargets(pDeviceContext,1, &g_pRenderTargetView, g_pDepthStencilView);		
                        CDXDeviceContext11::ClearDepthStencilView(pDeviceContext, g_pDepthStencilView, 1/*D3D11_CLEAR_DEPTH*/, 1.0f, 0 );


						CDXDeviceContext11::IASetInputLayout(pDeviceContext, g_pVertexLayout);
						UINT stride = sizeof( SimpleVertex );
						UINT offset = 0;
						CDXDeviceContext11::IASetVertexBuffers(pDeviceContext, 0, 1, &g_pVertexBuffer, &stride, &offset);
						CDXDeviceContext11::IASetPrimitiveTopology(pDeviceContext, 4/*D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/);
						CDXDeviceContext11::PSSetShader(pDeviceContext, g_pPixelShader, NULL, 0);
						CDXDeviceContext11::PSSetShaderResources(pDeviceContext, 0, 1, &g_pTextureRV);
						CDXDeviceContext11::RSSetState(pDeviceContext, g_pRasterizerState);
						float f[4] = {1.0f};
						CDXDeviceContext11::OMSetBlendState(pDeviceContext, g_pBlendState, f, (DWORD)-1);
						CDXDeviceContext11::OMSetDepthStencilState(pDeviceContext, g_pDepthStencilState, 0);
						CDXDeviceContext11::Draw(pDeviceContext, 6, 0);
                        /*************************************************************************************/

                        if(pOldGeometryShader)
                        {
                            CDXDeviceContext11::GSSetShader(pDeviceContext, pOldGeometryShader, NULL, NULL);
                            CUnknown::Release(pOldGeometryShader);
                        }

                        if(pOldVertexShader)
                        {
                            CDXDeviceContext11::VSSetShader(pDeviceContext, pOldVertexShader, NULL, NULL);
                            CUnknown::Release(pOldVertexShader);
                        }

                        if(pOldSampler)
                        {
                            CDXDeviceContext11::PSSetSamplers(pDeviceContext, 0, 1, &pOldSampler);
                            CUnknown::Release(pOldSampler);
                        }

                        if(pOldLayout)
                        {
                            CDXDeviceContext11::IASetInputLayout(pDeviceContext, pOldLayout);
                            CUnknown::Release(pOldLayout);
                        }
                        if(pOldVertextBuffers)
                        {
                            CDXDeviceContext11::IASetVertexBuffers(pDeviceContext, 0, 1, &pOldVertextBuffers, &uOldSride, &uOldOffset);
                            CUnknown::Release(pOldVertextBuffers);
                        }
                        if(uOldPrimitiveType)
                        {
                            CDXDeviceContext11::IASetPrimitiveTopology(pDeviceContext, uOldPrimitiveType);
                        }

                        if(pOldPixelShader)
                        {
                            CDXDeviceContext11::PSSetShader(pDeviceContext, pOldPixelShader, NULL, NULL);
                            CUnknown::Release(pOldPixelShader);
                        }

                        if(pOldRSState)
                        {
                            CDXDeviceContext11::RSSetState(pDeviceContext, pOldRSState);
                            CUnknown::Release(pOldRSState);
                        }

                        if (oldRenderTarget)
                        {
                            CDXDeviceContext11::OMSetRenderTargets(pDeviceContext, 1, &oldRenderTarget, oldDepthStencilView);
                            CUnknown::Release(oldRenderTarget);
                        }
                        if(pOldBlendState)
                        {
                            CDXDeviceContext11::OMSetBlendState(pDeviceContext, pOldBlendState, fOldFactor, uOldSampleMask);
                            CUnknown::Release(pOldBlendState);
                        }
                        if(pOldDepthStencilState)
                        {
                            CDXDeviceContext11::OMSetDepthStencilState(pDeviceContext, pOldDepthStencilState, uOldDepthRef);
                            CUnknown::Release(pOldDepthStencilState);
                        }

                        if (pDepthStencil)
                        {
                            CUnknown::Release(pDepthStencil);
                        }

                        if (g_pDepthStencilView)
                        {
                            CUnknown::Release(g_pDepthStencilView);
                        }

                        if (g_pRenderTargetView)
                        {
                            CUnknown::Release(g_pRenderTargetView);
                        }

                        CDXDeviceContext11::RSSetViewports(pDeviceContext, oldviewports, viewports);
					}
				}
				CUnknown::Release(pDeviceContext);
			}
			CUnknown::Release(pDevice);
		}
	}
}

HWND CCoreHook::GetGameWnd()
{
	return m_hGame;
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

CString CCoreHook::GetCreateProcessFileName( LPCTSTR szApplicationName, LPCTSTR szCommandLine )
{
    return GetFileNameByFullPath(GetCreateProcessFileFullPath(szApplicationName, szCommandLine));    
    
// 	TCHAR szPath[MAX_PATH] = {0};
// 	if(szApplicationName)
// 	{
// 		lstrcpyn(szPath, szApplicationName, Tsizeof(szPath));
// 		PathStripPath(szPath);
// 	}
// 	else if(szCommandLine[0] == _T('\"'))
// 	{
// 		int nFind = 1;
// 		for(; szCommandLine[nFind] && szCommandLine[nFind] != _T('\"'); nFind++);
// 		lstrcpyn(szPath, &szCommandLine[1], nFind);
// 		PathStripPath(szPath);
// 	}
// 	return szPath;
}



CString CCoreHook::GetCreateProcessFileFullPath( LPCTSTR szApplicationName, LPCTSTR szCommandLine )
{
    TCHAR szPath[MAX_PATH] = {0};
    if(szApplicationName)
    {
        lstrcpyn(szPath, szApplicationName, Tsizeof(szPath));
    }
    else if(szCommandLine[0] == _T('\"'))
    {
        int nFind = 1;
        for(; szCommandLine[nFind] && szCommandLine[nFind] != _T('\"'); nFind++);
        lstrcpyn(szPath, &szCommandLine[1], nFind);
    }
    return szPath;
}

CString CCoreHook::GetFileNameByFullPath(LPCTSTR lpFileFullPath)
{
    TCHAR szPath[MAX_PATH] = {0};

    lstrcpyn(szPath, lpFileFullPath, Tsizeof(szPath));
    PathStripPath(szPath);

    return szPath;
}

BOOL CCoreHook::ExeIsWebBrowser(LPCTSTR szExeFileName)
{
	if(lstrcmpi(szExeFileName, _T("iexplore.exe")) == 0
		|| lstrcmpi(szExeFileName, _T("firefox.exe")) == 0
		|| lstrcmpi(szExeFileName, _T("chrome.exe")) == 0
		|| lstrcmpi(szExeFileName, _T("opera.exe")) == 0
		|| lstrcmpi(szExeFileName, _T("Safari.exe")) == 0
		|| lstrcmpi(szExeFileName, _T("dxsetup.exe")) == 0)

	{
		return TRUE;
	}
	return FALSE;
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
{
	if(!g_pShowCursor)
	{
		return 0;
	}
	return (*g_pShowCursor)(bShow);
}

int CCoreHook::GetGameType()
{
	TCHAR szGameType[64];
	if(!GetEnvironmentVariable(_T("ArcGameType"), szGameType, Tsizeof(szGameType)))
	{
		return 0;
	}
	return _ttoi(szGameType);
}

WORD CCoreHook::FileMachineType( LPCTSTR lpExecuteFile )
{
    FILE *pfile = _tfopen(lpExecuteFile, L"rb");
    if (NULL == pfile)
        return (WORD)(0);

    IMAGE_DOS_HEADER idh;
    fread(&idh, sizeof(idh), 1, pfile);

    IMAGE_FILE_HEADER ifh;
    fseek(pfile, idh.e_lfanew + 4, SEEK_SET);
    fread(&ifh, sizeof(ifh), 1, pfile);
    fclose(pfile);

    return ifh.Machine;
}


bool CCoreHook::InitShimEngineHookFixHandler()
{
    return Get_Method_In_Ntdll();
}


void CCoreHook::FixShimEngineHookIssues()
{
    Hook_Proc_With_Method_In_Ntdll();
}


void CCoreHook::ReleaseShimEngineHookFixHandler()
{
    UnHook_Proc_Hooked_By_Method_In_Ntdll();
}

bool CCoreHook::Get_Method_In_Ntdll()
{
    HMODULE hMod = GetModuleHandle(_T("ntdll.dll"));
    if (NULL == hMod)
    {
        OutputDebugString(_T("GetModuleHandle ntdll.dll failed"));
        return false;
    }

    g_pRtlInitAnsiString = (FUNC_RtlInitAnsiString)GetProcAddress(hMod, "RtlInitAnsiString");
    g_pLdrGetProcedureAddress = (FUNC_LdrGetProcedureAddress)GetProcAddress(hMod, "LdrGetProcedureAddress");

    //win8 :LdrGetProcedureAddressForCaller replaces LdrGetProcedureAddress
    g_pLdrGetProcedureAddressForCaller = (FUNC_LdrGetProcedureAddressForCaller)GetProcAddress(hMod, "LdrGetProcedureAddressForCaller"); 
    if (NULL == g_pLdrGetProcedureAddressForCaller)
        OutputDebugString(_T("LdrGetProcedureAddressForCaller failed"));

    if (NULL == g_pLdrGetProcedureAddress || NULL == g_pRtlInitAnsiString)
    {
        OutputDebugString(_T("GetProcAddress RtlInitAnsiString or LdrGetProcedureAddress failed"));
        return false;
    }

    return true;
}

void CCoreHook::Hook_Proc_With_Method_In_Ntdll()
{
    HMODULE hMod = GetModuleHandle(_T("kernel32.dll"));
    if(NULL == hMod)
    {
        OutputDebugString(_T("GetModuleHandle kernel32.dll failed"));
        return; 
    }

    ANSI_STRING name;
    (*g_pRtlInitAnsiString)(&name, "CreateProcessW");

    (*g_pLdrGetProcedureAddress)(hMod, &name, 0, (PVOID*)&g_pLdrCreateProcessW);
    DetourAttach(&(PVOID&)g_pLdrCreateProcessW, &MyLdrCreateProcessW);   

    (*g_pRtlInitAnsiString)(&name, "CreateProcessA");

    (*g_pLdrGetProcedureAddress)(hMod, &name, 0, (PVOID*)&g_pLdrCreateProcessA);
    DetourAttach(&(PVOID&)g_pLdrCreateProcessA, &MyLdrCreateProcessA);  

    if (g_pLdrCreateProcessW == g_pCreateProcessW && NULL != g_pLdrGetProcedureAddressForCaller)  //means win8 platform
    {
        (*g_pRtlInitAnsiString)(&name, "CreateProcessW");
        (*g_pLdrGetProcedureAddressForCaller)(hMod, &name, 0, (PVOID*)&g_pLdrForCallerCreateProcessW, 0, NULL);
        DetourAttach(&(PVOID&)g_pLdrForCallerCreateProcessW, &MyLdrForCallerCreateProcessW);   

        (*g_pRtlInitAnsiString)(&name, "CreateProcessA");
        (*g_pLdrGetProcedureAddressForCaller)(hMod, &name, 0, (PVOID*)&g_pLdrForCallerCreateProcessA, 0, NULL);
        DetourAttach(&(PVOID&)g_pLdrForCallerCreateProcessA, &MyLdrForCallerCreateProcessA);  
    }
}

void CCoreHook::UnHook_Proc_Hooked_By_Method_In_Ntdll()
{
    if (NULL == g_pLdrCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pLdrCreateProcessW, &MyLdrCreateProcessW);   
    }

    if (NULL == g_pLdrCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pLdrCreateProcessA, &MyLdrCreateProcessA);   
    }

    if (NULL == g_pLdrForCallerCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pLdrForCallerCreateProcessW, &MyLdrForCallerCreateProcessW);   
    }

    if (NULL == g_pLdrForCallerCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pLdrForCallerCreateProcessA, &MyLdrForCallerCreateProcessA);   
    }
}


void CCoreHook::CheckAndHookDX()
{
    if(g_CoreHook.IsInGame() || g_CoreHook.GetGameType() > 0)
    {
        DXMODULE_INFO dmi = {0}; 
        if(dmi.hMod = GetModuleHandle(_T("d3d8.dll")))
        {
            dmi.dwVer = DX_D3D8;
            HookD3D(&dmi);
        }
        if(dmi.hMod = GetModuleHandle(_T("d3d9.dll")))
        {
            dmi.dwVer = DX_D3D9;
            HookD3D(&dmi);
        }
        if((dmi.hMod = GetModuleHandle(_T("d3d11.dll"))) && GetModuleHandle(_T("dxgi.dll")))
        {
            dmi.dwVer = DX_D3D11;
            HookD3D(&dmi);
        }
    }
}

void CCoreHook::HookIDXGIFactory( const GUID * riid, LPVOID pFactory )
{
	if(g_pCreateSwapChain)
	{
		return;
	}
	if(*riid == IID_IDXGIFactory1 || *riid == IID_IDXGIFactory2)
    {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		g_pCreateSwapChain = (FUNC_CreateSwapChain)GetVirtualCall(pFactory, 10);
		DetourAttach(&(PVOID&)g_pCreateSwapChain, &MyCreateSwapChain);
		if(*riid == IID_IDXGIFactory2)
		{
			g_pCreateSwapChainForHWND = (FUNC_CreateSwapChainForHWND)GetVirtualCall(pFactory, 15);
			DetourAttach(&(PVOID&)g_pCreateSwapChainForHWND, &MyCreateSwapChainForHWND);
		}
		DetourTransactionCommit();
	}
}

