#pragma once

#include <tchar.h>
#include <vector>
#include "tstring/tstring.h"
#include "tinyxml/tinyxml.h"
#include "json/elements.h"
#include "data/ThirdAppMessage.h"

using namespace String;

enum ThirdAppType
{
	ThirdApp_Normal,
	ThirdApp_Extend = 99,
};

enum ThirdAppTypeMask
{
	ThirdAppMask_Normal = 0x0001,
	ThirdAppMask_Extend = 0x0002
};

class IThirdApp
{
public:
	virtual ~IThirdApp(){}
	
	virtual bool Init() = 0;
	virtual bool Uninit() = 0;

	// update info
	virtual bool LoadAppInfo(TiXmlElement* pInfo,bool bUpdate) = 0;
	virtual bool LoadLocalInfo(TiXmlElement* pInfo) = 0;
	virtual bool SaveLocalInfoToXmlNode(TiXmlElement* pRoot) = 0;
	virtual bool SaveStateToJson(json::Object& obj) = 0;

	virtual bool OnMessage(ThirdAppMsgHeader& msg) = 0;

	// query attribute
	virtual ThirdAppType		GetType() = 0;
	virtual const _tstring		GetAppName() = 0;
	virtual const _tstring		GetAppArrb() = 0;
	virtual const _tstring		GetAppBkFile() = 0;
	virtual const _tstring  	GetAppLogoFile() = 0;
	virtual const _tstring		GetAppStrId() = 0;
	virtual const int			GetAppIntId() = 0;
	virtual const _tstring		GetAppLanguage() = 0;
	virtual const SYSTEMTIME	GetAddTime() = 0;

	virtual const _tstring 		GetPopupInfoLink() = 0;
	virtual const _tstring 		GetNewsLink() = 0;
	virtual _tstring			GetProductUrl() = 0;
	virtual bool				GetIsPreLoad() = 0;
	// query state
	//virtual bool IsHide() = 0;
	virtual bool IsInstalling() = 0;
	virtual bool IsExtracting() = 0;
	virtual bool IsPlaying() = 0;
	virtual bool IsDownloading() = 0;
	virtual bool IsUninstalling() = 0;
	virtual bool IsFullScreen() = 0;
	virtual bool IsWindowed() = 0;
	virtual bool IsResouceReady() = 0;
	virtual bool IsSaveLocalInfo() = 0;
	virtual bool IsInstalled() = 0;
	virtual bool IsPreLoad() = 0;
	virtual bool IsEnteredGame() = 0;


	virtual bool CanPlaying() = 0;

	virtual void AddToMyList() = 0;
	virtual void RemoveFromMyList() = 0;
	virtual bool IsInMyGameList() = 0;
	virtual bool GetActive() = 0;
	virtual void SetActive(bool bActive) = 0;
	virtual DWORD GetLocalStatus() = 0;
	virtual void SetLocalStatus(DWORD dwStatus) = 0;

	virtual int  GetUIOrder() = 0;
	virtual void SetUIOrder(int nIndex) = 0;
	virtual bool IsNewApp() = 0;
	virtual void ResetNewApp() = 0;
	//virtual void Hide() = 0;
	virtual bool GetIsFavorite() = 0;

	virtual bool CheckUninstall() = 0;
};
