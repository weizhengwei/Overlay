#pragma once

#include "tstring/tstring.h"
#include "data/WebService.h"
#include "thread/thread.h"
#include "thread/WQueue.h"

class CTracking : public Thread
{
public:

	static const int RESEND_TIMES = 3 ;

	enum
	{
		TRACK_INSTALL_CC ,
		TRACK_UNINSTALL_CC ,
		TRACK_USER_LOGIN ,
		TRACK_KEEPLIVE_LOGIN ,
		TRACK_USE_OVERLAY ,
		TRACK_DOWNLOAD_GAME ,
		TRACK_INSTALL_GAME ,
		TRACK_UNINSTALL_GAME ,
		TRACK_LAUNCH_GAME,
		TRACK_NOTIFICATION_CLICK,
		TRACK_NOTIFICATION_DISPLAY,
		TRACK_SYSTEM_INFO_SURVEY,
		TRACK_INSTALL_GAME_ERROR, 
		TRACK_USAGE_STATISTICS, 
		TRACK_GAME_CCU_BEGIN,
		TRACK_GAME_CCU_UPDATE,
		POST_PLAYGAME_INFO ,
		TRACK_USER_LOGIN_NEW,
		TRACK_GAME_CCU_NEW,
		TRACK_COUNTER_NEW
	} ;

	typedef struct _TRACK_INFO
	{
		int nType ;
		int nRetry ;        //re-send if failed, retry number is less than 3 times
		_tstring sUrl ;
		_tstring sPostData ;
		_TRACK_INFO()
		{
           nType = nRetry = 0;
           sUrl = sPostData = _T("");
		}

	}TRACK_INFO, *PTRACK_INFO ;

	CTracking():m_bStop(true),m_sTrackingId(_tstring()),m_sNotifyIdList(_tstring()) {} ;
	virtual ~CTracking(){} ;

	bool Init() ;
	void Uninit(int code = 0) ;
	
	bool UserLogin(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync = true) ;
	bool UserLoginKeepLive(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync = false) ;
	bool UseOverlayIngame(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr) ;
	bool GameDownloaded(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nDownloadTool) ;
	bool GameInstalled(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr) ;
	bool GameInstallError(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nErrorCode, int nDownloadType = -1, _tstring sComment = _T(""), bool bSync = false) ;
	bool GameLaunched(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nDownloadTool) ;
	bool GameUninstalled(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nDownloadTool, bool bSync = false) ;
	bool NotificationsDisplayed(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sNotifyIdList, bool bSync = false) ;
	bool NotificationClicked(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sNotifyId, _tstring sClickFrom) ;
	bool SystemInfoSurvey(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sMacAddress, _tstring sSysInfo) ;
	bool InstallUninstallSoftware(_tstring sUrl, _tstring sLang) ;
    bool UsageStatistics(_tstring sUrl, _tstring sUsername, _tstring sType, _tstring sSubType, UINT nValue=1) ;
    bool FocusTimeStatistics(_tstring sUrl, _tstring sUsername,bool bEnd=false,int nMinSeconds=60);
    bool GameCCUStatistics(_tstring sUrl,_tstring sUserName,_tstring slanguage,_tstring sGameAbbr,bool bBegin);
	_tstring GetNotifyIdList() ;
	void ClearNotifyIdList() ;
	void PushNotifyIdList(_tstring sId, int nShowCmd, bool bGamePlaying) ;
	bool IsInternalTest();
	virtual long run();
	void GenerateRandString();
	void GenerateNewRandString();
	bool PostPlayGameInfo(_tstring sUrl, _tstring sUsername, _tstring sGameAbbr, _tstring sGameName, _tstring sGameLang);

	bool UserLoginNew(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync = true) ;
	bool GameCCUNew(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, bool bSync = true) ;
	bool CounterTrackingNew(_tstring sUrl, _tstring sCounterName, _tstring sLanguage, bool bSync = true);
protected:
	bool   m_bStop ;
	WQueue<TRACK_INFO*> m_wTaskQueue ;

private:

	int t, r ;
	char vcode[32] ;
	CWebService m_sWebSvr ;
	_tstring m_sTrackingId;
	Mutex m_lockNotifyIdList ;
	_tstring m_sNotifyIdList ;
    std::map<_tstring,_tstring> m_mapSessionId;//<gameabbr,sessionid>
};

extern CTracking g_theTracking ;

// define for game install track
#define SendGameInstallTrack(abbr, lang, errCode) (g_theTracking.GameInstallError(theUIString._GetStringFromId( \
	_T("IDS_CONFIG_TRACK_INSTALL_GAME_ERROR")), theDataPool.GetUserInput(), \
	lang, abbr, errCode))

