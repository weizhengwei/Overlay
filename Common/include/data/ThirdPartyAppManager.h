///*************************************************
/**
 * @file      ThirdPartyApp.h
   @brief     The header file of third party application manager.
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

#include <map>
#include <list>
#include <vector>
#include "thread/Condition.h"
#include "tstring/tstring.h"
#include "thread/thread.h"
#include "CoreMsgBase.h"
#include "CoreMsgOverlay2Svr.h"
#include "data/IThirdApp.h"
#include "download/IDownLoader.h"

using namespace std;
using namespace String;

class CAppListWnd;
class UIMessageHeader;

typedef struct _tThridApp_PopInfo
{
	_tstring sname;
	_tstring sinfoURL;
}ThridApp_PopInfo;

typedef struct _tThirdApp_GameNotification
{
	_tstring szLink;
	_tstring szId;
	_tstring szTitle;
	_tstring szText;
	//RECT rect;
}ThirdApp_GameNotification;


//用于手动添加游戏
struct EXTERNALGAMEINFO
{
	_tstring sGamePath;		//游戏路径
	_tstring sName;			//游戏名
	_tstring sLogoFile;		//游戏logo文件路径
};

typedef std::list<IThirdApp*> MyGameList;

typedef BOOL (WINAPI * FUNC_CreateProcessWithDll)(LPCSTR lpApplicationName,
												  LPSTR lpCommandLine,
												  BOOL, DWORD, LPCSTR,
												  LPPROCESS_INFORMATION lpProcessInformation);


class CThirdPartyAppManager : public Thread
{
public:
	CThirdPartyAppManager();
	virtual ~CThirdPartyAppManager();

	/*
	*  @brief: init all app information. 
	*  @return: true if successful.
	*  @notes: called when start.
	*/
	bool Init();

	/*
	*  @brief: uninit apps and release memory. 
	*  @return: true if successful.
	*  @notes: called when exit.
	*/
	bool Uninit();

	/*
	*  @brief: update game info
	*  @return: true if successful.
	*  @notes: called when web push new game info.
	*/
	bool UpdateGameinfo(const char* data);

	/*
	*  @brief: check if app installing. 
	*  @return: true if there is a app installing.
	*/
	bool IsAnyAppInstalling(_tstring *sAppName = NULL);

	/*
	*  @brief: check if app uninstalling. 
	*  @return: true if there is a app uninstalling.
	*/
	bool IsAnyAppUninstalling(_tstring *sAppName = NULL);

	/*
	*  @brief: check if app extracting. 
	*  @return: true if there is a app extracting.
	*/
	bool IsAnyAppExtracting(_tstring *sAppName = NULL);

	/*
	*  @brief: check if app playing. 
	*  @return: true if there is a app playing.
	*/
	bool IsAnyAppPlaying(_tstring *sAppName = NULL);

	/*
	*  @brief: check if app is installing uninstalling playing extracting. 
	*  @return: true if app is doing something.
	*/
	bool IsAnyAppInProcessing();

	/*
	*  @brief: check if app playing. 
	*  @return: true if there is a app playing.
	*/
	BOOL IsGamePlaying(_tstring sGameAbbr=_T("")) ;

	/*
	*  @brief: check if there is a app windowed. 
	*/
	bool IsAnyAppFullScreen();

	/*
	*  @brief: check if the app windowed. 
	*/
	bool IsAppWindowed(UINT nId);

	/*
	*  @brief: get app by given id. 
	*/
	//IThirdApp* GetAppById(int nId);

	/*
	*  @brief: get app by given abbr. 
	*/
	//IThirdApp* GetAppByAbbr(_tstring sAbbr);

	IThirdApp* GetThirdAppByAbbrLang(_tstring sAbbr, _tstring sLang);
	IThirdApp* GetThirdAppByIdLang(DWORD nId, _tstring sLang);
	IThirdApp* GetDefaultAppByAbbr(_tstring sAbbr);	//in gamelist or same language with arc
	IThirdApp* GetDefaultAppById(DWORD nId);	//in gamelist or same language with arc
	IThirdApp* GetFirstAppByAbbr(_tstring sAbbr);	
	_tstring GetAppNameByAbbr(_tstring sAbbr);
	void InsertThirdApp(IThirdApp *pApp);

	/*
	*  @brief: get all show(added and not hide) my app 
	*/
	MyGameList GetMyApps();

	MyGameList GetActiveApps();

	IThirdApp* GetLastplayGame();
	/*
	*  @brief: get a new app. 
	*/
	IThirdApp* GetNewApp();

	/*
	*  @brief: get my app count. 
	*/
	int GetMyAppCount();

	/*
	*  @brief: get status of all games supported on CORE Client, status is orgnized in json format
	*/
	_tstring GetGameStatus(_tstring sGameAbbr=_T("")) ;

	/*
	*  @brief: get popup
	*/
    ThridApp_PopInfo GetGameInfoPopUp(_tstring sGameAbbr) ;

	/*
	*  @brief: add external game
	*/
	IThirdApp* AddExternalGame(const EXTERNALGAMEINFO& info);

	/*
	*  @brief: delete external game
	*/
	bool DeleteExternalGame(IThirdApp *pApp);
	
	/*
	*  @brief: mark that local information should be stored. 
	*/
	void SetModified();

	/*
	*  @brief: set update data flag
	*/
	void SetUpdateData();

	/*
	*  @brief: set ui window handle (hwnd)
	*/
	void SetUIWnd(CAppListWnd* pWnd);

	//bool SendMessageToUIApp(IThirdApp* pApp, UIMessageHeader* pMsgHeader);

	LRESULT SendMessageToUIWnd(UINT message, WPARAM wParam, LPARAM lParam);

	bool PostMessageToUIApp(IThirdApp* pApp, UIMessageHeader& msg);
	/*
	*  @brief: send notify to game
	*/
	bool SendNotificationToGame(const ThirdApp_GameNotification& notification, IThirdApp *pApp);
	
	/*
	*  @brief: send message to app
	*/
	bool SendMessageToApp(_tstring sAbbr, _tstring sLang, ThirdAppMsgHeader& msg);

	/*
	*  @brief: get Core Msg Server
	*/
	CCoreMsgServer& GetCoreMsgServer(){return m_msgServer;}

	IThirdApp* GetGameInPDLDownloadingStatus();
	void	   SetGameInPDLDownloadingStatus(IThirdApp* pApp);
	IThirdApp* GetGameInPDLDownloadingAndPlayStatus();

	bool IsFirstPlayingPDL(_tstring sAbbr, _tstring sLang);
	void OnLoginSuccess();

	bool StartDownload(_tstring sAbbr, _tstring sLang, IDownloaderListener* listener,int nDownloadTool, DOWNLOAD_SETTING& downloadSetting);
	bool CancelDownload(_tstring sAbbr, _tstring sLang);
	bool PauseDownload(_tstring sAbbr, _tstring sLang);
	bool ResumDownload(_tstring sAbbr, _tstring sLang);

	bool IsDownloadingTaskExist(_tstring sAbbr, _tstring sLang);

	bool GetDefaultProductPage(_tstring& sUrl, _tstring sGameAbbr);

	bool SendInboxToGame(LPCTSTR pszType, LPCTSTR pszId, LPCTSTR pszNickName, LPCTSTR pszContent, LPCTSTR pszFriendLink, LPCTSTR pszContentLink, LPCTSTR pszTitle, LPCTSTR pszArcMailSubject);
	bool SendFriendReqToGame(LPCTSTR pszId, LPCTSTR pszNickName, LPCTSTR pszContent, LPCTSTR pszFriendLink, LPCTSTR pszTitle, int nSenderId);
	bool SendFRStatusToGame(LPCTSTR pszNickName, LPCTSTR pszGameName, LPCTSTR pszGameUrl, int nType);
	bool SendReadyPlayToGame(LPCTSTR pszGameName, LPCTSTR pszGameAbbr, LPCTSTR pszGameLang, LPCTSTR pszContent, LPCTSTR pszGameProPage, LPCTSTR pszGameLogoPath);
	bool IsGameInLaunchState();
	bool IsGameInClientState();
	bool SendFRResultToGame(BOOL bAccept, BOOL bResult);

	bool StartPDLGame(_tstring sAbbr, _tstring sLang);

	bool GetBanList();
	bool IsGameBannedByIP(const _tstring& sAbbr);

	bool PostLocalGameList();

	bool GetCloudGames();
	bool AddLocalGamesToCloud();

	void SetExpandState(bool bExpanded[3]){ memcpy(m_bCategoryExpanded, bExpanded, sizeof(m_bCategoryExpanded));SetModified();}
	bool* GetExpandState(){return m_bCategoryExpanded;}

	void SetSelectedGame(_tstring sApp){ if(sApp == m_sSelectedApp) return; m_sSelectedApp = sApp;SetModified();}
	_tstring GetSelectedGame(){ return m_sSelectedApp; }

    bool CreateGameProcess(LPCSTR lpExecuteFile, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory);

	FUNC_CreateProcessWithDll GetInjectFunction();
