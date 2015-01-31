///*************************************************
/**
 * @file      ThirdPartyApp.h
   @brief     The header file of third party application.
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


#pragma once

#include <tchar.h>
#include <list>
#include "tstring/tstring.h"
#include "login/XmppLogin.h"
#include "datatype.h"
#include "thread/thread.h"
#include "CoreMsgOverlay2Svr.h"
#include "download/IDownLoader.h"
#include "tinyxml/tinyxml.h"
#include "thread/thread.h"
#include "data/ThirdPartyAppManager.h"
#include "data/ThirdAppBase.h"

using namespace std;
using namespace String;

typedef std::list<DWORD>	OPENCLIENTLIST;

//-------------------------------------------
struct APP_MEM_INFO : public DataBase
{
private:
	bool	 bNeedUpdate;
	int		 nCmd;
	bool	 bPaused;
	bool	 bTokenSuccess;
	bool	 bTokenFail;
	bool	 bTokenForPatcher;
	
	bool	 bAbortInstall;
	
	_tstring sDownSpeed;
	_tstring sDownProgress;

	_tstring sExtractProgress ;
	
	bool	 bDownloadIcon;
	bool	 bDownloadBkgnd;
	bool	 bDownloadInstallPic;

	bool	 bCloudGame;
	bool	 bFavorite;
	bool	 bDeletable;

public:
	APP_MEM_INFO();
	DEF_PROPERTY_RW(bool,	  IsNeedUpdate,bNeedUpdate);         
	DEF_PROPERTY_RW(int,	  Cmd,nCmd);
	DEF_PROPERTY_RW(bool,	  IsPaused,bPaused);
	DEF_PROPERTY_RW(bool,	  IsTokenSuccess,bTokenSuccess);
	DEF_PROPERTY_RW(bool,	  IsTokenFail, bTokenFail);
	DEF_PROPERTY_RW(bool,	  IsTokenForPatcher,bTokenForPatcher);
	DEF_PROPERTY_RW(bool,	  IsAbortInstall,bAbortInstall);
	DEF_PROPERTY_RW(_tstring, DownSpeed,sDownSpeed);
	DEF_PROPERTY_RW(_tstring, DownProgress,sDownProgress);
	DEF_PROPERTY_RW(_tstring, ExtractProgress,sExtractProgress);
	DEF_PROPERTY_RW(bool,	  DownloadIcon, bDownloadIcon);
	DEF_PROPERTY_RW(bool,	  DownloadBkgnd, bDownloadBkgnd);
	DEF_PROPERTY_RW(bool,	  DownloadInstallPic, bDownloadInstallPic);

	DEF_PROPERTY_RW(bool,	  IsCloudGame, bCloudGame);
	DEF_PROPERTY_RW(bool,	  IsFavorite, bFavorite);
	DEF_PROPERTY_RW(bool,	  IsDeletable, bDeletable);

};

//-------------------------------
class IAppCallBack
{
public:
	enum APPNOTIFY
	{
		NOTENOUGHSPACEFOREXTRACTION = 20,
		EXTRACTIONFAILED = 21,
		EXTRACTPATHNOTFOUND = 22,
		EXTRACTFILEINUSE = 23,
		EXTRACTACCESSDENIED = 24,
		SIGNUPSUCCESS = 30,
		INSTALLFINISH = 50,
		UNINSTALLFINISH = 51,
	};
public:

	virtual void OnUpdateAppStatus(int nStatus) = 0;
	virtual void OnUpdateStatus2Install() = 0;
	virtual void OnUpdateStatus2Installing() = 0;
	virtual void OnUpdateStatus2Paused() = 0;
	virtual void OnUpdateStatus2Extracting() = 0;
	virtual void OnUpdateStatus2Play() = 0;
	virtual void OnUpdateStatus2Uninstalling() = 0;
	virtual void OnAppNotify(int nNotify) = 0;
	virtual void OnUpdateExtracting(int nProgress) = 0 ;
};

#define SGIFILENAME	(_T("appinstallconfig.xml"))


class CThirdPartyApp : public ThirdAppBase, public Thread, ILoginListener, public IDownloaderListener
{
public:
	enum APP_LOCALSTATUS
	{
		NOTADDED = 0,
		ADDED = 1,
		DOWNLOADING = 2,
		DOWNLOADED = 3,
		EXTRACTED = 4,
		INSTALLED = 5,
	};

	enum APP_CMD
	{
		NOCMD = 0,
		INSTALLAPP,
		UNINSTALLAPP,
		STARTAPP,
		REPAIRAPP,
		EXTRACT,
		EXTRAT_PDL,
		UNINSTALL_PDL,
		CLEAR_PDLDATA
	};

public:
	CThirdPartyApp();
	virtual ~CThirdPartyApp();

	/*
	*  @brief: init and start thread. 
	*  @return: true if successful.
	*  @notes: called when start.
	*/
	virtual bool Init();

	virtual bool DownloadIcon();
	virtual bool DownloadBkgnd();
	virtual bool DownloadInstallPic();
	/*
	*  @brief: stop thread. 
	*  @return: true.
	*  @notes: called before exit.
	*/
	virtual bool Uninit();

	/*
	*  @brief: check if silence install. 
	*  @return: true if silence config file exists.
	*/
	virtual bool IsSilence();

	/*
	*  @brief: check if installed. 
	*  @return: true if installed.
	*  @notes: called when starts.
	*/
	virtual bool CheckIfInstalled();

	virtual bool CheckForUpdate();

	/*
	*  @brief: start application. 
	*  @return: true if successful.
	*/
	virtual bool StartApp();

	virtual bool ClearPDLData();
	/*
	*  @brief: repair application. 
	*  @return: true if successful.
	*/
	virtual bool StartPatcher();

	/*
	*  @brief: install application. 
	*  @return: true if successful.
	*/
	virtual bool Install();

	/*
	*  @brief: uninstall application. 
	*  @return: true if successful.
	*/
	virtual bool UnInstall();

	/*
	*  @brief: extrict download zip file. 
	*  @return: true if successful.
	*/
	virtual bool ExtractFile();

	/*
	*  @brief: check if the app was uninstalled. 
	*  @return: true if uninstalled.
	*/
	virtual bool CheckUninstall();

	virtual bool IsInstalledFlagSet();

	/*
	*  @brief: clear local info. 
	*/
	virtual void ClearLocalInfo();

	/*
	*  @brief: clear download info. 
	*/
	virtual void ClearDownloadInfo();

	void SetCurrentDownloadInfo(int nDownloadType);
	void ClearCurrentDownloadInfo();

	// ILoginListener interface
	virtual bool LOGIN_GameTokenSuccess(WPARAM wParam, LPARAM lParam);
	virtual bool LOGIN_GameTokenFail(WPARAM wParam, LPARAM lParam);

	// thread function
	virtual long run();

	virtual void CheckNewApp();

	virtual void SetOrder(int nOrder);

	virtual bool IsFullScreen();
	virtual bool IsWindowed();

	//-----------------------------------------------------------
	APP_INFO& GetAppInfo(){return m_AppInfo;}
	virtual APP_LOCAL_INFO& GetLocalInfo(){return m_LocalInfo;}
	virtual APP_MEM_INFO& GetMemInfo(){return m_MemInfo;}

	bool PatchGame();

	virtual ThirdAppType GetType(){ return ThirdApp_Normal; }

	virtual bool LoadAppInfo(TiXmlElement* pInfo,bool bUpdate);
	virtual bool LoadLocalInfo(TiXmlElement* pInfo);
	virtual bool SaveLocalInfoToXmlNode(TiXmlElement* pRoot);
	virtual bool SaveStateToJson(json::Object& obj);
	virtual const _tstring	GetAppBkFile();
	virtual const _tstring  GetAppLogoFile();
	virtual const _tstring  GetAppInstallPicFile();
	virtual bool  OnMessage(ThirdAppMsgHeader& msg);

	// query state
	virtual bool IsInMyGameList();
	virtual bool IsPlaying();
	virtual bool IsInstalling();
	virtual bool IsExtracting();
	virtual bool IsUninstalling();
	virtual bool IsDownloading();
	virtual bool IsNewApp();
	virtual void ResetNewApp();
	virtual bool IsResouceReady(){return true;}
	virtual bool IsSaveLocalInfo(); 
	virtual bool IsPreLoad();
	virtual bool IsEnteredGame();

	virtual bool IsInstalled();

	virtual bool CanPlaying();

	virtual bool GetActive();
	virtual void SetActive(bool bActive);

	virtual DWORD GetLocalStatus();
	virtual void SetLocalStatus(DWORD dwStatus);

	virtual bool GetIsFavorite(){return GetMemInfo().GetIsFavorite();}

	virtual void RemoveFromMyList();


	bool AddToCloud();
	bool RemoveFromCloud();
	bool UpdateToCloud();
	bool UpdateDeletable();

	bool ClearDownloadInfoBan();

	void SetInstallWnd(HWND hwnd){m_hwndInstall = hwnd;};

