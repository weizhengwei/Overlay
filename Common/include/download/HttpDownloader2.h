///*************************************************
/**
 * @file      HttpDownloader2.h
   @brief     The header file of http download tool.
   @author    liuxiangshu@pwrd.com
   @data      2012/06/06
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

#include "httpdownloader\HttpDownloader.h"
#include "IDownloader.h"
#include <map>

class CHttpDownloader2;
typedef std::map<IHttpDownTask*, CHttpDownloader2*> MAPDOWNLOADERTASK;

class CHttpDownloader2 : public IDownloader
{
public:
	CHttpDownloader2();
	virtual ~CHttpDownloader2();

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

protected:
	friend class CHttpDownloader2Helper;
	BOOL _HttpCallback(__in HDCallbackType type, __in DWORD dwValue);
	
private:
	bool OnCmdStart();
	bool OnCmdPause();
	bool OnCmdResume();
	bool OnCmdCancel();
	bool UpdateStatus();

private:
	IHttpDownTask *m_pIHttpTask;
	bool m_bAlwaysConnect;
};


class CHttpDownloader2Helper : public Mutex
{
public:
	CHttpDownloader2Helper(){m_hHttp2Module = NULL; }
	~CHttpDownloader2Helper(){m_hHttp2Module = NULL;}

	bool InitializeHTTPDownlader2(DWORD& dwErrorCode);
	bool UninitlizeHTTPDownlader2();
	bool IsLibaryInit(){return m_hHttp2Module != NULL;}

	IHttpDownTask* CreateTask(CHttpDownloader2* pDownloader,LPCTSTR lpszURL, LPCTSTR lpszSavePath, BOOL bMD5Check);
	bool	DestroyTask(IHttpDownTask* pTask);
private:
	static BOOL _HttpCallBack(IHttpDownTask *pIHttpDownTask, LPVOID lpCaller, HDCallbackType dwType, DWORD dwValue);
	
	void __InerUninitlizeHTTPDownlader2();
private:
	HMODULE			m_hHttp2Module;
};

extern CHttpDownloader2Helper theHTTP2DownloaderHelper;