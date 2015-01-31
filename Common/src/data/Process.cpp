#include <windows.h>
#include <tchar.h>
#include <Tlhelp32.h>
#include <Psapi.h>
#include "data/Process.h"

bool CProcess::IsProcessRunning(_tstring sProcessImagePath, PHANDLE pProcessHandle, BOOL bCompareFullPath)
{
	bool bRet = false ;
	HANDLE hProcess ;

	if (sProcessImagePath.empty())
	{
		return false ;
	}
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false ;
	}

	PROCESSENTRY32 pe32 = {0} ;
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process, and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		return false ;
	}

	if (!EnableDebugPrivilege(true))
	{
		return false ;
	}

	do
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		if( hProcess != NULL )
		{
			TCHAR szImagePath[MAX_PATH] = {0} ;
			GetProcessImageFileName (hProcess, szImagePath, MAX_PATH) ;
			TCHAR szDrive[3] = {_T("C:")} ; 
			szDrive[0] = sProcessImagePath[0] ;
			TCHAR szDeviceName[64] = {0} ;
			QueryDosDevice(szDrive, szDeviceName, 64) ;
			TCHAR* tmp = _tcsstr(szImagePath, szDeviceName) ;
			if (tmp == szImagePath)
			{///compare process image path name
				_tstring sImagePath ;
				sImagePath.Format(2, _T("%s%s"), szDrive, &szImagePath[_tcslen(szDeviceName)]) ;
				if (!sProcessImagePath.CompareNoCase(sImagePath.c_str()))
				{/// the game process is launched, so do not launch again.
					bRet = true ;
					if (pProcessHandle != NULL)
					{
						*pProcessHandle = hProcess ;
						KillChildProcesses(pe32.th32ProcessID) ;
					}
					break ;
				}
			}

			if (!bCompareFullPath)
			{
				int nPos = sProcessImagePath.find_last_of(_T("\\")) ;
				_tstring sProcessName = &sProcessImagePath[nPos+1] ;
				if (!sProcessName.CompareNoCase(pe32.szExeFile))
				{
					bRet = true ;
					if (pProcessHandle != NULL)
					{
						*pProcessHandle = hProcess ;
						KillChildProcesses(pe32.th32ProcessID) ;
					}
					break ;
				}

				_tstring sExeFile = pe32.szExeFile ;
				nPos = sExeFile.find_last_of(_T('.')) ;
				if (nPos != -1)
				{
					sExeFile[nPos] = 0 ;
					sProcessName[sProcessName.find_last_of(_T('.'))] = 0 ;
					size_t nsize = (sProcessName.length()  - _tcslen(_T(".exe")))*sizeof(TCHAR);
					if (!sProcessName.CompareNoCase(sExeFile) && (_ttoi(&sExeFile[nsize/sizeof(TCHAR)]) != 0))
					{
						bRet = true ;
						if (pProcessHandle != NULL)
						{
							*pProcessHandle = hProcess ;
							KillChildProcesses(pe32.th32ProcessID) ;
						}
						break ;
					}
				}
			}

			CloseHandle(hProcess) ;
		}

	} while( Process32Next( hProcessSnap, &pe32 ) );
	
	CloseHandle( hProcessSnap );

	if (!EnableDebugPrivilege(false))
	{
		return false ;
	}

	return bRet ;
}

bool CProcess::IsProcessRunningByEvent(LPTSTR szEventName)
{
	bool bRet= false;
	HANDLE hEvent = ::OpenEvent(SYNCHRONIZE,FALSE,szEventName);
	if (hEvent != NULL)
	{
		bRet= true ;
		::CloseHandle(hEvent);
		hEvent = NULL;
	}
	return bRet;
}

bool CProcess::IsProcessRunningInOtherSession(_tstring sProcessImagePath)
{
	bool bRet = false ;
	HANDLE hProcess ;

	if (sProcessImagePath.empty())
	{
		return false ;
	}
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false ;
	}

	PROCESSENTRY32 pe32 = {0} ;
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process, and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		return false ;
	}

	if (!EnableDebugPrivilege(true))
	{
		return false ;
	}

	do
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if( hProcess != NULL )
		{
			TCHAR szImagePath[MAX_PATH] = {0} ;
			GetProcessImageFileName (hProcess, szImagePath, MAX_PATH) ;
			TCHAR szDrive[3] = {_T("C:")} ; 
			szDrive[0] = sProcessImagePath[0] ;
			TCHAR szDeviceName[64] = {0} ;
			QueryDosDevice(szDrive, szDeviceName, 64) ;
			TCHAR* tmp = _tcsstr(szImagePath, szDeviceName) ;
			if (tmp == szImagePath)
			{///compare process image path name
				_tstring sImagePath ;
				sImagePath.Format(2, _T("%s%s"), szDrive, &szImagePath[_tcslen(szDeviceName)]) ;
				if (!_tcsicmp(sProcessImagePath.c_str(), sImagePath.c_str()))
				{/// the game process is launched, verify whether in another session
					if (GetSessionIdByProcessId(GetCurrentProcessId()) != GetSessionIdByProcessId(GetProcessId(hProcess)))
					{
						bRet = true ;
						CloseHandle(hProcess) ;
						break ;
					}
				}
			}
			CloseHandle(hProcess) ;
		}

	} while(Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);

	if (!EnableDebugPrivilege(false))
	{
		return false ;
	}

	return bRet ;
}

