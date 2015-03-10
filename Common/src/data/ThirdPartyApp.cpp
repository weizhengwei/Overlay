///*************************************************
/**
 * @file      ThirdPartyApp.cpp
   @brief     The implementation file of third party application.
   @author    liuxiangshu@pwrd.com
   @data      2012/08/04
   @version   01.00.00
 */

/* 
 *  Copyright(C) 2012. Perfect World Entertainment Inc
 *
 *  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
 *  its licensors and is protected by copyright and other intellectual 
 *  property laws and treaties.  We and our licensors own all rights, 
 *  title and interest in and to the SOURCE CODE, including all copyright 
 *  and other intellectual property rights in the SOURCE CODE.
 */

/*
 * $LastChangedBy:
 * $LastChangedDate:
 * $Revision:
 * $Id:
 * $notes:
 */
///**************************************************


#include <afx.h>
#include "data/ThirdPartyApp.h"
#include "data/ThirdPartyAppManager.h"
#include "data/DataPool.h"
#include "data/Process.h"
#include "data/Utility.h"
#include "data/UIString.h"
#include "data/SNSManager.h"
#include "login/XmppLogin.h"
#include <atlbase.h>
#include "log/local.h"
#include "data/Tracking.h"
#include "data/Overlay.h"
#include "detour/detours.h"
#include "data/GameInfo.h"
#include "tinyxml\tinyxml.h"
#include <shellapi.h>
#include <shlobj.h>
#include "crypt/md5.h"
#include "data/ExecuteAppInstallConfig.h"
#include "unzipper/CUnZip.h"
#include "download/IDownLoader.h"
#include "ui/UIMessage.h"
#include <afxwin.h>
#include "CoreMsgBrowser2Svr.h"

#define JSON_ATTRIBUTE_GAME_ABBR	"abbr"
#define JSON_ATTRIBUTE_GAME_FLAG	"flag"
#define JSON_ATTRIBUTE_GAME_STATUS	"status"
#define JSON_ATTRIBUTE_GAME_NEW		"new"


#define ___AutoLock_OpenClientList____ {MutexGuard xxxxx_xxx(&m_OpenClientLock);
#define ___AutoLockEnd____   }


///////////////////////////////////////////////////
APP_INFO::APP_INFO() : DataBase(static_cast<IDataChangeNotify*>(this))
{
	LockGuard<Mutex> lock(this);

	nId = 0;
	sId = _T("0");
	nStatus = GS_UNKNOWN;
	SetNewApp(0);
	nBypassLogin = 0;
	bSupportOverlay = false;
	bSupportRepair = false;
	bCheckMD5 = false;

	bEnablePDL = false;
	bSupportPDL = false;
	sPDLDownloadURL = _T("");
	sPDLTotleInitSize = _T("0");

	bPreLoad = false;
	sPreLoadNotice = _T("");
}

APP_LOCAL_INFO::APP_LOCAL_INFO() : DataBase(this)
{
	LockGuard<Mutex> lock(this);

	nStatus = CThirdPartyApp::NOTADDED;
	nDownTool = -1;
	nTraceDownloadTool = -1;
	nOrder = 0;
	bActive = false;
	eDownloadState = INVALIDE_DATA;

	ZeroMemory(&addedtime, sizeof(SYSTEMTIME));
	ZeroMemory(&lastplaytime, sizeof(SYSTEMTIME));

	bPassLanuchPoint = false;
	nDownloadProgress = 0;

	bCreateShortcut = false;
	bLaunchAfterInstall = false;
};

APP_MEM_INFO::APP_MEM_INFO()
{
	LockGuard<Mutex> lock(this);

	bNeedUpdate = false;
	nCmd = CThirdPartyApp::NOCMD;
	bPaused = false;
	bTokenSuccess = false;
	bTokenFail = false;
	bAbortInstall = false;
	bTokenForPatcher = false;
	bDownloadIcon = false;
	bDownloadBkgnd = false;
	bDownloadInstallPic = false;
	bCloudGame = false;
	bFavorite = false;
	bDeletable = true;
}

///////////////////////////////////////////////////

CThirdPartyApp::CThirdPartyApp()
{
	m_instance = 0;
	m_bExit = false;
	m_hwndInstall = NULL;
}

CThirdPartyApp::~CThirdPartyApp()
{
}

bool CThirdPartyApp::Init()
{
	
	CheckIfInstalled();

	CheckNewApp();

	//patch WOI
	if(GetAppIntId() == 25 && GetLocalInfo().GetLocalStatus() >= CThirdPartyApp::EXTRACTED)
	{
		PatchGame();
	}

	return true;
}

bool CThirdPartyApp::Uninit()
{
	m_bExit = true;
	TerminateThread(_threadHandle, 0);
	return true;
}

bool CThirdPartyApp::DownloadIcon()
{
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(GetAppLogoFile().c_str()))
	{
		m_MemInfo.SetDownloadIcon(true);
		return StartTask();
	}

	return false;
}

bool CThirdPartyApp::DownloadBkgnd()
{
	if((INVALID_FILE_ATTRIBUTES == GetFileAttributes(GetAppBkFile().c_str())))
	{
		m_MemInfo.SetDownloadBkgnd(true);
		return StartTask();
	}

	return false;
}


bool CThirdPartyApp::DownloadInstallPic()
{
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(GetAppInstallPicFile().c_str()) )
	{
		m_MemInfo.SetDownloadInstallPic(true);
		return StartTask();
	}
	return false;
}

bool DownloadProc(const _tstring& sFilePath, const _tstring& sFileURL)
{

	if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(sFilePath.c_str()))
	{
		_tstring stmp = sFilePath + _T(".tmp");
		if(g_theWebData.DownloadFile(sFileURL.c_str(), stmp.c_str()))
		{
			if(!MoveFile(stmp.c_str(), sFilePath.c_str()))
			{
				DWORD err = GetLastError();
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CThirdPartyApp::DownloadIconProc()
{
	return DownloadProc(GetAppLogoFile(), GetAppInfo().GetLogoLink());
}

bool CThirdPartyApp::DownloadBkgndProc()
{
	return DownloadProc(GetAppBkFile(), GetAppInfo().GetBgLink());
}

bool CThirdPartyApp::DownloadInstallPicProc()
{
	return DownloadProc(GetAppInstallPicFile(), GetAppInfo().GetInstallPicLink());
}

bool CThirdPartyApp::CheckIfInstalled()
{
	if(GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD   // downloading PDL do not check if install.
		&& GetLocalInfo().GetLocalStatus() == CThirdPartyApp::DOWNLOADING)
	{
		return false;
	}

	CRegKey regKey;
	_tstring sRegSub;
	sRegSub.Format(2, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s%s"), 
		GetAppInfo().GetStringId().c_str(), GetAppLanguage().c_str());
	
	if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
	{
		DWORD dwUncheckFlag = 0;
		if (ERROR_SUCCESS == regKey.QueryDWORDValue(_T("PDL_UNCHECKED"), dwUncheckFlag) 
			&& dwUncheckFlag != 0)
		{
			return false; // do not check downloading pdl 
		}
	}

	int nLastStatus = GetLocalInfo().GetLocalStatus();
		
	// check silence install file 
	if(GetLocalInfo().GetLocalStatus() >= CThirdPartyApp::EXTRACTED)
	{
		// check files
		bool bOk = false;
		do
		{
			if(GetLocalInfo().GetInstallPath().empty())
			{
				break;
			}
			//if launcher or patcher or client exists then the game is installed, for FW client name will change to pem.exe
			if (!GetLocalInfo().GetLauncherPath().empty() && (ENOENT != _taccess_s(GetLocalInfo().GetLauncherPath().c_str(), 0)))
			{
				bOk = true;
				break ;
			}
			if (!GetLocalInfo().GetPatcherPath().empty() && (ENOENT != _taccess_s(GetLocalInfo().GetPatcherPath().c_str(), 0)))
			{
				bOk = true;
				break ;
			}
			if (!GetLocalInfo().GetClientPath().empty() && (ENOENT != _taccess_s(GetLocalInfo().GetClientPath().c_str(), 0)))
			{
				bOk = true;
				break ;
			}
		}while(0);
		if(!bOk)
		{
			ClearLocalInfo();
			RemoveFromMyList();
		}
	}

	int nCheckAddedtime = CThirdPartyApp::INSTALLED;

	if(GetLocalInfo().GetLocalStatus() != CThirdPartyApp::INSTALLED)
	{
		if(CheckSilenceRegistry())
		{
			nCheckAddedtime = CThirdPartyApp::EXTRACTED;
		}
		else
		{
			GetAppLocalInfo(this);
		}
	}

	if(GetLocalInfo().GetLocalStatus() >= nCheckAddedtime && GetLocalInfo().GetAddedtime().wYear == 0)
	{
		FILETIME createftTime = {0};
		HANDLE hFile = CreateFile(GetLocalInfo().GetClientPath().c_str(), GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (GetFileTime(hFile,&createftTime,NULL,NULL))
			{
				SYSTEMTIME sysTime = {0};
				FileTimeToSystemTime(&createftTime,&sysTime);
				GetLocalInfo().SetAddedtime(sysTime);
			}
			CloseHandle(hFile);
		}
	}

	return (GetLocalInfo().GetLocalStatus() == CThirdPartyApp::INSTALLED);
}

bool CThirdPartyApp::CheckUninstall()
{
	if(GetLocalInfo().GetLocalStatus() < CThirdPartyApp::EXTRACTED ||
		GetMemInfo().GetCmd() != NOCMD || IsPlaying())
	{
		return false;
	}

	bool bInstalled = false;
	do 
	{
		if(!GetLocalInfo().GetLauncherPath().empty() &&
			INVALID_FILE_ATTRIBUTES != GetFileAttributes(GetLocalInfo().GetLauncherPath().c_str()))
		{
			bInstalled = true;
			break;
		}
		if(!GetLocalInfo().GetPatcherPath().empty() &&
			INVALID_FILE_ATTRIBUTES != GetFileAttributes(GetLocalInfo().GetPatcherPath().c_str()))
		{
			bInstalled = true;
			break;
		}
		if(!GetLocalInfo().GetClientPath().empty() && 
			INVALID_FILE_ATTRIBUTES != GetFileAttributes(GetLocalInfo().GetClientPath().c_str()))
		{
			bInstalled = true;
			break;
		}
		bool bInstalled = false;
	} while (0);

	return !bInstalled;
}

bool CThirdPartyApp::CheckForUpdate()
{
	return false;
}

void CThirdPartyApp::CheckNewApp()
{
	if(GetAppInfo().GetIsNewPopup() && GetLocalInfo().GetLocalStatus() > CThirdPartyApp::NOTADDED)
	{
		GetAppInfo().SetNewApp(0);
	}
}

void CThirdPartyApp::SetOrder(int nOrder)
{
	GetLocalInfo().SetOrder(nOrder);
}

bool CThirdPartyApp::StartApp()
{
	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::STARTAPP);
	return StartTask();
}

bool CThirdPartyApp::StartPatcher()
{
	if(IsPlaying())
	{
		return true;
	}

	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::REPAIRAPP);
	return StartTask();
}

bool CThirdPartyApp::Install()
{
	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::INSTALLAPP);
	return StartTask();
}

bool CThirdPartyApp::UnInstall()
{
	if(IsPlaying())
	{
		return false;
	}

	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::UNINSTALLAPP);
	return StartTask();
}

bool CThirdPartyApp::ExtractFile()
{
	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::EXTRACT);
	return StartTask();
}


bool CThirdPartyApp::UnstallPDLApp()
{
	if(IsPlaying())
	{
		return false;
	}

	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::UNINSTALL_PDL);
	return StartTask();
}

bool CThirdPartyApp::ExtratPdlApp()
{
	if(GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::EXTRAT_PDL);
	return StartTask();
}

bool  CThirdPartyApp::ClearPDLData()
{
	if (GetMemInfo().GetCmd() != CThirdPartyApp::NOCMD)
	{
		return false;
	}

	GetMemInfo().SetCmd(CThirdPartyApp::CLEAR_PDLDATA);
	return StartTask();
}

