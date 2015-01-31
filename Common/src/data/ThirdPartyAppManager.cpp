///*************************************************
/**
 * @file      ThirdPartyAppManager.cpp
   @brief     The implementation file of third party application manager.
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

#include "stdafx.h"
#include "data/ThirdPartyAppManager.h"
#include "data/DataPool.h"
#include "constant.h"
#include <shlobj.h>
#include <shlwapi.h>
#include "data/GameInfo.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"
#include "data/WebData.h"
#include "data/SNSManager.h"
#include "tinyxml/tinystr.h"
#include "tinyxml/tinyxml.h"
#include "download/IDownLoader.h"
#include "data/Tracking.h"
#include "data/UIString.h"
#include "data/Utility.h"
#include <algorithm>
#include "data/ThirdPartyApp.h"
#include "ui/UIList.h"
#include <algorithm>
#include "log/local.h"
#include "download/PDLDownloader.h"

//---------------------------
#define	FILE_CFG_GAME			_T("gameinfo.xml")
#define	FILE_CFG_FIRST_PLAYING	_T("ArcPlayingInfo.xml")
//windows srwlock is not available

#define ___AutoSRWBlockRead_AppMap____ {MutexGuard xxxxx_xxx(&m_appMapLock);
#define ___AutoSRWBlockWrite_AppMap____ {MutexGuard xxxxx_xxx(&m_appMapLock);
#define ___AutoSRWBlockRead_ExtendMap____ {MutexGuard xxxxx_xxx(&m_externalMapLock);
#define ___AutoSRWBlockWrite_ExtendMap____ {MutexGuard xxxxx_xxx(&m_externalMapLock);
#define ___AutoSRWBlockEnd____   }

#define MONITOR_RETRY_CLOSE_TIMES   10
#define MONITOR_ESCAPSE				200

//////////////////////////////////////////////////////////////////////////
_tstring GetLocalInfoPath()
{
	TCHAR szPath[MAX_PATH] = _T("");
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		return _T("");
	}
	
	PathAppend(szPath, PRODUCT_NAME);
		
	return _tstring(szPath);
}

_tstring GetFirstPlayingPath()
{
	TCHAR szPath[MAX_PATH] = _T("");
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		return _T("");
	}

	PathAppend(szPath, PRODUCT_NAME);
	PathAppend(szPath, FILE_CFG_FIRST_PLAYING);

	return _tstring(szPath);
}

struct ARC_LANGUAGE
{
	DWORD nId;
	_tstring sAbbr;
	_tstring sLang;
};

ARC_LANGUAGE ArcLanguageArray[] =
{
	{0, _T("en"), _T("english")},
	{1, _T("de"), _T("german")},
	{2, _T("fr"), _T("french")},
	{3, _T("pt"), _T("portuguese")},
	{4, _T("ru"), _T("russian")},
	{5, _T("tr"), _T("turkish")},
};

#define ARC_LANGUAGE_COUNT (sizeof(ArcLanguageArray)/sizeof(ARC_LANGUAGE))

//////////////////////////////////////////////////////////////////////////
CThirdPartyAppManager g_appManager;

CThirdPartyAppManager::CThirdPartyAppManager()
: m_nExternalId(50000),
m_pAppListWnd(NULL)
{

	m_autoResumeVecLock.lock();
	m_autoResumeVec.clear();
	m_autoResumeVecLock.unlock();

	for (int i=0; i<EventCount-1; ++i)
	{
		m_events[i] = CreateEvent(NULL, TRUE,FALSE, NULL);
	}

	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (hTimer==NULL)
	{
		OutputDebugString(_T("CreateWaitableTimer faied!"));
		return;
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart= 0;
	
	if (!SetWaitableTimer(hTimer, &liDueTime, MONITOR_ESCAPSE, NULL, NULL, 0))
	{
		OutputDebugString(_T("SetWaitableTimer faied"));
		hTimer = NULL;
		return ;
	}

	m_events[EventCount - 1] = hTimer;
	m_pCurDownloadingPDLApp = NULL;

	m_hModOverlayStub = NULL;
	m_CreateProcessWithDll = NULL;

	for(int i = 0; i < 3; i++)
	{
		m_bCategoryExpanded[i] = true;
	}
}

CThirdPartyAppManager::~CThirdPartyAppManager()
{
	for (int i=0; i<EventCount; ++i)
	{
		CloseHandle(m_events[i]);
		m_events[i] = NULL;
	}

	m_pAppListWnd = NULL;
	m_pCurDownloadingPDLApp = NULL;
}

bool CThirdPartyAppManager::Init()
{
	m_hModOverlayStub = LoadLibrary(FILE_DLL_OVERLAYSB);
	if(!m_hModOverlayStub)
	{
		return false;
	}
	m_CreateProcessWithDll = (FUNC_CreateProcessWithDll)GetProcAddress(m_hModOverlayStub, ("CoreCreateProcessWithDll"));
	if(!m_CreateProcessWithDll)
	{
		FreeLibrary(m_hModOverlayStub);
		m_hModOverlayStub = NULL;
		return false;
	}

	if(!LoadAppData())
	{
		return false;
	}

	LoadAppLocalData();

	m_sFirstPlayingInfoPath = GetFirstPlayingPath();
	if (m_sFirstPlayingInfoPath.empty())
	{
		return false;
	}
	
	___AutoSRWBlockRead_AppMap____
	ThirdAppMap::iterator it;
	for(it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		AppLangMap::iterator itAppLang;
		for(itAppLang = pAppLangMap->begin(); itAppLang != pAppLangMap->end(); itAppLang++)
		{
			(*itAppLang).second->Init();
		}
	}
	___AutoSRWBlockEnd____
	
	SaveAppLocalData(m_sLocalInfoPath);
	return start();
}

bool CThirdPartyAppManager::LoadAppData()
{
	int nLocalVersion = 0;
	int nLatestVersion = 0;

	// get app config file path
	m_sAppInfoPath = theDataPool.GetBaseDir() + FILE_CFG_GAME;

	TiXmlDocument xmlDoc;
	TiXmlElement *pRoot = NULL, *pElement = NULL;

	if(CUtility::LoadEncryptedXML(&xmlDoc, m_sAppInfoPath.c_str()))
	{
		pRoot = xmlDoc.RootElement();

		pElement = pRoot->FirstChildElement("version");
		if(pElement)
		{
			nLocalVersion = atoi(pElement->GetText());
		}
	}
	else
	{
		SendGameInstallTrackEx(_T("Arc"), theDataPool.GetLangAbbrString(), GAMETRACK_SERVICE_FAIL, -1, _T("load local gameinfo fail"));
	}
		
	if(g_theWebData.GetGameinfoVersion(theDataPool.GetLangAbbrString(), nLatestVersion))
	{
		if(nLocalVersion != nLatestVersion)
		{
			_tstring sData;
			if(g_theWebData.GetAllGameinfo(theDataPool.GetLangAbbrString(), sData))
			{
				pRoot = NULL;
				pElement = NULL;
				xmlDoc.Clear();
				
				// parse gameinfo
				xmlDoc.Parse(sData.toNarrowString().c_str());
				//xmlDoc.SaveFile(m_sAppInfoPath.toNarrowString().c_str());
				CUtility::SaveEncryptedXML(&xmlDoc, m_sAppInfoPath.c_str());

				pRoot = xmlDoc.RootElement();
			}
			else
			{
				SendGameInstallTrackEx(_T("Arc"), theDataPool.GetLangAbbrString(), GAMETRACK_SERVICE_FAIL, -1, _T("get gameinfo fail"));
			}
		}
	}
	else
	{
		SendGameInstallTrackEx(_T("Arc"), theDataPool.GetLangAbbrString(), GAMETRACK_SERVICE_FAIL, -1, _T("get version fail"));
	}

	if(!pRoot)
	{
		SendGameInstallTrackEx(_T("Arc"), theDataPool.GetLangAbbrString(), GAMETRACK_SERVICE_FAIL, -1, _T("load gameinfo fail"));
		return false;
	}

	pElement = pRoot->FirstChildElement("game");
	while(pElement)
	{
 		CThirdPartyApp *pApp = new CThirdPartyApp;
		if(pApp->LoadAppInfo(pElement,false))
		{
			___AutoSRWBlockWrite_AppMap____
			ThirdAppMap::iterator it = m_mapApps.find(pApp->GetAppArrb());
			AppLangMap *pAppLangMap = NULL;
			if(it == m_mapApps.end())
			{
				pAppLangMap = new AppLangMap;
				m_mapApps[pApp->GetAppArrb()] = pAppLangMap;
			}
			else
			{
				pAppLangMap = (*it).second;
			}
			if(pAppLangMap)
			{
				_tstring sLang = pApp->GetAppLanguage();
				pAppLangMap->insert(make_pair(sLang, pApp));
			}
			___AutoSRWBlockEnd____
		}
		else
		{
			delete pApp;
		}

		pElement = pElement->NextSiblingElement("game");
	}

	xmlDoc.Clear();

	return true;
}

bool CThirdPartyAppManager::UpdateGameinfo(const char* pData)
{
	TiXmlDocument xmlDoc ;

	xmlDoc.Parse(pData) ;

	TiXmlElement *pRoot = xmlDoc.RootElement();
	if(!pRoot)
		return false;

	for (TiXmlElement* pInfo=pRoot->FirstChildElement("game"); pInfo!=NULL; pInfo=pInfo->NextSiblingElement("game"))
	{
		___AutoSRWBlockWrite_AppMap____
		// update data
		const char *pid = pInfo->Attribute("id");
		const char *pabbr = pInfo->Attribute("abbr");
		const char *plang = pInfo->Attribute("lang");

		if(!pid || !pabbr || !plang)
		{
			return false;
		}

		_tstring sAbbr = pabbr;
		_tstring sLang = plang;
		if(sAbbr.empty() || sLang.empty())
		{
			return false;
		}

		int nId = atoi(pInfo->Attribute("id"));
		if(nId <= 0)
			return false;

		IThirdApp *pApp = NULL;
		bool bNewApp = false;
		
		pApp = GetThirdAppByAbbrLang(sAbbr, sLang);
		if(!pApp)
		{
			pApp = new CThirdPartyApp();
			bNewApp = true;
		}

		if(!pApp->LoadAppInfo(pInfo, true))
		{
			if(bNewApp)
			{
				delete pApp;
				return false;
			}
		}

		if(bNewApp)
		{
			InsertThirdApp(static_cast<CThirdPartyApp*>(pApp));
			pApp->Init();
		}
		
		if(m_pAppListWnd)
		{
			m_pAppListWnd->UpdateGame(sAbbr, sLang);
		}
		___AutoSRWBlockEnd____
	}

	g_appManager.SetUpdateData();	//update game icon
	return true;
}

bool CThirdPartyAppManager::Uninit()
{
	m_msgServer.Uninitialize();

	SetEvent(m_events[EventExit]);
	WaitForSingleObject(_threadHandle, INFINITE);

	// force store local info
	SaveAppLocalData(m_sLocalInfoPath);
	SaveFirstPlayingInfo(m_sFirstPlayingInfoPath);
 
	___AutoSRWBlockWrite_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			(*iter).second->Uninit();
			delete (*iter).second;
		}
		pAppLangMap->clear();
		delete pAppLangMap;
	}
	m_mapApps.clear();
	___AutoSRWBlockEnd____
	
	___AutoSRWBlockWrite_ExtendMap____
	for(MapItor iter = m_mapExternalApp.begin(); iter != m_mapExternalApp.end(); iter++)
	{
		(*iter).second->Uninit();
		delete (*iter).second;
	}
	m_mapExternalApp.clear();
	___AutoSRWBlockEnd____

	if(m_hModOverlayStub)
	{
		FreeLibrary(m_hModOverlayStub);
		m_hModOverlayStub = NULL;
		m_CreateProcessWithDll = NULL;
	}

	return true;
}

bool CThirdPartyAppManager::LoadAppLocalData()
{
	_tstring sLocalPath = GetLocalInfoPath();
	if (sLocalPath.length() <= 0)
	{
		return false;
	}
	if(sLocalPath[sLocalPath.length()-1] != '\\')
	{
		sLocalPath += '\\';
	}
	m_sLocalInfoPath = sLocalPath + FILE_CFG_GAME;

	if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(m_sLocalInfoPath.c_str()))
	{
		//load all the files
		for(DWORD n = 0; n < ARC_LANGUAGE_COUNT; n++)
		{
			_tstring sFileIndex;
			sFileIndex.Format(1, _T("_%d"), ArcLanguageArray[n].nId);
			_tstring sFile = m_sLocalInfoPath + sFileIndex;
			LoadAppLocalData(sFile, ArcLanguageArray[n].sAbbr);
		}

		SetModified();
		return true;
	}
	else
	{
		return LoadAppLocalData(m_sLocalInfoPath);
	}
}

bool CThirdPartyAppManager::LoadAppLocalData(_tstring sLocalPath, _tstring sLangAbbr)
{
	TiXmlDocument xmlDoc;
	TiXmlElement *pRoot = NULL;
	TiXmlElement *pElement = NULL;

	if(!CUtility::LoadEncryptedXML(&xmlDoc, sLocalPath.c_str())
		|| !(pRoot = xmlDoc.RootElement()) || xmlDoc.Error())
	{
		xmlDoc.ClearError();
		xmlDoc.Clear();

		_tstring sTempPath = sLocalPath + _T(".bak");
		if(!CUtility::LoadEncryptedXML(&xmlDoc, sLocalPath.c_str())
			|| !(pRoot = xmlDoc.RootElement()) || xmlDoc.Error())
		{
			return false;
		}
	}

	TiXmlElement *pCategory = pRoot->FirstChildElement("category");
	if(pCategory)
	{
		LoadCategoryDataFromXML(pCategory);
	}
	
	pElement = pRoot->FirstChildElement("game");

	const char *ptmp = NULL;
	TiXmlElement *pSub = NULL;
	_tstring sId;
	int nId = 0;

	while(pElement)
	{
		APP_LOCAL_INFO localInfo;
		IThirdApp *pApp = NULL;

		//read external game
		ptmp = pElement->Attribute("type");
		if(ptmp && atoi(ptmp) != ThirdApp_Normal)
		{
			EXTERNALGAMEINFO egi;
			ptmp = pElement->Attribute("name");
			if(ptmp)
			{
				egi.sName.fromUTF8(ptmp);
			}

			if(pSub = pElement->FirstChildElement("path"))
			{
				ptmp = pSub->Attribute("logo");
				if(ptmp)
				{
					egi.sLogoFile = ptmp;
				}
				ptmp = pSub->Attribute("clientpath");
				if(ptmp)
				{
					egi.sGamePath = ptmp;
				}
			}
			
			if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(egi.sGamePath.c_str()))
			{
				pApp = g_appManager.AddExternalGame(egi);
				if(pApp)
				{
					pApp->LoadLocalInfo(pElement);
				}
			}
		}
		else
		{
			//read internal game
			ptmp = pElement->Attribute("abbr");
			if(ptmp)
			{
				_tstring sAbbr = ptmp;
				_tstring sLang = sLangAbbr;
				if(sLang.empty())
				{
					ptmp = pElement->Attribute("lang");
					sLang = ptmp;
				}
				pApp = GetThirdAppByAbbrLang(sAbbr, sLang);
			}
			if(pApp)
			{
				pApp->LoadLocalInfo(pElement);
			}
		}

		pElement = pElement->NextSiblingElement();
	}

	//get select game
	pElement = pRoot->FirstChildElement("selgame");
	if(pElement)
	{
		const char *pszSelAbbr = pElement->Attribute("abbr");
		const char *pszSelLang = pElement->Attribute("lang");
		if(pszSelAbbr && pszSelLang)
		{
			SetSelectedGame(MakeAbbrLangId(pszSelAbbr, pszSelLang));
		}
	}

	xmlDoc.Clear();

	return true;
}

bool CThirdPartyAppManager::SaveAppLocalData(_tstring sLocalPath)
{
	if(_tcslen(sLocalPath.c_str())<=0)
	{
		return false;
	}

	// check disk space
	TCHAR szDrive[4] = _T("C:\\") ;
	szDrive[0] = sLocalPath[0]; 
	ULARGE_INTEGER ulFreeBytes = {0} ; 
	if (GetDiskFreeSpaceEx(szDrive, &ulFreeBytes , NULL, NULL))
	{
		if(ulFreeBytes.QuadPart < 1024*1024)
		{
			return false;
		}
	}

	TiXmlDocument xmlDoc;

	TiXmlDeclaration *pDecl = new TiXmlDeclaration("1.0", "utf-8", "yes");
	xmlDoc.LinkEndChild(pDecl);

	TiXmlElement *pRoot = new TiXmlElement("root");

	TiXmlElement *pCategory = new TiXmlElement("category");
	SaveCategoryDataToXML(pCategory);
	pRoot->LinkEndChild(pCategory);

	_tstring sSelAbbr, sSelLang;
	SplitAbbrLangId(m_sSelectedApp, sSelAbbr, sSelLang);
	if(!sSelAbbr.empty() && !sSelLang.empty())
	{
		TiXmlElement *pSelGame = new TiXmlElement("selgame");
		pSelGame->SetAttribute("abbr", sSelAbbr.toUTF8().c_str());
		pSelGame->SetAttribute("lang", sSelLang.toUTF8().c_str());
		pRoot->LinkEndChild(pSelGame);
	}

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if(!pApp->GetActive())
			{
				continue;
			}

			TiXmlElement *pGame = new TiXmlElement("game");
			pApp->SaveLocalInfoToXmlNode(pGame);
			pRoot->LinkEndChild(pGame);
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	//save external game
	for(MapItor iter = m_mapExternalApp.begin(); iter != m_mapExternalApp.end(); iter++)
	{
		IThirdApp* pApp = (*iter).second;

		TiXmlElement *pGame = new TiXmlElement("game");
		pApp->SaveLocalInfoToXmlNode(pGame);
		pRoot->LinkEndChild(pGame);
	}
	___AutoSRWBlockEnd____

	xmlDoc.LinkEndChild(pRoot);

	_tstring sTempSaveFile = sLocalPath + _T(".bak");
	CopyFile(sLocalPath.c_str(), sTempSaveFile.c_str(), FALSE);

	//if(!xmlDoc.SaveFile(sLocalPath.toNarrowString().c_str()))
	if(!CUtility::SaveEncryptedXML(&xmlDoc, sLocalPath.c_str()))
	{
		OutputDebugString(_T("save local info fail\n"));
	}

	xmlDoc.Clear();

	return true;
}

void CThirdPartyAppManager::InsertThirdApp(IThirdApp *pApp)
{
	if(!pApp)
	{
		return;
	}
	_tstring sAbbr = pApp->GetAppArrb();
	_tstring sLang = pApp->GetAppLanguage();
	if(sAbbr.empty() || sLang.empty())
	{
		return;
	}

	___AutoSRWBlockRead_AppMap____
	AppLangMap *pAppLangMap = NULL;
	ThirdAppMap::iterator it = m_mapApps.find(sAbbr);
	if(it != m_mapApps.end())
	{
		pAppLangMap = it->second;
	}
	if(!pAppLangMap)
	{
		pAppLangMap = new AppLangMap;
		m_mapApps.insert(make_pair(sAbbr, pAppLangMap));
	}
	pAppLangMap->insert(make_pair(sLang, pApp));
	___AutoSRWBlockEnd____
}

IThirdApp* CThirdPartyAppManager::GetThirdAppByAbbrLang(_tstring sAbbr, _tstring sLang)
{
	if(sAbbr.empty() || sLang.empty())
	{
		return NULL;
	}

	_tstring sAbbrTemp = sAbbr;
	sAbbrTemp.MakeLower();

	_tstring sLangTemp = sLang;
	sLangTemp.MakeLower();

	___AutoSRWBlockRead_AppMap____
	ThirdAppMap::iterator it = m_mapApps.find(sAbbrTemp);
	if(it != m_mapApps.end())
	{
		AppLangMap *pAppLangMap = it->second;
		AppLangMap::iterator iter = pAppLangMap->find(sLangTemp);
		if(iter != pAppLangMap->end())
		{
			return iter->second;
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	MapItor iterEx;
	for(iterEx = m_mapExternalApp.begin(); iterEx != m_mapExternalApp.end(); iterEx++)
	{
		if((*iterEx).second->GetAppArrb().CompareNoCase(sAbbr) == 0)
		{
			return ((*iterEx).second);
		}
	}
	___AutoSRWBlockEnd____

	return NULL;
}

IThirdApp* CThirdPartyAppManager::GetThirdAppByIdLang(DWORD nId, _tstring sLang)
{
	if(nId == 0 || sLang.empty())
	{
		return NULL;
	}

	_tstring sLangTemp = sLang;
	sLangTemp.MakeLower();

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = it->second;
		AppLangMap::iterator iter = pAppLangMap->find(sLangTemp);
		if(iter != pAppLangMap->end())
		{
			IThirdApp *pApp = iter->second;
			if(pApp->GetAppIntId() == nId)
			{
				return pApp;
			}
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	MapItor iterEx = m_mapExternalApp.find(nId);
	if(iterEx != m_mapExternalApp.end())
	{
		return ((*iterEx).second);
	}
	___AutoSRWBlockEnd____

	return NULL;
}

IThirdApp* CThirdPartyAppManager::GetDefaultAppByAbbr(_tstring sAbbr)
{
	if(sAbbr.empty())
	{
		return NULL;
	}

	_tstring sAbbrTemp = sAbbr;
	sAbbrTemp.MakeLower();

	IThirdApp *pActiveApp = NULL;
	IThirdApp *pLangApp = NULL;

	___AutoSRWBlockRead_AppMap____
	ThirdAppMap::iterator it = m_mapApps.find(sAbbrTemp);
	if(it != m_mapApps.end())
	{
		AppLangMap *pAppLangMap = it->second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if(iter->second->GetActive())
			{
				pActiveApp = iter->second;
			}
			if(iter->second->GetAppLanguage() == theDataPool.GetLangAbbrString())
			{
				pLangApp = iter->second;
			}
		}
	}
	___AutoSRWBlockEnd____

	if(pActiveApp)
	{
		return pActiveApp;
	}
	if(pLangApp)
	{
		return pLangApp;
	}
	return NULL;
}

IThirdApp* CThirdPartyAppManager::GetDefaultAppById(DWORD nId)
{
	if(nId == 0)
	{
		return NULL;
	}

	IThirdApp *pActiveApp = NULL;
	IThirdApp *pLangApp = NULL;

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = it->second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if(iter->second->GetAppIntId() == nId)
			{
				if(iter->second->GetActive())
				{
					pActiveApp = iter->second;
				}
				if(iter->second->GetAppLanguage() == theDataPool.GetLangAbbrString())
				{
					pLangApp = iter->second;
				}
			}
		}
	}
	___AutoSRWBlockEnd____

	if(pActiveApp)
	{
		return pActiveApp;
	}
	if(pLangApp)
	{
		return pLangApp;
	}
	return NULL;
}

IThirdApp* CThirdPartyAppManager::GetFirstAppByAbbr(_tstring sAbbr)	
{
	_tstring sAbbrTemp = sAbbr;
	sAbbrTemp.MakeLower();

	___AutoSRWBlockRead_AppMap____
	IThirdApp *pApp = NULL;
	if(!sAbbr.empty())
	{
		ThirdAppMap::iterator it = m_mapApps.find(sAbbrTemp);
		if(it != m_mapApps.end())
		{
			AppLangMap *pAppLangMap = it->second;
			if(!pAppLangMap->empty())
			{
				pApp = pAppLangMap->begin()->second;
			}
		}
	}
	return pApp;
	___AutoSRWBlockEnd____
}

_tstring CThirdPartyAppManager::GetAppNameByAbbr(_tstring sAbbr)
{
	_tstring sAbbrTemp = sAbbr;
	sAbbrTemp.MakeLower();

	_tstring sRet;
	IThirdApp *pApp = GetFirstAppByAbbr(sAbbrTemp);
	if(pApp)
	{
		sRet = pApp->GetAppName();
	}
	return sRet;
}

bool CThirdPartyAppManager::IsAnyAppInstalling(_tstring *sAppName)
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsInstalling())
			{
				if(sAppName)
				{
					*sAppName = (*iter).second->GetAppName();
				}
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____
	return false;
}

bool CThirdPartyAppManager::IsAnyAppUninstalling(_tstring *sAppName)
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsUninstalling())
			{
				if(sAppName)
				{
					*sAppName = (*iter).second->GetAppName();
				}
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____
	return false;
}

bool CThirdPartyAppManager::IsAnyAppExtracting(_tstring *sAppName)
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsExtracting())
			{
				if(sAppName)
				{
					*sAppName = (*iter).second->GetAppName();
				}
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____
	return false;
}

bool CThirdPartyAppManager::IsAnyAppPlaying(_tstring *sAppName)
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsPlaying())
			{
				if(sAppName)
				{
					*sAppName = (*iter).second->GetAppName();
				}
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____
	return false;
}

bool CThirdPartyAppManager::IsAnyAppInProcessing()
{
	return IsAnyAppPlaying() || IsAnyAppInstalling() || IsAnyAppUninstalling() || IsAnyAppExtracting();
}

bool SortMyApp(IThirdApp* plhs, IThirdApp* prhs)
{
	if(plhs->GetIsFavorite() ^ prhs->GetIsFavorite())
	{
		return plhs->GetIsFavorite();
	}
	if (plhs->GetUIOrder() != prhs->GetUIOrder())
	{
		return (plhs->GetUIOrder() - prhs->GetUIOrder()) <= 0;
	}
	else 
	{
		FILETIME ft1, ft2;
		SystemTimeToFileTime(&(plhs->GetAddTime()), &ft1);
		SystemTimeToFileTime(&(prhs->GetAddTime()), &ft2);

		return CompareFileTime(&ft1, &ft2) <= 0;
	}

	return true;
}

IThirdApp* GetHigherPriorityApp(IThirdApp* app1, IThirdApp* app2)
{
	if(!app1 || !app2)
	{
		return NULL;
	}
	_tstring sLang = theDataPool.GetLangAbbrString();
	if(app1->GetAppLanguage() == sLang)
	{
		return app1;
	}
	if(app2->GetAppLanguage() == sLang)
	{
		return app2;
	}
	if(app1->GetLocalStatus() >= app2->GetLocalStatus())
	{
		return app1;
	}
	else
	{
		return app2;
	}
	return NULL;
}

MyGameList CThirdPartyAppManager::GetMyApps()
{
	MyGameList listMyApp;
	MyGameList::iterator iterMyApp;

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;

		IThirdApp *pActiveApp = NULL;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			CThirdPartyApp* pApp = static_cast<CThirdPartyApp*>((*iter).second);
			if(pApp->GetMemInfo().GetIsCloudGame())
			{
				pActiveApp = pApp;
				break;
			}
			if(pApp->GetLocalStatus() > ThirdAppBase::ADDED)
			{
				if(pApp->GetActive())
				{
					pActiveApp = pApp;
					break;
				}
				if(!pActiveApp)
				{
					pActiveApp = pApp;
				}
				else
				{
					pActiveApp = GetHigherPriorityApp(pActiveApp, pApp);
				}
			}
		}

		if (pActiveApp)
		{
			listMyApp.push_back(pActiveApp);
			pActiveApp->SetActive(true);
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	for(MapItor iter = m_mapExternalApp.begin(); iter != m_mapExternalApp.end(); iter++)
	{
		IThirdApp* pApp = (*iter).second;
		if(pApp->IsInMyGameList())
		{
			listMyApp.push_back(pApp);
		}
	}
	___AutoSRWBlockEnd____

	listMyApp.sort(SortMyApp);

	return listMyApp;	
}

MyGameList CThirdPartyAppManager::GetActiveApps()
{
	MyGameList listMyApp;
	MyGameList::iterator iterMyApp;

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;

		IThirdApp *pActiveApp = NULL;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if(pApp->GetActive())
			{
				pActiveApp = pApp;
				break;
			}
		}

		if (pActiveApp)
		{
			listMyApp.push_back(pActiveApp);
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	for(MapItor iter = m_mapExternalApp.begin(); iter != m_mapExternalApp.end(); iter++)
	{
		IThirdApp* pApp = (*iter).second;
		if(pApp->IsInMyGameList())
		{
			listMyApp.push_back(pApp);
		}
	}
	___AutoSRWBlockEnd____

	listMyApp.sort(SortMyApp);

	return listMyApp;	
}

IThirdApp* CThirdPartyAppManager::GetNewApp()
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsInMyGameList())
			{
				continue;
			}
			if((*iter).second->IsNewApp())
			{
				return ((*iter).second);
			}
		}
	}
	___AutoSRWBlockEnd____

	return NULL;
}

BOOL CThirdPartyAppManager::IsGamePlaying(_tstring sGameAbbr)
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if(pApp->IsPlaying())
			{
				if(sGameAbbr.empty() || sGameAbbr.CompareNoCase(pApp->GetAppArrb()) == 0)
				{
					return TRUE;
				}
			}
		}
	}
	___AutoSRWBlockEnd____

	return FALSE ;
}

bool CThirdPartyAppManager::IsAnyAppFullScreen()
{
	MyGameList mylist = GetActiveApps();
	MyGameList::iterator it;
	for(it = mylist.begin(); it != mylist.end(); it++)
	{
		if((*it)->IsEnteredGame() && (*it)->IsFullScreen())
		{
			return true;
		}
	}
	return false;
}

int CThirdPartyAppManager::GetMyAppCount()
{
	int nCount = 0;
	int ret = 0;

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if(pApp->IsInMyGameList())
			{
				nCount++;
			}
		}
	}
	___AutoSRWBlockEnd____

	___AutoSRWBlockRead_ExtendMap____
	ret = nCount+m_mapExternalApp.size();
	___AutoSRWBlockEnd____
	return ret;
}

_tstring CThirdPartyAppManager::GetGameStatus(_tstring sGameAbbr)
{
	//json::Array GameStatusArray ;

	//if (sGameAbbr.empty())
	//{
	//	return _T("");
	//}

	//MapItor iter;

	//___AutoSRWBlockRead_AppMap____
	//for(iter = m_mapAppListById.begin(); iter != m_mapAppListById.end(); iter++)
	//{
	//	IThirdApp* pApp = (*iter).second;

	//	if (pApp->GetAppArrb().CompareNoCase(sGameAbbr))
	//	{
	//		continue ;
	//	}

	//	json::Object GameStatus ;
	//	
	//	pApp->SaveStateToJson(GameStatus);

	//	GameStatusArray.Insert(GameStatus) ;
	//}
	//___AutoSRWBlockEnd____

	//std::stringstream stream ;
	//json::Writer::Write(GameStatusArray, stream) ;

	//return _tstring(stream.str()) ;

	return _T("");
}

ThridApp_PopInfo CThirdPartyAppManager::GetGameInfoPopUp(_tstring sGameAbbr)
{
	ThridApp_PopInfo popinfo = {_T(""),_T("")};
	do 
	{
		if (sGameAbbr.empty())
		{
			break;
		}
		___AutoSRWBlockRead_AppMap____
		for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
		{
			AppLangMap *pAppLangMap = (*it).second;
			for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
			{
				IThirdApp* pApp = (*iter).second;

				if (pApp->GetAppArrb().CompareNoCase(sGameAbbr) == 0)
				{
					popinfo.sinfoURL = pApp->GetPopupInfoLink();
					popinfo.sname = pApp->GetAppName();
					break;
				}
			}
		}
		___AutoSRWBlockEnd____
	} while (FALSE);
	return popinfo;
}



bool CThirdPartyAppManager::SendNotificationToGame(const ThirdApp_GameNotification& notify, IThirdApp *pApp)
{
	core_msg_notification hNotification;

	hNotification.dwCmdId = CORE_MSG_NOTIFICATION;
	lstrcpyn(hNotification.sLink,notify.szLink.c_str(),1024);
	lstrcpyn(hNotification.sNotifId,notify.szId.c_str(),1024);
	lstrcpyn(hNotification.sTitle,notify.szTitle.c_str(),1024);
	lstrcpyn(hNotification.sText,notify.szText.c_str(),1024);
	
	SendArcMessageToGameMsg msg(&hNotification,false);
	if (pApp)
	{
		//this is a notification
		return pApp->OnMessage(msg);
	}
	else
	{
		//this is a EMS message
		MyGameList gameList = GetActiveApps();
		MyGameList::iterator iter = gameList.begin();
		for ( ; iter != gameList.end(); iter++)
		{
			if((*iter)->IsEnteredGame())
			{
				(*iter)->OnMessage(msg);
			}
		}
		return true;
	}
}

bool CThirdPartyAppManager::SendInboxToGame(LPCTSTR pszType, LPCTSTR pszId, 
											LPCTSTR pszNickName, LPCTSTR pszContent, 
											LPCTSTR pszFriendLink, LPCTSTR pszContentLink, 
											LPCTSTR pszTitle, LPCTSTR pszArcMailSubject)
{
	core_msg_inbox stInbox;
	lstrcpyn(stInbox.sType, pszType, 1024);
	lstrcpyn(stInbox.sId, pszId, 1024);
	lstrcpyn(stInbox.sNickName, pszNickName, 1024);
	lstrcpyn(stInbox.sContent, pszContent, 1024);
	lstrcpyn(stInbox.sFriendLink, pszFriendLink, 1024);
	lstrcpyn(stInbox.sContentLink, pszContentLink, 1024);
	lstrcpyn(stInbox.sTitle, pszTitle, 1024);
	lstrcpyn(stInbox.sArcMailSubject, pszArcMailSubject, 1024);
	SendArcMessageToGameMsg msg(&stInbox,false);

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsPlaying())
			{
				(*iter).second->OnMessage(msg);
			}
		}
	}
	___AutoSRWBlockEnd____

	return true;
}

bool CThirdPartyAppManager::SendFriendReqToGame(LPCTSTR pszId, LPCTSTR pszNickName, 
												LPCTSTR pszContent,LPCTSTR pszFriendLink, 
												LPCTSTR pszTitle,int nSenderId)
{
	core_msg_friendreq stFriendReq;
	lstrcpyn(stFriendReq.sId, pszId, 1024);
	lstrcpyn(stFriendReq.sNickName, pszNickName, 1024);
	lstrcpyn(stFriendReq.sContent, pszContent, 1024);
	lstrcpyn(stFriendReq.sFriendLink, pszFriendLink, 1024);
	lstrcpyn(stFriendReq.sTitle, pszTitle, 1024);
	stFriendReq.nSenderId = nSenderId;

	SendArcMessageToGameMsg msg(&stFriendReq,false);

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsPlaying())
			{
				(*iter).second->OnMessage(msg);
			}
		}
	}
	___AutoSRWBlockEnd____

	return true;
}

bool CThirdPartyAppManager::SendFRStatusToGame(LPCTSTR pszNickName, LPCTSTR pszGameName, LPCTSTR pszGameUrl, int nType)
{
	core_msg_FRStatus stFRStatus;
	lstrcpyn(stFRStatus.sNickName, pszNickName, 1024);
	lstrcpyn(stFRStatus.sGameName, pszGameName, 1024);
	lstrcpyn(stFRStatus.sGameUrl, pszGameUrl, 1024);
	stFRStatus.nType = nType;

	SendArcMessageToGameMsg msg(&stFRStatus,false);

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			//only send when the user is playing the same game
			if((*iter).second->IsPlaying() && !(*iter).second->GetAppName().CompareNoCase(pszGameName))
			{
				(*iter).second->OnMessage(msg);
			}
		}
	}
	___AutoSRWBlockEnd____

	return true;
}

bool CThirdPartyAppManager::SendReadyPlayToGame(LPCTSTR pszGameName, LPCTSTR pszGameAbbr,
												LPCTSTR pszGameLang, LPCTSTR pszContent, 
												LPCTSTR pszGameProPage, LPCTSTR pszGameLogoPath)
{
	core_msg_ready_play stGameReady;
	lstrcpyn(stGameReady.szGameName, pszGameName, 1024);
	lstrcpyn(stGameReady.szGameAbbr, pszGameAbbr, 1024);
	lstrcpyn(stGameReady.szGameLang, pszGameLang, 1024);
	lstrcpyn(stGameReady.szContent, pszContent, 1024);
	lstrcpyn(stGameReady.szGameProPage, pszGameProPage, 1024);
	lstrcpyn(stGameReady.szGameLogoPath, pszGameLogoPath, 1024);
	SendArcMessageToGameMsg msg(&stGameReady,false);

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsPlaying())
			{
				(*iter).second->OnMessage(msg);
			}
		}
	}
	___AutoSRWBlockEnd____

	return true;
}

bool CThirdPartyAppManager::SendFRResultToGame(BOOL bAccept, BOOL bResult)
{
	core_msg_FR_result stFRResult;
	stFRResult.bAccept = bAccept;
	stFRResult.bResult = bResult;

	SendArcMessageToGameMsg msg(&stFRResult,false);

	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if((*iter).second->IsPlaying())
			{
				(*iter).second->OnMessage(msg);
			}
		}
	}
	___AutoSRWBlockEnd____
	return true;
}

bool CThirdPartyAppManager::IsGameInLaunchState()
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if (pApp->IsPlaying() && !pApp->IsEnteredGame())
			{
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____

	return false;
}

bool CThirdPartyAppManager::IsGameInClientState()
{
	___AutoSRWBlockRead_AppMap____
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			IThirdApp* pApp = (*iter).second;
			if (pApp->IsEnteredGame())
			{
				return true;
			}
		}
	}
	___AutoSRWBlockEnd____
	return false;
}

IThirdApp* CThirdPartyAppManager::AddExternalGame(const EXTERNALGAMEINFO& info)
{
	DWORD nId = m_nExternalId;
	m_nExternalId += 20;

	CExternalApp* pApp = new CExternalApp(nId,info);
	if (!pApp->Init())
	{
		delete pApp;
		return NULL;
	}

	___AutoSRWBlockWrite_ExtendMap____
	m_mapExternalApp[nId] = pApp;
	___AutoSRWBlockEnd____

	return static_cast<IThirdApp*>(pApp);
}

bool CThirdPartyAppManager::DeleteExternalGame(IThirdApp *pApp)
{
	if(!pApp)
	{
		return false;
	}
	
	___AutoSRWBlockWrite_ExtendMap____
	MapItor it = m_mapExternalApp.find(pApp->GetAppIntId());
	if(it != m_mapExternalApp.end())
	{
		DeleteFile((*it).second->GetAppLogoFile().c_str());
		delete (*it).second;
		m_mapExternalApp.erase(it);
	}
	___AutoSRWBlockEnd____

	SetModified();
	return true;
}

void CThirdPartyAppManager::SetModified()
{
	SetEvent(m_events[EventModify]);
}

void CThirdPartyAppManager::SetUpdateData()
{
	SetEvent(m_events[EventUpdate]);
}

long CThirdPartyAppManager::run()
{
	while(1)
	{
		DWORD dwRet = WaitForMultipleObjects(EventCount, m_events, FALSE, INFINITE);
		switch(dwRet - WAIT_OBJECT_0)
		{
		case EventModify:
			{
				Sleep(50);  // dely for frequncy modify;
				ResetEvent(m_events[EventModify]);	
				SaveAppLocalData(m_sLocalInfoPath);
			}
			break;
		case EventUpdate:
			{
				ResetEvent(m_events[EventUpdate]);
				bool bReady = true;
				___AutoSRWBlockRead_AppMap____
				for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
				{
					AppLangMap *pAppLangMap = (*it).second;
					for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
					{
						IThirdApp *pApp = ((*iter).second);
						if(!pApp->IsResouceReady())
						{
							bReady = false;
							break;
						}
					}
				}
				___AutoSRWBlockEnd____
				if(bReady)
				{
					SendMessageToUIWnd(WM_UPDATE_GAMELIST, NULL, NULL);
				}
		
				Sleep(20);
			}
			break;
		case EventSaveFirstPlayingInfo:
			{
				ResetEvent(m_events[EventSaveFirstPlayingInfo]);
				SaveFirstPlayingInfo(m_sFirstPlayingInfoPath);
			}
			break; // !!!!!!!!!!
		case EventExit:
			return 0;
		case EventMonitor:
			OnMonitorGamePlayingState();
			break;
		default:
			break;
		}
	}
	return 0;
}


bool CThirdPartyAppManager::SendMessageToApp(_tstring sAbbr, _tstring sLang, ThirdAppMsgHeader& msg)
{
	bool bRet = false;
	if (sAbbr.empty())
	{
		___AutoSRWBlockRead_AppMap____
		for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
		{
			AppLangMap *pAppLangMap = (*it).second;
			for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
			{
				if(iter->second->OnMessage(msg))
				{
					return true;
				}
			}
		}
		___AutoSRWBlockEnd____

		___AutoSRWBlockRead_ExtendMap____
		for (MapItor it=m_mapExternalApp.begin(); it != m_mapExternalApp.end(); ++it)
		{
			if(it->second->OnMessage(msg))
			{
				return true;
			}
		}
		___AutoSRWBlockEnd____
	}
	else
	{
		IThirdApp* pApp = GetThirdAppByAbbrLang(sAbbr, sLang);
		if (pApp != NULL)
		{
			bRet = pApp->OnMessage(msg);
		}
	}

	return bRet;
}

void CThirdPartyAppManager::SetUIWnd(CAppListWnd* pWnd)
{
	LockGuard<Mutex> gurd(&m_appWndMutex);
	m_pAppListWnd = pWnd;
}

bool CThirdPartyAppManager::PostMessageToUIApp(IThirdApp* pApp, UIMessageHeader& msg)
{
	LockGuard<Mutex> gurd(&m_appWndMutex);
	if (m_pAppListWnd == NULL)
	{
		return false;
	}
	
	return m_pAppListWnd->PostUIThreadMsg(pApp, msg);
}

LRESULT CThirdPartyAppManager::SendMessageToUIWnd(UINT message, WPARAM wParam, LPARAM lParam)
{
	LockGuard<Mutex> gurd(&m_appWndMutex);
	if (m_pAppListWnd == NULL || IsWindow(m_pAppListWnd->GetSafeHwnd()))
	{
		return -1;
	}

	return m_pAppListWnd->SendMessage(message, wParam, lParam);
}

void CThirdPartyAppManager::OnLoginSuccess()
{
	LoadFirstPlayingInfo(m_sFirstPlayingInfoPath);
}

void CThirdPartyAppManager::OnGameStart(IThirdApp* pApp)
{
	___AutoSRWBlockRead_AppMap____

	AutoPauseDownloadingTaskExcept(pApp);
		
	MyGameList myGameList = GetActiveApps();
	for (MyGameList::iterator itCur = myGameList.begin(); itCur!=myGameList.end(); ++itCur)
	{
		if ((*itCur)->GetType() != ThirdApp_Extend)
		{
			PostMessageToUIApp(*itCur, UIMessageGameStartNotify());
		}
	}

	SendGameInstallTrackEx(pApp->GetAppArrb(), pApp->GetAppLanguage(), GAMETRACE_PDL_PLAY_START, CDownloaderMgr::PDL_DOWNLOAD);

	___AutoSRWBlockEnd____

}

void CThirdPartyAppManager::OnGameClose(IThirdApp* pApp)
{
	AutoResumeDownloadingTask();

	MyGameList myGameList = GetActiveApps();
	for (MyGameList::iterator itCur = myGameList.begin(); itCur!=myGameList.end(); ++itCur)
	{
		if ((*itCur)->GetType() != ThirdApp_Extend)
		{
			PostMessageToUIApp(*itCur, UIMessageGameEndNotify());
		}
	}

	SendGameInstallTrackEx(pApp->GetAppArrb(), pApp->GetAppLanguage(), GAMETRACE_PDL_PLAY_END, CDownloaderMgr::PDL_DOWNLOAD);
}

void CThirdPartyAppManager::MonitorGamePlayingState(IThirdApp* pApp)
{
	//OutputDebugString(_T("MonitorGamePlayingState beigin\n"));
	MutexGuard autoLock(&m_monitorGameMapLock);

	_tstring sId = MakeAbbrLangId(pApp->GetAppArrb(), pApp->GetAppLanguage());
	MonitorGameMap::iterator itFind = m_monitorGameMap.find(sId);
	if (itFind != m_monitorGameMap.end())
	{
		//OutputDebugString(_T("MonitorGamePlayingState is exist 1\n"));
		return ;
	}

	// Just test pdl downloading statue , do not test if it is playing now
	// sometime playing now is not alway be true, 
	if(GetGameInPDLDownloadingStatus() != pApp)
	{
		//OutputDebugString(_T("MonitorGamePlayingState is exist 2\n"));
		return ;
	}

	MonitorInfo& info = m_monitorGameMap[sId];
	info.bPlaying = false;
	info.retryTimes = 0;

	//OutputDebugString(_T("MonitorGamePlayingState end\n"));
}

bool CThirdPartyAppManager::_IsFirstPlaying(IThirdApp* pCurApp)
{
	_tstring sMsg;
	_tstring sId = MakeAbbrLangId(pCurApp->GetAppArrb(), pCurApp->GetAppLanguage());
	FirstPlayingInfoMap::iterator itFind = m_firstPlayingInfoMap.find(sId);
	if (itFind != m_firstPlayingInfoMap.end())
	{
		//if(itFind->second)
		//{
		//	sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, is first playing in local cache\n"), sId.c_str());
		//	OutputDebugString(sMsg.c_str());
		//}
		//else
		//{
		//	sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, is NOT first playing in local cache\n"), sId.c_str());
		//	OutputDebugString(sMsg.c_str());
		//}

		return itFind->second;
	}

	bool bSuccess = false;
	bool isFirstPlay = false;
	_tstring szFaildReason;
	if(!g_theWebData.GetIsFirstPlayingGameService(g_theSNSManager.GetSessionId().c_str(), 
		pCurApp->GetAppArrb(),pCurApp->GetAppLanguage(),bSuccess, isFirstPlay, szFaildReason))
	{
		//sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, webservice FAILED\n"), sId.c_str());
		//OutputDebugString(sMsg.c_str());

		Log(_T("CThirdPartyAppManager::IsFirstPlayingPDL error %s"), szFaildReason.c_str());
		return false;
	}
	else
	{
		if (bSuccess)
		{
			//if(isFirstPlay)
			//{
			//	sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, is first playing by webservice\n"), sId.c_str());
			//	OutputDebugString(sMsg.c_str());
			//}
			//else
			//{
			//	sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, is NOT first playing by webservice\n"), sId.c_str());
			//	OutputDebugString(sMsg.c_str());
			//}

			return isFirstPlay;
		}
		else
		{
			//sMsg.Format(1, _T("CThirdPartyAppManager::_IsFirstPlaying game:%s, webservice returned fail\n"), sId.c_str());
			//OutputDebugString(sMsg.c_str());

			Log(_T("CThirdPartyAppManager::IsFirstPlayingPDL error %s"), szFaildReason.c_str());
			return false;
		}
	}

	return true;
}

bool CThirdPartyAppManager::IsFirstPlayingPDL(_tstring sAbbr, _tstring sLang)
{
	MutexGuard autoLock(&m_firstPlayingInfoLock);

	IThirdApp* pCurApp = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurApp == NULL)
	{
		return false;
	}

	struct IsFirstPlayingCache
	{
		bool	isInit;
		bool	isFirstPlaying;
		_tstring preId;
		DWORD	dwCacheTime;
	};
	static IsFirstPlayingCache cache = {false, false, _T(""), 0};

	DWORD dwCurTime = GetTickCount();
	
	_tstring sId = MakeAbbrLangId(sAbbr, sLang);

	if (!cache.isInit)
	{
		cache.isInit = true;
		cache.preId = sId;
		cache.dwCacheTime = dwCurTime;
		cache.isFirstPlaying = _IsFirstPlaying(pCurApp);
	}
	else
	{	
		if (cache.preId == sId)
		{
			if (dwCurTime - cache.dwCacheTime >= 3000)
			{
				cache.dwCacheTime = dwCurTime;
				cache.isFirstPlaying = _IsFirstPlaying(pCurApp);
			}
			else
			{
				cache.dwCacheTime = dwCurTime;
			}
		}
		else
		{
			cache.preId = sId;
			cache.isFirstPlaying = _IsFirstPlaying(pCurApp);
			cache.dwCacheTime = dwCurTime;
		}
	}

	return cache.isFirstPlaying;
}

void CThirdPartyAppManager::ClearFirstPlayingCacheById(_tstring sAbbr, _tstring sLang)
{
	MutexGuard autoLock(&m_firstPlayingInfoLock);

	_tstring szID = MakeAbbrLangId(sAbbr, sLang);

	m_firstPlayingInfoMap.erase(szID);
	SetEvent(m_events[EventSaveFirstPlayingInfo]);
}

void CThirdPartyAppManager::KeepFirstPlayingCacheById(_tstring sAbbr, _tstring sLang)
{
	MutexGuard autoLock(&m_firstPlayingInfoLock);
	
	IThirdApp* pCurApp = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurApp == NULL || !IsFirstPlayingPDL(sAbbr, sLang))
	{
		return ;
	}
	_tstring sId = MakeAbbrLangId(sAbbr, sLang);
	m_firstPlayingInfoMap[sId] = true;  // cache is first playing info
	SetEvent(m_events[EventSaveFirstPlayingInfo]);
}

IThirdApp* CThirdPartyAppManager::GetGameInPDLDownloadingStatus()
{
	return m_pCurDownloadingPDLApp;
}

void CThirdPartyAppManager::SetGameInPDLDownloadingStatus(IThirdApp* pApp)
{
	m_pCurDownloadingPDLApp = pApp;
}

IThirdApp* CThirdPartyAppManager::GetGameInPDLDownloadingAndPlayStatus()
{
	___AutoSRWBlockRead_AppMap____
	IThirdApp* pDowndloadingGame = GetGameInPDLDownloadingStatus();
	if (pDowndloadingGame != NULL && pDowndloadingGame->IsPlaying())
	{
		return pDowndloadingGame;
	}
	___AutoSRWBlockEnd____

	return NULL;
}

bool CThirdPartyAppManager::StartDownload(_tstring sAbbr, _tstring sLang, IDownloaderListener* pListener,int nDownloadTool, DOWNLOAD_SETTING& downloadSetting)
{
	___AutoSRWBlockRead_AppMap____
	if(nDownloadTool != CDownloaderMgr::HTTP2 && nDownloadTool != CDownloaderMgr::PDL_DOWNLOAD)
		return false;

	IThirdApp* pCurDownloadingGame = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurDownloadingGame == NULL)
	{
		return false;
	}

	if (GetGameInPDLDownloadingAndPlayStatus() != NULL)
	{
		return false;
	}
	
	//First. Pause current PDL download task
	IThirdApp* pPDLDownloadingTask = GetGameInPDLDownloadingStatus();
	if (pPDLDownloadingTask != NULL && nDownloadTool == CDownloaderMgr::PDL_DOWNLOAD)
	{
		PostMessageToUIApp(pPDLDownloadingTask, UIMessageAutoPausePDLNotify());
	}
	
	if (nDownloadTool == CDownloaderMgr::PDL_DOWNLOAD)
	{
		SetGameInPDLDownloadingStatus(pCurDownloadingGame);
	}
	return g_downloaderMgr.Start(nDownloadTool,pListener, downloadSetting);
	___AutoSRWBlockEnd____
}

bool CThirdPartyAppManager::CancelDownload(_tstring sAbbr, _tstring sLang)
{
	___AutoSRWBlockRead_AppMap____
	// First. Cancel the downloading task
	IThirdApp* pCurDownloadingGame = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurDownloadingGame == NULL && !pCurDownloadingGame->IsDownloading())
	{
		return false;
	}

	_tstring sId = MakeAbbrLangId(sAbbr, sLang);

	IDownloader* pDownload = g_downloaderMgr.GetDownloader(sId);
	if (pDownload == NULL)
	{
		return false;
	}

	ClearFirstPlayingCacheById(sAbbr, sLang);
	g_downloaderMgr.Cancel(sId);
	if (GetGameInPDLDownloadingStatus() == pCurDownloadingGame)
	{
		SetGameInPDLDownloadingStatus(NULL);
	}
	
	return true;
	___AutoSRWBlockEnd____
}	

bool CThirdPartyAppManager::PauseDownload(_tstring sAbbr, _tstring sLang)
{
	___AutoSRWBlockRead_AppMap____
	
	// First. Check this app is an un-paused app;
	IThirdApp* pCurPauseApp = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurPauseApp == NULL)
	{
		return false;
	}
	_tstring sId = MakeAbbrLangId(sAbbr, sLang);
	g_downloaderMgr.Pause(sId);
	if (GetGameInPDLDownloadingStatus() == pCurPauseApp)
	{
		SetGameInPDLDownloadingStatus(NULL);
	}
	return true;
	___AutoSRWBlockEnd____
}

bool CThirdPartyAppManager::IsDownloadingTaskExist(_tstring sAbbr, _tstring sLang)
{
	___AutoSRWBlockRead_AppMap____

	// First. Check this app is an un-paused app;
	IThirdApp* pCurPauseApp = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurPauseApp == NULL)
	{
		Log(_T("CThirdPartyAppManager::IsDownloadingTaskExist occur an unkown game id(%d)"), pCurPauseApp->GetAppIntId());
		return false;
	}

	_tstring sId = MakeAbbrLangId(sAbbr, sLang);
	if (g_downloaderMgr.GetDownloader(sId) == NULL)
	{
		return false;
	}

	return true;
	___AutoSRWBlockEnd____
}


bool CThirdPartyAppManager::ResumDownload(_tstring sAbbr, _tstring sLang)
{
	___AutoSRWBlockRead_AppMap____
	
	// First. Check this app is an un-paused app;	
	IThirdApp* pCurResumeApp = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurResumeApp == NULL || !pCurResumeApp->IsDownloading())
	{
		return false;
	}

	_tstring sId = MakeAbbrLangId(sAbbr, sLang);
	IDownloader* pCurDownloadTask = g_downloaderMgr.GetDownloader(sId);
	if(pCurDownloadTask == NULL)
	{
		return false;
	}

	if (GetGameInPDLDownloadingAndPlayStatus() != NULL)
	{
		return false;
	}
	
	if (pCurDownloadTask->GetAttribute()->nStatus != IDownloader::PAUSING 
		&& pCurDownloadTask->GetAttribute()->nStatus != IDownloader::PAUSED)
	{
		return false;
	}
	

	IThirdApp* pPDLDownloadingTask = GetGameInPDLDownloadingStatus();
	if (pPDLDownloadingTask != NULL && pCurDownloadTask->GetDownloadType() == CDownloaderMgr::PDL_DOWNLOAD)
	{
		PostMessageToUIApp(pPDLDownloadingTask, UIMessageAutoPausePDLNotify());
	}

	DelAutoResumeItem(sId);
	g_downloaderMgr.Resume(sId);
	
	if (pCurDownloadTask->GetDownloadType() == CDownloaderMgr::PDL_DOWNLOAD)
	{
		SetGameInPDLDownloadingStatus(pCurResumeApp);
	}
	___AutoSRWBlockEnd____
	return true;
}

void CThirdPartyAppManager::AddAutoResumeItem(const _tstring& szId)
{
	MutexGuard autoLock(&m_autoResumeVecLock);
	if (std::find(m_autoResumeVec.begin(), m_autoResumeVec.end(), szId) == m_autoResumeVec.end())
	{
// 		_tstring debugStr;
// 		debugStr.Format(1, _T("____AddAutoResumeItem [%s]"), szId.c_str());
// 		OutputDebugString(debugStr.c_str());
		m_autoResumeVec.push_back(szId);
	}
}

void CThirdPartyAppManager::DelAutoResumeItem(const _tstring& szId)
{
	MutexGuard autoLock(&m_autoResumeVecLock);

	AutoResumeVec::iterator itFind = std::find(m_autoResumeVec.begin(), m_autoResumeVec.end(), szId);
	if (itFind != m_autoResumeVec.end())
	{
// 		_tstring debugStr;
// 		debugStr.Format(1, _T("___DelAutoResumeItem [%s]"), szId.c_str());
// 		OutputDebugString(debugStr.c_str());
		m_autoResumeVec.erase(itFind);
	}
	
}

void CThirdPartyAppManager::AutoPauseDownloadingTaskExcept(IThirdApp* pApp)
{
	___AutoSRWBlockRead_AppMap____
	MutexGuard autoLock(&m_autoResumeVecLock);
	for(ThirdAppMap::iterator it = m_mapApps.begin(); it != m_mapApps.end(); it++)
	{
		AppLangMap *pAppLangMap = (*it).second;
		for(AppLangMap::iterator iter = pAppLangMap->begin(); iter != pAppLangMap->end(); iter++)
		{
			if(iter->second->IsDownloading() && pApp != iter->second)
			{
				IDownloader* pDownloader = g_downloaderMgr.GetDownloader(MakeAbbrLangId(iter->second->GetAppArrb(), iter->second->GetAppLanguage()));
				if (pDownloader != NULL && (pDownloader->GetAttribute()->nStatus == IDownloader::DOWNLOADING 
					|| pDownloader->GetAttribute()->nStatus == IDownloader::RESUMING 
					|| pDownloader->GetAttribute()->nOperation == IDownloader::RESUME) 
					&& pDownloader->GetDownloadType() == CDownloaderMgr::HTTP2)
				{
					//OutputDebugString(_T("AutoPauseDownloadingTaskExcept ---- post AutoPauseNotify"));
					PostMessageToUIApp(iter->second, UIMessageAutoPauseNotify());
					AddAutoResumeItem(pDownloader->GetAttribute()->sId);
				}
			}
		}
	}
	___AutoSRWBlockEnd____
}

void CThirdPartyAppManager::AutoResumeDownloadingTask()
{
	___AutoSRWBlockRead_AppMap____
	MutexGuard autoLock(&m_autoResumeVecLock);

	//OutputDebugString(_T("___AutoResumeDownloadingTask--- > begin"));
	if (m_autoResumeVec.size() <= 0)
	{
		//OutputDebugString(_T("___AutoResumeDownloadingTask--- > size < 0 - return "));
		return ;
	}

	for (int i=0; i<m_autoResumeVec.size(); ++i)
	{
		//int nId = _ttoi(m_autoResumeVec[i].c_str());
		_tstring sAbbrLang = m_autoResumeVec[i];
		_tstring sAbbr, sLang;
		SplitAbbrLangId(sAbbrLang, sAbbr, sLang);
		IThirdApp* pThirdApp = GetThirdAppByAbbrLang(sAbbr, sLang);
		if (pThirdApp==NULL)
		{
			//OutputDebugString(_T("___AutoResumeDownloadingTask--- > pThirdApp == NULL - continue "));
			continue;
		}

		IDownloader* pDownloader = g_downloaderMgr.GetDownloader(m_autoResumeVec[i]);
		if (pDownloader == NULL)
		{
			//OutputDebugString(_T("___AutoResumeDownloadingTask--- > pDownloader == NULL - continue "));
			continue;
		}

// 		_tstring debugStr;
// 		debugStr.Format(1, _T("Post auto resume to %d\n"), nId);
// 		OutputDebugString(debugStr.c_str());
		PostMessageToUIApp(pThirdApp, UIMessageAutoResumeNotify());
	}

	m_autoResumeVec.clear();
	___AutoSRWBlockEnd____
}

void CThirdPartyAppManager::LoadFirstPlayingInfo(const _tstring& szPath)
{
	MutexGuard autoLock(&m_firstPlayingInfoLock);
	m_firstPlayingInfoMap.clear();

	TiXmlDocument doc;
	if(!doc.LoadFile(szPath.toNarrowString().c_str()))
	{
		doc.ClearError();
		TiXmlDeclaration *pDecl = new TiXmlDeclaration("1.0", "utf-8", "yes");
		doc.LinkEndChild(pDecl);

		TiXmlElement *pRoot = new TiXmlElement("root");
		doc.LinkEndChild(pRoot);

		doc.SaveFile(szPath.toNarrowString().c_str());
		return ;
	}

	TiXmlElement* pRootNode = doc.RootElement();
	if (pRootNode == NULL)
	{
		return ;
	}

	_tstring szAccountName = theDataPool.GetUserName();
	for (TiXmlElement* pElem=pRootNode->FirstChildElement("Account"); pElem!=NULL; pElem=pElem->NextSiblingElement("Account"))
	{
		const char* pszName = pElem->Attribute("name");
		if (pszName != NULL)
		{
			_tstring szName;
			szName.fromUTF8(pszName);

			if (szName == szAccountName)
			{
				for (TiXmlElement* pInfoElem = pElem->FirstChildElement("game"); pInfoElem!=NULL; pInfoElem = pInfoElem->NextSiblingElement("game"))
				{
					_tstring szGameName;
					bool bFirstPlaying = false;
					
					const char* pszGameName = pInfoElem->Attribute("name");
					if (pszGameName == NULL)
					{
						continue;
					}
					szGameName.fromUTF8(pszGameName);

					int nFirstPlaying = 0;
					if(NULL == pInfoElem->Attribute("IsFirstPlaying",&nFirstPlaying))
					{
						continue;
					}
					bFirstPlaying = nFirstPlaying > 0 ? true : false;

					m_firstPlayingInfoMap[szGameName] = bFirstPlaying;
				}

				break;
			}
		}
	}
	
}

void CThirdPartyAppManager::SaveFirstPlayingInfo(const _tstring& szPath)
{
	MutexGuard autoLock(&m_firstPlayingInfoLock);
	TiXmlDocument doc;
	if(!doc.LoadFile(szPath.toNarrowString().c_str()))
	{
		return ;
	}

	TiXmlElement* pRootNode = doc.RootElement();
	if (pRootNode == NULL)
	{
		return ;
	}

	_tstring szAccountName = theDataPool.GetUserName();
	for (TiXmlElement* pElem=pRootNode->FirstChildElement("Account"); pElem!=NULL; pElem=pElem->NextSiblingElement("Account"))
	{
		const char* pszName = pElem->Attribute("name");
		if (pszName != NULL)
		{
			_tstring szName;
			szName.fromUTF8(pszName);

			if (szName == szAccountName)
			{
				pElem->Parent()->RemoveChild(pElem);
				break;
			}
		}
	}

	TiXmlElement* pAccountElem = new TiXmlElement("Account");
	pAccountElem->SetAttribute("name", szAccountName.toUTF8().c_str());
	pRootNode->LinkEndChild(pAccountElem);

	for (FirstPlayingInfoMap::iterator it=m_firstPlayingInfoMap.begin(); it != m_firstPlayingInfoMap.end(); ++it)
	{
		TiXmlElement* pGameElem = new TiXmlElement("game");
		pGameElem->SetAttribute("name", (it->first).toUTF8().c_str());
		int nFirstPlay = it->second != false ? 1 : 0;
		pGameElem->SetAttribute("IsFirstPlaying", nFirstPlay);

		pAccountElem->LinkEndChild(pGameElem);
	}

	doc.SaveFile(szPath.toNarrowString().c_str());
}

void CThirdPartyAppManager::OnMonitorGamePlayingState()
{
 	MutexGuard autoMonitorGameSetLock(&m_monitorGameMapLock);

	static std::vector<_tstring> earseGameVec(10);
	earseGameVec.clear();

	for(MonitorGameMap::iterator it=m_monitorGameMap.begin(); it!=m_monitorGameMap.end(); ++it)
	{
		_tstring sAbbr, sLang;
		SplitAbbrLangId(it->first, sAbbr, sLang);
		IThirdApp* pApp = GetThirdAppByAbbrLang(sAbbr, sLang);
		if (pApp == NULL)
		{
			earseGameVec.push_back(it->first);
		}
		else
		{
			if (!it->second.bPlaying && pApp->IsPlaying())  
			{
				OnGameStart(pApp);
				it->second.bPlaying = true;
			}
			else if (it->second.bPlaying)
			{
				if (!pApp->IsPlaying())
				{
					it->second.retryTimes += 1;
					if (it->second.retryTimes >= MONITOR_RETRY_CLOSE_TIMES)
					{
						OnGameClose(pApp);
						earseGameVec.push_back(it->first);
					}
				}
				else
				{
					it->second.retryTimes = 0;
				}
			}
		}
	}

	for (int i=0; i<earseGameVec.size(); ++i)
	{
		m_monitorGameMap.erase(earseGameVec[i]);
	}

	earseGameVec.clear();
}

bool CThirdPartyAppManager::GetDefaultProductPage(_tstring& sUrl, _tstring sGameAbbr)
{
	sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_DEFAULT"));
	_tstring sData;
	if(g_theWebData.GetAllGameinfo(TSTRING_ABBR_LANGUAGE_EN, sData))	//Get en gameinfo
	{
		TiXmlElement *pRoot = NULL;
		TiXmlElement *pElement = NULL;
		TiXmlDocument xmlDoc;

		// parse gameinfo
		xmlDoc.Clear();
		xmlDoc.Parse(sData.toNarrowString().c_str());

		pRoot = xmlDoc.RootElement();
		pElement = pRoot->FirstChildElement("game");

		bool bRet = false;
		while(pElement)
		{
			CThirdPartyApp *pApp = new CThirdPartyApp;
			if(pApp->LoadAppInfo(pElement, false))
			{
				if (pApp->GetAppArrb().CompareNoCase(sGameAbbr) == 0)
				{
					_tstring sNickName;
					_tstring sNews = pApp->GetNewsLink();
					int nPos = sNews.ReverseFind(_T('/'));
					if (nPos)
					{
						sNickName = pApp->GetNewsLink().Right(sNews.size() - nPos - 1);
						sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_DEFAULTPROD"));
						sUrl.append(sNickName);
						bRet = true;
					}
				}
			}			
			delete pApp;

			if (bRet)
			{
				break;
			}

			pElement = pElement->NextSiblingElement("game");
		}

		xmlDoc.Clear();
	}

	return true;
}

bool CThirdPartyAppManager::StartPDLGame(_tstring sAbbr, _tstring sLang)
{
	IThirdApp* pCurDownloadingGame = GetThirdAppByAbbrLang(sAbbr, sLang);
	if (pCurDownloadingGame == NULL)
	{
		return false;
	}

	if (GetGameInPDLDownloadingAndPlayStatus() != NULL)
	{
		return false;
	}

	IDownloader* pDownloader = g_downloaderMgr.GetDownloader(MakeAbbrLangId(sAbbr, sLang));
	if (pDownloader == NULL || pDownloader->GetDownloadType()  != CDownloaderMgr::PDL_DOWNLOAD)
	{
		return false;
	}

	CPDLDownloader* pPDLDownloader = static_cast<CPDLDownloader*>(pDownloader);
	if (pPDLDownloader == NULL)
	{
		return false;
	}

	return pPDLDownloader->PlayPDL();
}

bool CThirdPartyAppManager::GetBanList()
{
	std::string resultXml;
	if (g_theWebData.GetBannedGameListService(g_theSNSManager.GetSessionId(), resultXml))
	{//Get banned game list
		TiXmlDocument  xmlDoc;
		TiXmlElement   *pRoot=NULL, *pElement=NULL;	

		do {

			xmlDoc.Parse(resultXml.c_str()) ;
			pRoot = xmlDoc.FirstChildElement("data");
			if(!pRoot)
			{
				break;
			}
			pElement = pRoot->FirstChildElement("result");
			if(!pElement)
			{
				break;
			}
			_tstring   szResult;
			szResult.fromUTF8(pElement->GetText( ) );
			if( 0!=szResult.CompareNoCase(_T("success")) )
			{
				break ;
			}

			pElement = pRoot->FirstChildElement("games");
			if(pElement)
			{
				_tstring sGames;
				sGames.fromUTF8(pElement->GetText( ) );

				_tstring sItem;
				while(!sGames.empty())
				{
					int nIndex = sGames.Find("|");
					if(nIndex>=0)
					{
						sItem = sGames.Left(nIndex);
						sGames = sGames.Right(sGames.size() - nIndex - 1);
					}
					else
					{
						sItem = sGames;
						sGames.clear();
					}
					m_vecBanList.insert(m_vecBanList.begin(), sItem);
				}
			}

		}while(false) ;
		xmlDoc.Clear();
	}

	return true;
}

bool CThirdPartyAppManager::IsGameBannedByIP(const _tstring& sAbbr)
{
	vector<_tstring>::iterator itor = find(m_vecBanList.begin(), m_vecBanList.end(), sAbbr);
	if (itor != m_vecBanList.end())
	{
		return true;
	}

	return false;
}

bool CThirdPartyAppManager::PostLocalGameList()
{
	_tstring sLocalGames;
	MyGameList myApps = g_appManager.GetMyApps();
	MyGameList::iterator iter = myApps.begin();
	for( ; iter != myApps.end(); iter++)
	{
		IThirdApp *pApp = (*iter);
		if (pApp && pApp->IsInstalled())
		{
			_tstring sApp;
			sApp.Format(2, _T("%s:%s"), pApp->GetAppArrb().c_str(), pApp->GetAppLanguage().c_str());
			if (sLocalGames.empty())
			{
				sLocalGames += sApp;
			} 
			else
			{
				sLocalGames += _T("|");
				sLocalGames += sApp;
			}
		}
	}
	_tstring sData;
	sData.Format(1, _T("games=%s"), sLocalGames.c_str());
	g_theWebData.PostLocalGameListService(g_theSNSManager.GetSessionId(), sData);

	return true;
}

FUNC_CreateProcessWithDll CThirdPartyAppManager::GetInjectFunction()
{
	return m_CreateProcessWithDll;
}

bool CThirdPartyAppManager::CreateGameProcess(LPCSTR lpExecuteFile, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory )
{
    if(Game_Is_64bit(lpExecuteFile))
        return Create_64bit_Game_Process(lpApplicationName, lpCommandLine, lpCurrentDirectory);
    else
        return Create_32bit_Game_Process(lpApplicationName, lpCommandLine, lpCurrentDirectory);
}   

bool CThirdPartyAppManager::Game_Is_64bit(LPCSTR lpExcuteFile)
{
    return IMAGE_FILE_MACHINE_AMD64 == CUtility::FileMachineType(lpExcuteFile);
}

bool CThirdPartyAppManager::Create_64bit_Game_Process( LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory)
{
    STARTUPINFOA si = {0};
    si.cb = sizeof(STARTUPINFOA);

    PROCESS_INFORMATION pi;
    return CreateProcessA(lpApplicationName, lpCommandLine, NULL, NULL, TRUE, 0, NULL, lpCurrentDirectory, &si, &pi);
}

bool CThirdPartyAppManager::Create_32bit_Game_Process( LPCSTR lpApplicationName, LPSTR lpCommandLine, LPCSTR lpCurrentDirectory )
{
    return m_CreateProcessWithDll(lpApplicationName, lpCommandLine, TRUE, 0, lpCurrentDirectory, NULL);
}


bool CThirdPartyAppManager::GetCloudGames()
{
	_tstring sGames;
	if(!g_theWebData.GetAllCloudGames(g_theSNSManager.GetSessionId(), sGames))
	{
		return false;
	}

	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL, *pElement=NULL;	
	int nCount = 0;

	do {

		xmlDoc.Parse(sGames.toNarrowString().c_str()) ;
		pRoot = xmlDoc.FirstChildElement("data");
		if(!pRoot)
		{
			break;
		}
		pElement = pRoot->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}
		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText( ) );
		if( 0!=szResult.CompareNoCase(_T("success")) )
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("game");
		while(pElement)
		{
			nCount++;
			TiXmlElement *pAbbrNode = pElement->FirstChildElement("abbr");
			TiXmlElement *pChildNode = pElement->FirstChildElement("lang");

			if(pAbbrNode && pChildNode)
			{
				const char *ptmp = pAbbrNode->GetText();
				const char *plang = pChildNode->GetText();

				IThirdApp *pIApp = GetThirdAppByAbbrLang(_tstring(ptmp), _tstring(plang));
				if(pIApp)
				{
					CThirdPartyApp *pApp = static_cast<CThirdPartyApp*>(pIApp);
					pApp->GetLocalInfo().SetIsActive(true);
					pApp->GetMemInfo().SetIsCloudGame(true);
					pChildNode = pElement->FirstChildElement("favorite");
					if(pChildNode)
					{
						pApp->GetMemInfo().SetIsFavorite(atoi(pChildNode->GetText()));
					}
					pChildNode = pElement->FirstChildElement("deletable");
					if(pChildNode)
					{
						pApp->GetMemInfo().SetIsDeletable(atoi(pChildNode->GetText()));
					}
				}
			}
			pElement = pElement->NextSiblingElement("game");
		}

	}while(false) ;
	xmlDoc.Clear();

	if(nCount == 0)
	{
		AddLocalGamesToCloud();
	}

	return true;
}

bool CThirdPartyAppManager::AddLocalGamesToCloud()
{
	MyGameList myApps = g_appManager.GetMyApps();

	int nCount = 0;

	_tstring sGames = _T("games=[");
	MyGameList::iterator iter = myApps.begin();
	for( ; iter != myApps.end(); iter++)
	{
		CThirdPartyApp *pApp = static_cast<CThirdPartyApp*>(*iter);
		if(pApp->GetType() == ThirdApp_Extend)
		{
			continue;
		}
		nCount++;
		if(nCount > 1)
		{
			sGames += _T(",");
		}
		pApp->GetMemInfo().SetIsFavorite(false);
		_tstring stmp;
		stmp.Format(3, _T("{\"favorite\":false,\"lang\":\"%s\",\"abbr\":\"%s\",\"order\":\"%s\"}"),
			pApp->GetAppLanguage().c_str(), pApp->GetAppArrb().c_str(), _tstring(pApp->GetUIOrder()).c_str());
		sGames += stmp;
	}
	sGames += _T("]");

	if(nCount == 0)
	{
		return true;
	}

	return g_theWebData.AddGameToCloud(g_theSNSManager.GetSessionId(), sGames);
}

IThirdApp* CThirdPartyAppManager::GetLastplayGame()
{
	CThirdPartyApp *pLastplay = NULL;

	MyGameList gamelist = GetMyApps();
	MyGameList::iterator it;
	for(it = gamelist.begin(); it != gamelist.end(); it++)
	{
		CThirdPartyApp *pApp = (CThirdPartyApp *)(*it);
		if(pApp->GetLocalStatus() < CThirdPartyApp::EXTRACTED)
		{
			continue;
		}
		SYSTEMTIME stm1 = pApp->GetLocalInfo().GetLastplaytime();
		FILETIME ft1={0};
		SystemTimeToFileTime(&stm1, &ft1);
		if(!pLastplay)
		{
			if(ft1.dwHighDateTime > 0 || ft1.dwLowDateTime > 0)
			{
				pLastplay = pApp;
			}
		}
		else
		{
			SYSTEMTIME stm2 = pLastplay->GetLocalInfo().GetLastplaytime();
			FILETIME ft2={0};
			SystemTimeToFileTime(&stm2, &ft2);
			if(CompareFileTime(&ft1, &ft2) > 0)
			{
				pLastplay = pApp;
			}
		}
	}
	return pLastplay;
}

bool CThirdPartyAppManager::SaveCategoryDataToXML(TiXmlElement *pCategory)
{
	char szBuf[16];
	TiXmlElement *pLastplay = new TiXmlElement("lastplay");
	pLastplay->SetAttribute("expand", itoa(m_bCategoryExpanded[0], szBuf, 10));
	pCategory->LinkEndChild(pLastplay);

	TiXmlElement *pFavorites = new TiXmlElement("favorites");
	pFavorites->SetAttribute("expand", itoa(m_bCategoryExpanded[1], szBuf, 10));
	pCategory->LinkEndChild(pFavorites);

	TiXmlElement *pAllgames = new TiXmlElement("allgames");
	pAllgames->SetAttribute("expand", itoa(m_bCategoryExpanded[2], szBuf, 10));
	pCategory->LinkEndChild(pAllgames);

	return true;
}

bool CThirdPartyAppManager::LoadCategoryDataFromXML(TiXmlElement *pCategory)
{
	TiXmlElement *pLastplay = pCategory->FirstChildElement("lastplay");
	if(pLastplay)
	{
		const char *ptmp = pLastplay->Attribute("expand");
		if(ptmp)
		{
			m_bCategoryExpanded[0] = atoi(ptmp);
		}
	}

	TiXmlElement *pFavorites = pCategory->FirstChildElement("favorites");
	if(pFavorites)
	{
		const char *ptmp = pFavorites->Attribute("expand");
		if(ptmp)
		{
			m_bCategoryExpanded[1] = atoi(ptmp);
		}
	}

	TiXmlElement *pAllgames = pCategory->FirstChildElement("allgames");
	if(pAllgames)
	{
		const char *ptmp = pAllgames->Attribute("expand");
		if(ptmp)
		{
			m_bCategoryExpanded[2] = atoi(ptmp);
		}
	}

	return true;
}
