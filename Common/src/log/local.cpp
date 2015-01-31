#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "log/local.h"

static FILE* g_fLogFile ;
static CRITICAL_SECTION g_csLog ;

bool InitializeLog(const TCHAR* szBaseDir, const TCHAR* szLogFileName, const TCHAR* szMode)
{
	if (szBaseDir == NULL || szLogFileName == NULL)
	{
		return false ;
	}

	if (_tcslen(szBaseDir) == 0)
	{
		return false ;
	}

	TCHAR szLogFilePath[MAX_PATH] = {0} ;
	_tcscpy_s(szLogFilePath, szBaseDir) ;
	_tcscat_s(szLogFilePath, szLogFileName) ;

	InitializeCriticalSection(&g_csLog) ;

	g_fLogFile = _tfopen(szLogFilePath, szMode);
	if (g_fLogFile == NULL)
	{
		return false ;
	}

	return true ;
}

bool UnintializeLog()
{
	if (g_fLogFile != NULL)
	{
		fclose(g_fLogFile) ;
		g_fLogFile = NULL;
	}

	DeleteCriticalSection(&g_csLog) ;

	return true ;
}

bool Log (const TCHAR * fmt, ...)
{
	/*
	** 1. verify parameter.
	*/
	if(!fmt) 
	{ 
		return false ; 
	}

	__try{

		/*
		** 2. acquire lock to synchronize log procedure
		*/
		EnterCriticalSection(&g_csLog) ;

		/*
		** 3. write log into file
		*/
		if(g_fLogFile != NULL)
		{
			va_list va_alist;
			TCHAR logbuf[2048] = {0};

			/*
			** 3.1 get local time
			*/
			SYSTEMTIME time ;
			GetLocalTime(&time) ;
			char logtime[256] = {0} ;
			sprintf_s(logtime, 256, "%04d-%02d-%02d %02d:%02d:%02d:%03d  ", \
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds) ;

			/*
			** 3.2 format log buffer
			*/
			va_start(va_alist, fmt);
			_vsntprintf_s(logbuf + _tcslen(logbuf), 2048 - _tcslen(logbuf) - sizeof(char)*4, \
				 _TRUNCATE/*2048 - _tcslen(logbuf)*/, fmt, va_alist);// fix-bug for log crush
			va_end (va_alist);

			char logbuftmp[2048] = {0} ;
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (const WCHAR *)logbuf, -1, (LPSTR)logbuftmp, 2048, NULL, NULL);
#else
			lstrcpyn(logbuftmp, logbuf,2048) ;
#endif
			strcat(logbuftmp, "\r\n") ;

			/*
			** 3.3 write log
			*/
			fwrite(logtime, 1, strlen(logtime), g_fLogFile) ;
			fwrite(logbuftmp, 1, strlen(logbuftmp), g_fLogFile) ;
			fflush(g_fLogFile);
		}

	}
	__finally{
		/*
		** release lock
		*/
		LeaveCriticalSection(&g_csLog) ;
	}

	return true ;
}