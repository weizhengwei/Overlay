#pragma once

#include "tstring/tstring.h"
using namespace String;
class CRegistry
{
public:
	CRegistry()  {};
	~CRegistry() {} ;

	bool AutoStart(bool bAuto) ;
	bool CheckAutoStart() ;
	bool IsSendSystemSurveyInfo() ;
    bool RestoreCoreInstallRegisttry();
    _tstring GetProcessorInfo();
	bool RegArcUrlProtocol();
	bool RegArcFlash();
protected:
	bool GetRegistryItem(HKEY hKey, \
		LPCTSTR lpszSubKey, \
		LPCTSTR lpszValueName, \
		DWORD dwOptions, \
		DWORD dwAccess, \
		DWORD dwType, \
		LPBYTE pData, \
		DWORD dwDataSize) ;

	bool SetRegistryItem(HKEY hKey, \
		LPCTSTR lpszSubKey, \
		LPCTSTR lpszValueName, \
		DWORD dwOptions, \
		DWORD dwAccess, \
		DWORD dwType, \
		LPBYTE pData, \
		DWORD dwDataSize ,bool bCreated = false) ;

	bool CreateRegistryItem() ;

	bool DeleteRegistryItem(HKEY hKey, \
		LPCTSTR lpszSubKey, \
		LPCTSTR lpszValueName, \
		DWORD dwOptions, \
		DWORD dwAccess) ;

} ;