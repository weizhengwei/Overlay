#include <tchar.h>
#include "data/Registry.h"
#include "data/DataPool.h"
#include "constant.h"

#define FOLDER_PLUGINS		_T("plugins")
//registry items
#define REG_ITEM_AUTO_START			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define REG_ITEM_KEY_PRODUCT		_T("Arc")
#define REG_ITEM_CLIENT				_T("Software\\Perfect World Entertainment\\Arc")
#define	REG_ITEM_KEY_SYSTEM_SURVEY	_T("SystemSurveryCollection")
#define REG_ITEM_KEY_CLIENT			_T("client")
#define REG_ITEM_KEY_PATCHER		_T("patcher")
#define REG_ITEM_KEY_LAUNCHER		_T("launcher")
#define REG_ITEM_CLIENT_LEGACY		_T("COREUpdate\\shell\\open\\command")
#define REG_ITEM_PROCESSOR_INFO		_T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0")
#define REG_ITEM_PROCESSOR_NAME     _T("ProcessorNameString")

bool CRegistry::AutoStart(bool bAuto)
{
	bool bRet = false ;
	const TCHAR szSubKey[]    = REG_ITEM_AUTO_START ;
	const TCHAR szValueName[] =	REG_ITEM_KEY_PRODUCT ;

	_tstring sPath = theDataPool.GetBaseDir() + FILE_EXE_LAUNCHER ;
	if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(sPath.c_str()))   
	{
		if (bAuto)
		{
			sPath.append(_T(" "));
            sPath.append(CMD_AUTO_RUN);

			bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, szSubKey, szValueName, 0, KEY_WRITE, REG_SZ, (LPBYTE)(sPath.c_str()), sPath.length()*sizeof(TCHAR)) ;
		}
		else
		{
			bRet = DeleteRegistryItem(HKEY_LOCAL_MACHINE, szSubKey, szValueName, 0, KEY_WRITE) ;
		}

		if (bRet)
		{
			WritePrivateProfileString(INI_CFG_CLIENT_LOGIN,INI_CFG_CLIENT_LOGIN_AUTOSTART, _tstring(bAuto).c_str(), theDataPool.GetUserProfilePath().c_str());
		}
	} 

	return bRet ;
}

bool CRegistry::CheckAutoStart()
{
	const TCHAR szSubKey[]    = REG_ITEM_AUTO_START ;
	const TCHAR szValueName[] = REG_ITEM_KEY_PRODUCT ;

	TCHAR szValue[MAX_PATH] = {0} ;
	if (GetRegistryItem(HKEY_LOCAL_MACHINE, szSubKey, szValueName, 0, KEY_QUERY_VALUE, REG_SZ, (LPBYTE)szValue, MAX_PATH*sizeof(TCHAR)))
	{
		_tstring sPath = theDataPool.GetBaseDir() + FILE_EXE_LAUNCHER;

		if (_tstring(szValue).Find(sPath.c_str()) !=-1)
		{
			return TRUE ;
		}
	}

	return FALSE ;
}

_tstring CRegistry::GetProcessorInfo()
{
	const TCHAR szSubKey[]    = REG_ITEM_PROCESSOR_INFO ;
	const TCHAR szValueName[] = REG_ITEM_PROCESSOR_NAME ;

	TCHAR szValue[MAX_PATH] = {0} ;
	if (GetRegistryItem(HKEY_LOCAL_MACHINE, szSubKey, szValueName, 0, KEY_QUERY_VALUE, REG_SZ, (LPBYTE)szValue, MAX_PATH*sizeof(TCHAR)))
	{
		return _tstring(szValue);
	}

	return _tstring(_T(""));
}

bool CRegistry::IsSendSystemSurveyInfo()
{
	const TCHAR szSubKey[] = REG_ITEM_CLIENT ;
	const TCHAR szValueName[] = REG_ITEM_KEY_SYSTEM_SURVEY ;

	/*
	** open registry item
	** note: the third parameter of RegOpenKeyEx must be set 0, otherwise access will be denied in some
	** operating system(such as win7 64bit).
	** note: the forth parameter of RegOpenKeyEx must be set KEY_QUERY_VALUE, otherwise ERROR_ACCESS_DENIED 
	** will return when load page from web server.
	*/
	DWORD  dwResult = 0 ;
	bool bRet = GetRegistryItem(HKEY_LOCAL_MACHINE, szSubKey, szValueName, 0, KEY_QUERY_VALUE, REG_DWORD, (LPBYTE)&dwResult, 4) ;
	if (bRet)
	{
		bRet = (bool)dwResult ;
	}

	return bRet ;
}

