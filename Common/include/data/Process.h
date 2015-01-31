#pragma once

#include "tstring/tstring.h"

using namespace String ;

class CProcess
{
public:
	static bool IsProcessRunning( \
		_tstring sProcessImagePath, \
		PHANDLE pProcessHandle = NULL, \
		BOOL bCompareFullPath = FALSE) ;

    static bool IsProcessRunningByEvent(LPTSTR szEventName);

	static bool IsProcessRunningInOtherSession( \
		_tstring sProcessImagePath);

    static bool IsNormalPrivilige();
private:
	static bool KillChildProcesses(UINT nParentId) ;
	static bool KillChildProcessesHandler(UINT nParentId) ;
	static bool EnableDebugPrivilege(bool bEnabled) ;
	static DWORD GetSessionIdByProcessId(DWORD dwProcessId) ;
} ;