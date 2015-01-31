#pragma once
// 常用公用函数合集
#include "atlbase.h"
#include "time.h"
#include <string>
#include <list>
#include "commctrl.h"
#include "shlwapi.h"
#include "shellapi.h"
#include "shobjidl.h"
#include "shlguid.h"
#include "shlobj.h"
#include "msi.h"
#pragma comment(lib, "version.lib")
#pragma comment(lib, "msi.lib")

#pragma warning(disable : 4290)
#include "Wininet.h"
#pragma comment(lib, "Wininet.lib")

#include "tchar.h"

// 常用宏
#define Tsizeof(x)		(sizeof(x) / sizeof(TCHAR))
#define LOSHORT(x)		((short)((DWORD)(x) & 0xffff))
#define HISHORT(x)		((short)(((DWORD)(x) >> 16) & 0xffff))

#ifdef _UNICODE
typedef std::wstring tstring ;
#else
typedef std::string tstring;
#endif

inline DWORD GetColorDifference(DWORD dwColor1, DWORD dwColor2)
{
	DWORD r1 = GetRValue(dwColor1);
	DWORD r2 = GetRValue(dwColor2);
	DWORD g1 = GetGValue(dwColor1);
	DWORD g2 = GetGValue(dwColor2);
	DWORD b1 = GetBValue(dwColor1);
	DWORD b2 = GetBValue(dwColor2);
	return (r2 -r1) * (r2 - r1) + (g2 - g1) * (g2 - g1) + (b2 - b1) * (b2 - b1);
}

inline void GetDesktopWorkingRect(RECT & rt, const POINT * ppt = NULL)
{
	if(!ppt)
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, sizeof(rt));
	}
	else
	{
		::SetRect(&rt, 0, 0, 0, 0);
		HMONITOR hMon = MonitorFromPoint(*ppt, MONITOR_DEFAULTTOPRIMARY);
		if(hMon)
		{
			MONITORINFO mi = {0};
			mi.cbSize = sizeof(mi);
			if(GetMonitorInfo(hMon, &mi))
			{
				rt = mi.rcWork;
			}
		}
	}
}

inline HWND GetTopParent(HWND hWnd)
{
	HWND hWndParent;
	while(hWndParent = GetParent(hWnd))
	{
		hWnd = hWndParent;
	}
	return hWnd;
}

inline BOOL IsXP()
{
	static BOOL s_bIsXP = -1;
	if(s_bIsXP < 0)
	{
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		s_bIsXP = (osvi.dwMajorVersion == 5);
	}
	return s_bIsXP;
}

inline DWORD GetPEFileVersion(LPCTSTR lpszFile, LPDWORD pMinor = NULL)
{
	DWORD dwSize = GetFileVersionInfoSize(lpszFile, NULL);
	if(!dwSize)
	{
		return 0;
	}
	LPVOID pBlock = new TCHAR[dwSize];
	GetFileVersionInfo(lpszFile, NULL, dwSize, pBlock);
	VS_FIXEDFILEINFO *pInfo;
	DWORD dwInfoSize;
	VerQueryValue(pBlock, _T("\\"), (LPVOID *)&pInfo, (UINT *)&dwInfoSize);
	DWORD dwMajor = pInfo->dwFileVersionMS;
	DWORD dwMinor = pInfo->dwFileVersionLS;
	delete []pBlock;
	if(pMinor)
	{
		*pMinor = dwMinor;
	}
	return dwMajor;
}

inline BOOL GetPEFileVersion(LPCTSTR lpszFile, LPTSTR szVersionText, DWORD dwSize)
{
	DWORD dwMajor, dwMinor;
	dwMajor = GetPEFileVersion(lpszFile, &dwMinor);
	if(!dwMajor)
	{
		return FALSE;
	}
	_sntprintf_s(szVersionText, dwSize, _TRUNCATE, _T("%d.%d.%d.%d"), HIWORD(dwMajor), LOWORD(dwMajor), HIWORD(dwMinor), LOWORD(dwMinor));
	return TRUE;
}

