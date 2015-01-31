#include <windows.h>
#include <tchar.h>
#include <dbgHelp.h>
#include "windowsx.h"
#include "data/ErrorReport.h"
#include "data/DataPool.h"
#include "constant.h"

static TCHAR	g_szDumpFileName[MAX_PATH + 1];
static HANDLE	g_hProcess;
static DWORD	g_dwProcessId;
static TCHAR	g_szBuffer[0x2000]; //防止错误发生时, 申请内存会失败
static TCHAR    g_szBaseDir[MAX_PATH + 1] ;
static TCHAR    g_szDumpType[32];

static LONG CALLBACK MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo);

typedef BOOL
(WINAPI* TMiniDumpWriteDump)(
				  IN HANDLE hProcess,
				  IN DWORD ProcessId,
				  IN HANDLE hFile,
				  IN MINIDUMP_TYPE DumpType,
				  IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
				  IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
				  IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
				  );

static TMiniDumpWriteDump WriteDump;

bool InitErrorReport(LPCTSTR lpszBaseDir /* = NULL */,LPCTSTR lpszDumpType)
{
	HMODULE hDbgHlp = LoadLibrary(_T("dbghelp.dll")) ;
	if (hDbgHlp == NULL)
	{
		return false ;
	}

	WriteDump = (TMiniDumpWriteDump )GetProcAddress(hDbgHlp, "MiniDumpWriteDump") ;
	if (WriteDump == NULL)
	{
		FreeLibrary(hDbgHlp) ;
		return false ;
	}

	g_hProcess  = GetCurrentProcess();
	g_dwProcessId = GetCurrentProcessId();
	lstrcpyn(g_szBaseDir, lpszBaseDir,MAX_PATH+1) ;
	lstrcpyn(g_szDumpType,lpszDumpType,32);

	SYSTEMTIME st;
	GetSystemTime(&st);
	wsprintf(g_szDumpFileName, _T("%s%02d%02d%02d-%d.dmp"), lpszBaseDir, st.wYear, 
		st.wMonth, st.wDay, GetTickCount());

	SetErrorMode(SEM_NOGPFAULTERRORBOX);		
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	/* DisableSetUnhandledExceptionFilter
	** a patch to avoid vc runtime library overwrites the exception handler, then crash will always
	** enter into MyUnhandledExceptionFilter.
	*/
#ifdef _M_IX86
	void *addr = (void*)GetProcAddress(LoadLibrary(_T("kernel32.dll")), "SetUnhandledExceptionFilter");
	if (addr)
	{
		//const unsigned char patch[] = {0xc2, 0x04, 0x00, 0x90,};
		const unsigned char patch[] = {0x33, 0xc0, 0xc2, 0x04,0x00};
		size_t nlen = sizeof(patch) ;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtect(addr, nlen, PAGE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, patch, nlen, NULL);
		//FlushInstructionCache(GetCurrentProcess(), addr, nlen);
		VirtualProtect(addr, nlen, dwOldFlag, &dwTempFlag);
	}
#endif

	return true;
}

static void OnSendDumpFile()
{
	STARTUPINFO si;
	PROCESS_INFORMATION	pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	_tstring sMd5File(theDataPool.GetBaseDir()+FILE_MD5_LIST_FILE);
	theDataPool.WriteMD5ListToLog(theDataPool.GetBaseDir().c_str(),FILE_MD5_LIST_FILE);

	_tstring sLogFile(theDataPool.GetBaseDir()+FILE_LOG_CLIENT);
	wsprintf(g_szBuffer, FILE_EXE_REPORT _T(" \"%s\" \"%s\" \"%s\" \"%s\""), g_szDumpFileName, sMd5File.c_str(), sLogFile.c_str(),g_szDumpType);
	::CreateProcess(NULL, g_szBuffer, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) ;
	
	TerminateProcess(g_hProcess,0);
	exit(1) ; //exit coreclient.exe process
}

static LONG CALLBACK MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	//控制内部error report 数量
	
	if (_wcsicmp(g_szDumpType,I_CRASHREPORT) == 0)
	{
	
		static long ntime = 0;
		static int ntotal = 0;
		static int temptime = 0;
		temptime = GetCurrentTime();
		ntime = GetPrivateProfileInt(INI_CFG_ERRORREPORT_TEMP,INI_CFG_ERRORREPORT_TIME,temptime,theDataPool.GetUserProfilePath().c_str());
		ntotal =  GetPrivateProfileInt(INI_CFG_ERRORREPORT_TEMP,INI_CFG_ERRORREPORT_TOTAL,5,theDataPool.GetUserProfilePath().c_str());
		if (temptime - ntime < 300000 )
		{
			if (++ntotal > 5)
			{
				WritePrivateProfileString(INI_CFG_ERRORREPORT_TEMP,INI_CFG_ERRORREPORT_TOTAL,_tstring(5).c_str(),theDataPool.GetUserProfilePath().c_str());
				TerminateProcess(g_hProcess,0);
				return EXCEPTION_EXECUTE_HANDLER;	
			}		
		}
		else
		{
			ntotal = 1;
			ntime = temptime;
			TCHAR ch[256];
			swprintf_s(ch,256,_T("%d"),ntime);
			WritePrivateProfileString(INI_CFG_ERRORREPORT_TEMP,INI_CFG_ERRORREPORT_TIME,ch,theDataPool.GetUserProfilePath().c_str());
		}
		WritePrivateProfileString(INI_CFG_ERRORREPORT_TEMP,INI_CFG_ERRORREPORT_TOTAL,_tstring(ntotal).c_str(),theDataPool.GetUserProfilePath().c_str());	
	}
	// 为了防止破坏出错现场信息, 将局部变量移出为全局变量
	static _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	static HANDLE hDumpFile;
	static BOOL Result;
    
	
	hDumpFile = CreateFile(g_szDumpFileName, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE) {
		
		ExInfo.ThreadId = ::GetCurrentThreadId() ;
		ExInfo.ExceptionPointers = ExceptionInfo ;
		ExInfo.ClientPointers = FALSE ;

		Result = WriteDump(g_hProcess, g_dwProcessId, hDumpFile, MiniDumpNormal, 
			&ExInfo, NULL, NULL);
		CloseHandle(hDumpFile);

		if (!Result) 
		{
			return EXCEPTION_EXECUTE_HANDLER;
		}

		OnSendDumpFile();
	}
	
	return EXCEPTION_EXECUTE_HANDLER;
}