public:
	virtual bool OnStartBegin(_tstring sId);
	virtual bool OnStartEnd(_tstring sId); 
	virtual bool OnPauseBegin(_tstring sId);
	virtual bool OnPauseEnd(_tstring sId);
	virtual bool OnResumeBegin(_tstring sId);
	virtual bool OnResumeEnd(_tstring sId);
	virtual bool OnCancelBegin(_tstring sId);
	virtual bool OnCancelEnd(_tstring sId);
	virtual bool OnComplete(_tstring sId);
	virtual bool OnLauchPoint(_tstring sId);
	virtual bool UpdateStatus(_tstring sId, DOWNLOAD_PARAM *pDownloadParam);
	virtual bool OnError(_tstring sId, int nErrType, _tstring sComment = _T(""));
	virtual bool OnTraceMessage(DWORD errorCode, const _tstring& comment);
	virtual bool OnPlayPDLGame(const _tstring& szId);
protected:
	bool UnstallPDLApp();
	bool ExtratPdlApp();

	bool SilenceInstallProc();
	bool SilenceUninstallProc();
	virtual bool StartProcNew();
	bool StartPatcherProcNew();

	bool SilenceUninstallPdlProc();
	bool SilenceRegistPdlProc();

	bool StartTask();

	bool DownloadIconProc();
	bool DownloadBkgndProc();
	bool DownloadInstallPicProc();

	bool ClearPDLDataProc();

	int  UnZipDownloadedPackage();
	bool OnGameTokenSuccess();
	bool OnGameTokenFail();
	bool WriteSilenceRegistry();
	void DeleteDir(const _tstring& sDir);
	bool CheckSilenceRegistry();
	bool TranslatePath(_tstring &sPath);
	bool GetCmdlinePara(_tstring &sCmd, HANDLE hToken = NULL);

	bool LaunchCrypticGame(bool bSupportBypass);
	bool LaunchGameClientBypass();
	bool LaunchGamePatcherBypass();
	bool SetArcGameEnvVar();
	bool CreateGameProcess(LPCSTR lpExecuteFile, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory);
	
	bool CheckIfClientRunning();

	static void ZipFCallback(INT32 nProgress, LPVOID lpCaller) ;

	void OnDownloading();

	// msg handle
	bool HandleIsUrlGameInfoMsg(QueryIsUrlInfoMsg& msg);
	bool HandleUrlRelativeAppMsg(QueryUrlRelatedAppMsg& msg);
	void HandleQueryTokenMsg(QueryTokenMsg& msg);
	void HandleQueryCookieMsg(QueryCookieMsg& msg);
	bool HandleSendArcMessageToGameMsg(SendArcMessageToGameMsg& msg);
