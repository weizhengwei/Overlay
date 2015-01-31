#pragma once

#include "data/IThirdApp.h"

#define DEF_PROPERTY_RW(typeName, name, dataName) \
	virtual const typeName Get##name(){LockGuard<Mutex> lock(this); return dataName;}   \
	virtual void  Set##name(const typeName& data){lock(); dataName = data; unlock(); Notify();}

class DataBase;

struct IDataChangeNotify
{
	virtual ~IDataChangeNotify(){}

	virtual void OnDataChange(DataBase*) = 0;
};

class DataBase : public Mutex, public IDataChangeNotify
{
public:
	DataBase(IDataChangeNotify* notify=NULL):m_notify(notify){}
	virtual ~DataBase(){}

	void Notify(){if (m_notify) m_notify->OnDataChange(this);}
private:
	virtual void OnDataChange(DataBase* pData);
	IDataChangeNotify* m_notify;
};

struct APP_INFO : public DataBase
{
private:
	int nId;
	_tstring sId;
	_tstring sType;
	_tstring sName;
	_tstring sLinkName;		//used to check if a url related to this game
	_tstring sAbbr;

	_tstring sLanguage;
	_tstring sLogoFile;
	_tstring sLogoLink;
	_tstring sProduceUrl;
	_tstring sInstallPicLink;
	_tstring sInstallPicFile;
	_tstring sBgFile;
	_tstring sBgLink;

	_tstring sHomeLink;
	_tstring sNewsLink;
	_tstring sInfoLink;
	_tstring sForumLink;
	_tstring sPopupInfoLink;
	_tstring sMediaLink;

	_tstring sHttpUrl;
	_tstring sDownFileName;
	_tstring sDownFileSize;
	_tstring sInstallSize;
	_tstring sDownFileCheckSum;

	bool bCheckMD5;

	int nStatus;	//up,cb,ob,live,patch

	int nBypassLogin;	//0,1 has the same meaning with before, 2 apply for NeverWinter
	bool bSupportOverlay;
	bool bSupportRepair;
	_tstring sRegistryPath;

	_tstring exeName;

	bool bNewPopup;
	int nNewApp;	//=1 to popup new game dialog

	//PDL 
	bool bSupportPDL;
	bool bEnablePDL;
	_tstring sPDLTotleInitSize;
	_tstring sPDLDownloadURL;

	//PreLoad
	bool bPreLoad;
	_tstring sPreLoadNotice;
public:
	APP_INFO();

	DEF_PROPERTY_RW(bool,IsNewPopup, bNewPopup);
	DEF_PROPERTY_RW(int, NewApp, nNewApp);

	DEF_PROPERTY_RW(int,IntId, nId);
	DEF_PROPERTY_RW(_tstring,StringId, sId);
	DEF_PROPERTY_RW(_tstring,Type, sType);
	DEF_PROPERTY_RW(_tstring,Name, sName);
	DEF_PROPERTY_RW(_tstring, Abbr, sAbbr);
	DEF_PROPERTY_RW(_tstring, LinkName, sLinkName);
	DEF_PROPERTY_RW(_tstring, Language, sLanguage);
	DEF_PROPERTY_RW(_tstring, LogoFile, sLogoFile);
	DEF_PROPERTY_RW(_tstring, LogoLink, sLogoLink);
	DEF_PROPERTY_RW(_tstring, ProductUrl, sProduceUrl);
	DEF_PROPERTY_RW(_tstring, InstallPicLink, sInstallPicLink);
	DEF_PROPERTY_RW(_tstring, InstallPicFile, sInstallPicFile);
	DEF_PROPERTY_RW(_tstring, BgFile, sBgFile);
	DEF_PROPERTY_RW(_tstring, BgLink, sBgLink);

	DEF_PROPERTY_RW(_tstring, HomeLink, sHomeLink);
	DEF_PROPERTY_RW(_tstring, NewsLink, sNewsLink);
	DEF_PROPERTY_RW(_tstring, InfoLink, sInfoLink);
	DEF_PROPERTY_RW(_tstring, ForumLink, sForumLink);
	DEF_PROPERTY_RW(_tstring, PopupInfoLink, sPopupInfoLink);
	DEF_PROPERTY_RW(_tstring, MediaLink, sMediaLink);

	DEF_PROPERTY_RW(_tstring, DownloadHttpUrl, sHttpUrl);
	DEF_PROPERTY_RW(_tstring, DownFileName, sDownFileName);
	DEF_PROPERTY_RW(_tstring, DownFileSize, sDownFileSize);
	DEF_PROPERTY_RW(_tstring, InstallSize, sInstallSize);
	DEF_PROPERTY_RW(_tstring, DownFileCheckSum, sDownFileCheckSum);

