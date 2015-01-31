/**
* @file      CoreInfo.cpp
@brief     The implementation file for exporting some useful functions.
@author    shenhui@pwrd.com
@data      2010/09/26
@version   01.00.00
*/

/* 
*  Copyright(C) 2010. Perfect World Entertainment Inc
*
*  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
*  its licensors and is protected by copyright and other intellectual 
*  property laws and treaties.  We and our licensors own all rights, 
*  title and interest in and to the SOURCE CODE, including all copyright 
*  and other intellectual property rights in the SOURCE CODE.
*/

/*
* $LastChangedBy: shenhui@pwrd.com
* $LastChangedDate: 
* $Revision: 
* $Id: N/A
*/

#include <tchar.h>
#include <io.h>
#include <errno.h>
#include <Shobjidl.h>
#include <Shlobj.h>
#include "data/GameInfo.h"
#include "data/ThirdPartyApp.h"


static HRESULT GetLinkPath(HWND hWnd, LPCTSTR lpwsLinkName, _tstring& sLinkPath) ;
static _tstring GetStartupProgramPath(DWORD nType) ;

static _tstring GetStartupProgramPath(DWORD nType)
{
	LPITEMIDLIST pidl=NULL;
	LPMALLOC pShellMalloc=NULL; 
	TCHAR szDir[MAX_PATH] = {0} ;
	if(SUCCEEDED(SHGetMalloc(&pShellMalloc)) && pShellMalloc) 
	{ 
		if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,nType,&pidl)) && pidl) 
		{ 
			SHGetPathFromIDList(pidl, szDir); 
			pShellMalloc->Free(pidl); 
		} 
		pShellMalloc->Release(); 
	} 
	
	_tstring sPath ;
	sPath.assign(szDir) ;

	return sPath ;
}

static HRESULT GetLinkPath(HWND hWnd, LPCTSTR lpwsLinkName, _tstring& sLinPath)
{
    HRESULT hResult;
    IShellLink *pIShellLink;
    WIN32_FIND_DATA wfd;
	TCHAR szLinkPath[MAX_PATH] = {0} ;
 
    CoInitialize(NULL);
    hResult = CoCreateInstance((REFIID)CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,(REFIID)IID_IShellLink,(LPVOID *)&pIShellLink);
    if (SUCCEEDED(hResult))
    {
        IPersistFile *pIPersistFile;
        hResult = pIShellLink->QueryInterface((REFIID)IID_IPersistFile,(LPVOID *)&pIPersistFile);
        if (SUCCEEDED(hResult))
        {
            hResult = pIPersistFile->Load(lpwsLinkName, STGM_READ);
            if (SUCCEEDED(hResult))
            {
                hResult = pIShellLink->Resolve(hWnd,SLR_ANY_MATCH | SLR_NO_UI);
                if (SUCCEEDED(hResult))
                {
                    hResult = pIShellLink->GetPath(szLinkPath,MAX_PATH,&wfd,SLGP_RAWPATH);
                }
            }
            pIPersistFile->Release();
        }
        pIShellLink->Release();
    }

	sLinPath.assign(szLinkPath) ;
 
    return hResult;
}