public:
	_tstring GetGameStartExe();

protected:
	APP_INFO m_AppInfo;			// 基本信息
	APP_LOCAL_INFO m_LocalInfo;	// 本地信息
	APP_MEM_INFO m_MemInfo;		// 临时信息

	Mutex m_OpenClientLock;
	OPENCLIENTLIST m_OpenClientList;
	DWORD m_instance;
	bool m_bExit;

	HWND m_hwndInstall;
};

//------------------------------------------------------

////--------------------------
/* @brief
** declare game flag, indicated by "nstatus" in m_sGameinfo
*/
enum GAME_STATUS
{
	GS_UNKNOWN = -1,
	LIVE = 0 ,
	PATCH,
	OPENBETA,
	UPCOMING, // this status now is new 
	CLOSEDBETA,
	NEXTSTATUS,
} ;

/* @brief
** declare game status in memory
*/

/* @brief
** declare game status just for all game page
*/
enum ALLGame_STATUS
{
	ALLGame_DOWNLOAD = NEXTSTATUS,
	ALLGAME_PLAY,
	ALLGAME_ADD,
	ALLGAME_ADDED,
	ALLGAME_INSTALL,
	ALLGAME_SEERVEROFFLINE,
};

//=====================================================================

class CExternalApp : public CThirdPartyApp
{
public:
	CExternalApp(int iID, const EXTERNALGAMEINFO& info);

	virtual ~CExternalApp();

	virtual ThirdAppType GetType() { return ThirdApp_Extend; }

	virtual bool Init();

	virtual const _tstring GetAppLogoFile();
	virtual const _tstring GetAppBkFile();
	virtual const _tstring GetNewsLink();

	virtual bool LoadAppInfo(TiXmlElement* pInfo,bool bUpdate);
	virtual bool LoadLocalInfo(TiXmlElement* pInfo);
	virtual bool SaveLocalInfoToXmlNode(TiXmlElement* pRoot);
	virtual bool SaveStateToJson(json::Object& obj);

	virtual bool  OnMessage(ThirdAppMsgHeader& msg);
	void HandleQueryCookieMsg(QueryCookieMsg& msg); 
	bool HandleIsUrlGameInfoMsg(QueryIsUrlInfoMsg& msg);
	bool HandleUrlRelativeAppMsg(QueryUrlRelatedAppMsg& msg);
	virtual bool CanPlaying(){return true;}
	virtual bool IsInMyGameList(){	return GetLocalInfo().GetLocalStatus() >= ADDED;}
protected:
	virtual bool StartProcNew();
	virtual void UpdateNewsLinks();
};

//=====================================================================

#define MAKETOTALID(GameId, GameLang, GameInstance) ((GameInstance << 24) | (GameLang << 16) | GameId)
bool SplitTotalId(DWORD totalId, DWORD &appId, DWORD &langId, DWORD &inst);

_tstring MakeAbbrLangId(_tstring sAbbr, _tstring sLang);
bool SplitAbbrLangId(_tstring sAbbrLang, _tstring &sAbbr, _tstring &sLang);