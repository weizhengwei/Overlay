#pragma once


//////////////////////////////////////////////////////////////////////////
// win32 hooked APIs
typedef int (WINAPI * FUNC_ShowCursor)(BOOL bShow);
int WINAPI MyShowCursor(BOOL bShow);

typedef HCURSOR (WINAPI * FUNC_SetCursor)(HCURSOR hCursor);
HCURSOR WINAPI MySetCursor(HCURSOR hCursor);

typedef BOOL (WINAPI * FUNC_GetCursorPos)(LPPOINT lpPoint);
BOOL WINAPI MyGetCursorPos(LPPOINT lpPoint);

typedef BOOL (WINAPI * FUNC_SetCursorPos)(int X, int Y);
BOOL WINAPI MySetCursorPos(int X, int Y);

typedef SHORT (WINAPI * FUNC_GetAsyncKeyState)(int vKey);
SHORT WINAPI MyGetAsyncKeyState(int vKey);

typedef UINT (WINAPI * FUNC_GetRawInputBuffer)(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);
UINT WINAPI MyGetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);

typedef HMODULE (WINAPI * FUNC_LoadLibraryExW)(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI MyLoadLibraryExW(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags);

typedef UINT (WINAPI * FUNC_GetRawInputData)(
    HRAWINPUT          hRawInput,
    UINT               uiCommand,
    LPVOID             pData,
    PUINT              pcbSize,
    UINT               cbSizeHeader);
UINT WINAPI MyGetRawInputData(
    HRAWINPUT          hRawInput,
    UINT               uiCommand,
    LPVOID             pData,
    PUINT              pcbSize,
    UINT               cbSizeHeader);


BOOL WINAPI MyCreateProcessA(
    LPCSTR                lpApplicationName, 
    LPSTR                 lpCommandLine, 
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles, 
    DWORD                 dwCreationFlags, 
    LPVOID                lpEnvironment, 
    LPSTR                 lpCurrentDirectory, 
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);


BOOL WINAPI MyCreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);



BOOL WINAPI MyLdrCreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPSTR                 lpCurrentDirectory, 
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);



BOOL WINAPI MyLdrCreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory, 
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);



typedef NTSTATUS (NTAPI* FUNC_LdrGetProcedureAddress)(
    HMODULE              ModuleHandle,
    PANSI_STRING         FunctionName OPTIONAL,
    WORD                 Oridinal OPTIONAL,
    PVOID               *FunctionAddress );


typedef NTSTATUS (NTAPI* FUNC_LdrGetProcedureAddressForCaller)(
    HMODULE              ModuleHandle,
    PANSI_STRING         FunctionName OPTIONAL,
    WORD                 Oridinal OPTIONAL,
    PVOID               *FunctionAddress, 
    BOOL                 bValue,
    PVOID               *CallbackAddress);

typedef VOID (NTAPI *FUNC_RtlInitAnsiString)( PANSI_STRING DestinationString, PCSZ SourceString );


BOOL WINAPI MyLdrForCallerCreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPSTR                 lpCurrentDirectory, 
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);


BOOL WINAPI MyLdrForCallerCreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory, 
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);


