///*************************************************
/**
 * @file      IDownloader.h
   @brief     The header file of downloader interface.
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

#pragma once

#include <map>
#include "tstring/tstring.h"
#include "thread/thread.h"

using namespace String;

struct DOWNLOAD_SETTING
{
	_tstring sId;
	_tstring sUrl;
	_tstring sDst;
	_tstring sGameName;
	bool bCheckMD5;
	bool bAlwaysConnect;	//网络断开后是否一直重连，下载游戏重连，更新Arc不重连

	DOWNLOAD_SETTING()
	{
		bCheckMD5 = false;
		bAlwaysConnect = true;
	}
};

struct DOWNLOAD_PARAM
{
	_tstring sUrl;
	_tstring sId;
	_tstring sDst;
	_tstring sFileName;
	int nRate;
	long long totleSize;
	long long downloadSize;
	int nDownloadPercent;
	int nStatus;
	int nErrCode;
	int nOperation;

	DOWNLOAD_PARAM()
	{
		nRate = 0;
		totleSize = 0;
		downloadSize = 0;
		nDownloadPercent = 0;
		nStatus = 0;
		nErrCode = 0;
		nOperation = 0;
	}
};

//--------------------------------------------

class CDownloaderMgr;
class IDownloaderListener;

class IDownloader : public Thread
{
public:
	enum DOWNLOAD_STATUS
	{
		INIT = 0,
		STARTING,
		DOWNLOADING,
		PAUSING,
		PAUSED,
		RESUMING,
		CANCELLING,
		CANCELLED,
		COMPLETE,
		ERR,
	};

	enum DOWNLOAD_ERROR
	{
		NOERR = 0,
		ERRINTERNAL,
		ERRSTART,
		ERRPAUSE,
		ERRRESUME,
		ERRCANCEL,
		NOTENOUGHSPACE = 10,
		PATHNOTFOUND,
		UNKONWNSTATUS,
		ERRCONNECT,
		ERRTOOL,
		ERRPERMISSION,
		ERRLAST,
	};

	enum DOWNLOAD_OPERATION
	{
		NOOPERATION = 0,
		START,
		PAUSE,
		RESUME,
		CANCEL
	};

public:
	IDownloader()
		: m_csAPIMutex(NULL)
		, m_pListener(NULL)
		, m_pDownloaderMgr(NULL)
		, m_bExit(false)
		, m_bP2P(true)
	{
	}

	virtual ~IDownloader()
	{

	}

	/*
	*  @brief: init a download task and start download thread. 
	*  @param[in] sUrl: download url.
	*  @param[in] sId: game(download) id.
	*  @param[in] sDst: path to store the download data.
	*  @param[in] pListener: listener to handle download event.
	*  @param[in] pDownloadMgr: pointer to manager of download task.
	*  @param[in] pMutex: download api mutex.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds) = 0;

	/*
	*  @brief: pause if downloading and exit the download thread. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Uninit() = 0;

	/*
	*  @brief: start. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool StartDownload() = 0;

	/*
	*  @brief: pause. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Pause() = 0;

	/*
	*  @brief: resume. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Resume() = 0;

	/*
	*  @brief: cancel. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Cancel() = 0;

	/*
	*  @brief: GetAttribute. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual DOWNLOAD_PARAM* GetAttribute() = 0;

	virtual bool IsDownloading(){ return (m_sParam.nStatus == IDownloader::DOWNLOADING); }

	virtual bool IsPaused() { return (m_sParam.nStatus == IDownloader::PAUSED); }

	virtual int		GetDownloadType(){return m_type;}
	virtual void	SetDownloadType(int nType){m_type = nType;}
protected:
	CRITICAL_SECTION *m_csAPIMutex;
	DOWNLOAD_PARAM m_sParam;
	IDownloaderListener *m_pListener;
	CDownloaderMgr *m_pDownloaderMgr;
	int  m_type;
	bool m_bExit;
	bool m_bP2P;
};

//----------------------------------------------


class IDownloaderListener
{
public:
	virtual bool OnStartBegin(_tstring sId) = 0;
	virtual bool OnStartEnd(_tstring sId) = 0;
	virtual bool OnPauseBegin(_tstring sId) = 0;
	virtual bool OnPauseEnd(_tstring sId) = 0;
	virtual bool OnResumeBegin(_tstring sId) = 0;
	virtual bool OnResumeEnd(_tstring sId) = 0;
	virtual bool OnCancelBegin(_tstring sId) = 0;
	virtual bool OnCancelEnd(_tstring sId) = 0;
	virtual bool OnComplete(_tstring sId) = 0;
	virtual bool UpdateStatus(_tstring sId, DOWNLOAD_PARAM *pDownloadParam) = 0;
	virtual bool OnLauchPoint(_tstring sId) {return true;}
	virtual bool OnError(_tstring sId, int nErrType, _tstring sComment = _T("")) = 0;
	virtual bool OnTraceMessage(DWORD errorCode, const _tstring& comment){return true;}
	virtual bool OnPlayPDLGame(const _tstring& szId){return true;}
};

//-------------------------------------------


typedef std::map<_tstring, IDownloader*> MAPDOWNLOADER;

class CDownloaderMgr : public Mutex
{
public:
	enum DOWNLOADTOOL
	{
		PANDO = 0,
		AKAMAI,
		HTTP,
		HTTP2,
		PDL_DOWNLOAD,
	};

public:
	CDownloaderMgr();
	virtual ~CDownloaderMgr();

	/*
	*  @brief: save the . 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called when coreclient init.
	*/
	virtual bool Init();

	/*
	*  @brief: stop downloads and exit threads. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called when coreclient exit.
	*/
	virtual bool Uninit();

	/*
	*  @brief: stop download with sId. 
	*  @param[in] sId: game(download) id. 
	*  @return: true if succeed.
	*  @notes: called when coreclient exit.
	*/