	DEF_PROPERTY_RW(bool, IsCheckMD5, bCheckMD5);
	DEF_PROPERTY_RW(int,  BaseStatus, nStatus);
	DEF_PROPERTY_RW(int,  BypassLogin, nBypassLogin);
	DEF_PROPERTY_RW(bool, IsSupportOverlay, bSupportOverlay);
	DEF_PROPERTY_RW(bool, IsSupportRepair, bSupportRepair);

	DEF_PROPERTY_RW(_tstring, Registry, sRegistryPath);
	DEF_PROPERTY_RW(_tstring, ExeName, exeName);

	//PDL
	DEF_PROPERTY_RW(_tstring, PDLTotalDownloadSize, sPDLTotleInitSize);
	DEF_PROPERTY_RW(_tstring, PDLDownloadURL, sPDLDownloadURL);
	DEF_PROPERTY_RW(bool,IsSupportPDL, bSupportPDL);
	DEF_PROPERTY_RW(bool,IsEnablePDL, bEnablePDL);

	DEF_PROPERTY_RW(bool,IsPreLoad, bPreLoad);
	DEF_PROPERTY_RW(_tstring, PreLoadNotice, sPreLoadNotice);
};

//-----------------------------

struct APP_LOCAL_INFO : public DataBase
{
	enum DOWNLOAD_STATE{DOWNLOADING = 0, PAUSED, INVALIDE_DATA};
private:
	int nStatus;
	int nDownTool;
	int nTraceDownloadTool;
	int nOrder;		//the order in game list
	//bool bHide;
	bool bActive;	

	bool bPassLanuchPoint;

	_tstring sDownloadUrl;
	_tstring sDownFileName;
	_tstring sDownFileSize;
	_tstring sInstallSize;
	_tstring sDownFileCheckSum;
	int		 nDownloadProgress;
	DOWNLOAD_STATE	eDownloadState; // 0 表示downloading 1 表示paused

	_tstring sVersion;
	_tstring sEulaVersion;
	_tstring sInstallDate;
	_tstring sDownloadTime;
	SYSTEMTIME addedtime;
	SYSTEMTIME lastplaytime;

	_tstring sDownloadPath;
	_tstring sInstallPath;
	_tstring sAppClientFilePath;
	_tstring sAppClientFileCmd;
	_tstring sAppLauncherPath;
	_tstring sAppLauncherCmd;
	_tstring sAppPatcherPath;
	_tstring sAppPatcherCmd;
	_tstring sAppUninstallFilePath;
	_tstring sAppUninstallFileCmd;

	_tstring qacmd;	//for cryptic games testing

	bool	 bCreateShortcut;
	bool	 bLaunchAfterInstall;

public:
	APP_LOCAL_INFO();

	DEF_PROPERTY_RW(int, LocalStatus, nStatus);
	DEF_PROPERTY_RW(int, DownTool, nDownTool);
	DEF_PROPERTY_RW(int, TraceDownloadTool, nTraceDownloadTool);
	DEF_PROPERTY_RW(int, Order, nOrder);
	//DEF_PROPERTY_RW(bool,IsHide, bHide);
	DEF_PROPERTY_RW(bool, IsActive, bActive);
	DEF_PROPERTY_RW(bool,IsPassLanuchPoint, bPassLanuchPoint)

	DEF_PROPERTY_RW(_tstring, LocalDownloadUrl, sDownloadUrl);
	DEF_PROPERTY_RW(_tstring, LocalDownFileName, sDownFileName);
	DEF_PROPERTY_RW(_tstring, LocalDownFileSize, sDownFileSize);
	DEF_PROPERTY_RW(_tstring, LocalInstallSize, sInstallSize);
	DEF_PROPERTY_RW(_tstring, LocalDownFileChecksum, sDownFileCheckSum);
	DEF_PROPERTY_RW(int, DownloadProgress, nDownloadProgress);
	DEF_PROPERTY_RW(DOWNLOAD_STATE, DownloadState, eDownloadState);// only for PDL

	DEF_PROPERTY_RW(_tstring, Version, sVersion);
	DEF_PROPERTY_RW(_tstring, EulaVersion, sEulaVersion);
	DEF_PROPERTY_RW(_tstring, InstallDate, sInstallDate);
	DEF_PROPERTY_RW(_tstring, DownloadTime, sDownloadTime);
	DEF_PROPERTY_RW(SYSTEMTIME, Addedtime, addedtime);
	DEF_PROPERTY_RW(SYSTEMTIME, Lastplaytime, lastplaytime);