protected:

	friend class CThirdPartyApp; 

	void AddAutoResumeItem(const _tstring& szId);
	void DelAutoResumeItem(const _tstring& szId);
	void AutoResumeDownloadingTask();
	void AutoPauseDownloadingTaskExcept(IThirdApp* pApp);
	
	void LoadFirstPlayingInfo(const _tstring& szPath);
	void SaveFirstPlayingInfo(const _tstring& szPath);
	void ClearFirstPlayingCacheById(_tstring sAbbr, _tstring sLang);
	void KeepFirstPlayingCacheById(_tstring sAbbr, _tstring sLang);

	void MonitorGamePlayingState(IThirdApp* pApp);

	long run();
	bool LoadAppData();
	bool LoadAppLocalData();
	bool LoadAppLocalData(_tstring sLocalPath, _tstring sLangAbbr = _T(""));
	bool SaveAppLocalData(_tstring sLocalPath);

	bool SaveCategoryDataToXML(TiXmlElement *pCategory);
	bool LoadCategoryDataFromXML(TiXmlElement *pCategory);

private:
	void OnMonitorGamePlayingState();
	void OnGameStart(IThirdApp* pApp);
	void OnGameClose(IThirdApp* pApp);

	bool _IsFirstPlaying(IThirdApp* pApp);

    bool Game_Is_64bit(LPCSTR lpApplicationName);
    bool Create_64bit_Game_Process(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory);
    bool Create_32bit_Game_Process(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory);