static HKEY StringToHKey(_tstring sKey)
{
	if(sKey == _T("HKEY_CLASSES_ROOT"))
	{
		return HKEY_CLASSES_ROOT;
	}
	else if(sKey == _T("HKEY_CURRENT_CONFIG"))
	{
		return HKEY_CURRENT_CONFIG;
	}
	else if(sKey == _T("HKEY_CURRENT_USER"))
	{
		return HKEY_CURRENT_USER;
	}
	else if(sKey == _T("HKEY_LOCAL_MACHINE"))
	{
		return HKEY_LOCAL_MACHINE;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_DATA"))
	{
		return HKEY_PERFORMANCE_DATA;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_NLSTEXT"))
	{
		return HKEY_PERFORMANCE_NLSTEXT;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_TEXT"))
	{
		return HKEY_PERFORMANCE_TEXT;
	}
	else if(sKey == _T("HKEY_USERS"))
	{
		return HKEY_USERS;
	}
	else
	{
		return (HKEY)INVALID_HANDLE_VALUE;
	}
}


////////////////////////////////////////////////////////////////
static BOOL GetLocalInfo_BeijingHandler(CThirdPartyApp *pApp);
static BOOL GetLocalInfo_ShanghaiHandler(CThirdPartyApp *pApp);
static BOOL GetLocalInfo_Others(CThirdPartyApp *pApp);

//static BOOL GetFWInfo(CThirdPartyApp *pApp);
//static BOOL GetRHInfo(CThirdPartyApp *pApp);
//static BOOL GetBLInfo(CThirdPartyApp *pApp);

static HRESULT GetInfoFromRegistry(_tstring sRelPath, CThirdPartyApp *pApp);
static bool GetCryticGamesInfoFromRegistry(CThirdPartyApp *pApp);

BOOL GetAppLocalInfo(CThirdPartyApp *pApp)
{
	if(!pApp)
	{
		return FALSE;
	}

	BOOL bRet = FALSE ;

	switch(pApp->GetAppInfo().GetIntId())
	{
	//case 11:	//FW
	//	bRet = GetFWInfo(pApp) ;
	//	break;
	//case 1000:	//RH
	//	bRet = GetRHInfo(pApp) ;
	//	break;
	//case 1001:	//BL
	//	bRet = GetBLInfo(pApp) ;
	//	break;
	case 1003:
	case 1004:
	case 2002:
		bRet = GetCryticGamesInfoFromRegistry(pApp);
		break;
	default:
		if(GetInfoFromRegistry(pApp->GetAppInfo().GetRegistry(), pApp) == S_OK)
		{
			bRet = TRUE;
		}
		break;
	}

	return bRet ;

	//int nType = atoi(pApp->GetType().toNarrowString().c_str());

	//switch(nType)
	//{
	//case 0:///Games developed by Beijing
	//	return GetLocalInfo_BeijingHandler(pApp) ;
	//case 1:///Games developed by Shanghai
	//	return GetLocalInfo_ShanghaiHandler(pApp) ;
	//default:///Games developed by third party company
	//	return GetLocalInfo_Others(pApp) ;
	//}

	//return TRUE;
}

//static BOOL GetLocalInfo_BeijingHandler(CThirdPartyApp *pApp)
//{
//	do{
//		//recognize game from startup program folder of the computer
//		_tstring sInstallPath, sFilePath;
//		_tstring::size_type nPos ;
//		if (pApp->GetStartMenuPath1().empty())
//		{
//			break ;
//		}
//
//		DWORD dwMajorVersion = 0 ;
//		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(GetVersion()))) ;
//
//		int nType = 0 ;
//		if (dwMajorVersion >= 6)
//		{
//			nType = CSIDL_COMMON_PROGRAMS ;
//		}
//		else
//		{
//			nType = CSIDL_PROGRAMS ;
//		}
//
//		_tstring sStartupPath = GetStartupProgramPath(nType) ;
//		if (sStartupPath.empty()) 
//		{
//			break ;
//		}
//		_tstring sLinkPath, sShortCut;
//		sShortCut.assign(sStartupPath.c_str()) ;
//		sShortCut.append(pApp->GetStartMenuPath1().c_str()) ;
//		HRESULT hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//		if (hRes != S_OK)
//		{
//			sShortCut.assign(sStartupPath.c_str()) ;
//			sShortCut.append(pApp->GetStartMenuPath2().c_str()) ;
//			hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//			if (hRes != S_OK)
//			{
//				break ;
//			}
//		}
//		sFilePath.assign(sLinkPath.c_str()) ;
//		nPos = sFilePath.find(_T("patcher\\patcher.exe")) ;
//		if (nPos == _tstring::npos)
//		{
//			break ;
//		}
//		sFilePath[nPos] = 0 ;
//		sInstallPath = sFilePath ;		
//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		if (sInstallPath.empty())
//		{
//			break ;
//		}
//
//		pApp->SetInstallPath(sInstallPath);
//
//		pApp->GetLocalInfo().sAppPatcherPath = sInstallPath;
//		pApp->GetLocalInfo().sAppPatcherPath += _T("patcher\\patcher.exe") ;
//
//		sFilePath.Format(1, _T("%selement\\elementclient.exe"), sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppClientFilePath = sFilePath;
//
//
//		//pGameData->patcherpath.assign(sInstallPath.c_str()) ;
//		//pGameData->patcherpath.append(_T("patcher\\patcher.exe")) ; /*< save patcher.exe path */
//
//		//pGameData->installpath.clear() ;
//		//pGameData->installpath.assign(sInstallPath.c_str()) ; /*< save install path */
//
//		//sFilePath.Format(1, _T("%selement\\elementclient.exe"), sInstallPath.c_str()) ;
//		//pGameData->launcherpath.assign(sFilePath.c_str()) ;
//		if (ENOENT == _taccess_s(pApp->GetLocalInfo().sAppClientFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "uninstall.exe"*/
//		sFilePath.Format(1, _T("%suninstall.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			pApp->GetLocalInfo().sAppUninstallFilePath.assign(_T("")) ;
//			break ;
//		}
//		pApp->GetLocalInfo().sAppUninstallFilePath.assign(sFilePath.c_str()) ; /*< save uninstall program file path */
//
//		/*check for existence of "uninstall.ini"*/
//		sFilePath.Format(1, _T("%suninstall.ini"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "uninstall.dat"*/
//		sFilePath.Format(1, _T("%suninstall.dat"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "patcher\\patcher.exe"*/
//		sFilePath.Format(1, _T("%spatcher\\patcher.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		pApp->GetLocalInfo().sAppLauncherPath.assign(sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppLauncherPath.append(_T("launcher\\launcher.exe")) ;
//
//		pApp->GetLocalInfo().nStatus = CThirdPartyApp::INSTALLED;
//
//	}while(FALSE);
//
//	if (pApp->GetLocalInfo().nStatus != CThirdPartyApp::INSTALLED)
//	{//if game not found from start menu item, find from registry
//		GetInfoFromRegistry(pApp->GetRegistry(), pApp);
//	}
//	if (pApp->GetLocalInfo().nStatus == CThirdPartyApp::INSTALLED)
//	{
//		if (pApp->GetLocalInfo().selservergroupid.empty())
//		{//must be set for compatible with games from Beijing team to support bypass login(select area info)
//			pApp->GetLocalInfo().selservergroupid.assign(_T("ServerGroupSelect")) ;
//		}
//	}
//
//	return TRUE ;
//}

//static BOOL GetLocalInfo_ShanghaiHandler(CThirdPartyApp *pApp)
//{
//	do{
//		//recognize game from startup program folder of the computer
//		_tstring sInstallPath, sFilePath;
//		_tstring::size_type nPos ;
//		if (pApp->GetStartMenuPath1().empty())
//		{
//			break ;
//		}
//
//		_tstring sStartupPath = GetStartupProgramPath(CSIDL_COMMON_PROGRAMS) ;
//		if (sStartupPath.empty())
//		{
//			break ;
//		}
//		_tstring sLinkPath, sShortCut;
//		sShortCut.assign(sStartupPath.c_str()) ;
//		sShortCut.append(pApp->GetStartMenuPath1().c_str()) ;
//		HRESULT hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//		if (hRes != S_OK)
//		{
//			sShortCut.assign(sStartupPath.c_str()) ;
//			sShortCut.append(pApp->GetStartMenuPath2().c_str()) ;
//			hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//			if (hRes != S_OK)
//			{
//				break ;
//			}
//		}
//		sFilePath.assign(sLinkPath.c_str()) ;
//		nPos = sFilePath.find(_T("Launcher.exe")) ;
//		if (nPos == _tstring::npos)
//		{
//			break ;
//		}
//		sFilePath[nPos] = 0 ;
//		sInstallPath = sFilePath ;		
//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//verify the existence of particular files
//		if (sInstallPath.empty())
//		{
//			break ;
//		}
//		pApp->GetLocalInfo().sInstallPath.assign(sInstallPath.c_str()) ;
//
//		pApp->GetLocalInfo().sAppClientFilePath.assign(sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppClientFilePath.append(_T("Bin\\game.exe")) ;
//		if (ENOENT == _taccess_s(pApp->GetLocalInfo().sAppClientFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "unins000.dat"*/
//		sFilePath.Format(1, _T("%sunins000.dat"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "unins000.exe"*/
//		sFilePath.Format(1, _T("%sunins000.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			pApp->GetLocalInfo().sAppUninstallFilePath = _T("") ;
//			break ;
//		}
//		pApp->GetLocalInfo().sAppUninstallFilePath.assign(sFilePath.c_str()) ;
//
//		/*check for existence of "Launcher.exe"*/
//		sFilePath.Format(1, _T("%sLauncher.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		pApp->GetLocalInfo().sAppPatcherPath.assign(sFilePath.c_str()) ; /*< save launcher.exe path */ 
//
//		pApp->GetLocalInfo().nStatus = CThirdPartyApp::INSTALLED;
//	}while(FALSE) ;
//
//	if (pApp->GetLocalInfo().nStatus != CThirdPartyApp::INSTALLED)
//	{//if game not found from start menu item, find from registry
//		GetInfoFromRegistry(pApp->GetRegistry(), pApp);
//	}
//	return TRUE ;
//}

//static BOOL GetLocalInfo_Others(CThirdPartyApp *pApp)
//{
//	BOOL bRet = FALSE ;
//
//	switch(pApp->GetIntId())
//	{
//	case 11:	//FW
//		bRet = GetFWInfo(pApp) ;
//		break;
//	case 1000:	//RH
//		bRet = GetRHInfo(pApp) ;
//		break;
//	case 1001:	//BL
//		bRet = GetBLInfo(pApp) ;
//		break;
//	case 1003:
//	case 1004:
//	case 2002:
//		bRet = GetCryticGamesInfoFromRegistry(pApp);
//		break;
//	default:
//		if(GetInfoFromRegistry(pApp->GetRegistry(), pApp) == S_OK)
//		{
//			bRet = TRUE;
//		}
//		break;
//	}
//
//	return bRet ;
//}

//static BOOL GetFWInfo(CThirdPartyApp *pApp)
//{
//	do{
//		//recognize game from startup program folder of the computer
//		_tstring sInstallPath, sFilePath;
//		_tstring::size_type nPos ;
//		if (pApp->GetStartMenuPath1().empty())
//		{
//			break ;
//		}
//
//		DWORD dwMajorVersion = 0 ;
//		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(GetVersion()))) ;
//
//		int nType = 0 ;
//		if (dwMajorVersion >= 6)
//		{
//			nType = CSIDL_COMMON_PROGRAMS ;
//		}
//		else
//		{
//			nType = CSIDL_PROGRAMS ;
//		}
//
//		_tstring sStartupPath ;
//		sStartupPath = GetStartupProgramPath(nType) ;
//		if(sStartupPath.empty())
//		{
//			break ;
//		}
//		_tstring sLinkPath, sShortCut;
//		sShortCut.assign(sStartupPath.c_str()) ;
//		sShortCut.append(pApp->GetStartMenuPath1().c_str()) ;
//		HRESULT hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//		if (hRes != S_OK)
//		{
//			sShortCut.assign(sStartupPath.c_str()) ;
//			sShortCut.append(pApp->GetStartMenuPath2().c_str()) ;
//			hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//			if (hRes != S_OK)
//			{
//				break ;
//			}
//		}
//		sFilePath.assign(sLinkPath.c_str()) ;
//		nPos = sFilePath.find(_T("patcher.exe")) ;
//		if (nPos == -1)
//		{
//			break ;
//		}
//
//		sFilePath[nPos] = 0 ;
//		sInstallPath = sFilePath ;		
//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//verify the existence of particular files
//		if (sInstallPath.empty())
//		{
//			break ;
//		}
//
//		pApp->GetLocalInfo().sAppPatcherPath.assign(sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppPatcherPath.append(_T("patcher.exe")) ; /*< save patcher.exe path */
//
//		pApp->GetLocalInfo().sInstallPath.assign(sInstallPath.c_str()) ; /*< save install path */
//
//		sFilePath.Format(1, _T("%supdate\\game.exe"), sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppClientFilePath.assign(sFilePath.c_str()) ;
//		if (ENOENT == _taccess_s(pApp->GetLocalInfo().sAppClientFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "uninstall.exe"*/
//		sFilePath.Format(1, _T("%suninstall.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			pApp->GetLocalInfo().sAppUninstallFilePath = _T("") ;
//			break ;
//		}
//		pApp->GetLocalInfo().sAppUninstallFilePath = sFilePath ; /*< save uninstall program file path */
//
//		/*check for existence of "uninstall.ini"*/
//		sFilePath.Format(1, _T("%suninstall.ini"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "uninstall.dat"*/
//		sFilePath.Format(1, _T("%suninstall.dat"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		/*check for existence of "patcher\\patcher.exe"*/
//
//		sFilePath.Format(1, _T("%spatcher.exe"), sInstallPath.c_str()) ;
//		if (ENOENT == _taccess_s(sFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		sFilePath.Format(1, _T("%spatchersync\\patchersync.exe"), sInstallPath.c_str()) ;
//		if (ENOENT != _taccess_s(sFilePath.c_str(), 0))
//		{
//			pApp->GetLocalInfo().sAppLauncherPath.assign(sFilePath.c_str()) ;
//		}
//
//		pApp->GetLocalInfo().nStatus = CThirdPartyApp::INSTALLED;
//
//	}while(FALSE);
//
//	if (pApp->GetLocalInfo().nStatus != CThirdPartyApp::INSTALLED)
//	{//if game not found from start menu item, find from registry
//		GetInfoFromRegistry(pApp->GetRegistry(), pApp);
//	}
//
//	return TRUE;
//}
//
//static BOOL GetRHInfo(CThirdPartyApp *pApp)
//{
//	do{
//		_tstring sInstallPath, sFilePath;
//
//		if (pApp->GetStartMenuPath1().empty())
//		{
//			break ;
//		}
//
//		_tstring sStartupPath ;
//		sStartupPath = GetStartupProgramPath(CSIDL_COMMON_PROGRAMS) ;
//		if(sStartupPath.empty())
//		{
//			break ;
//		}
//		_tstring sLinkPath, sShortCut;
//		sShortCut.assign(sStartupPath.c_str()) ;
//		sShortCut.append(pApp->GetStartMenuPath1().c_str()) ;
//		HRESULT hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//		if (hRes != S_OK)
//		{
//			sShortCut.assign(sStartupPath.c_str()) ;
//			sShortCut.append(pApp->GetStartMenuPath2().c_str()) ;
//			hRes = GetLinkPath(NULL, sShortCut.c_str(), sLinkPath) ;
//			if (hRes != S_OK)
//			{
//				break ;
//			}
//		}
//		sFilePath.assign(sLinkPath.c_str()) ;
//		_tstring::size_type nPos = sFilePath.find(_T("ClientLauncher.exe")) ;
//		sFilePath[nPos] = 0 ;
//		sInstallPath = sFilePath ;
//		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//verify the existence of particular files
//		if (sInstallPath.empty())
//		{
//			break ;
//		}
//
//		pApp->GetLocalInfo().sInstallPath.assign(sInstallPath.c_str()) ; /*< save install path */
//
//		sFilePath.Format(1, _T("%sClientLauncher.exe"), sInstallPath.c_str()) ;
//		pApp->GetLocalInfo().sAppPatcherPath.assign(sFilePath.c_str()) ;
//		pApp->GetLocalInfo().sAppClientFilePath.assign(sFilePath.c_str()) ;
//		if (ENOENT == _taccess_s(pApp->GetLocalInfo().sAppClientFilePath.c_str(), 0))
//		{
//			break ;
//		}
//
//		pApp->GetLocalInfo().nStatus = CThirdPartyApp::INSTALLED;
//
//	}while(FALSE);
//
//	if (pApp->GetLocalInfo().nStatus != CThirdPartyApp::INSTALLED)
//	{//if game not found from start menu item, find from registry
//		GetInfoFromRegistry(pApp->GetRegistry(), pApp);
//	}
//
//	return TRUE;
//}
//
//static BOOL GetBLInfo(CThirdPartyApp *pApp)
//{
//	GetInfoFromRegistry(pApp->GetRegistry(), pApp) ;
//	return TRUE;
//}
//
static HRESULT GetInfoFromRegistry(_tstring sRelPath, CThirdPartyApp *pApp)
{
	HKEY hKey = NULL ;

	do{
		size_t nPos = sRelPath.find('\\');
		_tstring sKeyRoot = sRelPath.Left(nPos);
		_tstring sRelKey = sRelPath.Right(sRelPath.length() - nPos);
		HKEY hKeyRoot = StringToHKey(sKeyRoot);
		if(hKey == INVALID_HANDLE_VALUE)
		{
			return S_OK;
		}
		// check registry key
		if (ERROR_SUCCESS != RegOpenKeyEx(hKeyRoot, sRelKey.c_str(), 0, KEY_QUERY_VALUE, &hKey))
		{
			break ;
		}

		DWORD  dwSize = 1280 ;
		DWORD  dwType = REG_SZ ;
		TCHAR  szResult[1280] = {0} ;

		_tstring sInstallPath, sLauncherPath, sPatcherPath, sClientPath;

		// check install path
		if (ERROR_SUCCESS != RegQueryValueEx(hKey, _T("INSTALL_PATH"), NULL, &dwType, (LPBYTE)szResult, &dwSize))
		{
			break ;
		}
		sInstallPath = szResult;
		if(sInstallPath.empty())
		{
			break;
		}

		// check client path
		dwSize = 1280 ;
		memset(szResult, 0, sizeof(szResult));
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, _T("CLIENT_PATH"), NULL, &dwType, (LPBYTE)szResult, &dwSize))
		{
			if (ENOENT == _taccess_s(szResult, 0))
			{
				break ;
			}
			sClientPath = szResult;
		}

		// check launcher path
		dwSize = 1280 ;
		memset(szResult, 0, sizeof(szResult));
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, _T("LAUNCHER_PATH"), NULL, &dwType, (LPBYTE)szResult, &dwSize))
		{
			sLauncherPath = szResult;
			if (!sLauncherPath.empty() && ENOENT == _taccess_s(szResult, 0))
			{
				break ;
			}
		}

		// check patcher path
		dwSize = 1280 ;
		memset(szResult, 0, sizeof(szResult));
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, _T("PATCHER_PATH"), NULL, &dwType, (LPBYTE)szResult, &dwSize))
		{
			sPatcherPath = szResult;
			if (!sPatcherPath.empty() && ENOENT == _taccess_s(szResult, 0))
			{
				break ;
			}
		}

		// check if all paths are empty
		if(sLauncherPath.empty() && sPatcherPath.empty() && sClientPath.empty())
		{
			break;
		}


		pApp->GetLocalInfo().SetInstallPath(sInstallPath);
		pApp->GetLocalInfo().SetLauncherPath(sLauncherPath);
		pApp->GetLocalInfo().SetPatcherPath(sPatcherPath);
		pApp->GetLocalInfo().SetClientPath(sClientPath);

		////modify patcher path to adjust with old pattern
		//if(sPatcherPath.empty())
		//{
		//	if(!sLauncherPath.empty())
		//	{
		//		pApp->GetLocalInfo().sAppPatcherPath = sLauncherPath;
		//		pApp->GetLocalInfo().sAppLauncherPath = _T("");
		//	}
		//	else
		//	{
		//		pApp->GetLocalInfo().sAppPatcherPath = sClientPath;
		//	}
		//}
		//else
		//{
		//	pApp->GetLocalInfo().sAppPatcherPath = sPatcherPath;
		//}

		pApp->GetLocalInfo().SetLocalStatus(CThirdPartyApp::INSTALLED);

	}while(FALSE) ;

	if (hKey != NULL)
	{
		RegCloseKey(hKey) ;
	}

	return S_OK;
}

