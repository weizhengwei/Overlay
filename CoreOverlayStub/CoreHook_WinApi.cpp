#include "stdafx.h"
#include "CoreHook_WinApi.h"
#include "CoreHook.h"
#include "CoreHook_Imp.h"


#define FAKE_CMDLINE_LENGTH		4096

extern Hook g_Hook;
//////////////////////////////////////////////////////////////////////////
// detoured functions
// switch off the complier optimization to assure standard function stack.
#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
// win32 hooked APIs
extern PDETOUR_CREATE_PROCESS_ROUTINEA g_pCreateProcessA;
extern PDETOUR_CREATE_PROCESS_ROUTINEW g_pCreateProcessW;

BOOL WINAPI MyCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                             BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
                             LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(lpApplicationName, lpCommandLine);

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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


// begin :vista and above, handle shim engine [10/24/2014 liuyu]

extern PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrCreateProcessA;
extern PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrCreateProcessW;

BOOL WINAPI MyLdrCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
                                LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pLdrCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(lpApplicationName, lpCommandLine);

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pLdrCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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



extern FUNC_LdrGetProcedureAddressForCaller g_pLdrGetProcedureAddressForCaller;
extern PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrForCallerCreateProcessA;
extern PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrForCallerCreateProcessW;

BOOL WINAPI MyLdrForCallerCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                         BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPSTR lpCurrentDirectory, 
                                         LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    USES_CONVERSION;
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(A2T(lpApplicationName), A2T(lpCommandLine));

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pLdrForCallerCreateProcessA)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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
    CString strExeName = CoreHook::Unity::GetCreateProcessFileName(lpApplicationName, lpCommandLine);

    if(CoreHook::Unity::ExeIsWebBrowser(strExeName) || (IMAGE_FILE_MACHINE_AMD64 == CoreHook::Unity::FileMachineType(strExeName)))
    {
        return (*g_pLdrForCallerCreateProcessW)(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    }
    else
    {
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


extern FUNC_RtlInitAnsiString g_pRtlInitAnsiString;


// end [10/24/2014 liuyu]

extern FUNC_GetCursorPos g_pGetCursorPos;
BOOL WINAPI MyGetCursorPos(LPPOINT lpPoint)
{
    if(g_CoreHook.IsOverlayPoppingup())
    {
        lpPoint->x = g_CoreHook.m_pt.x;
        lpPoint->y = g_CoreHook.m_pt.y;
        return TRUE;
    }
    return (*g_pGetCursorPos)(lpPoint);
}

extern FUNC_SetCursorPos g_pSetCursorPos;
BOOL WINAPI MySetCursorPos(int X, int Y)
{
    if(g_CoreHook.IsOverlayPoppingup())
    {
        return TRUE;
    }
    return (*g_pSetCursorPos)(X, Y);
}

extern FUNC_ShowCursor g_pShowCursor;
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

extern FUNC_GetAsyncKeyState g_pGetAsyncKeyState;
SHORT WINAPI MyGetAsyncKeyState(int vKey)
{
    if(g_CoreHook.IsOverlayPoppingup())
    {
        return 0;
    }
    return (*g_pGetAsyncKeyState)(vKey);
}

extern FUNC_GetRawInputBuffer g_pGetRawInputBuffer;
UINT WINAPI MyGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
    if(g_CoreHook.IsOverlayPoppingup())
    {
        return -1;
    }
    return (*g_pGetRawInputBuffer)(pData, pcbSize, cbSizeHeader);
}

extern FUNC_GetRawInputData g_pGetRawInputData;
UINT WINAPI MyGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
    if(g_CoreHook.IsOverlayPoppingup())
    {
        return -1;
    }
    return (*g_pGetRawInputData)(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
}

extern FUNC_LoadLibraryExW g_pLoadLibraryExW;
HMODULE WINAPI MyLoadLibraryExW(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE hMod = (*g_pLoadLibraryExW)(lpFileName, hFile, dwFlags);
    if(lpFileName)
    {
        g_Hook.HookD3D();
    }	

    return hMod;
}

extern FUNC_SetCursor g_pSetCursor;
HCURSOR WINAPI MySetCursor(HCURSOR hCursor)
{
    if(g_CoreHook.IsOverlayPoppingup() && g_CoreHook.m_hCurrCursor)
    {
        hCursor = g_CoreHook.m_hCurrCursor;
    }
    return (*g_pSetCursor)(hCursor);
}

#pragma optimize("", on)