private:
	CCoreMsgServer m_msgServer;	//bypass msg server

	_tstring m_sAppInfoPath;
	_tstring m_sLocalInfoPath;
	_tstring m_sFirstPlayingInfoPath;
	enum EventType{EventModify, EventUpdate, EventSaveFirstPlayingInfo,EventExit,EventMonitor,EventCount};
	HANDLE m_events[EventCount];
	
	typedef std::map<int, IThirdApp*> AppMap;
	typedef AppMap::iterator MapItor;
	typedef AppMap::const_iterator MapCItor;

	typedef std::map<_tstring, IThirdApp*> AppLangMap;		//key=language
	typedef std::map<_tstring, AppLangMap*> ThirdAppMap;	//key=abbr
	ThirdAppMap m_mapApps;
	
	Mutex	m_appMapLock;
	AppMap  m_mapAppListById;
	Mutex	m_externalMapLock;
	AppMap  m_mapExternalApp;
	
	int  m_nExternalId;

	IThirdApp* m_pCurDownloadingPDLApp;

	Mutex m_appWndMutex;
	CAppListWnd* m_pAppListWnd;

	typedef std::vector<_tstring> AutoResumeVec;
	Mutex m_autoResumeVecLock;
	AutoResumeVec m_autoResumeVec; 

	typedef std::map<_tstring, bool> FirstPlayingInfoMap;
	FirstPlayingInfoMap m_firstPlayingInfoMap;
	Mutex	m_firstPlayingInfoLock;

	typedef struct _tagMonitorInfo{
		bool bPlaying;
		int  retryTimes;
	} MonitorInfo;
	typedef std::map<_tstring, MonitorInfo> MonitorGameMap;
	MonitorGameMap m_monitorGameMap;
	Mutex m_monitorGameMapLock;

	vector<_tstring> m_vecBanList;

	FUNC_CreateProcessWithDll m_CreateProcessWithDll;
	HMODULE m_hModOverlayStub;

	bool m_bCategoryExpanded[3];	//indicate lastplay, favorites, allgames expand state

	_tstring m_sSelectedApp;

};

extern CThirdPartyAppManager g_appManager;

