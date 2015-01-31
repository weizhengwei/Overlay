///*************************************************
/**
 * @file      HttpDownloader.h
   @brief     The header file of http download tool.
   @author    liuxiangshu@pwrd.com
   @data      2013/01/08
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

#include "IDownloader.h"
#include "curl/curl.h"

class CHttpDownloader : public IDownloader
{
public:
	CHttpDownloader(void);
	virtual ~CHttpDownloader(void);

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
	virtual bool Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds);

	/*
	*  @brief: pause if downloading and exit the download thread. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Uninit();

	/*
	*  @brief: start. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool StartDownload();

	/*
	*  @brief: pause. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Pause();

	/*
	*  @brief: resume. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Resume();

	/*
	*  @brief: cancel. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual bool Cancel();

	/*
	*  @brief: GetAttribute. 
	*  @param: none.
	*  @return: true if succeed.
	*  @notes: called by CDownloaderMgr.
	*/
	virtual DOWNLOAD_PARAM* GetAttribute();

	/*
	*  @brief: process operation command and call listener. 
	*  @param: none.
	*  @return: 
	*  @notes: called by thread function.
	*/
	virtual long run();

public:
	static size_t Callback_WriteFunc(void *pBuffer, size_t nSize, size_t nMemb, VOID* pParam);
	static int Callback_Progress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow); 

public:
	DWORD m_dwTick;
	__int64 m_StartSize;
	double m_fDownSize;

	bool m_bAdd;

	int m_nNetworkRetryCount;
	int m_nPartFileRetryCount;
protected:

	CURL *m_curl;
};