	DEF_PROPERTY_RW(_tstring, DownloadPath,sDownloadPath);
	DEF_PROPERTY_RW(_tstring, InstallPath, sInstallPath);
	DEF_PROPERTY_RW(_tstring, ClientPath,sAppClientFilePath); 
	DEF_PROPERTY_RW(_tstring, ClientCmd,sAppClientFileCmd);
	DEF_PROPERTY_RW(_tstring, LauncherPath, sAppLauncherPath);
	DEF_PROPERTY_RW(_tstring, LauncherCmd, sAppLauncherCmd);
	DEF_PROPERTY_RW(_tstring, PatcherPath, sAppPatcherPath);
	DEF_PROPERTY_RW(_tstring, PatcherCmd, sAppPatcherCmd);
	DEF_PROPERTY_RW(_tstring, UninstallPath, sAppUninstallFilePath);
	DEF_PROPERTY_RW(_tstring, UninstallCmd, sAppUninstallFileCmd);

	DEF_PROPERTY_RW(_tstring, QACmd, qacmd);

	DEF_PROPERTY_RW(bool,	  IsCreateShortcut, bCreateShortcut);
	DEF_PROPERTY_RW(bool,	  IsLaunchAfterInstall, bLaunchAfterInstall);
};


class ThirdAppBase  : public IThirdApp
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
public:
	virtual ~ThirdAppBase(void){}

	virtual bool Init() = 0;
	virtual bool Uninit() = 0;

	virtual bool LoadAppInfo(TiXmlElement* pInfo, bool bUpdate) = 0;
	virtual bool LoadLocalInfo(TiXmlElement* pInfo) = 0;
	virtual bool SaveLocalInfoToXmlNode(TiXmlElement* pRoot) = 0;
	virtual bool SaveStateToJson(json::Object& obj) = 0;

	virtual bool OnMessage(ThirdAppMsgHeader& msg){return 0;}

	// query attribute
	virtual ThirdAppType	GetType() = 0;
	virtual const _tstring	GetAppName(){return GetAppInfo().GetName();}
	virtual const _tstring	GetAppArrb(){return GetAppInfo().GetAbbr();}
	virtual const _tstring	GetAppStrId(){return GetAppInfo().GetStringId();}
	virtual const int		GetAppIntId(){return GetAppInfo().GetIntId();}
	virtual const _tstring	GetAppLanguage(){return GetAppInfo().GetLanguage();}
	virtual const SYSTEMTIME GetAddTime(){return GetLocalInfo().GetAddedtime();}
	virtual const _tstring	GetAppBkFile() = 0;
	virtual const _tstring  GetAppLogoFile() = 0;
	const _tstring GetPopupInfoLink() {return GetAppInfo().GetPopupInfoLink();}
	const _tstring GetNewsLink(){return GetAppInfo().GetNewsLink();}
	_tstring GetProductUrl(){return GetAppInfo().GetProductUrl();}
	bool	GetIsPreLoad() { return GetAppInfo().GetIsPreLoad();}
	// query state
	virtual bool IsInMyGameList() = 0;
	//virtual bool IsHide() = 0;
	virtual bool IsInstalling() = 0;
	virtual bool IsExtracting() = 0;
	virtual bool IsPlaying() = 0;
	virtual bool IsDownloading() = 0;
	virtual bool IsUninstalling() = 0;
	virtual bool IsNewApp() = 0;
	virtual bool IsFullScreen() = 0;
	virtual bool IsWindowed() = 0;
	virtual bool IsResouceReady() = 0;
	virtual bool IsPreLoad() = 0;
	virtual bool IsEnteredGame() = 0;

	virtual void ResetNewApp() = 0;
	virtual void AddToMyList();
	virtual void RemoveFromMyList();

	virtual bool IsInstalled() = 0;

	virtual int  GetUIOrder(){return GetLocalInfo().GetOrder();};
	virtual void SetUIOrder(int nIndex){return GetLocalInfo().SetOrder(nIndex);}

	virtual APP_INFO& GetAppInfo() = 0;
	virtual APP_LOCAL_INFO& GetLocalInfo() = 0;

	virtual bool GetActive() = 0;
	virtual void SetActive(bool bActive) = 0;

	virtual DWORD GetLocalStatus() = 0;
	virtual void SetLocalStatus(DWORD dwStatus) = 0;
};