bool CProcess::IsNormalPrivilige()
 {
	 HANDLE hToken=NULL;
	 bool bAdmin = false;
	 if (OpenProcessToken(GetCurrentProcess(),MAXIMUM_ALLOWED,&hToken))
	 {
		 PTOKEN_PRIVILEGES pTokenPri =NULL;
		 DWORD dwPriLen =0;
		 if (!GetTokenInformation(hToken,TokenPrivileges,(LPVOID)pTokenPri,0,&dwPriLen))
		 {
			 if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
			 {
				 pTokenPri = (PTOKEN_PRIVILEGES)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwPriLen);
			 }
		 }
		 GetTokenInformation(hToken,TokenPrivileges,(LPVOID)pTokenPri,dwPriLen,&dwPriLen);
		 for (int i=0;i<pTokenPri->PrivilegeCount;i++)
		 {
			 TCHAR szPriName[MAX_PATH]={0};
			 DWORD dwPriNameLen=MAX_PATH;
			 LUID luid = pTokenPri->Privileges[i].Luid;
			 BOOL bRet = LookupPrivilegeName(NULL,&luid,szPriName,&dwPriNameLen);
			 if (!wcscmp(szPriName,SE_CREATE_GLOBAL_NAME) && (pTokenPri->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED))
			 {
				 bAdmin = true;
				 break;
			 }
		 }
		 if (pTokenPri)
		 {
			 HeapFree(GetProcessHeap(),0,(LPVOID)pTokenPri);
		 }

		 if (!bAdmin)
		 {
			 return !bAdmin;
		 }

		 //check token groups again,for xp guest user
		 bAdmin =false;

		 PTOKEN_GROUPS pTokenGroups =NULL;
		 DWORD dwGroupsLen =0;
		 if (!GetTokenInformation(hToken,TokenGroups,(LPVOID)pTokenGroups,0,&dwGroupsLen))
		 {
			 if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
			 {
				 pTokenGroups = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwGroupsLen);
			 }
		 }
		 GetTokenInformation(hToken,TokenGroups,(LPVOID)pTokenGroups,dwGroupsLen,&dwGroupsLen);
		 for (int i=0;i<pTokenGroups->GroupCount;i++)
		 {
			 if (pTokenGroups->Groups[i].Attributes & SE_GROUP_OWNER)
			 {
				 bAdmin = true;
				 break;
			 }
		 }
		 if (pTokenGroups)
		 {
			 HeapFree(GetProcessHeap(),0,(LPVOID)pTokenGroups);
		 }
	 }
	 CloseHandle(hToken);
	 return !bAdmin;
 }

bool CProcess::KillChildProcesses(UINT nParentId)
{
	bool bRet = false ;

	__try {

		bRet = KillChildProcessesHandler(nParentId) ;

	} __except(EXCEPTION_EXECUTE_HANDLER) {

		bRet = false ;

	}

	return bRet ;
}

bool CProcess::KillChildProcessesHandler(UINT nParentId)
{
	bool bRet = false ;
	HANDLE hProcess ;

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0 );
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false ;
	}

	PROCESSENTRY32 pe32 = {0} ;
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process, and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		return false ;
	}

	do
	{
		hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
		if( hProcess != NULL )
		{
			if (pe32.th32ParentProcessID == nParentId)
			{
				KillChildProcesses(pe32.th32ProcessID) ;
				TerminateProcess(hProcess, 4) ;
				CloseHandle(hProcess) ;
				hProcess = NULL ;
			}
		}

	} while( Process32Next( hProcessSnap, &pe32 ) );
	
	CloseHandle( hProcessSnap );

	return bRet ;
}

bool CProcess::EnableDebugPrivilege(bool bEnabled) 
{ 
	HANDLE hToken ; 
	bool bRet = false ;  
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken)) 
	{ 
		TOKEN_PRIVILEGES tp; 
		tp.PrivilegeCount=1; 
		
		if(LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid))
		{
			if (bEnabled)
			{
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ; 
			}
			else
			{
				tp.Privileges[0].Attributes = 0 ;
			}
			if(AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL))
			{
				bRet = true ;
			}
		}
		CloseHandle(hToken); 
	} 
	return bRet ; 
}

DWORD CProcess::GetSessionIdByProcessId(DWORD dwProcessId)
{
	DWORD dwSessionId = 0 ;

	//Open process
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, 0, dwProcessId);
	if(hProcess)
	{
		HANDLE hProcessToken ;
		//Open The Process's Token which can be used to achieve more security information
		if(::OpenProcessToken(hProcess, TOKEN_READ, &hProcessToken))
		{
			DWORD ReturnLength;

			//Get session id from the Token
			if(::GetTokenInformation(hProcessToken,TokenSessionId, &dwSessionId,
				sizeof(DWORD), &ReturnLength)) 
			{
				CloseHandle(hProcess) ;
				return dwSessionId ;
			}
		}
	}
	return -1 ;
}