inline BOOL GetIEPath(LPTSTR szPath, DWORD dwSize)
{
	HKEY hKey;
	BOOL bRet = FALSE;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"), 0, KEY_READ, &hKey);
	if(hKey)
	{
		if(RegQueryValueEx(hKey, NULL, 0, NULL, (LPBYTE)szPath, &dwSize) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
		RegCloseKey(hKey);
	}
	return bRet;
}

inline BOOL GetDefaultBrowserPath(LPTSTR szPath, DWORD dwSize)
{
	HKEY hKey;
	BOOL bRet = FALSE;
	RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("HTTP\\shell\\open\\command"), 0, KEY_READ, &hKey);
	if(hKey)
	{
		if(RegQueryValueEx(hKey, NULL, 0, NULL, (LPBYTE)szPath, &dwSize) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
		RegCloseKey(hKey);
	}
	return bRet;
}

inline BOOL IsIEDefault()
{
	static BOOL bRet = -1;
	if(bRet < 0)
	{
		bRet = FALSE;
		TCHAR szPath[MAX_PATH];
		if(GetDefaultBrowserPath(szPath, MAX_PATH))
		{
			if(StrStrI(szPath, _T("iexplore.exe")))
			{
				bRet = TRUE;
				return bRet;
			}
		}
	}
	return bRet;
}

inline DWORD GetIEVersion()
{
	static BOOL bRet = -1;
	if(bRet < 0)
	{
		bRet = 0;
		TCHAR szPath[MAX_PATH];
		if(GetIEPath(szPath, MAX_PATH))
		{
			bRet = GetPEFileVersion(szPath) >> 16;
		}
	}
	return (DWORD)bRet;
}

inline BOOL GetTrayIconPosition(LPCTSTR lpszIconText, POINT & pt)
{
	if(lpszIconText == NULL)
	{
		return FALSE;
	}
	HWND hTrayBar = FindWindowEx(GetDesktopWindow(), NULL, _T("Shell_TrayWnd"), NULL);
	if(!hTrayBar)
	{
		return FALSE;
	}
	hTrayBar = FindWindowEx(hTrayBar, NULL, _T("TrayNotifyWnd"), NULL);
	if(!hTrayBar)
	{
		return FALSE;
	}
	hTrayBar = FindWindowEx(hTrayBar, NULL, _T("SysPager"), NULL);
	if(!hTrayBar)
	{
		return FALSE;
	}
	hTrayBar = FindWindowEx(hTrayBar, NULL, _T("ToolbarWindow32"), NULL);
	if(!hTrayBar)
	{
		return FALSE;
	}
	DWORD dwProcesssId;
	GetWindowThreadProcessId(hTrayBar, &dwProcesssId);
	if(dwProcesssId == 0)
	{
		return FALSE;
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcesssId);
	if(!hProcess)
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	RECT rt;
	const DWORD dwRemoteSize = 4096;
	LPBYTE pRemote = (LPBYTE)VirtualAllocEx(hProcess, NULL, dwRemoteSize, MEM_COMMIT, PAGE_READWRITE);
	if(pRemote)
	{
		int nCount = (int)SendMessage(hTrayBar, TB_BUTTONCOUNT, 0, 0);
		for(int i = 0; i < nCount; i++)
		{
			SendMessage(hTrayBar, TB_GETBUTTON, i, (LPARAM)pRemote);
			TBBUTTON tbb;
			if(ReadProcessMemory(hProcess, pRemote, &tbb, sizeof(tbb), NULL))
			{
				SendMessage(hTrayBar, TB_GETBUTTONTEXT, tbb.idCommand, (LPARAM)pRemote);
				TCHAR szText[MAX_PATH];
				if(ReadProcessMemory(hProcess, pRemote, szText, MAX_PATH, NULL))
				{
					if(lstrcmp(szText, lpszIconText) == 0)
					{
						if(SendMessage(hTrayBar, TB_GETITEMRECT, i, (LPARAM)pRemote))
						{						
							if(ReadProcessMemory(hProcess, pRemote, &rt, sizeof(rt), NULL))
							{
								bRet = TRUE;
								pt.x = rt.left;
								pt.y = rt.top;
								ClientToScreen(hTrayBar, &pt);
							}
						}
					}
				}
			}
		}
		VirtualFreeEx(hProcess, pRemote, dwRemoteSize, MEM_FREE);
	}
	CloseHandle(hProcess);

	if(!bRet)
	{
		HWND hButton = FindWindowEx(GetParent(GetParent(hTrayBar)), NULL, _T("Button"), NULL);
		if(hButton && IsWindowVisible(hButton))
		{
			bRet = TRUE;
			GetWindowRect(hButton, &rt);
			pt.x = rt.left;
			pt.y = rt.top;
		}
	}
	return bRet;
}

inline void PopupUrl(LPCTSTR lpszUrl)
{
	if(IsIEDefault() && GetIEVersion() < 7)
	{
		ShellExecute(NULL, _T("open"), _T("iexplore.exe"), lpszUrl, NULL, SW_SHOWMAXIMIZED);
	}
	else
	{
		ShellExecute(NULL, _T("open"), lpszUrl, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

inline BOOL _DeleteFile(LPCTSTR lpFileName, LPCTSTR szDelExt = NULL)
{
	if(!PathFileExists(lpFileName))
	{
		return TRUE;
	}
	SetFileAttributes(lpFileName, 0);
	if(!DeleteFile(lpFileName))
	{
		TCHAR szName[MAX_PATH];
		if(szDelExt && szDelExt[0])
		{
			_sntprintf_s(szName, MAX_PATH, _TRUNCATE, _T("%s.%s"), lpFileName, szDelExt);
		}
		else
		{
			_sntprintf_s(szName, MAX_PATH, _TRUNCATE, _T("%s.del"), lpFileName);
		}
		if(!MoveFileEx(lpFileName, szName, MOVEFILE_REPLACE_EXISTING))
		{
			return FALSE;
		}
		MoveFileEx(szName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		return TRUE;
	}
	return TRUE;
}

inline BOOL _CreateDirectory(LPCTSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL)
{
	int nLen = (int)lstrlen(lpPathName);
	for(int i = 0; i <= nLen; i++)
	{
		if(lpPathName[i] == _T('\\') || lpPathName[i] == _T('/') || i == nLen)
		{
			TCHAR chTemp = lpPathName[i];
			((LPTSTR)lpPathName)[i] = 0;
			if(!PathIsDirectory(lpPathName))
			{
				if(!CreateDirectory(lpPathName, lpSecurityAttributes))
				{
					((LPTSTR)lpPathName)[i] = chTemp;
					return FALSE;
				}
			}
			((LPTSTR)lpPathName)[i] = chTemp;
		}
	}
	return TRUE;
}

enum enErrorCode
{
	ecNone = 0,
	ecNetUnavailable,
	ecInitFailed,
	ecGetFileFailed,
};

class CHttpHandlerException
{
public:
	enErrorCode nErrorCode;
	tstring		strMsg;
	CHttpHandlerException(enErrorCode err, LPCTSTR msg = NULL)
	{
		nErrorCode = err;
		if(msg)
		{
			strMsg = msg;
		}
	}
};

class CHttpHandler
{
public:
	typedef struct HttpProxy
	{
		tstring strProxyName;
		tstring strUser;
		tstring strPass;
	}HTTP_PROXY;

	typedef struct GetFileParam
	{
		int nTotal;
		int nCurr;
		LPCSTR szFileName;
		LPVOID pParam;
	}GET_FILE_PARAM;
public:
	void InitValue()
	{
		m_hSession = NULL;
		m_dwTimeOut = 0;
		m_hConetext = NULL;
		m_hTemp = NULL;
	}

	CHttpHandler()
	{
		InitValue();
	}
	CHttpHandler(LPCTSTR szUserAgent, HTTP_PROXY * pProxy = NULL, DWORD dwTimeOut = 10)
	{
		InitValue();
		InitSession(szUserAgent, pProxy, dwTimeOut);
	}
	~CHttpHandler(void)
	{
		CloseSession();
	}

	void InitSession(LPCTSTR szUserAgent, HTTP_PROXY * pProxy = NULL, DWORD dwTimeOut = 10) throw(CHttpHandlerException)
	{
		m_dwTimeOut = dwTimeOut;
		if(pProxy)
		{

		}
		else
		{
			m_hSession = InternetOpen(szUserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
			if(m_hSession == NULL)
			{
				throw CHttpHandlerException(ecInitFailed);
			}
			m_hConetext = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(m_hConetext == NULL)
			{
				throw CHttpHandlerException(ecInitFailed);
			}
			dwTimeOut *= 1000;
			if(InternetSetOption(m_hSession, INTERNET_OPTION_RECEIVE_TIMEOUT , &dwTimeOut, sizeof(dwTimeOut)) == FALSE)
			{
				throw CHttpHandlerException(ecInitFailed);
			}
			if(InternetSetOption(m_hSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut)) == FALSE)
			{
				throw CHttpHandlerException(ecInitFailed);
			}
			InternetSetStatusCallback(m_hSession, InternetStatusCallback);
		}
	}
	void CloseSession()
	{
		if(m_hConetext)
		{
			CloseHandle(m_hConetext);
			m_hConetext = NULL;
		}
		if(m_hSession)
		{
			InternetCloseHandle(m_hSession);
			m_hSession = NULL;
		}
	}
	void GetFile(LPCTSTR szHost, LPCTSTR szURL, LPCTSTR szLocalName, DWORD nStartPos = 0, LPTHREAD_START_ROUTINE pFuncCallBack = NULL, LPVOID pParam = NULL, int nPort = 80) throw(CHttpHandlerException)
	{
		URL_COMPONENTS url;
		memset(&url, 0, sizeof(url));
		url.dwStructSize = sizeof(URL_COMPONENTS);
		url.nScheme = INTERNET_SCHEME_HTTP;
		url.nPort = nPort;
		url.lpszHostName = (LPTSTR)szHost;
		url.lpszUrlPath = (LPTSTR)szURL;
		TCHAR szConverUrl[1024];
		DWORD dwUrlLen = sizeof(szConverUrl);
		if(InternetCreateUrl(&url, 0, szConverUrl, &dwUrlLen) == FALSE)
		{
			throw CHttpHandlerException(ecGetFileFailed, szLocalName);
		}
		GetFile(szConverUrl, szLocalName, nStartPos, pFuncCallBack, pParam);
	}

	void GetFile(LPCTSTR szUrl, LPCTSTR szLocalName, DWORD nStartPos = 0, LPTHREAD_START_ROUTINE pFuncCallBack = NULL, LPVOID pParam = NULL) throw (CHttpHandlerException)
	{
		HINTERNET hGet = NULL;
		FILE * fp = NULL;
		const DWORD dwReadLen = 10240;
		char * p =  new char[dwReadLen];

		BOOL bRet = FALSE;
		__try
		{
			if(m_hSession == NULL)
			{
				__leave;
			}

			TCHAR szTempDir[MAX_PATH];
			StrCpyN(szTempDir, szLocalName, MAX_PATH);
			PathRemoveFileSpec(szTempDir);
			if(!_CreateDirectory(szTempDir))
			{
				__leave;
			}
			TCHAR szHeader[64];
			DWORD dwBuffLen = Tsizeof(szHeader);
			if(nStartPos == 0)
			{
				_tfopen_s(&fp, szLocalName, _T("wb"));
			}
			else
			{
				_tfopen_s(&fp, szLocalName, _T("ab"));
			}
			if(fp == NULL)
			{
				__leave;
			}
			if(nStartPos)
			{
				_sntprintf_s(szHeader, dwBuffLen, _TRUNCATE, _T("Range:bytes=%d-"), nStartPos);
				hGet = InternetOpenUrl(m_hSession, szUrl, szHeader, -1, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE, (DWORD_PTR)this);
			}
			else
			{
				hGet = InternetOpenUrl(m_hSession, szUrl, NULL, 0, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE, (DWORD_PTR)this);
			}
			if(hGet == NULL)
			{
				__leave;
			}
			if(HttpQueryInfo(hGet, HTTP_QUERY_STATUS_CODE, szHeader, &dwBuffLen, NULL) == FALSE)
			{
				__leave;
			}
			DWORD dwStatus = _ttoi(szHeader);
			if(dwStatus != HTTP_STATUS_OK && dwStatus != HTTP_STATUS_PARTIAL_CONTENT)
			{
				__leave;
			}
			dwBuffLen = Tsizeof(szHeader);
			if(HttpQueryInfo(hGet, HTTP_QUERY_CONTENT_LENGTH, szHeader, &dwBuffLen, NULL) == FALSE)
			{
				__leave;
			}
			DWORD dwLength = _ttoi(szHeader);
			DWORD dwRead;
			DWORD dwCurrLen = 0;
			while(1)
			{
				if(InternetReadFile(hGet, p, dwReadLen, &dwRead))
				{
					if(dwRead == 0)
					{
						break;
					}
					fwrite(p, dwRead, 1, fp);
					dwCurrLen += dwRead;
					if(pFuncCallBack)
					{
						GET_FILE_PARAM info;
						info.nTotal = dwLength;
						info.nCurr = dwCurrLen;
						info.szFileName = NULL;
						info.pParam = pParam;
						(*pFuncCallBack)(&info);
					}
				}
				else
				{
					__leave;
				}
			}
			bRet = TRUE;
		}
		__finally
		{
			if(p)
			{
				delete []p;
				p = NULL;
			}
			if(hGet)
			{
				InternetCloseHandle(hGet);
			}
			if(fp)
			{
				fclose(fp);
			}
			if(bRet == FALSE)
			{
				throw CHttpHandlerException(ecGetFileFailed, szLocalName);
			}
		}
	}

	char * GetMem(LPCTSTR szUrl, DWORD & dwSize, DWORD nStartPos = 0, LPTHREAD_START_ROUTINE pFuncCallBack = NULL, LPVOID pParam = NULL) throw (CHttpHandlerException)
	{
		HINTERNET hGet = NULL;
		BOOL bRet = FALSE;
		char * pRet = NULL;

		__try
		{
			if(m_hSession == NULL)
			{
				__leave;
			}

			TCHAR szHeader[64];
			DWORD dwBuffLen = Tsizeof(szHeader);
			if(nStartPos)
			{
				_sntprintf_s(szHeader, dwBuffLen, _TRUNCATE, _T("Range:bytes=%d-"), nStartPos);
				hGet = InternetOpenUrl(m_hSession, szUrl, szHeader, -1, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE, (DWORD_PTR)this);
			}
			else
			{
				hGet = InternetOpenUrl(m_hSession, szUrl, NULL, 0, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE, (DWORD_PTR)this);
			}
			if(hGet == NULL)
			{
				__leave;
			}
			if(HttpQueryInfo(hGet, HTTP_QUERY_STATUS_CODE, szHeader, &dwBuffLen, NULL) == FALSE)
			{
				__leave;
			}
			DWORD dwStatus = _ttoi(szHeader);
			if(dwStatus != HTTP_STATUS_OK)
			{
				__leave;
			}
			dwBuffLen = Tsizeof(szHeader);
			if(HttpQueryInfo(hGet, HTTP_QUERY_CONTENT_LENGTH, szHeader, &dwBuffLen, NULL) == FALSE)
			{
				__leave;
			}
			DWORD dwLength = _ttoi(szHeader);
			dwSize = dwLength;

			DWORD dwRead;
			const DWORD dwReadLen = 10240;
			pRet = new char[dwSize + dwReadLen];
			char * p = pRet;
			DWORD dwCurrLen = 0;
			while(1)
			{
				if(InternetReadFile(hGet, p, dwReadLen, &dwRead))
				{
					if(dwRead == 0)
					{
						break;
					}
					p += dwRead;
					dwCurrLen += dwRead;
					if(pFuncCallBack)
					{
						GET_FILE_PARAM info;
						info.nTotal = dwLength;
						info.nCurr = dwCurrLen;
						info.szFileName = NULL;
						info.pParam = pParam;
						(*pFuncCallBack)(&info);
					}
				}
				else
				{
					__leave;
				}
			}
			bRet = TRUE;
		}
		__finally
		{
			if(!bRet)
			{
				if(pRet)
				{
					delete []pRet;
					pRet = NULL;
				}
			}
			if(hGet)
			{
				InternetCloseHandle(hGet);
			}
		}
		return pRet;
	}
protected:
	HINTERNET	m_hSession;
private:
	static void CALLBACK InternetStatusCallback(
		HINTERNET hInternet,
		DWORD_PTR dwContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation,
		DWORD dwStatusInformationLength
		)
	{
		if(dwInternetStatus ==  INTERNET_STATUS_REQUEST_COMPLETE)
		{
			CHttpHandler * pHandler = (CHttpHandler *)dwContext;
			INTERNET_ASYNC_RESULT *Result = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
			pHandler->m_hTemp = (HINTERNET)Result->dwResult;
			SetEvent(pHandler->m_hConetext);
		}
	}
	HANDLE m_hConetext;
	DWORD	m_dwTimeOut;
	HINTERNET m_hTemp;
	BOOL Pend()
	{
		if(WaitForSingleObject(m_hConetext, m_dwTimeOut * 1000) == WAIT_OBJECT_0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
};

__declspec(selectany) const TCHAR * g_szFuncName = NULL;;
typedef void (* LOG_FUN)(const TCHAR *str, ...);
inline void Glog(const TCHAR * str, ...)
{
	const int nSize = 1024;
	TCHAR message[nSize];
	va_list argList;
	va_start(argList, str);
	_sntprintf_s(message, nSize, _TRUNCATE, _T("COCO_SX:%s, "), g_szFuncName);
	int nLen = (int)lstrlen(message);
	_vsntprintf_s(&message[nLen], nSize - nLen - 2, _TRUNCATE, str, argList);
	va_end(argList);
	lstrcat(message, _T("\n"));
	OutputDebugString(message);
}
inline LOG_FUN log_fun(const TCHAR * szFunName)
{
	g_szFuncName = szFunName;
	return Glog;
}
#define GLOG log_fun(_T(__FUNCTION__))

#ifndef _M_AMD64
inline __int64 GetCycleCount()
{
	__asm
	{
		rdtsc
	};
}
__declspec(selectany) __int64 g_i64Last = 0;
#define BEGIN_MEASURE	g_i64Last = GetCycleCount()
#define MEASURE(text)	GLOG(_T("%s : %I64d"), text, GetCycleCount() - g_i64Last);g_i64Last = GetCycleCount()
#endif

typedef BOOL (WINAPI * FUNC_EXTFOUND_SHORTCUT)(LPCTSTR, LPVOID);
inline BOOL ReadShortcut(LPCTSTR lpszLink, LPTSTR szPath, DWORD dwSize)
{
	TCHAR szProductCode[40], szComponentCode[40];
	if(MsiGetShortcutTarget(lpszLink, szProductCode, NULL, szComponentCode) == ERROR_SUCCESS)
	{
		if(MsiGetComponentPath(szProductCode, szComponentCode, szPath, &dwSize) == INSTALLSTATE_LOCAL)
		{
			return TRUE;
		}
	}

	USES_CONVERSION;
	BOOL bRet = FALSE;
	IShellLink * pLink;
	if(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pLink) >= 0)
	{
		IPersistFile * pFile;
		pLink->QueryInterface(IID_IPersistFile, (void **)&pFile);
		if(pFile)
		{
			if(pFile->Load(T2OLE((LPTSTR)lpszLink), TRUE) >= 0)
			{
				if(pLink->Resolve(NULL, 0) == NOERROR && pLink->GetPath(szPath, dwSize, NULL, 0) == NOERROR)
				{
					bRet = TRUE;
				}
			}
			pFile->Release();
		}
		pLink->Release();
	}
	return bRet;
}

inline BOOL EnumFolderToFindShortCut(LPCTSTR lpszDir, LPCTSTR lpszFileName, LPTSTR szPath, DWORD dwSize, FUNC_EXTFOUND_SHORTCUT pExtFunc = NULL, LPVOID pParam = NULL)
{
	BOOL bRet = FALSE;
	TCHAR szTemp[MAX_PATH];
	lstrcpy(szTemp, lpszDir);
	lstrcat(szTemp, _T("\\*.*"));
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(szTemp, &data);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if(lstrcmpi(data.cFileName, _T(".")) == 0 || lstrcmpi(data.cFileName, _T("..")) == 0)
			{
				continue;
			}
			lstrcpy(szTemp, lpszDir);
			PathAppend(szTemp, data.cFileName);
			if(ReadShortcut(szTemp, szTemp, Tsizeof(szTemp)))
			{
				LPTSTR pFound = StrStrI(szTemp, lpszFileName);
				if(pFound && *(pFound - 1) == _T('\\'))
				{
					lstrcpyn(szPath, szTemp, dwSize);
					bRet = TRUE;
					if(pExtFunc && pExtFunc(szPath, pParam))
					{
						bRet++;
						continue;
					}
					break;
				}
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return bRet;
}

inline BOOL FindShortcut(LPCTSTR lpszFileName, LPTSTR szPath, DWORD dwSize, FUNC_EXTFOUND_SHORTCUT pExtFunc = NULL, LPVOID pParam = NULL)
{
	BOOL bFound = FALSE;
	TCHAR szDir[MAX_PATH];
	if(SHGetSpecialFolderPath(NULL, szDir, CSIDL_STARTUP, FALSE))
	{
		bFound = EnumFolderToFindShortCut(szDir, lpszFileName, szPath, dwSize, pExtFunc, pParam);
	}
	if(SHGetSpecialFolderPath(NULL, szDir, CSIDL_COMMON_STARTUP, FALSE) && bFound != TRUE)
	{
		bFound |= EnumFolderToFindShortCut(szDir, lpszFileName, szPath, dwSize, pExtFunc, pParam);
	}
	if(SHGetSpecialFolderPath(NULL, szDir, CSIDL_APPDATA, FALSE) && bFound != TRUE)
	{
		lstrcat(szDir, _T("\\Microsoft\\Internet Explorer\\Quick Launch"));
		bFound |= EnumFolderToFindShortCut(szDir, lpszFileName, szPath, dwSize, pExtFunc, pParam);
	}
	if(SHGetSpecialFolderPath(NULL, szDir, CSIDL_DESKTOP, FALSE) && bFound != TRUE)
	{
		bFound |= EnumFolderToFindShortCut(szDir, lpszFileName, szPath, dwSize, pExtFunc, pParam);
	}
	if(SHGetSpecialFolderPath(NULL, szDir, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE) && bFound != TRUE)
	{
		bFound |= EnumFolderToFindShortCut(szDir, lpszFileName, szPath, dwSize, pExtFunc, pParam);
	}
	return bFound;
}

// 两个传入path必须都是有效的
inline BOOL PathIsEqual(LPCTSTR lpszPath1, LPCTSTR lpszPath2)
{
	TCHAR szPath1[MAX_PATH], szPath2[MAX_PATH];
	if(!GetLongPathName(lpszPath1, szPath1, sizeof(szPath1)) || !GetLongPathName(lpszPath2, szPath2, sizeof(szPath2)))
	{
		return FALSE;
	}
	PathRemoveBackslash(szPath1);
	PathRemoveBackslash(szPath2);
	return (lstrcmpi(szPath1, szPath2) == 0);
}

// 比较两个字符串版本号的大小，版本号格式应为x.x.x.x
// lpszVer1比lpszVer2版本号大的时候返回1，相等返回0，小于返回-1
inline int CompareVersion(LPCTSTR lpszVer1, LPCTSTR lpszVer2)
{
	int nVer1[4], nVer2[4];
	_stscanf_s(lpszVer1, _T("%d.%d.%d.%d"), &nVer1[0], &nVer1[1], &nVer1[2], &nVer1[3]);
	_stscanf_s(lpszVer2, _T("%d.%d.%d.%d"), &nVer2[0], &nVer2[1], &nVer2[2], &nVer2[3]);

	for(int i = 0; i < 4; i++)
	{
		if(nVer1[i] > nVer2[i])
		{
			return 1;
		}
		else if(nVer1[i] < nVer2[i])
		{
			return -1;
		}
	}
	return 0;
}

inline BOOL CopyPEFile(LPCTSTR lpszSrc, LPCTSTR lpszDst, BOOL bMove = TRUE)
{
	if(!lpszSrc || !lpszDst || lstrcmpi(lpszSrc, lpszDst) == 0)
	{
		return FALSE;
	}
	TCHAR szVerSrc[32], szVerDst[32];
	if(GetPEFileVersion(lpszSrc, szVerSrc, sizeof(szVerSrc)) && GetPEFileVersion(lpszDst, szVerDst, sizeof(szVerDst)))
	{
		if(CompareVersion(szVerSrc, szVerDst) <= 0)
		{
			// 没有目标文件版本新，不操作
			return FALSE;
		}
	}
	_DeleteFile(lpszDst);
	if(bMove)
	{
		return MoveFileEx(lpszSrc, lpszDst, MOVEFILE_REPLACE_EXISTING);
	}
	else
	{
		return CopyFile(lpszSrc, lpszDst, FALSE);
	}
}

#ifdef USE_ZLIB
#include "zlib.h"
#ifdef _DEBUG
#pragma comment(lib, "zlibD.lib")
#else
#pragma comment(lib, "zlib.lib")
#endif
#define ZLIB_FILE_MAGIC				0x83767
typedef struct tagZlibFileBlock
{
	DWORD dwMagic;	// magic number
	DWORD dwSize;	// file size
	DWORD dwOriginalSize;	// original file size
	BYTE byType;	// 0 for file, 1 for directory
	char cName[64];	// Name
}ZLIB_FILE_BLOCK;

inline BYTE * CompressFileToBuffer(LPCSTR lpszFilePath, DWORD &dwSize)
{
	BYTE * pRet = NULL;
	HANDLE hFile = CreateFile(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if(hMapping)
		{
			LPVOID pView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
			if(pView)
			{
				ULONG uSize = GetFileSize(hFile, NULL);
				DWORD dwBuffSize = compressBound(uSize) + sizeof(ZLIB_FILE_BLOCK) + 1024;
				pRet = new BYTE[dwBuffSize];
				ZLIB_FILE_BLOCK * pBlock = (ZLIB_FILE_BLOCK *)pRet;
				pBlock->dwMagic = ZLIB_FILE_MAGIC;
				pBlock->dwOriginalSize = uSize;
				pBlock->byType = 0;
				lstrcpy(pBlock->cName, PathFindFileName(lpszFilePath));
				dwSize = dwBuffSize - sizeof(ZLIB_FILE_BLOCK);
				if(compress(pRet + sizeof(ZLIB_FILE_BLOCK), &dwSize, (PBYTE)pView, uSize) != Z_OK)
				{
					dwSize = 0;
					delete []pRet;
					pRet = NULL;
				}
				else
				{
					pBlock->dwSize = dwSize;
					dwSize += sizeof(ZLIB_FILE_BLOCK);
				}
				UnmapViewOfFile(pView);
			}
			CloseHandle(hMapping);
		}
		CloseHandle(hFile);
	}
	return pRet;
}

inline BOOL CompressFile(LPCSTR lpszPath, LPCSTR lpszOutput, FILE * fp = NULL)
{
	if(!lpszPath || !PathFileExists(lpszPath))
	{
		return FALSE;
	}
	if(lpszOutput)
	{
		if(lstrcmpi(lpszPath, lpszOutput) == 0)
		{
			return FALSE;
		}
		_DeleteFile(lpszOutput);
	}
	else if(!fp)
	{
		return FALSE;
	}

	if(PathIsDirectory(lpszPath))
	{
		// directory
		FILE * localfp = NULL;
		if(!fp)
		{
			localfp = fopen(lpszOutput, "wb");
		}
		else
		{
			localfp = fp;
		}

		if(localfp)
		{
			char szTemp[MAX_PATH];
			typedef std::list<tstring> FILE_NAME_LIST;
			FILE_NAME_LIST listName;
			WIN32_FIND_DATA data;
			lstrcpy(szTemp, lpszPath);
			PathAppend(szTemp, "*.*");
			HANDLE hFind = FindFirstFile(szTemp, &data);
			if(hFind)
			{
				do 
				{
					if(lstrcmp(data.cFileName, "..") == 0 || lstrcmp(data.cFileName, ".") == 0 || lstrcmp(data.cFileName, ".svn") == 0)
					{
						continue;
					}
					listName.push_back(data.cFileName);
				} while (FindNextFile(hFind, &data));
				FindClose(hFind);
			}
			ZLIB_FILE_BLOCK block;
			block.byType = 1;
			block.dwMagic = ZLIB_FILE_MAGIC;
			block.dwSize = (DWORD)listName.size();
			lstrcpy(block.cName, PathFindFileName(lpszPath));
			fwrite(&block, sizeof(block), 1, localfp);

			for(FILE_NAME_LIST::iterator it = listName.begin(); it != listName.end(); it++)
			{
				lstrcpy(szTemp, lpszPath);
				PathAppend(szTemp, it->c_str());
				CompressFile(szTemp, NULL, localfp);
			}			
			if(!fp)
			{
				fclose(localfp);
			}
		}
	}
	else
	{
		// single file
		DWORD dwSize;
		PBYTE pBuff = CompressFileToBuffer(lpszPath, dwSize);
		if(pBuff)
		{
			FILE * localfp = NULL;
			if(!fp)
			{
				localfp = fopen(lpszOutput, "wb");
			}
			else
			{
				localfp = fp;
			}

			if(localfp)
			{
				fwrite(pBuff, dwSize, 1, localfp);
				if(!fp)
				{
					fclose(localfp);
				}
			}
			delete []pBuff;
		}		
	}
	return TRUE;
}

inline BOOL UncompressFile(LPCSTR lpszPath, LPCSTR lpszOutputDirectory, FILE * fp = NULL, BOOL bEnablePEVersionCtrl = FALSE)
{
	if(!lpszOutputDirectory || !PathIsDirectory(lpszOutputDirectory))
	{
		return FALSE;
	}
	if(lpszPath)
	{
		if(!PathFileExists(lpszPath))
		{
			return FALSE;
		}
	}
	else if(!fp)
	{
		return FALSE;
	}
	
	BOOL bRet = FALSE;
	FILE * localfp = NULL;
	if(!fp)
	{
		localfp = fopen(lpszPath, "rb");
	}
	else
	{
		localfp = fp;
	}
	if(localfp)
	{
		char szTemp[MAX_PATH];
		while (!feof(localfp))
		{
			ZLIB_FILE_BLOCK block;
			if(fread(&block, sizeof(ZLIB_FILE_BLOCK), 1, localfp) == 0 || block.dwMagic != ZLIB_FILE_MAGIC)
			{
				break;
			}
			if(block.byType == 1)
			{
				// 文件夹
				lstrcpy(szTemp, lpszOutputDirectory);
				PathAppend(szTemp, block.cName);
				CreateDirectory(szTemp, NULL);
				for(DWORD i = 0; i < block.dwSize; i++)
				{
					if(!UncompressFile(NULL, szTemp, localfp, bEnablePEVersionCtrl))
					{
						break;
					}
				}
			}
			else if(block.byType == 0)
			{
				// single file
				ULONG uBuffSize = block.dwOriginalSize + 1024;
				char * pBuff = new char[uBuffSize];
				char * pCompressBuff = new char[block.dwSize];
				if(fread(pCompressBuff, block.dwSize, 1, localfp))
				{
					if(uncompress((PBYTE)pBuff, &uBuffSize, (PBYTE)pCompressBuff, block.dwSize) == Z_OK)
					{
						lstrcpy(szTemp, lpszOutputDirectory);
						PathAppend(szTemp, block.cName);
						if((lstrcmpi(PathFindExtension(szTemp), ".dll") == 0 || lstrcmpi(PathFindExtension(szTemp), ".exe") == 0) && bEnablePEVersionCtrl)
						{
							char szPETemp[MAX_PATH];
							lstrcpy(szPETemp, lpszOutputDirectory);
							PathAppend(szPETemp, "Temp_");
							lstrcat(szPETemp, block.cName);
							FILE * nfp = fopen(szPETemp, "wb");
							if(nfp)
							{
								fwrite(pBuff, uBuffSize, 1, nfp);
								fclose(nfp);
							}
							if(!CopyPEFile(szPETemp, szTemp))
							{
								_DeleteFile(szPETemp);
							}
							bRet = TRUE;
						}
						else
						{
							_DeleteFile(szTemp);
							FILE * nfp = fopen(szTemp, "wb");
							if(nfp)
							{
								bRet = TRUE;
								fwrite(pBuff, uBuffSize, 1, nfp);
								fclose(nfp);
							}
						}
					}
				}				
				delete []pBuff;
				delete []pCompressBuff;
				break;
			}
		}
		if(!fp)
		{
			fclose(localfp);
		}
	}
	return bRet;
}
#endif

// 释放dll中的资源文件，bEnablePEVersionCtrl为TRUE时，当要释放的文件为PE文件时，将根据版本号大小决定是否替换，否则无条件替换
inline BOOL ReleaseResFile(HMODULE hMod, UINT uResID, LPCTSTR lpszFilePath)
{
	BOOL bRet = FALSE;
	HRSRC hSrc = FindResource(hMod, MAKEINTRESOURCE(uResID), _T("RESFILE"));
	if(hSrc)
	{
		HGLOBAL hg = LoadResource(hMod, hSrc);
		if(hg)
		{
			LPSTR pBuf = (LPSTR)LockResource(hg);
			if(pBuf)
			{
				DWORD dwSize = SizeofResource(hMod, hSrc);
				LPTSTR szExt = PathFindExtension(lpszFilePath);

				TCHAR szTempFile[MAX_PATH];
				lstrcpy(szTempFile, lpszFilePath);
				PathRemoveFileSpec(szTempFile);
				_CreateDirectory(szTempFile);
				
					// 直接替换
				_DeleteFile(lpszFilePath);
				FILE * fp;
				_tfopen_s(&fp, lpszFilePath, _T("wb"));
				if(fp)
				{
					fwrite(pBuf, dwSize, 1, fp);
					fclose(fp);
				}
				bRet = TRUE;
			}
			FreeResource(hg);
		}
	}
	return bRet;
}