#define SendGameInstallTrackEx(abbr, lang, errCode, ...) (g_theTracking.GameInstallError(theUIString._GetStringFromId( \
	_T("IDS_CONFIG_TRACK_INSTALL_GAME_ERROR")), theDataPool.GetUserInput(), \
	lang, abbr, errCode, __VA_ARGS__))

#define GAMETRACK_DOWNLOAD_START		1
#define GAMETRACK_DOWNLOAD_STARTED		2
#define GAMETRACK_DOWNLOAD_STARTFAIL	3
#define GAMETRACK_DOWNLOAD_CONTINUE		4
#define GAMETRACK_DOWNLOAD_AUTORETRY	5
#define GAMETRACK_ARC_START				7
#define GAMETRACK_ARC_EXIT				8
#define GAMETRACK_DOWNLOAD_PAUSE		11
#define GAMETRACK_DOWNLOAD_RESUME		12
#define GAMETRACK_DOWNLOAD_CANCEL		13
#define GAMETRACK_DOWNLOAD_NOSPACE		14
#define GAMETRACK_DOWNLOAD_PATHERROR	15
#define GAMETRACK_DOWNLOAD_FINISH		16
#define GAMETRACK_PREUNZIP_NOSPACE		20
#define GAMETRACK_UNZIP_NOZIPFILE		21
#define GAMETRACK_UNZIP_NOSPACE			22
#define GAMETRACK_UNZIP_MD5ERROR		23
#define GAMETRACK_UNZIP_OTHERREASON		24
#define GAMETRACK_UNZIP_FINISH			25
#define GAMETRACK_UNZIP_FILEINUSE		26
#define GAMETRACK_UNZIP_FILEOPENFAIL	27
#define GAMETRACK_INSTALL_XMLERROR		30
#define GAMETRACK_INSTALL_WRITEREGFAIL	31
#define GAMETRACK_INSTALL_START			32
#define GAMETRACK_INSTALL_CANCEL		33
#define GAMETRACK_INSTALL_FINISH		34
#define GAMETRACK_PLAY					35

#define GAMETRACK_AKAMAI_NOSPEED		40
#define GAMETRACK_AKAMAI_ERRSTATUS		45
#define GAMETRACK_AKAMAI_ERROR			50

//AK_ERR_INVALIDARGUMENT,				51
//
//AK_ERR_NOTINSTALLED,					52
//
//AK_ERR_INTERNAL,						53
//
//AK_ERR_INSUFFICIENTMEMORY,			54
//
//AK_ERR_NETWORK,						55
//
//AK_ERR_DISK,							56
//
//AK_ERR_PLATFORM,						57
//
//AK_ERR_START,							58
//
//AK_ERR_FILEINUSE,						59
//
//AK_ERR_FILENOTFOUND,					60
//
//AK_ERR_UNABLE_TO_DOWNLOAD,			61
//
//AK_ERR_DISK_WRITE,					62
//
//AK_ERR_FILE_DOWNLOADED,				63
//
//AK_ERR_RUN_TASKS_THROTTLED,			64
//
//AK_ERR_CHALLENGE_RESPONSE,			65
//
//AK_ERR_PACKAGE_INSTALL,				66
//
//AK_ERR_IMAGE_MOUNT,					67
//
//AK_ERR_INSTALLER_FAIL					68

#define GAMETRACK_AKAMAI_NULLPOINTER	70

#define GAMETRACK_HTTP_RETRYOK			75
#define GAMETRACK_HTTP_RETRYFAIL		76

//#define GAMETRACK_HTTP_ERROR			80
//#define GAMETRACK_HTTP_ERRORMAX		166

//#define GAMETRACK_PANDO_ERROR			200
//#define GAMETRACK_PANDO_ERRORMAX		1199

//#define GAMETRACK_HTTP2_ERROR			1200
//#define  GAMETRACK_HTTP2_ERRORMAX		1220

#define  GAMETRACK_SERVICE_FAIL			1250
#define  GAMETRACK_RESOURCE_LOAD		1300

#define GAMETRACE_PDL_PASS_LANCHPOINT	2000
#define GAMETRACE_PDL_PLAY_START		2001
#define GAMETRACE_PDL_PLAY_END			2002
#define GAMETRACE_PDL_DOWNLOAD_ERROR	2003
#define GAMETRACE_PDL_SPEED_TRACE		2004
#define GAMETRACE_PDL_INTERNAL_TRACE	2005
#define GAMETRACE_NOTCHOOSE_PDLINSTALL	2006
#define GAMETRACE_CHOOSE_PDLINSTALL		2007