long CThirdPartyApp::run()
{
	while(GetMemInfo().GetDownloadIcon() || GetMemInfo().GetDownloadBkgnd() || 
		GetMemInfo().GetIsTokenSuccess() || GetMemInfo().GetIsTokenFail() ||
		GetMemInfo().GetCmd() != NOCMD || GetMemInfo().GetDownloadInstallPic())
	{
		if(GetMemInfo().GetDownloadIcon())
		{
			if(DownloadIconProc())
			{
				g_appManager.PostMessageToUIApp(this, UIMessageUpdateIcon());
			}
			GetMemInfo().SetDownloadIcon(false);
		}

		if(GetMemInfo().GetDownloadBkgnd())
		{
			if(DownloadBkgndProc())
			{
				CWnd *pwnd = AfxGetApp()->GetMainWnd();
				if(pwnd)
				{
					pwnd->PostMessage(WM_UPDATE_BKGND, 0, LPARAM(static_cast<IThirdApp*>(this)));
				}
			}
			GetMemInfo().SetDownloadBkgnd(false);
		}

		if (GetMemInfo().GetDownloadInstallPic())
		{
			DownloadInstallPicProc();
			GetMemInfo().SetDownloadInstallPic(false);
		}

		if(GetMemInfo().GetIsTokenSuccess())
		{
			OnGameTokenSuccess();
		}

		if(GetMemInfo().GetIsTokenFail())
		{
			OnGameTokenFail();
		}

		// process command
		_tstring sCounterName;
		switch(GetMemInfo().GetCmd())
		{
		case STARTAPP:
			g_theTracking.GameLaunched(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_LAUNCH_GAME")), \
				theDataPool.GetUserName(), GetAppLanguage(), GetAppInfo().GetAbbr(), GetLocalInfo().GetTraceDownloadTool());

			sCounterName = _T("gamelaunch_") + GetAppName();
			g_theTracking.CounterTrackingNew(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_NEW_COUNTER")), \
				sCounterName, GetAppLanguage(), false) ;
			
			PatchGame();
			StartProcNew();
			GetMemInfo().SetCmd(NOCMD);
			break;
		case REPAIRAPP:
			StartPatcherProcNew();
			GetMemInfo().SetCmd(NOCMD);
			break;
		case INSTALLAPP:
			if(IsSilence())
			{
				SilenceInstallProc();
			}
			GetMemInfo().SetCmd(NOCMD);
			if(GetMemInfo().GetIsAbortInstall())
			{
				GetMemInfo().SetIsAbortInstall(false);
			}
			if((GetLocalInfo().GetLocalStatus() == CThirdPartyApp::INSTALLED) 
				|| (GetLocalInfo().GetLocalStatus() == CThirdPartyApp::DOWNLOADING && GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD))
			{
				g_appManager.PostMessageToUIApp(this,UIMessageAppNotify(IAppCallBack::INSTALLFINISH));
			}
			break;
		case UNINSTALLAPP:
			if(IsSilence())
			{
				SilenceUninstallProc();
			}
			GetMemInfo().SetCmd(NOCMD);

			if(GetLocalInfo().GetLocalStatus() == CThirdPartyApp::NOTADDED)
			{
				g_theTracking.GameUninstalled(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_UNINSTALL_GAME")), \
					theDataPool.GetUserName(), GetAppLanguage(), GetAppInfo().GetAbbr().MakeLowerConst(), GetLocalInfo().GetTraceDownloadTool());

				sCounterName = _T("gameunins_") + GetAppName();
				g_theTracking.CounterTrackingNew(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_NEW_COUNTER")), \
					sCounterName, GetAppLanguage(), false) ;

				g_appManager.PostMessageToUIApp(this,UIMessageAppNotify(IAppCallBack::UNINSTALLFINISH));
			}
			else
			{
				g_appManager.PostMessageToUIApp(this,UIMessageAppNotify(0));
			}
			break;
		case EXTRACT:
			{
				int nRet = 0, nRetry = 3 ;
				do{//retry 3 times if zip file is in use, if it is still in use, throw error to user
					nRet = UnZipDownloadedPackage() ;
					if (nRet == IAppCallBack::EXTRACTFILEINUSE) 
					{
						Sleep(10000) ;
					}
					-- nRetry ;
				}while((nRet == IAppCallBack::EXTRACTFILEINUSE) && (nRetry >= 0));

				if (nRet != IAppCallBack::EXTRACTFILEINUSE && nRet != IAppCallBack::NOTENOUGHSPACEFOREXTRACTION
					&& nRet != IAppCallBack::EXTRACTACCESSDENIED)
				{
					ClearDownloadInfo();
					ClearCurrentDownloadInfo();
				}
	
				GetMemInfo().SetCmd(NOCMD);

				if(nRet == 0)
				{
					//Postback to hasoffers
					_tstring sGameAbbr = theDataPool.GetRegisterGame();
					_tstring sLang = theDataPool.GetRegisterLang();
					_tstring sType = theDataPool.GetRegisterType();
					_tstring sOfferID = theDataPool.GetOfferID();
					if (!sGameAbbr.CompareNoCase(this->GetAppArrb()) && !sType.CompareNoCase(_T("hasoffers")))
					{
						_tstring sRefID, sGoalID;
						sRefID.Format(2, _T("%s_%s_complete"), sGameAbbr.c_str(), sLang.c_str());
						g_theWebData.GetGoalInfo(g_theSNSManager.GetSessionId(), sOfferID, sRefID, sGoalID);
						if (!sGoalID.empty())
						{
							g_theWebData.PostBackHasoffersGoal(theDataPool.GetTransactionID(), sGoalID);	//Post back hasoffers tracking url
						}
					}

					g_appManager.PostMessageToUIApp(this,UIMessageToPlay());
				}
				else
				{ 
					g_appManager.PostMessageToUIApp(this,UIMessageAppNotify(nRet));
				}
			}
			break;
		case UNINSTALL_PDL:
			if(IsSilence())
			{
				SilenceUninstallPdlProc();
			}
			GetMemInfo().SetCmd(NOCMD);
		
			break;
		case EXTRAT_PDL:
			SilenceRegistPdlProc(); // just add reg info .
			GetMemInfo().SetCmd(NOCMD);
			break;
		case CLEAR_PDLDATA:
			g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(CancelBegin));
			ClearPDLDataProc();
			g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(CancelEnd));
			GetMemInfo().SetCmd(NOCMD);
			break;
		default:
			GetMemInfo().SetCmd(NOCMD);
			break;
		}
	}

	return 0;
}

bool CThirdPartyApp::ClearPDLDataProc()
{
	// unregister registry
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;
	ExecuteInstallConfig(sPath, 1);

	// delete game files
	DeleteDir(GetLocalInfo().GetInstallPath());

	// delete pdl download cache
	_tstring sGameName;
	sGameName.Format(2, _T("\\%s_%s"), GetAppInfo().GetName().c_str(), GetAppLanguage().c_str());
	_tstring szPDLRootDir = theDataPool.GetBaseDir() + PDL_CONFIG_DIR;
	szPDLRootDir.append(sGameName);
	DeleteDir(szPDLRootDir);

	// clear download info
	ClearCurrentDownloadInfo();
	ClearDownloadInfo();

	GetLocalInfo().SetLocalStatus(CThirdPartyApp::ADDED);
	GetMemInfo().SetIsPaused(false);

	GetLocalInfo().SetIsPassLanuchPoint(false);
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
	_tstring sId = MakeAbbrLangId(GetAppArrb(), GetAppLanguage());
	g_appManager.DelAutoResumeItem(sId);
	return true;
}

bool CThirdPartyApp::IsSilence()
{
	// return true if silence install config file exists
	if(GetLocalInfo().GetInstallPath().empty())
	{
		return false;
	}
	_tstring sConfigFile = SGIFILENAME;
	_tstring sPath = GetLocalInfo().GetInstallPath() + sConfigFile;
	if(GetFileAttributes(sPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CThirdPartyApp::SilenceInstallProc()
{
	_tstring sConfigFile = SGIFILENAME;
	_tstring sPath;
	_tstring sInstallPath = GetLocalInfo().GetInstallPath();
	sPath = sInstallPath + sConfigFile;
	if(GetFileAttributes(sPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_INSTALL_XMLERROR, GetLocalInfo().GetTraceDownloadTool());
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pElement = NULL, *pRoot = NULL, *pRegistry, *pDependency;

	if(!xmlDoc.LoadFile(sPath.toNarrowString().c_str())
		|| !(pRoot = xmlDoc.RootElement()))
	{
		SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_INSTALL_XMLERROR, GetLocalInfo().GetTraceDownloadTool());
		return false;
	}

	//get dependency count
	int nDependencyCount = 0;
	pDependency = pRoot->FirstChildElement("dependency");
	if(pDependency)
	{
		pElement = pDependency->FirstChildElement("item");
		while(pElement)
		{
			nDependencyCount++;
			pElement = pElement->NextSiblingElement("item");
		}
	}
	int nMaxProgress = 100/nDependencyCount;
	int nMinProgress = 0;

	SendMessage(m_hwndInstall, WM_INSTALL_PROGRESS, nMinProgress, nMaxProgress);

	// install dependency
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	pDependency = pRoot->FirstChildElement("dependency");
	if(pDependency)
	{
		pElement = pDependency->FirstChildElement("item");
		while(pElement)
		{
			// generate path,
			_tstring sFile, sCmd, sCurDir, sfullcmd;
			LPCTSTR pszCurDir = NULL;
			sFile = pElement->Attribute("image");
			sCmd = pElement->Attribute("command");

			// transform %INSTALLDIR% %SYSTEMDIR%
			if(!sFile.empty())
			{
				TranslatePath(sFile);
				size_t pos = sFile.ReverseFind(_T('\\'));
				if(pos != _tstring::npos)
				{
					sCurDir = sFile.Left(pos);
					pszCurDir = sCurDir.c_str();
				}

				sfullcmd.Format(1, _T("\"%s\" "), sFile.c_str());
			}
			if(!sCmd.empty())
			{
				TranslatePath(sCmd);
				sfullcmd += sCmd;
			}

			TCHAR tsCmd[4096] = _T("");
			lstrcpyn(tsCmd,sfullcmd.c_str(),4096);

			STARTUPINFO si = { 0 };
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi = { 0 };
			
			if(!CreateProcess(NULL, tsCmd, NULL, NULL, 
				FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, pszCurDir, &si, &pi))
			{
				DWORD dwErr = GetLastError();
				//return false;
			}

			while(WAIT_OBJECT_0 != WaitForSingleObject(pi.hProcess, 1000))
			{
				// install aborted
				if(GetMemInfo().GetIsAbortInstall() || m_bExit)
				{
					TerminateProcess(pi.hProcess, 0);
					return false;
				}
			}

			DWORD dwExitCode = 0;
			if(!GetExitCodeProcess(pi.hProcess, &dwExitCode) || dwExitCode)
			{
				 //return false;
			}

			pElement = pElement->NextSiblingElement();
			nMinProgress = nMaxProgress;
			nMaxProgress += 100/nDependencyCount;
			SendMessage(m_hwndInstall, WM_INSTALL_PROGRESS, nMinProgress, nMaxProgress);
		}
	}
	SendMessage(m_hwndInstall, WM_INSTALL_PROGRESS, 100, 100);
	Sleep(1000);
	// write install flag
	CRegKey regKey;
	_tstring sRegSub;
	sRegSub.Format(2, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s%s"), 
		 GetAppInfo().GetStringId().c_str(), GetAppInfo().GetLanguage().c_str());
	if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) != ERROR_SUCCESS)
	{
		return false;
	}
	regKey.SetDWORDValue(_T("installed"), 1);
	regKey.Close();
	
	if (!(GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD && GetLocalInfo().GetLocalStatus() == DOWNLOADING))
	{
		GetLocalInfo().SetLocalStatus(CThirdPartyApp::INSTALLED);
	}

	SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_INSTALL_FINISH, GetLocalInfo().GetTraceDownloadTool());

	return true;
}

bool CThirdPartyApp::WriteSilenceRegistry()
{
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;

	if(ExecuteInstallConfig(sPath, 0, GetAppLanguage()))
	{
		// write APP_ABBR registry
		CRegKey regKey;
		_tstring sRegSub, sRegTemp;
		sRegTemp.Format(1, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s"), 
			 GetAppInfo().GetStringId().c_str());
		sRegSub = sRegTemp + _T("en");
		if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
		{
			regKey.SetStringValue(_T("APP_ABBR"), GetAppInfo().GetAbbr().c_str());
			regKey.Close();
		}
		sRegSub = sRegTemp + _T("de");
		if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
		{
			regKey.SetStringValue(_T("APP_ABBR"), GetAppInfo().GetAbbr().c_str());
			regKey.Close();
		}

		sRegSub = sRegTemp + _T("fr");
		if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
		{
			regKey.SetStringValue(_T("APP_ABBR"), GetAppInfo().GetAbbr().c_str());
			regKey.Close();
		}

		sRegSub = sRegTemp + _T("pt");
		if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
		{
			regKey.SetStringValue(_T("APP_ABBR"), GetAppInfo().GetAbbr().c_str());
			regKey.Close();
		}

		CheckSilenceRegistry();
		SYSTEMTIME stm;
		GetLocalTime(&stm);
		_tstring installData;
		installData.Format(3, _T("%d-%d-%d"), stm.wYear, stm.wMonth, stm.wDay);
		GetLocalInfo().SetInstallDate(installData);
		return true;
	}
	return false;
}

bool CThirdPartyApp::IsInstalledFlagSet()
{
	CRegKey regkey;
	TCHAR keyValue[1024] = {0};
	ULONG nSize = 1024;
	_tstring sKeyName;
	sKeyName.Format(2, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s%s"), 
		GetAppInfo().GetStringId().c_str(), GetAppInfo().GetLanguage().c_str());

	if(regkey.Open(HKEY_LOCAL_MACHINE, sKeyName.c_str()) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD dwInstall = 0;
	if(regkey.QueryDWORDValue(_T("installed"), dwInstall) == ERROR_SUCCESS && dwInstall != 0)
	{
		regkey.Close();
		return true;
	}

	regkey.Close();

	return false;
}

bool CThirdPartyApp::CheckSilenceRegistry()
{
	CRegKey regkey;
	TCHAR keyValue[1024] = {0};
	ULONG nSize = 1024;
	_tstring sKeyName;
	sKeyName.Format(2, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s%s"), 
		GetAppInfo().GetStringId().c_str(), GetAppInfo().GetLanguage().c_str());

	if(regkey.Open(HKEY_LOCAL_MACHINE, sKeyName.c_str()) != ERROR_SUCCESS)
	{
		return false;
	}

	bool bOk = false;
	do 
	{
		if(regkey.QueryStringValue(_T("INSTALL_PATH"), keyValue, &nSize) != ERROR_SUCCESS || 
			_tcslen(keyValue) == 0 || INVALID_FILE_ATTRIBUTES == GetFileAttributes(keyValue))
		{
			break;
		}
		GetLocalInfo().SetInstallPath(keyValue);
		
		nSize = 1024;
		if(regkey.QueryStringValue(_T("LAUNCHER_PATH"), keyValue, &nSize) == ERROR_SUCCESS &&
			_tcslen(keyValue) > 0 && INVALID_FILE_ATTRIBUTES != GetFileAttributes(keyValue))
		{
			bOk = true;
			GetLocalInfo().SetLauncherPath(keyValue);
		}

		nSize = 1024;
		if(regkey.QueryStringValue(_T("PATCHER_PATH"), keyValue, &nSize) == ERROR_SUCCESS &&
			_tcslen(keyValue) > 0 && INVALID_FILE_ATTRIBUTES != GetFileAttributes(keyValue))
		{
			bOk = true;
			GetLocalInfo().SetPatcherPath(keyValue);
		}

		nSize = 1024;
		if(regkey.QueryStringValue(_T("CLIENT_PATH"), keyValue, &nSize) != ERROR_SUCCESS ||
			_tcslen(keyValue) == 0 || INVALID_FILE_ATTRIBUTES != GetFileAttributes(keyValue))
		{
			bOk = true;
			GetLocalInfo().SetClientPath(keyValue);
		}

	} while (0);

	if(bOk)
	{
		if(GetLocalInfo().GetLocalStatus() < CThirdPartyApp::EXTRACTED 
			&& !(GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD && IsDownloading()))
		{
			DWORD dwInstall = 0;
			if(regkey.QueryDWORDValue(_T("installed"), dwInstall) == ERROR_SUCCESS && dwInstall == 1)
			{
				GetLocalInfo().SetLocalStatus(CThirdPartyApp::INSTALLED);
			}
			else
			{
				GetLocalInfo().SetLocalStatus(CThirdPartyApp::EXTRACTED);
			}
		}
	}
	else
	{
		GetLocalInfo().SetInstallPath(_T(""));
		GetLocalInfo().SetLauncherPath(_T(""));
		GetLocalInfo().SetPatcherPath(_T(""));
		GetLocalInfo().SetClientPath(_T(""));
	}

	regkey.Close();

	return bOk;
}

bool CThirdPartyApp::SilenceUninstallProc()
{
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;

	ExecuteInstallConfig(sPath, 1);
	DeleteShortcut(sPath);

	// delete game directory
	DeleteDir(GetLocalInfo().GetInstallPath());
	ClearLocalInfo();
	return true;
}

bool CThirdPartyApp::PatchGame()
{
	//patch for WOI
	if(GetAppInfo().GetIntId() == 25)
	{
		_tstring srcLauncher = theDataPool.GetBaseDir() + _T("gamepatch\\woi\\launcher.exe");
		_tstring destLauncher = GetLocalInfo().GetInstallPath() + _T("launcher.exe");

		//check md5
		unsigned char MD5[16] = "";
		CUtility::GenerateMD5ForFile(destLauncher, MD5);
		_tstring sMd5= CUtility::ConvertHexToString(MD5, sizeof(MD5));
		if(sMd5 == _T("EB84042CA40A1BECCD1091BCF55E053D"))
		{
			//delete old launcher
			if(!CUtility::DeleteFileEx(destLauncher.c_str(), NULL))
			{
				return false;
			}
			//copy launcher.exe
			if(!CopyFile(srcLauncher.c_str(), destLauncher.c_str(), FALSE))
			{
				return false;
			}
			//copy arcwoi.ini
			_tstring destKeyfile = GetLocalInfo().GetInstallPath() + _T("LauncherCfg\\arcwoi.ini");
			if(GetFileAttributes(destKeyfile.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				_tstring srcKeyfile = theDataPool.GetBaseDir() + _T("gamepatch\\woi\\arcwoi.ini");
				CopyFile(srcKeyfile.c_str(), destKeyfile.c_str(), FALSE);
			}
		}
	}
	return true;
}

static HANDLE create_password_pipe(const char *password)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE pipe_in, pipe_out;
	BOOL success;
	DWORD len;

	// Create security attributes.
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create pipe.
	success = CreatePipe(&pipe_out, &pipe_in, &sa, 4096);
	if(!success)
	{
		return NULL;
	}

	// generate MD5
	MD5Context ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)password, strlen(password));
	unsigned char sum[16];
	MD5Final(sum, &ctx);

	_tstringA sMD5, stmp;
	for(int i = 0; i < 16; i++)
	{
		stmp.Format(1, "%02x", sum[i]);
		sMD5 += stmp;
	}

	if (theDataPool.IsFacebookAccount())
	{
		sMD5 = theDataPool.GetByPassToken(); //for facebook account, just used token 
	}
	// Write MD5 into pipe.
	success = WriteFile(pipe_in, sMD5.MakeUpper().c_str(), sMD5.size(), &len, NULL);
	CloseHandle(pipe_in);
	if(!success || len != sMD5.size())
	{
		return NULL;
	}

	return pipe_out;
}

bool CThirdPartyApp::StartProcNew()
{
	if(CheckIfClientRunning())
	{
		m_instance++;
	}
	else
	{
		___AutoLock_OpenClientList____
		m_OpenClientList.clear();
		___AutoLockEnd____
		m_instance = 1;
	}
	___AutoLock_OpenClientList____
	m_OpenClientList.push_back(m_instance);
	___AutoLockEnd____

	SetArcGameEnvVar();

	if(GetAppInfo().GetBypassLogin() == 2)
	{//fix bug: bypass failed for cryptic games when legacy user logged in with email address. 
	 //reason: MD5(email+password) is not saved in database. 
	 //temporary solution: use username for bypass login for legacy user(new username starts with "#1#").
		if(m_instance == 1)
		{
			LaunchCrypticGame(true);
		}
		else
		{
			LaunchCrypticGame(false);
		}
	}
	else
	{
		if(GetAppInfo().GetBypassLogin() == 1 && m_instance == 1)
		{
			LaunchGamePatcherBypass();
		}
		else
		{
			LaunchGameClientBypass();
		}
	}

	return true;
}

bool CThirdPartyApp::LaunchCrypticGame(bool bSupportBypass)
{
	_tstring sExe = GetGameStartExe();

	if(sExe.empty())
	{
		return false;
	}

	_tstring sGameDir = sExe.Left(sExe.ReverseFind(_T('\\')));

	_tstring sCmd;
	HANDLE pipe = NULL;
	if(bSupportBypass)
	{
		_tstringA sPwd;
		if(theDataPool.GetUserName().Find(_T("#1#"))==_tstring::npos)
		{
			sPwd = theDataPool.GetUserName().toNarrowString();
		}
		else
		{
			int nLoginType = 0;
			_tstring salt;
			if(g_theWebData.GetSaltService(theDataPool.GetUserInput(), nLoginType, salt))
			{
				sPwd = salt;
			}
			else
			{
				sPwd = theDataPool.GetUserInput().toNarrowString();
			}
		}
		sPwd += theDataPool.GetPassword();

		HANDLE pipe = create_password_pipe(sPwd.c_str());

		GetCmdlinePara(sCmd, pipe);
	}
	else
	{
		sCmd.Format(1, _T("-CoreLocale %s"), theDataPool.GetLangString().c_str());
	}

	_tstring sFullCmd;
	sFullCmd.Format(2, _T("\"%s\" %s"), sExe.c_str(), sCmd.c_str());

	char szCmd[1024] = "";
	lstrcpynA(szCmd,sFullCmd.toNarrowString().c_str(),1024);

	if (!CreateGameProcess(sExe.toNarrowString().c_str(), NULL, szCmd, sGameDir.toNarrowString().c_str()))
	{
		if(pipe)
		{
			CloseHandle(pipe);
		}
		return false;
	}

	// add game play monitor
	g_appManager.MonitorGamePlayingState(this);

	if(pipe)
	{
		CloseHandle(pipe);
	}

	return true;
}

bool CThirdPartyApp::LaunchGameClientBypass()
{
	_tstring sExe = GetGameStartExe();

	if(sExe.empty())
	{
		return false;
	}

	_tstring sGameDir = sExe.Left(sExe.ReverseFind(_T('\\')));

	_tstring sLanguage = theDataPool.GetLangString() ;
	_tstring sCmd = _T("coreclient:1 arc:1");
	if(!GetLocalInfo().GetQACmd().empty())
	{
		sCmd += _T(" ");
		sCmd += GetLocalInfo().GetQACmd();
	}
	char szCmd[1024] = "";
	lstrcpynA(szCmd,sCmd.toNarrowString().c_str(),1024);

    if (!CreateGameProcess(sExe.toNarrowString().c_str(), sExe.toNarrowString().c_str(), szCmd, sGameDir.toNarrowString().c_str()))
	{
		return false;
	}

	// add game play monitor
	g_appManager.MonitorGamePlayingState(this);
	return true;
}

bool CThirdPartyApp::LaunchGamePatcherBypass()
{
	GetMemInfo().SetIsTokenForPatcher(true);

	g_theLoginMgr.RegisterListener(this);
	g_theLoginMgr.GetGameToken(GetAppInfo().GetAbbr());
	return true;
}

bool CThirdPartyApp::SetArcGameEnvVar()
{
	DWORD dwTotalId = MAKETOTALID(GetAppInfo().GetIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), m_instance);
	_tstring sGameId;
	sGameId.Format(1, _T("%u"), dwTotalId);
	_tstring sProcessId;
	sProcessId.Format(1, _T("%d"), GetCurrentProcessId());
	SetEnvironmentVariable(_T("CoreGameId"), sGameId.c_str());
	SetEnvironmentVariable(_T("ArcGameExeName"), GetAppInfo().GetExeName().c_str());
	SetEnvironmentVariable(_T("ArcGameInstallPath"), GetLocalInfo().GetInstallPath().c_str());
	SetEnvironmentVariable(_T("ArcProcessId"), sProcessId.c_str());
	if(GetAppInfo().GetIsSupportOverlay())
	{
		SetEnvironmentVariable(_T("ArcEnableOverlay"), _T("1"));
	}
	else
	{
		SetEnvironmentVariable(_T("ArcEnableOverlay"), _T("0"));
	}
	if(GetAppInfo().GetIntId() == 1002) //temporarily disable overlay on RZ
	{
		SetEnvironmentVariable(_T("ArcEnableOverlay"), _T("0"));
	}
	return true;
}

