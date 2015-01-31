#pragma once

bool InitializeLog(const TCHAR* szBaseDir, const TCHAR* szLogFileName, const TCHAR* szMode=_T("w+")) ;

bool UnintializeLog() ;

bool Log (const TCHAR * fmt, ...) ;