//	virtual bool Uninit(_tstring sId);

	/*
	*  @brief: start a download. 
	*  @param[in] nDownloadToolType: download tool type, pando or akamai. 
	*  @param[in] sUrl: download url. 
	*  @param[in] sId: game(download) id. 
	*  @param[in] sDst: path to store the download data.
	*  @param[in] pListener: listener to handle download event.
	*  @param[in] sVersion: download tool version.
	*  @return: true if succeed.
	*  @notes: called by Game item when user click download.
	*/
	virtual bool Start(int nDownloadToolType, IDownloaderListener *pListener, DOWNLOAD_SETTING ds);

	/*
	*  @brief: pause download with sId. 
	*  @param[in] sId: game(download) id. 
	*  @return: true if succeed.
	*  @notes: called by Game item when user click pause.
	*/
	virtual bool Pause(_tstring sId);

	/*
	*  @brief: resume download with sId. 
	*  @param[in] sId: game(download) id. 
	*  @return: true if succeed.
	*  @notes: called by Game item when user click resume.
	*/
	virtual bool Resume(_tstring sId);

	/*
	*  @brief: cancel download with sId. 
	*  @param[in] sId: game(download) id. 
	*  @return: true if succeed.
	*  @notes: called by Game item when user click cancel.
	*/
	virtual bool Cancel(_tstring sId);

	/*
	*  @brief: get download attribute with sId. 
	*  @param[in] sId: game(download) id. 
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual DOWNLOAD_PARAM* GetAttribute(_tstring sId);

	IDownloader* GetDownloader(const _tstring& sId);
public:
	MAPDOWNLOADER m_mapDownloader;
	CRITICAL_SECTION m_csAPIMutex;
	int m_nDownloadTool;
	bool m_bInit;
};

//-------------------------------------------

extern CDownloaderMgr g_downloaderMgr;