_tstring CThirdPartyApp::GetGameStartExe()
{
	_tstring sExe;
	if(!GetLocalInfo().GetLauncherPath().empty())
	{
		sExe = GetLocalInfo().GetLauncherPath();
	}
	else if(!GetLocalInfo().GetPatcherPath().empty())
	{
		sExe = GetLocalInfo().GetPatcherPath();
	}
	else
	{
		sExe = GetLocalInfo().GetClientPath();
	}

	return sExe;
}

bool CThirdPartyApp::CheckIfClientRunning()
{
	___AutoLock_OpenClientList____
	for(OPENCLIENTLIST::iterator it = m_OpenClientList.begin(); it != m_OpenClientList.end(); it++)
	{
		DWORD dwTotalId = MAKETOTALID(GetAppInfo().GetIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), (*it));
		_tstring sEventName;
		sEventName.Format(2, _T("arc_%d_%u"), GetCurrentProcessId(), dwTotalId);
		HANDLE h = OpenEvent(EVENT_ALL_ACCESS, FALSE, sEventName.c_str());
		if(h)
		{
			CloseHandle(h);
			return true;
		}
	}
	return false;
	___AutoLockEnd____
}

bool CThirdPartyApp::StartPatcherProcNew()
{
	if(!GetAppInfo().GetIsSupportRepair() || GetLocalInfo().GetPatcherPath().empty())
	{
		return false;
	}

	_tstring sExe = GetLocalInfo().GetPatcherPath();

	SetArcGameEnvVar();

	_tstring sGameDir = sExe.Left(sExe.ReverseFind(_T('\\')));

	_tstring sLanguage = theDataPool.GetLangString() ;
	_tstring sCmd = _T("coreclient:1 arc:1");
	char szCmd[1024] = "";
	lstrcpynA(szCmd,sCmd.toNarrowString().c_str(),1024);
	
    if (!CreateGameProcess(sExe.toNarrowString().c_str(), sExe.toNarrowString().c_str(), szCmd, sGameDir.toNarrowString().c_str()))
	{
		return false;
	}

	return true;
}

bool CThirdPartyApp::LOGIN_GameTokenSuccess(WPARAM wParam, LPARAM lParam)
{
	GetMemInfo().SetIsTokenSuccess(true);
	return StartTask();
}

bool CThirdPartyApp::LOGIN_GameTokenFail(WPARAM wParam, LPARAM lParam)
{
	GetMemInfo().SetIsTokenFail(true);
	return StartTask();
}

bool CThirdPartyApp::OnGameTokenSuccess()
{
	GetMemInfo().SetIsTokenSuccess(false);
	g_theLoginMgr.UnRegisterListener(this);

	_tstring sGameToken = theDataPool.GetGameToken(GetAppInfo().GetAbbr()) ;
	if ( sGameToken.empty() || (sGameToken.find_first_of(_T(':')) != string::npos))
	{//token is not allowed to contain ':', otherwise, PWI will not be bypass logined.
		g_theLoginMgr.RegisterListener(this);
		g_theLoginMgr.GetGameToken(GetAppInfo().GetAbbr());
		return 0 ;
	}

	//-------------------------
	//implement new bypass
	_tstring sCmd;
	GetCmdlinePara(sCmd);

	if(GetMemInfo().GetIsTokenForPatcher())
	{
		GetMemInfo().SetIsTokenForPatcher(false);

		_tstring sExe = GetGameStartExe();
		if(sExe.empty())
		{
			return false;
		}

		_tstring sGameDir = sExe.Left(sExe.ReverseFind(_T('\\')));

		_tstring sFullCmd;
		sFullCmd.Format(2, _T("\"%s\" %s"), sExe.c_str(), sCmd.c_str());
		char szCmd[1024] = "";
		lstrcpynA(szCmd, sFullCmd.toNarrowString().c_str(),1024);
		   
        if (!CreateGameProcess(sExe.toNarrowString().c_str(), NULL, szCmd, sGameDir.toNarrowString().c_str()))
		{
			return false;
		}

		// add game play monitor
		g_appManager.MonitorGamePlayingState(this);

	}
	else
	{
		DWORD dwTotalId = MAKETOTALID(GetAppInfo().GetIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), 1);
		core_msg_token msgToken;
		msgToken.dwCmdId = CORE_MSG_TOKEN;
		msgToken.dwClientId = dwTotalId;
		msgToken.dwRet = 0;
		lstrcpynA(msgToken.csGameToken,sCmd.toNarrowString().c_str(),1024);
		g_appManager.GetCoreMsgServer().SendCoreMsg(dwTotalId, &msgToken);
	}

	return true;
}

bool CThirdPartyApp::OnGameTokenFail()
{
	GetMemInfo().SetIsTokenFail(false);
	g_theLoginMgr.UnRegisterListener(this);
	if(GetMemInfo().GetIsTokenForPatcher())
	{
		return LaunchGameClientBypass();
	}
	return true;
}

int CThirdPartyApp::UnZipDownloadedPackage()
{
	int nRet = 0 ;
	_tstring sZipFile, sDstFolder ;

	do{
		// get zip file path
		sZipFile = GetLocalInfo().GetDownloadPath().Left(3) + FILEPATH_ARCTEMP_DOWNLOAD + GetLocalInfo().GetLocalDownFileName();
		if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(sZipFile.c_str()))
		{
			sZipFile = GetLocalInfo().GetDownloadPath().Left(3) + FILEPATH_DEFAULT_DOWNLOAD + GetLocalInfo().GetLocalDownFileName();
		}

		TCHAR szFolderName[MAX_PATH] = {0} ;
		_tstring sConfigPath = theDataPool.GetUserProfilePath() ;
		GetPrivateProfileString(INI_CFG_CLIENT_NETWORK,INI_CFG_CLIENT_NETWORK_DOWNPATH,NULL,szFolderName,MAX_PATH, sConfigPath.c_str());

		// create dest folder
		if(!PathIsDirectory(szFolderName))
		{
			CUtility::MakeDir(szFolderName);
		}

		// get dest game path
		sDstFolder.Format(3, _T("%s%s_%s\\"), szFolderName, 
			GetAppInfo().GetName().c_str(), GetAppInfo().GetLanguage().c_str());

		// check dest game path
		if(PathIsDirectory(sDstFolder.c_str()))
		{
			DeleteDir(sDstFolder);
			if(PathIsDirectory(sDstFolder.c_str()))
			{
				SYSTEMTIME stm;
				GetLocalTime(&stm);
				sDstFolder.Format(9, _T("%s%s_%s_%4d%02d%02d%02d%02d%02d\\"), szFolderName,
					GetAppInfo().GetName().c_str(), GetAppInfo().GetLanguage().c_str(), 
					stm.wYear, stm.wMonth, stm.wDay, stm.wHour, stm.wMinute, stm.wSecond);
			}
		}

		// check disk space
		ULARGE_INTEGER ulFreeBytes = {0} ; 
		TCHAR szDrive[4] = _T("C:\\") ;
		szDrive[0] = szFolderName[0]; 
		if (!GetDiskFreeSpaceEx(szDrive, &ulFreeBytes, NULL, NULL))
		{//api failed.
			SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_OTHERREASON, GetLocalInfo().GetTraceDownloadTool());
			nRet = IAppCallBack::EXTRACTIONFAILED ;
			break ;
		}

		if ((ulFreeBytes.QuadPart/1024/1024) < _ttoi(GetLocalInfo().GetLocalInstallSize().c_str()))
		{//disk space is not enough
			SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_PREUNZIP_NOSPACE, GetLocalInfo().GetTraceDownloadTool());
			nRet = IAppCallBack::NOTENOUGHSPACEFOREXTRACTION ;
			break ;
		}

		// start to extract package
		Log(_T("CThirdPartyApp::UnZipDownloadedPackage: %s to %s"), sZipFile.c_str(), sDstFolder.c_str()) ;
		CUnZip zipFile ;
		if (zipFile.unzip(sZipFile.c_str(), sDstFolder.c_str(), CThirdPartyApp::ZipFCallback, this))
		{
			DWORD dwError = GetLastError() ;// last error code is maintained on a per-thread basis.
			if(ERROR_FILE_NOT_FOUND == dwError)
			{// path not found
				nRet = IAppCallBack::EXTRACTPATHNOTFOUND;
				SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_NOZIPFILE, GetLocalInfo().GetTraceDownloadTool());
			}
			else if (ERROR_PATH_NOT_FOUND == dwError)
			{
				nRet = IAppCallBack::EXTRACTPATHNOTFOUND;
				SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_NOZIPFILE, GetLocalInfo().GetTraceDownloadTool());
			}
			else if (ERROR_SHARING_VIOLATION == dwError)
			{// file in use
				nRet = IAppCallBack::EXTRACTFILEINUSE ;
				SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_FILEINUSE, GetLocalInfo().GetTraceDownloadTool());
			}
			else if (ERROR_LOCK_VIOLATION == dwError)
			{// file in use
				nRet = IAppCallBack::EXTRACTFILEINUSE ;
				SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_FILEINUSE, GetLocalInfo().GetTraceDownloadTool());
			}
			else if (ERROR_DISK_FULL == dwError)
			{// disk is full
				nRet = IAppCallBack::NOTENOUGHSPACEFOREXTRACTION ;
				SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_NOSPACE, GetLocalInfo().GetTraceDownloadTool());
			}
			else if (ERROR_ACCESS_DENIED == dwError)
			{
				nRet = IAppCallBack::EXTRACTACCESSDENIED;
			}
			else
			{// extraction failed
				nRet = IAppCallBack::EXTRACTIONFAILED ;
				unsigned char MD5[16] = "";
				CUtility::GenerateMD5ForFile(sZipFile, MD5);
				_tstring sMd5= CUtility::ConvertHexToString(MD5, sizeof(MD5));
				if(GetLocalInfo().GetLocalDownFileChecksum() != sMd5)
				{// md5 incorrect

					//get zip file size
					LARGE_INTEGER nZipSize = {0};
					HANDLE hFile = CreateFile(sZipFile.c_str(), 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
					if (hFile != NULL)
					{
						GetFileSizeEx(hFile, &nZipSize) ;
						CloseHandle(hFile) ;
					}

					_tstring sComment;
					sComment.Format(4, _T("errMD5:%s, okMD5:%s, size:%I64u, lasterror:%d"), sMd5.c_str(), 
						GetLocalInfo().GetLocalDownFileChecksum().c_str(), nZipSize.QuadPart, dwError);
					SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_MD5ERROR, GetLocalInfo().GetTraceDownloadTool(), sComment);
				}
			}
			Log(_T("CUIListItem::UnZipDownloadedPackage: unzip package failed. %s. package name: %s, dst path: %s, tid:%d, err: %d."), 
				GetLocalInfo().GetLocalDownFileName().c_str(), sZipFile.c_str(), sDstFolder.c_str(), GetCurrentThreadId(), dwError) ;
			break ;
		}

		// send data to server
		g_theTracking.GameDownloaded(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_DOWNLOAD_GAME")), \
			theDataPool.GetUserName(), GetAppLanguage(), GetAppInfo().GetAbbr(), GetLocalInfo().GetTraceDownloadTool()) ;

		_tstring sCounterName = _T("gamedownload_") + GetAppName();
		g_theTracking.CounterTrackingNew(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_NEW_COUNTER")), \
			sCounterName, GetAppLanguage(), false) ;

		SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_UNZIP_FINISH, GetLocalInfo().GetTraceDownloadTool());

		// write registry items
		GetLocalInfo().SetInstallPath(sDstFolder);
		if(!WriteSilenceRegistry())
		{
			nRet = IAppCallBack::EXTRACTIONFAILED;
			SendGameInstallTrackEx(GetAppInfo().GetAbbr(), GetAppLanguage(), GAMETRACK_INSTALL_WRITEREGFAIL, GetLocalInfo().GetTraceDownloadTool());
			break ;
		}

	}while(FALSE) ;

	if (nRet != IAppCallBack::EXTRACTFILEINUSE && nRet != IAppCallBack::NOTENOUGHSPACEFOREXTRACTION
		&& nRet != IAppCallBack::EXTRACTACCESSDENIED)
	{//if zip file not in use, delete it.
		if (!DeleteFile(sZipFile.c_str()))
		{
			Sleep(1000) ;
			DeleteFile(sZipFile.c_str()) ;
		}
	}

	if (nRet != 0)
	{
		if (nRet != IAppCallBack::EXTRACTFILEINUSE && nRet != IAppCallBack::NOTENOUGHSPACEFOREXTRACTION
			&& nRet != IAppCallBack::EXTRACTACCESSDENIED)
		{//do not update game item status and clear download path if zip file is in use.
			GetLocalInfo().SetLocalStatus(CThirdPartyApp::ADDED);
			GetLocalInfo().SetDownloadPath(_T(""));
			DeleteDir(sDstFolder.c_str());
		}
	}

	return nRet ;
}

