///*************************************************
/**
 * @file      Downloadermgr.cpp
   @brief     The implementation file of downloader manager.
   @author    liuxiangshu@pwrd.com
   @data      2012/04/16
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
#include "download/HttpDownloader.h"
#include "download/HttpDownloader2.h"
#include "constant.h"
#include "log/local.h"
#include "data/WebData.h"
#include "download/PDLDownloader.h"

//-------------------------------------

CDownloaderMgr g_downloaderMgr;

CDownloaderMgr::CDownloaderMgr()
: m_nDownloadTool(-1)
{
	m_bInit = false;
}

CDownloaderMgr::~CDownloaderMgr()
{
	m_bInit = false;
}

bool CDownloaderMgr::Init()
{
	MutexGuard mg(this);

	//// get ACE download tool, used to start new download
	//int nDownTool = -1;
	//if(g_theWebData.GetDownloadOptionService(nDownTool))
	//{
	//	m_nDownloadTool = nDownTool;
	//}
	DWORD dwPDLRet = 0;
	thePDLHelper.InitializePDL(dwPDLRet);

	DWORD dwHTT2eDownload = 0;
	theHTTP2DownloaderHelper.InitializeHTTPDownlader2(dwHTT2eDownload);

	InitializeCriticalSection(&m_csAPIMutex);

	m_bInit = true;

	return true;
}

bool CDownloaderMgr::Uninit()
{
	MutexGuard mg(this);
	
	if (!m_bInit)
	{
		return true;
	}

	m_bInit = false;

	MAPDOWNLOADER::iterator iter;
	for(iter = m_mapDownloader.begin(); iter != m_mapDownloader.end(); iter++)
	{
		(*iter).second->Uninit();
		delete (*iter).second;
	}
	m_mapDownloader.clear();

	theHTTP2DownloaderHelper.UninitlizeHTTPDownlader2();
	thePDLHelper.UninitlizePDL();

	DeleteCriticalSection(&m_csAPIMutex);

	return true;
}


bool CDownloaderMgr::Start(int nDownloadToolType, IDownloaderListener *pListener, DOWNLOAD_SETTING ds)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(ds.sId);
	if(iter != m_mapDownloader.end())
	{
		//(*iter).second->stop();
		delete (*iter).second;
		m_mapDownloader.erase(iter);
	}

	IDownloader *pDownloader = NULL;

	switch(nDownloadToolType)
	{
	case HTTP:
		pDownloader = new CHttpDownloader;
		break;
	case HTTP2:
		pDownloader = new CHttpDownloader2;
		break;
	case PDL_DOWNLOAD:
		pDownloader = new CPDLDownloader;
		break;
	default:
		pDownloader = new CHttpDownloader2;
		break;
	}

	m_mapDownloader[ds.sId] = pDownloader;
	pDownloader->SetDownloadType(nDownloadToolType);
	if(!pDownloader->Init(pListener, this, &m_csAPIMutex, ds))
	{
		m_mapDownloader.erase(ds.sId);
		delete pDownloader;
		return false;
	}

	return true;
}

bool CDownloaderMgr::Pause(_tstring sId)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(sId);
	if(iter != m_mapDownloader.end())
	{
		return (*iter).second->Pause();
	}
	return false;
}

bool CDownloaderMgr::Resume(_tstring sId)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(sId);
	if(iter != m_mapDownloader.end())
	{
		return (*iter).second->Resume();
	}
	return false;
}

bool CDownloaderMgr::Cancel(_tstring sId)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(sId);
	if(iter != m_mapDownloader.end())
	{
		return (*iter).second->Cancel();
	}
	return false;
}

DOWNLOAD_PARAM *CDownloaderMgr::GetAttribute(_tstring sId)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(sId);
	if(iter != m_mapDownloader.end())
	{
		return (*iter).second->GetAttribute();
	}
	return NULL;
}

IDownloader* CDownloaderMgr::GetDownloader(const _tstring& sId)
{
	MutexGuard mg(this);

	MAPDOWNLOADER::iterator iter = m_mapDownloader.find(sId);
	if(iter != m_mapDownloader.end())
	{
		return (*iter).second;
	}
	return NULL;
}