bool GetCryticGamesInfoFromRegistry(CThirdPartyApp *pApp)
{
	_tstring sReg, sLauncher;

	switch(pApp->GetAppInfo().GetIntId())
	{
	case 1003:
		sReg = _T("Software\\Cryptic\\Star Trek Online");
		sLauncher = _T("Star Trek Online.exe");
		break;
	case 1004:
		sReg = _T("Software\\Cryptic\\Champions Online");
		sLauncher = _T("Champions Online.exe");
		break;
	case 2002:
		sReg = _T("Software\\Cryptic\\Neverwinter");
		sLauncher = _T("Neverwinter.exe");
		break;
	default:
		return true;
	}

	HKEY hKey = NULL ;

	do
	{
		// check registry key
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, sReg.c_str(), 0, KEY_QUERY_VALUE, &hKey))
		{
			break ;
		}

		DWORD  dwSize = 1280 ;
		DWORD  dwType = REG_SZ ;
		TCHAR  szResult[1280] = {0} ;

		_tstring sInstallPath, sLauncherPath;

		// check install path
		if (ERROR_SUCCESS != RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)szResult, &dwSize))
		{
			break ;
		}
		sInstallPath = szResult;
		if(sInstallPath.empty())
		{
			break;
		}

		if(sInstallPath[sInstallPath.size()-1] != _T('\\'))
		{
			sInstallPath += _T('\\');
		}

		sLauncherPath = sInstallPath + sLauncher;

		if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(sLauncherPath.c_str()))
		{
			pApp->GetLocalInfo().SetInstallPath(sInstallPath);
			pApp->GetLocalInfo().SetLauncherPath(sLauncherPath);
			pApp->GetLocalInfo().SetClientPath(sLauncherPath);
			pApp->GetLocalInfo().SetLocalStatus(CThirdPartyApp::INSTALLED);
		}
	}while(FALSE) ;

	if (hKey != NULL)
	{
		RegCloseKey(hKey) ;
	}

	return true;
}