void CThirdPartyApp::DeleteDir(const _tstring& sDir)
{
	if (sDir.empty())
	{
		return ;
	}
	
	WIN32_FIND_DATA stFindFileData = {0};
	_tstring strTemp = sDir;
	if (strTemp[strTemp.size()-1] == _T('\\'))
	{
		strTemp += _T("*");
	}
	else
	{
		strTemp += _T("\\*");
	}

	HANDLE hFind = ::FindFirstFile(strTemp.c_str(), &stFindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return ;
	}

	do
	{
		strTemp.Format(2, _T("%s\\%s"), sDir.c_str(), stFindFileData.cFileName);
		if (stFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	// 目录
		{
			if (!_tcscmp(stFindFileData.cFileName, _T(".")) || !_tcscmp(stFindFileData.cFileName, _T("..")))
			{
				continue;
			}
			DeleteDir(strTemp);
		}
		else	// 文件
		{
			::DeleteFile(strTemp.c_str());
		}
	}
	while (FindNextFile(hFind, &stFindFileData));

	::FindClose(hFind);

	// 删除空目录
	::RemoveDirectory(sDir.c_str());
}

void CThirdPartyApp::ClearLocalInfo()
{
	GetLocalInfo().SetLocalStatus(CThirdPartyApp::NOTADDED);
	GetLocalInfo().SetDownTool(-1);
	GetLocalInfo().SetOrder(0);
	GetLocalInfo().SetVersion(_T(""));
	GetLocalInfo().SetEulaVersion(_T(""));
	
	SYSTEMTIME sysTime = {0};
	GetLocalInfo().SetAddedtime(sysTime);
	GetLocalInfo().SetLastplaytime(sysTime);
	GetLocalInfo().SetInstallDate(_T(""));
	GetLocalInfo().SetDownloadTime(_T(""));
	GetLocalInfo().SetDownloadPath(_T(""));
	GetLocalInfo().SetInstallPath(_T(""));
	GetLocalInfo().SetClientPath(_T(""));
	GetLocalInfo().SetClientCmd(_T(""));
	GetLocalInfo().SetLauncherPath(_T(""));
	GetLocalInfo().SetLauncherCmd(_T(""));
	GetLocalInfo().SetPatcherPath(_T(""));
	GetLocalInfo().SetPatcherCmd(_T(""));
	GetLocalInfo().SetUninstallPath(_T(""));
	GetLocalInfo().SetUninstallCmd(_T(""));

	GetLocalInfo().SetEulaVersion(_T("0"));

	GetLocalInfo().SetIsPassLanuchPoint(false);
	GetLocalInfo().SetDownloadState(APP_LOCAL_INFO::DOWNLOADING);
	GetLocalInfo().SetDownloadProgress(0);
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
}

void CThirdPartyApp::ClearDownloadInfo()
{
	GetLocalInfo().SetDownloadPath(_T(""));
	GetLocalInfo().SetDownloadTime(_T(""));
	GetLocalInfo().SetDownTool(-1);
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
}

void CThirdPartyApp::SetCurrentDownloadInfo(int nDownloadType)
{
	GetLocalInfo().SetDownTool(nDownloadType);

	if(nDownloadType == CDownloaderMgr::PDL_DOWNLOAD)
	{
		GetLocalInfo().SetLocalDownloadUrl(GetAppInfo().GetPDLDownloadURL());
		GetLocalInfo().SetLocalInstallSize(GetAppInfo().GetPDLTotalDownloadSize());
	}
	else
	{
		GetLocalInfo().SetLocalDownFileName(GetAppInfo().GetDownFileName());
		GetLocalInfo().SetLocalDownFileChecksum(GetAppInfo().GetDownFileCheckSum());
		GetLocalInfo().SetLocalDownFileSize(GetAppInfo().GetDownFileSize());
		GetLocalInfo().SetLocalInstallSize(GetAppInfo().GetInstallSize());
		GetLocalInfo().SetLocalDownloadUrl(GetAppInfo().GetDownloadHttpUrl());
	}
}

void CThirdPartyApp::ClearCurrentDownloadInfo()
{
	GetLocalInfo().SetLocalDownFileName(_T(""));
	GetLocalInfo().SetLocalDownFileChecksum(_T(""));
	GetLocalInfo().SetLocalDownFileSize(_T(""));
	GetLocalInfo().SetLocalInstallSize(_T(""));
	GetLocalInfo().SetLocalDownloadUrl(_T(""));
	GetLocalInfo().SetIsPassLanuchPoint(false);
	GetLocalInfo().SetDownloadProgress(0);
}

bool CThirdPartyApp::IsInMyGameList()
{
	return GetMemInfo().GetIsCloudGame() || (GetLocalInfo().GetIsActive() && GetLocalStatus() > CThirdPartyApp::ADDED);
}

void CThirdPartyApp::ResetNewApp()
{
	GetAppInfo().SetNewApp(0);
}

bool CThirdPartyApp::IsNewApp()
{
	return GetAppInfo().GetIsNewPopup() && GetAppInfo().GetNewApp();
}

bool CThirdPartyApp::IsSaveLocalInfo()
{
	return (GetLocalInfo().GetLocalStatus() != NOTADDED || (GetAppInfo().GetIsNewPopup() && GetAppInfo().GetNewApp()));
}

bool CThirdPartyApp::IsPlaying()
{
	if(GetMemInfo().GetCmd() == CThirdPartyApp::STARTAPP
		|| GetMemInfo().GetCmd() == CThirdPartyApp::REPAIRAPP)
	{
		return true;
	}

	return CheckIfClientRunning();
}

bool CThirdPartyApp::IsEnteredGame()
{
	___AutoLock_OpenClientList____
	for(OPENCLIENTLIST::iterator it = m_OpenClientList.begin(); it != m_OpenClientList.end(); it++)
	{
		DWORD dwTotalId = MAKETOTALID(GetAppInfo().GetIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), (*it));
		_tstring sEventName;
		sEventName.Format(2, _T("arc_%d_%u_game"), GetCurrentProcessId(), dwTotalId);
		HANDLE h = OpenEvent(EVENT_ALL_ACCESS, FALSE, sEventName.c_str());
		if(h)
		{
			CloseHandle(h);
			return true;
		}
	}
	return false;
	___AutoLockEnd____
}

bool CThirdPartyApp::IsInstalling()
{
	return (GetMemInfo().GetCmd() == CThirdPartyApp::INSTALLAPP);
}

bool CThirdPartyApp::IsUninstalling()
{
	return (GetMemInfo().GetCmd() == CThirdPartyApp::UNINSTALLAPP);
}

bool CThirdPartyApp::IsExtracting()
{
	return (GetMemInfo().GetCmd() == CThirdPartyApp::EXTRACT);
}

bool CThirdPartyApp::IsDownloading()
{
	return (GetLocalInfo().GetLocalStatus() == CThirdPartyApp::DOWNLOADING);
}

bool CThirdPartyApp::IsPreLoad()
{
	return (GetLocalInfo().GetLocalStatus() >= CThirdPartyApp::EXTRACTED && GetAppInfo().GetIsPreLoad());
}

bool CThirdPartyApp::TranslatePath(_tstring &sPath)
{
	_tstring sInstallDir = _T("%INSTALLDIR%");
	_tstring sSystemDir = _T("%SYSTEMDIR%");

	//replace %INSTALLDIR%
	_tstring sGameInstallPath = GetLocalInfo().GetInstallPath();
	sGameInstallPath = sGameInstallPath.Left(sGameInstallPath.length()-1);
	sPath.Replace(sInstallDir, sGameInstallPath);

	//replace %SYSTEMDIR%
	TCHAR sysPath[MAX_PATH] = _T("");
	if(GetSystemDirectory(sysPath, MAX_PATH) > 0)
	{
		_tstring sSystemRealPath = sysPath;
		sPath.Replace(sSystemDir, sSystemRealPath);
	}
	return true;
}

void CThirdPartyApp::HandleQueryTokenMsg(QueryTokenMsg& msg)
{
	//RZ can't login if pass param
	if((msg.GetGameId() & 0xFFFF) == 1002)
	{
		msg.SetRetValue(false);
		return;
	}

	DWORD dwInstalce = (msg.GetGameId() >> 24);
	if(dwInstalce > 1)
	{
		msg.SetRetValue(false);
		return;
	}

	if(GetAppInfo().GetBypassLogin() == 2)
	{
		msg.SetRetValue(false);
		return ;
	}
	else if(GetAppInfo().GetBypassLogin() == 1)
	{
		//do not need token if patcher support bypass
		if(g_appManager.GetCoreMsgServer().IsClientValid(msg.GetGameId()) &&
			GetLocalInfo().GetClientPath() == GetLocalInfo().GetPatcherPath())
		{
			msg.SetRetValue(false);
			return ;
		}
		g_theLoginMgr.RegisterListener(this);
		g_theLoginMgr.GetGameToken(GetAppInfo().GetAbbr());
	}
	else
	{
		_tstring sCmd;
		GetCmdlinePara(sCmd);
		core_msg_token msgToken;
		msgToken.dwCmdId = CORE_MSG_TOKEN;
		msgToken.dwRet = 0;
		msgToken.dwClientId = msg.GetGameId();
		lstrcpynA(msgToken.csGameToken, sCmd.toNarrowString().c_str(),1024);
		g_appManager.GetCoreMsgServer().SendCoreMsg(msg.GetGameId(), &msgToken);
	}

	msg.SetRetValue(true);
	return ;
}