bool CRegistry::RestoreCoreInstallRegisttry()
{
	bool bRet= false;
	_tstring tsRegValue(_T(""));
    DWORD dwRegValue=0;

	do 
	{
		//restore Client launching registry items.
		tsRegValue = theDataPool.GetBaseDir() + FILE_EXE_CLIENT;
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, REG_ITEM_CLIENT, REG_ITEM_KEY_CLIENT, 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true) ;
        if (!bRet)
        {


			break;
        }


		//restore Update launching registry items.
		tsRegValue = theDataPool.GetBaseDir() + FILE_EXE_PATCHER;
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, REG_ITEM_CLIENT,REG_ITEM_KEY_PATCHER, 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		//restore launcher launching registry items.
		tsRegValue = theDataPool.GetBaseDir() + FILE_EXE_LAUNCHER;
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, REG_ITEM_CLIENT,REG_ITEM_KEY_LAUNCHER, 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		//restore legacy Client launching registry items.
		tsRegValue = theDataPool.GetBaseDir() + FILE_EXE_CLIENT;
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, REG_ITEM_CLIENT_LEGACY,_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true) ;
		if (!bRet)
		{
			break;
		}

		//restore legacy Update launching registry items.
		tsRegValue = theDataPool.GetBaseDir() + FILE_EXE_PATCHER;
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, REG_ITEM_CLIENT_LEGACY,REG_ITEM_KEY_PATCHER, 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		//restore IE Plugin registry items
		tsRegValue = _T("{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("ArcPluginIE.ArcPluginIEBHO\\CLSID"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIEBHO Class");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("{6A1E1BB0-F751-4b5d-BABD-8B10E05625B2}");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}"),_T("AppID"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIE.ArcPluginIEBHO.1");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\ProgID"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\Programmable"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("{}");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\TypeLib"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIE.ArcPluginIEBHO");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\VersionIndependentProgID"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIEBHO Class");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("ArcPluginIE.ArcPluginIEBHO"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIE.ArcPluginIEBHO.1");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("ArcPluginIE.ArcPluginIEBHO\\CurVer"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPluginIEBHO");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
       dwRegValue = 0x00000001;

		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}"),_T("NoExplorer"), 0, KEY_WRITE, REG_DWORD, (LPBYTE)&dwRegValue, sizeof(DWORD),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = theDataPool.GetBaseDir()+ FOLDER_PLUGINS + _T("\\") + FILE_DLL_PUGLINIE;
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\InprocServer32"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("Apartment");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("CLSID\\{84BFE29A-8139-402a-B2A4-C23AE9E1A75F}\\InprocServer32"),_T("ThreadingModel"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		

		//Restore firefox Plugin registry items
		tsRegValue = theDataPool.GetBaseDir()+ FOLDER_PLUGINS + _T("\\") + FILE_DLL_PLUGINFF;
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin"),_T("Path"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("PlayNow plugin for Mozilla browsers");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin"),_T("Description"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("ArcPlugin");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin"),_T("ProductName"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("Perfect World Entertainment,Inc");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin"),_T("Vendor"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("1.0");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin"),_T("Version"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin\\MimeTypes"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MozillaPlugins\\@perfectworld.com/npArcPlayNowPlugin\\MimeTypes\\application/firefox-interactwithclient-plugin"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		// arc urlprotocol
		tsRegValue = _T("URL:arc protocol");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc"),_T("URL Protocol"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		tsRegValue = FILE_EXE_LAUNCHER;
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc\\DefaultIcon"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		_tstring strTemp = theDataPool.GetBaseDir() + FILE_EXE_LAUNCHER;
		tsRegValue.Format(2,_T("\"%s\" \"\%s1\""),strTemp.c_str(),_T("%"));
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc\\shell\\open\\command"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		//arc flash
		_tstring tsSubKey = _T("SOFTWARE\\MozillaPlugins\\@adobe.com/FlashPlayer");
		BOOL bisWow64;
		IsWow64Process(GetCurrentProcess(),&bisWow64);
		if (bisWow64)
		{
			tsSubKey = _T("SOFTWARE\\Wow6432Node\\MozillaPlugins\\@adobe.com/FlashPlayer");
		}

		TCHAR szValue[MAX_PATH] = {0} ;
		
		
		//_tstring tsPath = szValue;
		_tstring tsRegPath = theDataPool.GetBaseDir()+ FOLDER_PLUGINS + _T("\\") + FILE_DLL_FLASH;

		if (!GetRegistryItem(HKEY_LOCAL_MACHINE, tsSubKey.c_str(),_T("Path"),0,KEY_QUERY_VALUE, REG_SZ, (LPBYTE)szValue, MAX_PATH*sizeof(TCHAR)) || tsRegPath.CompareNoCase(szValue) == 0)
		{
			//return true;
			tsRegValue = _T("");
			bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, tsSubKey.c_str(),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
			if (!bRet)
			{
				break;
			}
			tsRegValue = theDataPool.GetBaseDir()+ FOLDER_PLUGINS + _T("\\") + FILE_DLL_FLASH;
			bRet = SetRegistryItem(HKEY_LOCAL_MACHINE,tsSubKey.c_str(),_T("Path"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
			if (!bRet)
			{
				break;
			}
		}
	} while (false);

	return bRet;
}
bool CRegistry::GetRegistryItem(HKEY hKey, LPCTSTR lpszSubKey, LPCTSTR lpszValueName, DWORD dwOptions, DWORD dwAccess, DWORD dwType, LPBYTE pData, DWORD dwDataSize)
{
	bool bRet = true ;

	HKEY hRegKey = NULL ;

	__try{

		if (ERROR_SUCCESS != RegOpenKeyEx(hKey, lpszSubKey, dwOptions, dwAccess, &hRegKey))
		{
			bRet = false ;
			__leave ;
		}

		if (ERROR_SUCCESS != RegQueryValueEx(hRegKey, lpszValueName, NULL, &dwType, (LPBYTE)pData, &dwDataSize))
		{
			bRet = false ;
			__leave ;
		}
	}
	__finally{

		if (hRegKey != NULL)
		{
			RegCloseKey(hRegKey) ;
		}
	}

	return bRet ;
}

bool CRegistry::SetRegistryItem(HKEY hKey, LPCTSTR lpszSubKey, LPCTSTR lpszValueName, DWORD dwOptions, DWORD dwAccess, DWORD dwType, LPBYTE pData, DWORD dwDataSize,bool bCreated)
{
	bool bRet = true ;

	HKEY hRegKey = NULL ;

	__try{

		if (ERROR_SUCCESS != RegOpenKeyEx(hKey, lpszSubKey, dwOptions, dwAccess, &hRegKey))
		{
			if (!bCreated || ERROR_SUCCESS != RegCreateKey(hKey,lpszSubKey,&hRegKey))
			{
				bRet = false ;
				__leave ;
			}
		}

		if (ERROR_SUCCESS != RegSetValueEx(hRegKey, lpszValueName, 0, dwType, pData, dwDataSize))
		{
			bRet = false ;
			__leave ;
		}
	}
	__finally{

		if (hKey != NULL)
		{
			RegCloseKey(hKey) ;
		}
	}

	return bRet ;
}

bool CRegistry::CreateRegistryItem()
{
	return true ;
}

bool CRegistry::DeleteRegistryItem(HKEY hKey, LPCTSTR lpszSubKey, LPCTSTR lpszValueName, DWORD dwOptions, DWORD dwAccess)
{
	bool bRet = true ;

	HKEY hRegKey = NULL ;

	__try{

		if (ERROR_SUCCESS != RegOpenKeyEx(hKey, lpszSubKey, dwOptions, dwAccess, &hRegKey))
		{
			bRet = false ;
			__leave ;
		}

		if(ERROR_SUCCESS != RegDeleteValue(hRegKey,lpszValueName)) 
		{
			bRet = false ;
			__leave ;
		}
	}
	__finally{

		if (hRegKey != NULL)
		{
			RegCloseKey(hRegKey) ;
		}
	}

	return bRet ;
}

bool CRegistry::RegArcUrlProtocol()
{
	bool bRet= false;
	_tstring tsRegValue(_T(""));
	DWORD dwRegValue=0;

	TCHAR szValue[MAX_PATH] = {0} ;
	if (GetRegistryItem(HKEY_CLASSES_ROOT, _T("arc"),_T("URL Protocol"),0,KEY_QUERY_VALUE, REG_SZ, (LPBYTE)szValue, MAX_PATH*sizeof(TCHAR)))
	{
		return true;
	}
	do 
	{
		tsRegValue = _T("URL:arc protocol");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc"),_T("URL Protocol"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}

		tsRegValue = FILE_EXE_LAUNCHER;
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc\\DefaultIcon"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		_tstring strTemp = theDataPool.GetBaseDir() + FILE_EXE_LAUNCHER;
		tsRegValue.Format(2,_T("\"%s\" \"\%s1\""),strTemp.c_str(),_T("%"));
		bRet = SetRegistryItem(HKEY_CLASSES_ROOT, _T("arc\\shell\\open\\command"),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
	} while (false);
	return bRet;	
}
bool CRegistry::RegArcFlash()
{
	bool bRet= false;
	_tstring tsRegValue(_T(""));
	_tstring tsSubKey = _T("SOFTWARE\\MozillaPlugins\\@adobe.com/FlashPlayer");
	DWORD dwRegValue=0;
	BOOL bisWow64;
	IsWow64Process(GetCurrentProcess(),&bisWow64);
	if (bisWow64)
	{
		tsSubKey = _T("SOFTWARE\\Wow6432Node\\MozillaPlugins\\@adobe.com/FlashPlayer");
	}

	TCHAR szValue[MAX_PATH] = {0} ;
	if (GetRegistryItem(HKEY_LOCAL_MACHINE, tsSubKey.c_str(),_T("Path"),0,KEY_QUERY_VALUE, REG_SZ, (LPBYTE)szValue, MAX_PATH*sizeof(TCHAR)))
	{
		return true;
	}
	do 
	{
		tsRegValue = _T("");
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE, tsSubKey.c_str(),_T(""), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
		tsRegValue = theDataPool.GetBaseDir()+ FOLDER_PLUGINS + _T("\\") + FILE_DLL_FLASH;
		bRet = SetRegistryItem(HKEY_LOCAL_MACHINE,tsSubKey.c_str(),_T("Path"), 0, KEY_WRITE, REG_SZ, (LPBYTE)(tsRegValue.c_str()), tsRegValue.length()*sizeof(TCHAR),true);
		if (!bRet)
		{
			break;
		}
	} while (false);
	return bRet;

}