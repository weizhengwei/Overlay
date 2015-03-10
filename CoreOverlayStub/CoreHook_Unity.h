#pragma once

namespace CoreHook
{
    class Unity
    {
    public:
        static LPVOID GetVirtualCall(LPVOID pObject, int nIndex);

        static LPVOID ScanCode(LPVOID pStart, DWORD dwLen, const BYTE pChar[], DWORD dwCharLen);

        static HMODULE GetModuleByPrefix(LPCWSTR lpPrefix);

        static WORD FileMachineType( LPCTSTR lpExecuteFile );

        static CString GetFileNameByFullPath(LPCTSTR lpFileFullPath);

        static BOOL ExeIsWebBrowser(LPCTSTR szExeFileName);

        static CString GetCreateProcessFileFullPath(LPCTSTR szApplicationName, LPCTSTR szCommandLine);

        static CString GetCreateProcessFileName(LPCTSTR szApplicationName, LPCTSTR szCommandLine);



    };

}