void CThirdPartyApp::HandleQueryCookieMsg(QueryCookieMsg& msg)
{
	core_msg_cookie& hCookie = *msg.GetCoreCookieMsg();

	lstrcpyn(hCookie.csCookiePage,theUIString._GetStringFromId(_T("IDS_COOKIE_URL")).c_str(),1024);
	lstrcpyn(hCookie.csBillingPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BILLING")).c_str(),1024);
	lstrcpyn(hCookie.csGameAbbrName,GetAppInfo().GetAbbr().c_str(),1024);
	lstrcpyn(hCookie.csSupportPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SUPPORT")).c_str(),1024);
	lstrcpyn(hCookie.csMyAccountPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ACCOUNT")).c_str(),1024);
	lstrcpyn(hCookie.csWebPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str(),1024);
	
	lstrcpyn(hCookie.csNewsPage, GetAppInfo().GetNewsLink().c_str(),1024);
	if(!GetAppInfo().GetForumLink().empty())
	{
		lstrcpyn(hCookie.csForumPage,GetAppInfo().GetForumLink().c_str(),1024);
	}
	else
	{
		lstrcpyn(hCookie.csForumPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_DEFAULTFORUM")).c_str(),1024);
	}
	if (!GetAppInfo().GetMediaLink().empty())
	{
		lstrcpyn(hCookie.csMediaPage,GetAppInfo().GetMediaLink().c_str(),1024);
	}
	else
	{
		lstrcpyn(hCookie.csMediaPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str(),1024);
	}
	

	lstrcpyn(hCookie.csGameName,  GetAppInfo().GetName().c_str(),1024);
	lstrcpyn(hCookie.csBackToGame, theUIString._GetStringFromId(_T("IDS_OVERLAY_BACKTOGAME")).c_str(),1024);
	lstrcpyn(hCookie.csClose,  theUIString._GetStringFromId(_T("IDS_OVERLAY_CLOSE")).c_str(),1024);
	lstrcpyn(hCookie.csNews, theUIString._GetStringFromId(_T("IDS_OVERLAY_NEWS")).c_str(),1024);
	lstrcpyn(hCookie.csBilling,  theUIString._GetStringFromId(_T("IDS_OVERLAY_BILLING")).c_str(),1024);
	lstrcpyn(hCookie.csSupport,  theUIString._GetStringFromId(_T("IDS_OVERLAY_SUPPORT")).c_str(),1024);
	lstrcpyn(hCookie.csForum,  theUIString._GetStringFromId(_T("IDS_OVERLAY_COMMUNITY")).c_str(),1024);
	lstrcpyn(hCookie.csWeb,  theUIString._GetStringFromId(_T("IDS_OVERLAY_WEB")).c_str(),1024);
	lstrcpyn(hCookie.csMedia,  theUIString._GetStringFromId(_T("IDS_OVERLAY_MEDIA")).c_str(),1024);
	lstrcpyn(hCookie.csInvite,  theUIString._GetStringFromId(_T("IDS_OVERLAY_INVITE")).c_str(),1024);
	lstrcpyn(hCookie.csFriendSearchPage,  theUIString._GetStringFromId(_T("IDS_SEARCH_FRIENDS")).c_str(),1024);
	lstrcpyn(hCookie.csChatView,  theUIString._GetStringFromId(_T("IDS_OVERLAY_CHATVIEW")).c_str(),1024);
	lstrcpyn(hCookie.csDialogTitle, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_TITLE")).c_str(),1024);
	lstrcpyn(hCookie.csInGameTitle,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TITLE")).c_str(),1024);
//	lstrcpyn(hCookie.csText1,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TEXT1")).c_str(),1024);
	lstrcpyn(hCookie.csText2,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TEXT2")).c_str(),1024);
	lstrcpyn(hCookie.csOk, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_OK")).c_str(),1024);
	lstrcpyn(hCookie.csCancel, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_CANCEL")).c_str(),1024);
	lstrcpyn(hCookie.csUserName,  theDataPool.GetUserName().c_str(),1024);
	lstrcpyn(hCookie.csGameId,  GetAppInfo().GetStringId().c_str(),1024);
	lstrcpyn(hCookie.csAccept, theUIString._GetStringFromId(_T("IDS_DIALOG_FRIENDREQUEST_OK")).c_str(), 1024);
	lstrcpyn(hCookie.csHi, theUIString._GetStringFromId(_T("IDS_MENU_HI")).c_str(), 1024);
	
	lstrcpyn(hCookie.csIgnore, theUIString._GetStringFromId(_T("IDS_DIALOG_FRIENDREQUEST_IGNORE")).c_str(), 1024);
	lstrcpyn(hCookie.csMyFriendUrl, theUIString._GetStringFromId(_T("IDS_WEBSITE_MYFRIEND")).c_str(), 1024);
	lstrcpyn(hCookie.csAcceptText, theUIString._GetStringFromId(_T("IDS_DIALOG_FRIENDREQUEST_OPTION2")).c_str(), 1024);
	lstrcpyn(hCookie.csIgnortext, theUIString._GetStringFromId(_T("IDS_DIALOG_FRIENDREQUEST_OPTION5")).c_str(), 1024);
	lstrcpyn(hCookie.csYourFriend, theUIString._GetStringFromId(_T("IDS_NOTIFY_YOURFRIEND")).c_str(), 1024);
	lstrcpyn(hCookie.csPlayGame, theUIString._GetStringFromId(_T("IDS_NOTIFY_PLAYGAME")).c_str(), 1024);
	lstrcpyn(hCookie.csOnLine, theUIString._GetStringFromId(_T("IDS_NOTIFY_ONLINE")).c_str(), 1024);
	lstrcpyn(hCookie.csReadyPlay, theUIString._GetStringFromId(_T("IDS_NOTIFY_GAMEREADYPLAY")).c_str(), 1024);
	lstrcpyn(hCookie.csCurrentPlay, theUIString._GetStringFromId(_T("IDS_STR_CURRENTPLAY")).c_str(), 1024);

	lstrcpyn(hCookie.csMonth[0],  theUIString._GetStringFromId(_T("IDS_MONTH_S1")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[1],  theUIString._GetStringFromId(_T("IDS_MONTH_S2")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[2],  theUIString._GetStringFromId(_T("IDS_MONTH_S3")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[3],  theUIString._GetStringFromId(_T("IDS_MONTH_S4")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[4],  theUIString._GetStringFromId(_T("IDS_MONTH_S5")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[5],  theUIString._GetStringFromId(_T("IDS_MONTH_S6")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[6],  theUIString._GetStringFromId(_T("IDS_MONTH_S7")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[7],  theUIString._GetStringFromId(_T("IDS_MONTH_S8")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[8],  theUIString._GetStringFromId(_T("IDS_MONTH_S9")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[9],  theUIString._GetStringFromId(_T("IDS_MONTH_S10")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[10],  theUIString._GetStringFromId(_T("IDS_MONTH_S11")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[11],  theUIString._GetStringFromId(_T("IDS_MONTH_S12")).c_str(),1024);

	lstrcpyn(hCookie.csWeek[0],  theUIString._GetStringFromId(_T("IDS_WEEK_7")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[1],  theUIString._GetStringFromId(_T("IDS_WEEK_1")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[2],  theUIString._GetStringFromId(_T("IDS_WEEK_2")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[3],  theUIString._GetStringFromId(_T("IDS_WEEK_3")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[4],  theUIString._GetStringFromId(_T("IDS_WEEK_4")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[5],  theUIString._GetStringFromId(_T("IDS_WEEK_5")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[6],  theUIString._GetStringFromId(_T("IDS_WEEK_6")).c_str(),1024);
	_tstring sUserIcon = _T("") ;
	if (!g_theSNSManager.GetUserCharData()._szHeaderPath.empty())
	{
		sUserIcon = _T("\\") + g_theSNSManager.GetUserCharData()._szHeaderPath ;
		int nPos = sUserIcon.find_last_of('.') ;
		_tstring sExt = &sUserIcon[nPos] ;
		lstrcpyn(hCookie.csUserIcon,sUserIcon.c_str(),1024);
	}

	lstrcpyn(hCookie.csSessionId, g_theSNSManager.GetSessionId().toWideString().c_str(),1024);

	g_appManager.GetCoreMsgServer().SendCoreMsg(hCookie.dwClientId, &hCookie);

	// enter the pdl game playing
	if (GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD && IsDownloading() && IsPlaying())
	{
		core_msg_first_fixed_notification notify;
		_tstring szText = theUIString._GetStringFromId(_T("IDS_NOTIFY_PDL_GAME_ENTER_TIP"));
		_tcscpy_s(notify.szText, sizeof(notify.szText), szText.c_str());

		g_appManager.GetCoreMsgServer().SendCoreMsg(hCookie.dwClientId, &notify);
	}

	msg.SetRetValue(true);
}

bool CThirdPartyApp::GetCmdlinePara(_tstring &sCmd, HANDLE hToken)
{
	_tstring sLanguage = theDataPool.GetLangString() ;

	if(GetAppInfo().GetBypassLogin() == 1)
	{//fix bug: bypass failed for PWE games when legacy user logged in with email address and the email address has not been changed before. 
	 //reason: MD5(email+password) is not saved in database. 
	 //temporary solution: use username for bypass login for legacy user(new username starts with "#1#").
		_tstring sAccount = (theDataPool.GetUserName().Find(_T("#1#"))==_tstring::npos)?theDataPool.GetUserName():theDataPool.GetUserInput() ;
		sCmd = _T("user:") + sAccount + _T(" token:") + theDataPool.GetGameToken(GetAppInfo().GetAbbr()) + 
			_T(" coreclient:1 arc:1");
	}
	else if(GetAppInfo().GetBypassLogin() == 2)
	{//fix bug: bypass failed for cryptic games when legacy user logged in with email address. 
	 //reason: MD5(email+password) is not saved in database. 
	 //temporary solution: use username for bypass login for legacy user(new username starts with "#1#").
		_tstring sAccount = (theDataPool.GetUserName().Find(_T("#1#"))==_tstring::npos)?theDataPool.GetUserName():theDataPool.GetUserInput() ;
		//sCmd.Format(3, _T("-CoreLocale %s -Bypass %s %p"), theDataPool.GetLangString().c_str(), sAccount.c_str(), hToken);
		sCmd.Format(2, _T("-Bypass %s %p"), sAccount.c_str(), hToken);
	}
	else
	{
		sCmd = _T("coreclient:1 arc:1");
	}

	if(!GetLocalInfo().GetQACmd().empty())
	{
		sCmd += _T(" ");
		sCmd += GetLocalInfo().GetQACmd();
	}

	return true;
}

bool CThirdPartyApp::IsFullScreen()
{
	___AutoLock_OpenClientList____
	for(OPENCLIENTLIST::iterator it = m_OpenClientList.begin(); it != m_OpenClientList.end(); it++)
	{
		DWORD dwTotalId = MAKETOTALID(GetAppInfo().GetIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), (*it));

		core_msg_check_game_windowed hCheckWindowed;
		hCheckWindowed.dwCmdId = CORE_MSG_CHECK_GAME_WINDOWED;
		hCheckWindowed.dwClientId = dwTotalId;
		if(g_appManager.GetCoreMsgServer().SendCoreMsg(dwTotalId, &hCheckWindowed))
		{
			if(!hCheckWindowed.dwRet)
			{
				return true;
			}
		}
	}
	return false;
	___AutoLockEnd____
}

bool CThirdPartyApp::IsWindowed()
{
	return !IsFullScreen();
}

void SendEtractMessageToUIApp(INT32 nProgress, LPVOID lpCaller)
{
	CThirdPartyApp* pApp = (CThirdPartyApp*)lpCaller ;
	if (!pApp)
	{
		return ;
	}
	g_appManager.PostMessageToUIApp(pApp,UIMessageEtractNotify(nProgress));

}

void CThirdPartyApp::ZipFCallback(INT32 nProgress, LPVOID lpCaller)
{
	__try{
		SendEtractMessageToUIApp(nProgress, lpCaller);
	}
	__finally{

	}
}

void CThirdPartyApp::OnDownloading()
{
	if (GetLocalInfo().GetLocalStatus() <= CThirdPartyApp::DOWNLOADING)
	{
		GetLocalInfo().SetLocalStatus(CThirdPartyApp::DOWNLOADING);
		GetMemInfo().SetIsPaused(false);
		if(GetLocalInfo().GetDownloadTime().empty())
		{
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);
			unsigned int ntime = sysTime.wMonth*100000000 + sysTime.wDay*1000000 + 
				sysTime.wHour*10000 + sysTime.wMinute*100 + sysTime.wSecond ;
			_tstring downloadTime;
			downloadTime.Format(1, _T("%u"), ntime);
			GetLocalInfo().SetDownloadTime(downloadTime); 
		}
	}
}

bool CThirdPartyApp::OnStartBegin(_tstring sId)
{
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(StartBegin));
	return true;
}

bool CThirdPartyApp::IsInstalled()
{
	return GetLocalInfo().GetLocalStatus() >= EXTRACTED;
}

bool CThirdPartyApp::OnStartEnd(_tstring sId)
{
	OnDownloading();
	
	GetLocalInfo().SetDownloadState(APP_LOCAL_INFO::DOWNLOADING);
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(StartEnd));
	return true;
}

bool CThirdPartyApp::OnPauseBegin(_tstring sId)
{
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(PauseBegin));
	return true;
}

bool CThirdPartyApp::OnPauseEnd(_tstring sId) 
{
	GetMemInfo().SetIsPaused(true);
	GetLocalInfo().SetDownloadState(APP_LOCAL_INFO::PAUSED);
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(PauseEnd));
	return true;
}

bool CThirdPartyApp::OnResumeBegin(_tstring sId)
{
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(ResumeBegin));
	return true;
}

bool CThirdPartyApp::OnResumeEnd(_tstring sId) 
{
	OnDownloading();

	GetLocalInfo().SetDownloadState(APP_LOCAL_INFO::DOWNLOADING);
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(ResumeEnd));
	return true;
}

bool CThirdPartyApp::OnCancelBegin(_tstring sId)
{
	if (GetLocalInfo().GetIsPassLanuchPoint() 
		&& GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD)
	{
		UnstallPDLApp();
	}

	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(CancelBegin));
	return true;
}

bool CThirdPartyApp::ClearDownloadInfoBan()
{
	ClearDownloadInfo();
	ClearCurrentDownloadInfo();

	GetLocalInfo().SetLocalStatus(CThirdPartyApp::ADDED);
	GetMemInfo().SetIsPaused(false);

	GetLocalInfo().SetIsPassLanuchPoint(false);
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
	return true;
}

bool CThirdPartyApp::OnCancelEnd(_tstring sId)
{
	ClearDownloadInfo();
	ClearCurrentDownloadInfo();

	GetLocalInfo().SetLocalStatus(CThirdPartyApp::ADDED);
	GetMemInfo().SetIsPaused(false);

	GetLocalInfo().SetIsPassLanuchPoint(false);
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
	_tstring sAbbrLang = MakeAbbrLangId(GetAppArrb(), GetAppLanguage());
	g_appManager.DelAutoResumeItem(sAbbrLang);
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(CancelEnd));
	return true;
}

bool CThirdPartyApp::OnComplete(_tstring sId)
{
	g_appManager.ClearFirstPlayingCacheById(GetAppArrb(), GetAppLanguage());
	_tstring sAbbrLang = MakeAbbrLangId(GetAppArrb(), GetAppLanguage());
	g_appManager.DelAutoResumeItem(sAbbrLang);
	if (g_appManager.GetGameInPDLDownloadingStatus() == static_cast<IThirdApp*>(this))
	{
		g_appManager.SetGameInPDLDownloadingStatus(NULL);
	}
	GetLocalInfo().SetIsPassLanuchPoint(false);
		
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;
	SetPDLUncheckRegFlag(sPath, GetAppStrId(), false);

	if (GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD)
	{
		g_theTracking.GameDownloaded(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_DOWNLOAD_GAME")), \
			theDataPool.GetUserName(), GetAppLanguage(), GetAppInfo().GetAbbr(), GetLocalInfo().GetTraceDownloadTool()) ;

		_tstring sCounterName = _T("gamedownload_") + GetAppName();
		g_theTracking.CounterTrackingNew(theUIString._GetStringFromId(_T("IDS_CONFIG_TRACK_NEW_COUNTER")), \
			sCounterName, GetAppLanguage(), false) ;
	}

	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(FullComplete));
	return true;
}

bool  CThirdPartyApp::OnLauchPoint(_tstring sId)
{
	g_appManager.KeepFirstPlayingCacheById(GetAppArrb(), GetAppLanguage()); // because this is relative with user account , so the flag will be keep always
	if (!GetLocalInfo().GetIsPassLanuchPoint())
	{
		GetLocalInfo().SetIsPassLanuchPoint(true);
		ExtratPdlApp();
		SendGameInstallTrackEx(GetAppArrb(), GetAppLanguage(), GAMETRACE_PDL_PASS_LANCHPOINT, GetLocalInfo().GetDownTool());

		g_appManager.PostMessageToUIApp(this, UIMessageLaunchGameNotify());
	}

	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(PassLauchPoint));
	return true;
}

bool CThirdPartyApp::UpdateStatus(_tstring sId, DOWNLOAD_PARAM *pDownloadParam)
{
	int nProgress = pDownloadParam->nDownloadPercent;
	GetLocalInfo().SetDownloadProgress(nProgress);
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadUpdateProgressNotify(pDownloadParam));
	return true;
}
bool CThirdPartyApp::OnError(_tstring sId, int nErrType, _tstring sComment)
{
	switch(nErrType)
	{
	case IDownloader::NOTENOUGHSPACE:
		GetMemInfo().SetIsPaused(true);
		break;

	}
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadError(nErrType,sComment));
	return true;
}

bool CThirdPartyApp::OnTraceMessage(DWORD errorCode, const _tstring& comment)
{
	SendGameInstallTrackEx(GetAppArrb(), GetAppLanguage(), errorCode, GetLocalInfo().GetDownTool(), comment);
	
	return true;
}

bool CThirdPartyApp::OnPlayPDLGame(const _tstring& szId)
{
	g_appManager.PostMessageToUIApp(this, UIMessageDownloadNotify(PlayPDLGame));

	return true;
}

const _tstring CThirdPartyApp::GetAppLogoFile()
{
	_tstring sLogoPath = theDataPool.GetBaseDir();
	sLogoPath += FILEPATH_UI;
	sLogoPath += GetAppInfo().GetLogoFile();
	return sLogoPath; 
}

const _tstring  CThirdPartyApp::GetAppInstallPicFile()
{
	_tstring sBgPath = theDataPool.GetBaseDir();
	sBgPath += _T("resources\\install_pics\\");
	sBgPath += GetAppInfo().GetInstallPicFile();
	return sBgPath; 
}

const _tstring CThirdPartyApp::GetAppBkFile()
{
	_tstring sPicFilePath = theDataPool.GetBaseDir();
	sPicFilePath += _T("resources\\image\\");
	sPicFilePath += GetAppInfo().GetBgFile();
	return sPicFilePath; 
}

bool CThirdPartyApp::LoadAppInfo(TiXmlElement* pInfo,bool bUpdate)
{
	const char *ptmp = NULL;
	TiXmlElement *pSub = NULL;

	ptmp = pInfo->Attribute("id");
	if(!ptmp)
	{
		return false;
	}
	GetAppInfo().SetStringId(ptmp);
	GetAppInfo().SetIntId(atoi(ptmp));

	ptmp = pInfo->Attribute("abbr");
	if(!ptmp)
	{
		return false;
	}
	_tstring sAbbr = ptmp;
	sAbbr.MakeLower();
	GetAppInfo().SetAbbr(sAbbr);

	ptmp = pInfo->Attribute("name");
	if(ptmp)
	{
		_tstring sTempName;
		sTempName.fromUTF8(ptmp);
		GetAppInfo().SetName(sTempName);
	}

	ptmp = pInfo->Attribute("link_name");
	if(ptmp)
	{
		GetAppInfo().SetLinkName(ptmp);
	}

	ptmp = pInfo->Attribute("lang");
	if(ptmp)
	{
		_tstring sLang = ptmp;
		sLang.MakeLower();
		GetAppInfo().SetLanguage(sLang);
	}
	ptmp = pInfo->Attribute("bypass");
	if(ptmp)
	{
		GetAppInfo().SetBypassLogin(atoi(ptmp));
	}
	ptmp = pInfo->Attribute("overlay");
	if(ptmp)
	{
		GetAppInfo().SetIsSupportOverlay(atoi(ptmp));
	}
	ptmp = pInfo->Attribute("repair");
	if(ptmp)
	{
		GetAppInfo().SetIsSupportRepair(atoi(ptmp));
	}
	ptmp = pInfo->Attribute("status");
	if(ptmp)
	{
		GetAppInfo().SetBaseStatus(GAME_STATUS::LIVE);
		//m_AppInfo.nStatus = atoi(ptmp);    //delete by sw temporary 
	}
	ptmp = pInfo->Attribute("exe");
	if(ptmp)
	{
		GetAppInfo().SetExeName(ptmp);
	}

	pSub = pInfo->FirstChildElement("news");
	if(pSub)
	{
		GetAppInfo().SetNewsLink(pSub->GetText());
	}

	pSub = pInfo->FirstChildElement("prod");
	if (pSub)
	{
		GetAppInfo().SetProductUrl(pSub->GetText());
	}

	pSub = pInfo->FirstChildElement("info");
	if(pSub)
	{
		GetAppInfo().SetInfoLink(pSub->GetText());
	}

	pSub = pInfo->FirstChildElement("infopopup");
	if(pSub)
	{
		GetAppInfo().SetPopupInfoLink(pSub->GetText());
	}

	pSub = pInfo->FirstChildElement("forum");
	if(pSub)
	{
		GetAppInfo().SetForumLink(pSub->GetText());
	}
	pSub = pInfo->FirstChildElement("media");
	if(pSub)
	{
		GetAppInfo().SetMediaLink(pSub->GetText());
	}
	pSub = pInfo->FirstChildElement("logo");
	if(pSub)
	{
		GetAppInfo().SetLogoLink(pSub->GetText());
		int nPos = GetAppInfo().GetLogoLink().ReverseFind(_T('/'));
		int nLength = GetAppInfo().GetLogoLink().length();
		GetAppInfo().SetLogoFile(GetAppInfo().GetLogoLink().Right(nLength - nPos - 1));
	}

	pSub = pInfo->FirstChildElement("bg");
	if(pSub)
	{
		// add by sw
		ptmp = pSub->Attribute("image");
		if(ptmp)
		{
			GetAppInfo().SetBgLink(ptmp);
			int nPos = GetAppInfo().GetBgLink().ReverseFind(_T('/'));
			int nLength = GetAppInfo().GetBgLink().length();
			GetAppInfo().SetBgFile(GetAppInfo().GetBgLink().Right(nLength - nPos - 1));
		}

		if(GetAppInfo().GetBgFile().empty())
		{
			GetAppInfo().SetBgFile(_T("defaultbg.jpg"));
		}
	}

	pSub = pInfo->FirstChildElement("install_image");
	if(pSub)
	{
		ptmp = pSub->GetText();
		if(ptmp)
		{
			GetAppInfo().SetInstallPicLink(ptmp);
			int nPos = GetAppInfo().GetInstallPicLink().ReverseFind(_T('/'));
			int nLength = GetAppInfo().GetInstallPicLink().length();
			GetAppInfo().SetInstallPicFile(GetAppInfo().GetInstallPicLink().Right(nLength - nPos - 1));
		}

		if(GetAppInfo().GetInstallPicFile().empty())
		{
			GetAppInfo().SetInstallPicFile(_T("default_pic.jpg"));
		}
	}

	if(pSub = pInfo->FirstChildElement("installpack"))
	{
		ptmp = pSub->Attribute("name");
		if(ptmp)
		{
			GetAppInfo().SetDownFileName(ptmp);
		}
		ptmp = pSub->Attribute("checksum");
		if(ptmp)
		{
			GetAppInfo().SetDownFileCheckSum(ptmp);
		}
		ptmp = pSub->Attribute("http");
		if(ptmp)
		{
			GetAppInfo().SetDownloadHttpUrl(ptmp);
		}
		ptmp = pSub->Attribute("size");
		if(ptmp)
		{
			GetAppInfo().SetDownFileSize(ptmp);
		}
		if(ptmp = pSub->Attribute("installsize"))
		{
			GetAppInfo().SetInstallSize(ptmp);
		}
	}

	if(pSub = pInfo->FirstChildElement("registry"))
	{
		GetAppInfo().SetRegistry(pSub->GetText());
	}

	if(pSub = pInfo->FirstChildElement("new"))
	{
		ptmp = pSub->Attribute("popup");
		if(ptmp)
		{
			GetAppInfo().SetIsNewPopup(atoi(ptmp));
			GetAppInfo().SetNewApp(atoi(ptmp));
		}
	}

	if (pSub = pInfo->FirstChildElement("pdl"))
	{
		ptmp = pSub->Attribute("support");
		if (ptmp)
		{
			int nSuportPDL = atoi(ptmp);
			GetAppInfo().SetIsSupportPDL(nSuportPDL != 0? true : false);
		}

		ptmp = pSub->Attribute("disable");
		if (ptmp)
		{
			int nDisable = atoi(ptmp);
			GetAppInfo().SetIsEnablePDL(nDisable != 0? false : true);
		}
		
		ptmp = pSub->Attribute("url");
		if (ptmp)
		{
			GetAppInfo().SetPDLDownloadURL(ptmp);
		}
		
		ptmp = pSub->Attribute("size");
		if (ptmp)
		{
			GetAppInfo().SetPDLTotalDownloadSize(ptmp);
		}
	}

	if (pSub = pInfo->FirstChildElement("play"))
	{

		ptmp = pSub->Attribute("disable");
		if (ptmp)
		{
			int nDisable = atoi(ptmp);
			GetAppInfo().SetIsPreLoad(nDisable != 0? true : false);
		}

		ptmp = pSub->Attribute("notice");
		if (ptmp)
		{
			_tstring sTmp;
			sTmp.fromUTF8(ptmp);
			GetAppInfo().SetPreLoadNotice(sTmp);
		}
	}

	if (bUpdate)
	{
		if(GetLocalInfo().GetLocalStatus() < CThirdPartyApp::EXTRACTED)
		{
			CheckIfInstalled();
		}

		CheckNewApp();
	}

	DownloadInstallPic();

	return true;
}

bool CThirdPartyApp::LoadLocalInfo(TiXmlElement* pInfo)
{
	const char *ptmp = NULL;
	TiXmlElement *pSub = NULL;

	//force save abbr to local gameinfo
	ptmp = pInfo->Attribute("abbr");
	if(!ptmp)
	{
		g_appManager.SetModified();
	}

	ptmp = pInfo->Attribute("status");
	if(ptmp)
	{
		GetLocalInfo().SetLocalStatus(atoi(ptmp));
	}

	ptmp = pInfo->Attribute("active");
	if(ptmp)
	{
		SetActive(atoi(ptmp));
	}

	ptmp = pInfo->Attribute("downtool");
	if(ptmp)
	{
		GetLocalInfo().SetDownTool(atoi(ptmp));
	}

	GetLocalInfo().SetVersion(pInfo->Attribute("version"));

	ptmp = pInfo->Attribute("traceDownloadTool");
	if(ptmp)
	{
		GetLocalInfo().SetTraceDownloadTool(atoi(ptmp));

		if (GetLocalInfo().GetTraceDownloadTool() < 0)
		{
			GetLocalInfo().SetTraceDownloadTool(GetLocalInfo().GetDownTool());
		}
	}

	ptmp = pInfo->Attribute("eulaversion");
	if(ptmp)
	{
		GetLocalInfo().SetEulaVersion(ptmp);
	}

	ptmp = pInfo->Attribute("installdate");
	if(ptmp)
	{
		GetLocalInfo().SetInstallDate(ptmp);
	}

	ptmp = pInfo->Attribute("downtime");
	if(ptmp)
	{
		GetLocalInfo().SetDownloadTime(ptmp);
	}

	// new game flag
	ptmp = pInfo->Attribute("new");
	if(ptmp && GetAppInfo().GetNewApp() == 1)
	{
		int nNew = atoi(ptmp);
		if(nNew != 1)
		{
			nNew = 0;
		}
		GetAppInfo().SetNewApp(nNew);
	}

	// order
	ptmp = pInfo->Attribute("order");
	if(ptmp)
	{
		GetLocalInfo().SetOrder(atoi(ptmp));
	}

	ptmp = pInfo->Attribute("IsPassLanuchPoint");
	if (ptmp)
	{
		GetLocalInfo().SetIsPassLanuchPoint(atoi(ptmp));
	}

	ptmp = pInfo->Attribute("qacmd");
	if(ptmp)
	{
		_tstring sTempCmd;
		sTempCmd.fromUTF8(ptmp);
		GetLocalInfo().SetQACmd(sTempCmd);
	}

	ptmp = pInfo->Attribute("favorite");
	if(ptmp)
	{
		GetMemInfo().SetIsFavorite(atoi(ptmp));
	}

	ptmp = pInfo->Attribute("launchgame");
	if(ptmp)
	{
		GetLocalInfo().SetIsLaunchAfterInstall(atoi(ptmp));
	}
	ptmp = pInfo->Attribute("createshortcut");
	if(ptmp)
	{
		GetLocalInfo().SetIsLaunchAfterInstall(atoi(ptmp));
	}

	SYSTEMTIME t;
	if(pSub = pInfo->FirstChildElement("addedtime"))
	{
		t.wYear = atoi(pSub->Attribute("year"));
		t.wMonth = atoi(pSub->Attribute("month"));
		t.wDay = atoi(pSub->Attribute("day"));
		t.wHour = atoi(pSub->Attribute("hour"));
		t.wMinute = atoi(pSub->Attribute("minute"));
		t.wSecond = atoi(pSub->Attribute("second"));
		GetLocalInfo().SetAddedtime(t);
	}

	if(pSub = pInfo->FirstChildElement("lastplaytime"))
	{
		t.wYear = atoi(pSub->Attribute("year"));
		t.wMonth = atoi(pSub->Attribute("month"));
		t.wDay = atoi(pSub->Attribute("day"));
		t.wHour = atoi(pSub->Attribute("hour"));
		t.wMinute = atoi(pSub->Attribute("minute"));
		t.wSecond = atoi(pSub->Attribute("second"));
		GetLocalInfo().SetLastplaytime(t);
	}

	if(pSub = pInfo->FirstChildElement("download"))
	{
		const char *pName, *pCheckSum, *pUrl, *pSize, *pInstallSize;
		pName = pSub->Attribute("name");
		pCheckSum = pSub->Attribute("md5");
		pUrl = pSub->Attribute("url");
		pSize = pSub->Attribute("size");
		pInstallSize = pSub->Attribute("installsize");
		if(pName && pCheckSum && pUrl && pSize && pInstallSize)
		{
			GetLocalInfo().SetLocalDownFileName(pName);
			GetLocalInfo().SetLocalDownFileChecksum(pCheckSum);
			GetLocalInfo().SetLocalDownloadUrl(pUrl);
			GetLocalInfo().SetLocalDownFileSize(pSize);
			GetLocalInfo().SetLocalInstallSize(pInstallSize);
		}
		else if (pUrl && pInstallSize)
		{
			GetLocalInfo().SetLocalDownFileName(_T(""));
			GetLocalInfo().SetLocalDownFileChecksum(_T(""));
			GetLocalInfo().SetLocalDownloadUrl(pUrl);
			GetLocalInfo().SetLocalDownFileSize(_T(""));
			GetLocalInfo().SetLocalInstallSize(pInstallSize);
		}

		int nDownloadState = -1;
		if(TIXML_SUCCESS != pSub->QueryIntAttribute("downloadstate", &nDownloadState))
		{
			GetLocalInfo().SetDownloadState(APP_LOCAL_INFO::INVALIDE_DATA);
		}
		else
		{
			GetLocalInfo().SetDownloadState((APP_LOCAL_INFO::DOWNLOAD_STATE)nDownloadState);
		}

		int nProgress = 0;
		pSub->QueryIntAttribute("downloadprogress", &nProgress);
		GetLocalInfo().SetDownloadProgress(nProgress);
	}

	if(pSub = pInfo->FirstChildElement("path"))
	{
		GetLocalInfo().SetInstallPath(pSub->Attribute("installpath"));
		GetLocalInfo().SetDownloadPath(pSub->Attribute("downloadpath"));
		GetLocalInfo().SetClientPath(pSub->Attribute("clientpath"));
		GetLocalInfo().SetClientCmd(pSub->Attribute("clientcmd"));
		GetLocalInfo().SetLauncherPath(pSub->Attribute("launcherpath"));
		GetLocalInfo().SetLauncherCmd(pSub->Attribute("launchercmd"));
		GetLocalInfo().SetPatcherPath(pSub->Attribute("patcherpath"));
		GetLocalInfo().SetPatcherCmd(pSub->Attribute("patchercmd"));
	}

	//check and download game icon and background
	if(GetLocalInfo().GetLocalStatus() >= CThirdPartyApp::ADDED)
	{
		DownloadIcon();
		DownloadBkgnd();
	}

	return true;
}

bool CThirdPartyApp::SaveLocalInfoToXmlNode(TiXmlElement* pRoot)
{
	char szBuf[128];
	char abbr[1024] = "";

	pRoot->SetAttribute("type", itoa(GetType(), szBuf, 10));
	WideCharToMultiByte( CP_UTF8, 0, GetAppInfo().GetAbbr().c_str(), -1, abbr, 1024, NULL, NULL );

	pRoot->SetAttribute("abbr", abbr);

	pRoot->SetAttribute("id", GetAppInfo().GetStringId().toNarrowString().c_str());
	pRoot->SetAttribute("abbr", GetAppArrb().toNarrowString().c_str());
	pRoot->SetAttribute("lang", GetAppLanguage().toNarrowString().c_str());
	pRoot->SetAttribute("status", itoa(GetLocalInfo().GetLocalStatus(), szBuf, 10));
	pRoot->SetAttribute("active", itoa(GetLocalInfo().GetIsActive(), szBuf, 10));
	pRoot->SetAttribute("downtool", itoa(GetLocalInfo().GetDownTool(), szBuf, 10));
	pRoot->SetAttribute("traceDownloadTool", itoa(GetLocalInfo().GetTraceDownloadTool(), szBuf, 10));
	pRoot->SetAttribute("version", GetLocalInfo().GetVersion().toNarrowString().c_str());
	pRoot->SetAttribute("eulaversion", GetLocalInfo().GetEulaVersion().toNarrowString().c_str());
	pRoot->SetAttribute("installdate", GetLocalInfo().GetInstallDate().toNarrowString().c_str());
	pRoot->SetAttribute("downtime", GetLocalInfo().GetDownloadTime().toNarrowString().c_str());
	if(GetAppInfo().GetIsNewPopup() && GetAppInfo().GetNewApp() == 0)
	{
		pRoot->SetAttribute("new", itoa(GetAppInfo().GetNewApp(), szBuf, 10));
	}
	pRoot->SetAttribute("order", itoa(GetLocalInfo().GetOrder(), szBuf, 10));
	pRoot->SetAttribute("IsPassLanuchPoint", itoa(GetLocalInfo().GetIsPassLanuchPoint(), szBuf, 10));
	if(GetLocalInfo().GetQACmd().length())
	{
		pRoot->SetAttribute("qacmd", GetLocalInfo().GetQACmd().toUTF8().c_str());
	}
	pRoot->SetAttribute("favorite", itoa(GetMemInfo().GetIsFavorite(), szBuf, 10));
	if(GetLocalInfo().GetIsLaunchAfterInstall())
	{
		pRoot->SetAttribute("launchgame", "1");
	}
	if(GetLocalInfo().GetIsCreateShortcut())
	{
		pRoot->SetAttribute("createshortcut", "1");
	}

	SYSTEMTIME t;
	// added time
	t = GetLocalInfo().GetAddedtime();
	TiXmlElement* pSub = new TiXmlElement("addedtime");
	pSub->SetAttribute("year", itoa(t.wYear, szBuf, 10));
	pSub->SetAttribute("month", itoa(t.wMonth, szBuf, 10));
	pSub->SetAttribute("day", itoa(t.wDay, szBuf, 10));
	pSub->SetAttribute("hour", itoa(t.wHour, szBuf, 10));
	pSub->SetAttribute("minute", itoa(t.wMinute, szBuf, 10));
	pSub->SetAttribute("second", itoa(t.wSecond, szBuf, 10));
	pRoot->LinkEndChild(pSub);

	// last play time
	t =  GetLocalInfo().GetLastplaytime();
	pSub = new TiXmlElement("lastplaytime");
	pSub->SetAttribute("year", itoa(t.wYear, szBuf, 10));
	pSub->SetAttribute("month", itoa(t.wMonth, szBuf, 10));
	pSub->SetAttribute("day", itoa(t.wDay, szBuf, 10));
	pSub->SetAttribute("hour", itoa(t.wHour, szBuf, 10));
	pSub->SetAttribute("minute", itoa(t.wMinute, szBuf, 10));
	pSub->SetAttribute("second", itoa(t.wSecond, szBuf, 10));
	pRoot->LinkEndChild(pSub);

	if(GetLocalInfo().GetLocalStatus() == CThirdPartyApp::DOWNLOADING || GetLocalInfo().GetLocalStatus() == CThirdPartyApp::DOWNLOADED)
	{
		if(GetLocalInfo().GetDownTool() != CDownloaderMgr::PDL_DOWNLOAD 
			&& !GetLocalInfo().GetLocalDownFileName().empty() && !GetLocalInfo().GetLocalDownFileChecksum().empty()
			&& !GetLocalInfo().GetLocalDownloadUrl().empty() && !GetLocalInfo().GetLocalDownFileSize().empty()
			&& !GetLocalInfo().GetLocalInstallSize().empty())
		{
			pSub = new TiXmlElement("download");
			pSub->SetAttribute("name", GetLocalInfo().GetLocalDownFileName().toNarrowString().c_str());
			pSub->SetAttribute("md5", GetLocalInfo().GetLocalDownFileChecksum().toNarrowString().c_str());
			pSub->SetAttribute("url", GetLocalInfo().GetLocalDownloadUrl().toNarrowString().c_str());
			pSub->SetAttribute("size", GetLocalInfo().GetLocalDownFileSize().toNarrowString().c_str());
			pSub->SetAttribute("installsize", GetLocalInfo().GetLocalInstallSize().toNarrowString().c_str());
			pSub->SetAttribute("downloadstate", (int)GetLocalInfo().GetDownloadState());
			pRoot->LinkEndChild(pSub);
		}
		else if(GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD 
			&& !GetLocalInfo().GetLocalDownloadUrl().empty() 
			&& !GetLocalInfo().GetLocalInstallSize().empty()
			)
		{
			pSub = new TiXmlElement("download");
			pSub->SetAttribute("url", GetLocalInfo().GetLocalDownloadUrl().toNarrowString().c_str());
			pSub->SetAttribute("installsize", GetLocalInfo().GetLocalInstallSize().toNarrowString().c_str());
			pSub->SetAttribute("downloadstate", (int)GetLocalInfo().GetDownloadState());
			pSub->SetAttribute("downloadprogress", GetLocalInfo().GetDownloadProgress());
			
			pRoot->LinkEndChild(pSub);
		}
	}

	// path
	pSub = new TiXmlElement("path");
	pSub->SetAttribute("installpath", GetLocalInfo().GetInstallPath().toNarrowString().c_str());
	pSub->SetAttribute("downloadpath", GetLocalInfo().GetDownloadPath().toNarrowString().c_str());
	pSub->SetAttribute("clientpath",GetLocalInfo().GetClientPath().toNarrowString().c_str());
	pSub->SetAttribute("clientcmd", GetLocalInfo().GetClientCmd().toNarrowString().c_str());
	pSub->SetAttribute("launcherpath", GetLocalInfo().GetLauncherPath().toNarrowString().c_str());
	pSub->SetAttribute("launchercmd", GetLocalInfo().GetLauncherCmd().toNarrowString().c_str());
	pSub->SetAttribute("patcherpath", GetLocalInfo().GetPatcherPath().toNarrowString().c_str());
	pSub->SetAttribute("patchercmd", GetLocalInfo().GetPatcherCmd().toNarrowString().c_str());
	pRoot->LinkEndChild(pSub);

	return true;
}

bool CThirdPartyApp::SaveStateToJson(json::Object& GameStatus)
{
	GameStatus[JSON_ATTRIBUTE_GAME_ABBR]  = json::String(GetAppInfo().GetAbbr().MakeLowerConst().toNarrowString().c_str()) ;
	GameStatus[JSON_ATTRIBUTE_GAME_FLAG]  = json::String(_tstring(GetAppInfo().GetBaseStatus()).toNarrowString().c_str()) ;

	//judge this flag to show or hide new logo
	int nNew = 0;

	GameStatus[JSON_ATTRIBUTE_GAME_NEW] = json::String(_tstring(nNew).toNarrowString().c_str());

	int nStatus2 ;
	if(GetLocalInfo().GetLocalStatus() == CThirdPartyApp::NOTADDED)
	{
		nStatus2 = ALLGame_STATUS::ALLGAME_ADD;
	}
	else if(GetLocalInfo().GetLocalStatus() < CThirdPartyApp::EXTRACTED)
	{
		nStatus2 = ALLGame_STATUS::ALLGAME_INSTALL;
	}
	else
	{
		nStatus2 = ALLGame_STATUS::ALLGAME_PLAY;
	}

	GameStatus[JSON_ATTRIBUTE_GAME_STATUS] = json::String(_tstring(nStatus2).toNarrowString().c_str()) ; 

	return true;

}

bool CThirdPartyApp::HandleIsUrlGameInfoMsg(QueryIsUrlInfoMsg& msg)
{
	_tstring url = msg.GetUrl();
	if(!GetAppInfo().GetInfoLink().empty() && 0 == url.find(GetAppInfo().GetInfoLink().c_str()))
	{
		msg.SetRetValue(true);
		return true;
	}
	else if(!GetAppInfo().GetPopupInfoLink().empty() && 0 == url.find(GetAppInfo().GetPopupInfoLink().c_str()))
	{
		msg.SetRetValue(true);
		return true;
	}

	return false;
}

bool CThirdPartyApp::HandleUrlRelativeAppMsg(QueryUrlRelatedAppMsg& msg)
{
	if(GetAppInfo().GetLinkName().empty())
	{
		return false;
	}
	_tstring url = msg.GetUrl();
	_tstring sLinkName = _T("/") + GetAppInfo().GetLinkName();
	_tstring sLinkNameEx = _T("/") + GetAppInfo().GetLinkName() + _T("/");
	_tstring sLang = _T("/") + GetAppLanguage() + _T("/");
	if(url.length() <= sLinkName.length())
	{
		return false;
	}
	
	if((sLinkName.CompareNoCase(url.Right(sLinkName.length())) == 0 || url.Find(sLinkNameEx) != _tstring::npos) && url.Find(sLang) != _tstring::npos)
	{
		DownloadBkgnd();
		msg.SetRetValue(static_cast<IThirdApp*>(this));
		return true;
	}

	return false;
}

bool CThirdPartyApp::HandleSendArcMessageToGameMsg(SendArcMessageToGameMsg& msg)
{
	core_msg_header *hMsg = msg.GetArcMsg();
	DWORD *pTotalId = NULL;

	___AutoLock_OpenClientList____
	switch(hMsg->dwCmdId)
	{
	case CORE_MSG_NOTIFICATION:
		{
			core_msg_notification *pcoremsg = (core_msg_notification*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_INBOX:
		{
			core_msg_inbox *pcoremsg = (core_msg_inbox*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_FRIENDREQ:
		{
			core_msg_friendreq *pcoremsg = (core_msg_friendreq*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_FRSTATUS:
		{
			core_msg_FRStatus *pcoremsg = (core_msg_FRStatus*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_FR_RESULT:
		{
			core_msg_FR_result *pcoremsg = (core_msg_FR_result*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_READY_PLAY:
		{
			core_msg_ready_play *pcoremsg = (core_msg_ready_play*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	case CORE_MSG_BROWSER_SETCOOOKIE:
		{
			//just send to game
		}
		break;
	case CORE_MSG_FIRST_FIXED_NOTIFICATION:
		{
			core_msg_first_fixed_notification *pcoremsg = (core_msg_first_fixed_notification*)hMsg;
			pTotalId = &pcoremsg->dwClientId;
		}
		break;
	default:
		return false;
	}
	
	OPENCLIENTLIST::iterator it;
	for(it = m_OpenClientList.begin(); it != m_OpenClientList.end(); it++)
	{
		DWORD instance = *it;
		DWORD lang = theDataPool.GetLangIdByAbbr(GetAppLanguage());
		DWORD dwTotalId = MAKETOTALID(GetAppIntId(), lang, instance);
		if(pTotalId)
		{
			*pTotalId = dwTotalId;
		}
		g_appManager.GetCoreMsgServer().SendCoreMsg(dwTotalId, hMsg);
	}
	___AutoLockEnd____

		msg.SetRetValue(true);
	return true;
}

bool CThirdPartyApp::OnMessage(ThirdAppMsgHeader& msg)
{
	bool bRet = false;
	switch (msg.getMsgType())
	{
	case TAMT_QueryToken:
		HandleQueryTokenMsg(static_cast<QueryTokenMsg&>(msg));
		bRet = true;
		break;
	case TAMT_QueryCookie:
		HandleQueryCookieMsg(static_cast<QueryCookieMsg&>(msg));
		bRet = true;
		break;
	case TAMT_QueryIsUrlInfo:
		bRet = HandleIsUrlGameInfoMsg(static_cast<QueryIsUrlInfoMsg&>(msg));
		break;
	case TAMT_QueryUrlRelatedApp:
		bRet = HandleUrlRelativeAppMsg(static_cast<QueryUrlRelatedAppMsg&>(msg));
		break;
	case TAMT_SendArcMessageToGame:
		bRet = HandleSendArcMessageToGameMsg(static_cast<SendArcMessageToGameMsg&>(msg));
		break;
	default:
		return ThirdAppBase::OnMessage(msg);
	}


	return bRet;
}

bool CThirdPartyApp::SilenceUninstallPdlProc()
{
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;

	ExecuteInstallConfig(sPath, 1);
	ClearCurrentDownloadInfo();
	//not need to delete game directory, because cancel download has already do it
	///DeleteDir(GetLocalInfo().GetInstallPath());

	return true;
}

bool CThirdPartyApp::SilenceRegistPdlProc()
{
	GetLocalInfo().SetInstallPath(GetLocalInfo().GetDownloadPath());
	WriteSilenceRegistry();
	
	_tstring sPath = GetLocalInfo().GetInstallPath() + SGIFILENAME;
	SetPDLUncheckRegFlag(sPath, GetAppStrId(), true);

	return true;
}

bool CThirdPartyApp::StartTask()
{
	if(_threadHandle == NULL || WaitForSingleObject(_threadHandle, 10) == WAIT_OBJECT_0)
	{
		if(_threadHandle)
		{
			CloseHandle(_threadHandle);
			_threadHandle = NULL;
		}
		if(!start())
		{
			OutputDebugString(_T("start failed\n"));
			return false;
		}
	}
	return true;
}

bool CThirdPartyApp::CanPlaying()
{
	APP_LOCALSTATUS nStatus = static_cast<APP_LOCALSTATUS>(GetLocalInfo().GetLocalStatus());
	if ((nStatus == DOWNLOADING 
			&& GetLocalInfo().GetDownTool() == CDownloaderMgr::PDL_DOWNLOAD 
			&& GetAppInfo().GetIsEnablePDL() 
			&& GetAppInfo().GetIsSupportPDL() 
			&& g_appManager.IsFirstPlayingPDL(GetAppArrb(), GetAppLanguage())) 
		|| (nStatus >= EXTRACTED
			&& GetLocalInfo().GetDownTool() != CDownloaderMgr::PDL_DOWNLOAD))
	{
		return true;
	}

	return false;
}


bool CThirdPartyApp::GetActive()
{
	return GetLocalInfo().GetIsActive();
}

void CThirdPartyApp::SetActive(bool bActive)
{
	GetLocalInfo().SetIsActive(bActive);
	if(bActive)
	{
		if(GetLocalStatus() == ThirdAppBase::NOTADDED)
		{
			SetLocalStatus(ThirdAppBase::ADDED);
		}
	}
}

DWORD CThirdPartyApp::GetLocalStatus()
{
	return GetLocalInfo().GetLocalStatus();
}

void CThirdPartyApp::SetLocalStatus(DWORD dwStatus)
{
	GetLocalInfo().SetLocalStatus(dwStatus);
}

bool CThirdPartyApp::CreateGameProcess(LPCSTR lpExecuteFile, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory)
{
	struct INJECT_GAME_INFO
	{
		_tstring sAbbr;
		int nId;
		_tstring sLangInclude;
		_tstring sLangExclude;
	};
	INJECT_GAME_INFO sInjectGames[] = 
	{
		{ _T("pwi"),	3,		_T("all"), 	_T("") },
		{ _T("jd"),		4,		_T("all"), 	_T("") },
		{ _T("eso"),	7,		_T("all"), 	_T("") },
		{ _T("boi"),	8, 		_T("all"), 	_T("") },
		{ _T("fw"),		11, 	_T("all"), 	_T("") },
		{ _T("woi"),	25, 	_T("all"), 	_T("") },
		{ _T("swm"),	30, 	_T("all"), 	_T("") },
		{ _T("bl"),		1001, 	_T(""), 	_T("ru") },
		{ _T("rz"),		1002, 	_T("all"), 	_T("") },
		{ _T("sto"),	1003, 	_T("all"), 	_T("") },
		{ _T("co"),		1004, 	_T("all"), 	_T("") },
		{ _T("nw"),		2002, 	_T("all"), 	_T("") },
		//{ _T("apb"),	4001, 	_T("all"), 	_T("") },
		//{ _T("dzl"),	4100, 	_T("all"), 	_T("") },
		{ _T("sc"),	4200, 	_T("all"), 	_T("") },
		//{ _T("sk"), 	4400, 	_T("all"), 	_T("") },
		{ _T("pwo"), 	4600, 	_T("all"), 	_T("") },
		{ _T("tl"), 	5000, 	_T("all"), 	_T("") },
		{ _T("tl2"), 	5001, 	_T("all"), 	_T("") },
	};

	int nInjectlistsize = sizeof(sInjectGames)/sizeof(INJECT_GAME_INFO);
	_tstring sLang = GetAppLanguage();
	sLang.MakeLower();
	bool bInject = false;
	for(int i = 0; i < nInjectlistsize; i++)
	{
		if(sInjectGames[i].nId == GetAppIntId())
		{
			if(!sInjectGames[i].sLangExclude.empty())
			{
				if(sInjectGames[i].sLangExclude.Find(sLang) == _tstring::npos)
				{
					bInject = true;
				}
			}
			else
			{
				if(sInjectGames[i].sLangInclude.Find(_T("all")) != _tstring::npos ||
					sInjectGames[i].sLangInclude.Find(sLang) != _tstring::npos)
				{
					bInject = true;
				}
			}
			break;
		}
	}

	if(bInject)
	{
		return g_appManager.CreateGameProcess(lpExecuteFile, lpApplicationName, lpCommandLine, lpCurrentDirectory);
	}
	else
	{
		STARTUPINFOA si = {0};
		si.cb = sizeof(STARTUPINFOA);
		PROCESS_INFORMATION pi;
		return CreateProcessA(lpApplicationName, lpCommandLine, NULL, NULL, TRUE, 0, NULL, lpCurrentDirectory, &si, &pi);
	}
}

bool CThirdPartyApp::AddToCloud()
{
	if(GetType() == ThirdApp_Extend)
	{
		return true;
	}

	_tstring sGames;
	_tstring sFavorite = GetMemInfo().GetIsFavorite()? _T("true") : _T("false");
	sGames.Format(4, _T("games=[{\"favorite\":%s,\"lang\":\"%s\",\"abbr\":\"%s\",\"order\":\"%s\"}]"),
		sFavorite.c_str(), GetAppLanguage().c_str(), GetAppArrb().c_str(), _tstring(GetUIOrder()).c_str());
	if(g_theWebData.AddGameToCloud(g_theSNSManager.GetSessionId(), sGames))
	{
		GetMemInfo().SetIsCloudGame(true);
		UpdateDeletable();
		return true;
	}
	return false;
}

bool CThirdPartyApp::RemoveFromCloud()
{
	_tstring sGames;
	sGames.Format(2, _T("abbr=%s&lang=%s"),
		GetAppArrb().c_str(), GetAppLanguage().c_str());
	if(g_theWebData.DeleteCloudGame(g_theSNSManager.GetSessionId(), sGames))
	{
		GetMemInfo().SetIsCloudGame(false);
		return true;
	}
	return false;
}

bool CThirdPartyApp::UpdateToCloud()
{
	if(!GetMemInfo().GetIsCloudGame() || GetType() == ThirdApp_Extend)
	{
		return true;
	}
	_tstring sGames;
	_tstring sFavorite = GetMemInfo().GetIsFavorite()? _T("true") : _T("false");
	sGames.Format(4, _T("games=[{\"favorite\":%s,\"lang\":\"%s\",\"abbr\":\"%s\",\"order\":\"%s\"}]"),
		sFavorite.c_str(), GetAppLanguage().c_str(), GetAppArrb().c_str(), _tstring(GetUIOrder()).c_str());
	return g_theWebData.UpdateCloudGame(g_theSNSManager.GetSessionId(), sGames);
}

bool CThirdPartyApp::UpdateDeletable()
{
	if(!GetMemInfo().GetIsCloudGame() || GetType() == ThirdApp_Extend)
	{
		return true;
	}
	_tstring sGames;
	sGames.Format(2, _T("abbr=%s&lang=%s"),
		GetAppArrb().c_str(), GetAppLanguage().c_str());
	GetMemInfo().SetIsDeletable(g_theWebData.IsGameDeletable(g_theSNSManager.GetSessionId(), sGames));
	return true;
}

void CThirdPartyApp::RemoveFromMyList()
{
	ThirdAppBase::RemoveFromMyList();
	GetMemInfo().SetIsCloudGame(false);
	GetMemInfo().SetIsFavorite(false);
}


//==============================================================

CExternalApp::CExternalApp(int iID, const EXTERNALGAMEINFO& info)
{
	GetAppInfo().SetIntId(iID);
	_tstring sId;
	sId.Format(1, _T("%d"), iID);
	GetAppInfo().SetStringId(sId);
	GetAppInfo().SetLogoFile(info.sLogoFile);
	GetAppInfo().SetName(info.sName);
	GetLocalInfo().SetClientPath(info.sGamePath);

	UpdateNewsLinks();

	//set game abbr
	_tstring sAbbr;
	sAbbr.Format(1, _T("_%u"), GetAppIntId());
	GetAppInfo().SetAbbr(sAbbr);
}

CExternalApp::~CExternalApp()
{

}

bool CExternalApp::Init()
{
	GetAppInfo().SetLanguage(theDataPool.GetLangAbbrString());

	GetLocalInfo().SetLocalStatus(CThirdPartyApp::INSTALLED);

	return CThirdPartyApp::StartTask();
}

bool CExternalApp::StartProcNew()
{
	_tstring sExe;
	if(!GetLocalInfo().GetLauncherPath().empty())
	{
		sExe = GetLocalInfo().GetLauncherPath();
	}
	else if(!GetLocalInfo().GetPatcherPath().empty())
	{
		sExe = GetLocalInfo().GetPatcherPath();
	}
	else
	{
		sExe = GetLocalInfo().GetClientPath();
	}

	if(sExe.empty())
	{
		return false;
	}

	_tstring sProcessId;
	sProcessId.Format(1, _T("%d"), GetCurrentProcessId());
	SetEnvironmentVariable(_T("ArcGameType"), _tstring(GetType()).c_str());
	_tstring sTotalId = MAKETOTALID(GetAppIntId(), theDataPool.GetLangIdByAbbr(GetAppLanguage()), ++m_instance);
	SetEnvironmentVariable(_T("CoreGameId"), sTotalId.c_str());
	SetEnvironmentVariable(_T("ArcProcessId"), sProcessId.c_str());

	_tstring sGameDir = sExe.Left(sExe.ReverseFind(_T('\\')));

	if(!g_appManager.GetInjectFunction()(sExe.toNarrowString().c_str(), NULL, TRUE, 0, sGameDir.toNarrowString().c_str(), NULL))
	{
		return false;
	}

	return true;
}

const _tstring CExternalApp::GetAppLogoFile()
{
	return GetAppInfo().GetLogoFile();
}

void CExternalApp::HandleQueryCookieMsg(QueryCookieMsg& msg)
{
	core_msg_cookie &hCookie = *msg.GetCoreCookieMsg();

	lstrcpyn(hCookie.csCookiePage,theUIString._GetStringFromId(_T("IDS_COOKIE_URL")).c_str(),1024);
	lstrcpyn(hCookie.csBillingPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BILLING")).c_str(),1024);

	lstrcpyn(hCookie.csSupportPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SUPPORT")).c_str(),1024);
	lstrcpyn(hCookie.csMyAccountPage,theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ACCOUNT")).c_str(),1024);
	lstrcpyn(hCookie.csWebPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str(),1024);
	lstrcpyn(hCookie.csMediaPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_STORE")).c_str(),1024);
	lstrcpyn(hCookie.csNewsPage, theUIString._GetStringFromId(_T("IDS_FILTER_WEBSITE_PWRD_NEWS")).c_str(), 1024);
	lstrcpyn(hCookie.csForumPage, theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_COMMUNITY")).c_str(), 1024);
	lstrcpyn(hCookie.csGameName,  GetAppInfo().GetName().c_str(),1024);
	lstrcpyn(hCookie.csBackToGame, theUIString._GetStringFromId(_T("IDS_OVERLAY_BACKTOGAME")).c_str(),1024);
	lstrcpyn(hCookie.csClose,  theUIString._GetStringFromId(_T("IDS_OVERLAY_CLOSE")).c_str(),1024);
	lstrcpyn(hCookie.csNews, theUIString._GetStringFromId(_T("IDS_OVERLAY_NEWS")).c_str(),1024);
	lstrcpyn(hCookie.csBilling,  theUIString._GetStringFromId(_T("IDS_OVERLAY_BILLING")).c_str(),1024);
	lstrcpyn(hCookie.csSupport,  theUIString._GetStringFromId(_T("IDS_OVERLAY_SUPPORT")).c_str(),1024);
	lstrcpyn(hCookie.csForum,  theUIString._GetStringFromId(_T("IDS_OVERLAY_COMMUNITY")).c_str(),1024);
	lstrcpyn(hCookie.csWeb,  theUIString._GetStringFromId(_T("IDS_OVERLAY_WEB")).c_str(),1024);
	lstrcpyn(hCookie.csMedia,  theUIString._GetStringFromId(_T("IDS_OVERLAY_MEDIA")).c_str(),1024);
	lstrcpyn(hCookie.csInvite,  theUIString._GetStringFromId(_T("IDS_OVERLAY_INVITE")).c_str(),1024);
	lstrcpyn(hCookie.csFriendSearchPage,  theUIString._GetStringFromId(_T("IDS_SEARCH_FRIENDS")).c_str(),1024);
	lstrcpyn(hCookie.csChatView,  theUIString._GetStringFromId(_T("IDS_OVERLAY_CHATVIEW")).c_str(),1024);
	lstrcpyn(hCookie.csDialogTitle, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_TITLE")).c_str(),1024);
	lstrcpyn(hCookie.csInGameTitle,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TITLE")).c_str(),1024);
//	lstrcpyn(hCookie.csText1,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TEXT1")).c_str(),1024);
	lstrcpyn(hCookie.csText2,  theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_INGAME_TEXT2")).c_str(),1024);
	lstrcpyn(hCookie.csOk, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_OK")).c_str(),1024);
	lstrcpyn(hCookie.csCancel, theUIString._GetStringFromId(_T("IDS_DIALOG_SETTING_CANCEL")).c_str(),1024);
	lstrcpyn(hCookie.csUserName,  theDataPool.GetUserName().c_str(),1024);
	lstrcpyn(hCookie.csGameId,  GetAppInfo().GetStringId().c_str(),1024);
	//lstrcpyn(hCookie.csGameAbbrName,GetAppInfo().GetAbbr().c_str(),1024);
	lstrcpyn(hCookie.csCurrentPlay, theUIString._GetStringFromId(_T("IDS_STR_CURRENTPLAY")).c_str(), 1024);
	lstrcpyn(hCookie.csMonth[0],  theUIString._GetStringFromId(_T("IDS_MONTH_S1")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[1],  theUIString._GetStringFromId(_T("IDS_MONTH_S2")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[2],  theUIString._GetStringFromId(_T("IDS_MONTH_S3")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[3],  theUIString._GetStringFromId(_T("IDS_MONTH_S4")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[4],  theUIString._GetStringFromId(_T("IDS_MONTH_S5")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[5],  theUIString._GetStringFromId(_T("IDS_MONTH_S6")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[6],  theUIString._GetStringFromId(_T("IDS_MONTH_S7")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[7],  theUIString._GetStringFromId(_T("IDS_MONTH_S8")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[8],  theUIString._GetStringFromId(_T("IDS_MONTH_S9")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[9],  theUIString._GetStringFromId(_T("IDS_MONTH_S10")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[10],  theUIString._GetStringFromId(_T("IDS_MONTH_S11")).c_str(),1024);
	lstrcpyn(hCookie.csMonth[11],  theUIString._GetStringFromId(_T("IDS_MONTH_S12")).c_str(),1024);

	lstrcpyn(hCookie.csWeek[0],  theUIString._GetStringFromId(_T("IDS_WEEK_7")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[1],  theUIString._GetStringFromId(_T("IDS_WEEK_1")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[2],  theUIString._GetStringFromId(_T("IDS_WEEK_2")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[3],  theUIString._GetStringFromId(_T("IDS_WEEK_3")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[4],  theUIString._GetStringFromId(_T("IDS_WEEK_4")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[5],  theUIString._GetStringFromId(_T("IDS_WEEK_5")).c_str(),1024);
	lstrcpyn(hCookie.csWeek[6],  theUIString._GetStringFromId(_T("IDS_WEEK_6")).c_str(),1024);
    lstrcpyn(hCookie.csHi, theUIString._GetStringFromId(_T("IDS_MENU_HI")).c_str(), 1024);


	_tstring sUserIcon = _T("") ;
	if (!g_theSNSManager.GetUserCharData()._szHeaderPath.empty())
	{
		sUserIcon = _T("\\") + g_theSNSManager.GetUserCharData()._szHeaderPath ;
		int nPos = sUserIcon.find_last_of('.') ;
		_tstring sExt = &sUserIcon[nPos] ;
		lstrcpyn(hCookie.csUserIcon,sUserIcon.c_str(),1024);
	}

	lstrcpyn(hCookie.csSessionId, g_theSNSManager.GetSessionId().toWideString().c_str(),1024);

	g_appManager.GetCoreMsgServer().SendCoreMsg(hCookie.dwClientId, &hCookie);

	msg.SetRetValue(true);
}

void CExternalApp::UpdateNewsLinks()
{
	//game info/news url
	_tstring sBaseDir = theDataPool.GetBaseDir();
	_tstring sUrl = _T("file:///") + sBaseDir + _T("resources\\externalgame\\arcgames.html?");
	std::string s = sUrl.toUTF8();
	sUrl = CWebService::miniURLEncode(s);

	sUrl += _T("l=");
	sUrl += theDataPool.GetLangAbbrString().MakeLowerConst();

	sUrl += _T("&n=");
	_tstring sName = CWebService::miniURLEncodeComponent(GetAppInfo().GetName().toUTF8());
	sUrl += sName;

	sUrl += _T("&i=file:///");
	_tstring sLogoFile = CWebService::miniURLEncodeComponent(GetAppInfo().GetLogoFile().toUTF8());
	sUrl += sLogoFile;

	GetAppInfo().SetNewsLink(sUrl);
}

const _tstring CExternalApp::GetAppBkFile()
{
	_tstring sBgPath = theDataPool.GetBaseDir();
	sBgPath += _T("resources\\externalgame\\bg.jpg");
	return sBgPath; 
}

bool CExternalApp::LoadAppInfo(TiXmlElement* pInfo,bool bUpdate)
{
	// do nothting there
	OutputDebugString(_T("ERROR !!!! CExternalApp::LoadAppInfo must not be called\n")); 
	return true;
}

bool CExternalApp::LoadLocalInfo(TiXmlElement* pElement)
{	
	const char* ptmp = pElement->Attribute("order");
	if(ptmp)
	{
		GetLocalInfo().SetOrder(atoi(ptmp));
	}

	ptmp = pElement->Attribute("favorite");
	if(ptmp)
	{
		GetMemInfo().SetIsFavorite(atoi(ptmp));
	}

	SYSTEMTIME t;
	TiXmlElement* pSub = pElement->FirstChildElement("lastplaytime");
	if(pSub != NULL)
	{
		t.wYear = atoi(pSub->Attribute("year"));
		t.wMonth = atoi(pSub->Attribute("month"));
		t.wDay = atoi(pSub->Attribute("day"));
		t.wHour = atoi(pSub->Attribute("hour"));
		t.wMinute = atoi(pSub->Attribute("minute"));
		t.wSecond = atoi(pSub->Attribute("second"));
		GetLocalInfo().SetLastplaytime(t);
	}

	return true;
}

bool CExternalApp::SaveLocalInfoToXmlNode(TiXmlElement* pGame)
{
	char szBuf[256] = {0};
	pGame->SetAttribute("type", itoa(GetType(), szBuf, 10));

	char name[1024] = "";
	WideCharToMultiByte( CP_UTF8, 0, GetAppInfo().GetName().c_str(), -1, name, 1024, NULL, NULL );

	pGame->SetAttribute("name", name);
	pGame->SetAttribute("order", itoa(GetLocalInfo().GetOrder(), szBuf, 10));
	pGame->SetAttribute("favorite", itoa(GetMemInfo().GetIsFavorite(), szBuf, 10));

	// last play time
	SYSTEMTIME t = GetLocalInfo().GetLastplaytime();
	TiXmlElement *pSub = new TiXmlElement("lastplaytime");
	pSub->SetAttribute("year", itoa(t.wYear, szBuf, 10));
	pSub->SetAttribute("month", itoa(t.wMonth, szBuf, 10));
	pSub->SetAttribute("day", itoa(t.wDay, szBuf, 10));
	pSub->SetAttribute("hour", itoa(t.wHour, szBuf, 10));
	pSub->SetAttribute("minute", itoa(t.wMinute, szBuf, 10));
	pSub->SetAttribute("second", itoa(t.wSecond, szBuf, 10));
	pGame->LinkEndChild(pSub);

	// path
	pSub = new TiXmlElement("path");
	pSub->SetAttribute("logo", GetAppLogoFile().toNarrowString().c_str());
	pSub->SetAttribute("clientpath", GetLocalInfo().GetClientPath().toNarrowString().c_str());
	pGame->LinkEndChild(pSub);

	return true;
}

bool CExternalApp::SaveStateToJson(json::Object& obj)
{
	// do nothting there
	OutputDebugString(_T("ERROR !!!! CExternalApp::SaveStateToJson must not be called\n"));
	return true;
}

bool CExternalApp::OnMessage(ThirdAppMsgHeader& msg)
{
	bool bRet = false;
	switch (msg.getMsgType())
	{
	case TAMT_QueryCookie:
		HandleQueryCookieMsg(dynamic_cast<QueryCookieMsg&>(msg));
		bRet = true;
		break;
	case TAMT_QueryToken:
		CThirdPartyApp::HandleQueryTokenMsg(dynamic_cast<QueryTokenMsg&>(msg));
		bRet = true;
		break;
	case TAMT_QueryIsUrlInfo:
		bRet = HandleIsUrlGameInfoMsg(dynamic_cast<QueryIsUrlInfoMsg&>(msg));
		break;
	case TAMT_QueryUrlRelatedApp:
		bRet = HandleUrlRelativeAppMsg(dynamic_cast<QueryUrlRelatedAppMsg&>(msg));
		break;
	default:
		return ThirdAppBase::OnMessage(msg);
	}

	return bRet;
}

bool CExternalApp::HandleIsUrlGameInfoMsg(QueryIsUrlInfoMsg& msg)
{
	if(0 == msg.GetUrl().find(GetAppInfo().GetInfoLink().c_str()))
	{
		msg.SetRetValue(true);
		return true;
	}

	return false;
}

bool CExternalApp::HandleUrlRelativeAppMsg(QueryUrlRelatedAppMsg& msg)
{
	_tstring url = msg.GetUrl();
	_tstring sLinkName = GetAppInfo().GetNewsLink();
	int len = sLinkName.length()-12;
	if(url.Right(len) == sLinkName.Right(len))
	{
		msg.SetRetValue(static_cast<IThirdApp*>(this));
		return true;
	}

	return false;
}

const _tstring CExternalApp::GetNewsLink()
{
	UpdateNewsLinks();
	return GetAppInfo().GetNewsLink();
}

/////////////////////////////////////////////////////////

bool SplitTotalId(DWORD totalId, DWORD &appId, DWORD &langId, DWORD &inst)
{
	appId = (totalId & 0xffff);
	langId = (totalId >> 16) & 0xff;
	inst = totalId >> 24;
	return true;
}

_tstring MakeAbbrLangId(_tstring sAbbr, _tstring sLang)
{
	_tstring sAbbrLang = sAbbr + _T(":") + sLang;
	return sAbbrLang;
}

bool SplitAbbrLangId(_tstring sAbbrLang, _tstring &sAbbr, _tstring &sLang)
{
	size_t pos = sAbbrLang.Find(_T(":"));
	if(pos <= 0)
	{
		return false;
	}
	sAbbr = sAbbrLang.Left(pos);
	sLang = sAbbrLang.Right(sAbbrLang.length() - pos - 1);
	return true;
}
