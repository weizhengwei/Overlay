///*************************************************
/**
 * @file      HttpDownloader2.cpp
   @brief     The implementation file of http download tool.
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


#include "stdafx.h"
#include "download/HttpDownloader2.h"
#include "data/DataPool.h"
#include "constant.h"
#include "log/local.h"




#define IF_BREAK(exp)	{if(exp)break;}

//-------------------------------------




CHttpDownloader2::CHttpDownloader2()
: m_pIHttpTask(NULL)
, m_bAlwaysConnect(true)
{

}

CHttpDownloader2::~CHttpDownloader2()
{
	Uninit();
}

bool CHttpDownloader2::Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds)
{
	if(ds.sUrl.empty() || ds.sId.empty() || ds.sDst.empty() || !pListener || !pDownloaderMgr || !pMutex)
	{
		return false;
	}
	
	size_t nPos = ds.sDst.ReverseFind('\\');
	if(nPos == _tstring::npos)
	{
		return false;
	}

	m_pListener = pListener;
	m_pDownloaderMgr = pDownloaderMgr;
	m_csAPIMutex = pMutex;
	m_bAlwaysConnect = ds.bAlwaysConnect;

	m_sParam.sUrl = ds.sUrl;
	m_sParam.sId = ds.sId;
	m_sParam.sDst = ds.sDst.Left(nPos);
	m_sParam.sFileName = ds.sDst.Right(ds.sDst.length() - nPos - 1);
	m_sParam.totleSize = 0;
	m_sParam.downloadSize = 0;
	m_sParam.nDownloadPercent = 0;
	m_sParam.nStatus = IDownloader::INIT;
	m_sParam.nErrCode = IDownloader::NOERR;
	m_sParam.nOperation = IDownloader::NOOPERATION;

	if(!theHTTP2DownloaderHelper.IsLibaryInit())
	{
		return false;
	}

	m_pIHttpTask = theHTTP2DownloaderHelper.CreateTask(this, ds.sUrl.c_str(), ds.sDst.c_str(),  ds.bCheckMD5);
	if (m_pIHttpTask == NULL)
	{
		return false;
	}

	return StartDownload();
}

bool CHttpDownloader2::Uninit()
{
	m_bExit = true;

	if(_threadHandle != NULL)
	{
		while(MsgWaitForMultipleObjects(1, &_threadHandle, 
			FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
		{
			MSG msg;
			if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			{ 
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		CloseHandle(_threadHandle);
		_threadHandle = NULL;
		_threadId = 0;
	}

	if (m_pIHttpTask != NULL)
	{
		theHTTP2DownloaderHelper.DestroyTask(m_pIHttpTask);
		m_pIHttpTask = NULL;
	}

	return true;
}

bool CHttpDownloader2::StartDownload()
{
	m_sParam.nOperation = IDownloader::START;

	if(_threadHandle != NULL)
	{
		CloseHandle(_threadHandle);
		_threadId = 0;
	}

	return start();
}

bool CHttpDownloader2::Pause()
{
	m_sParam.nOperation = IDownloader::PAUSE;
	return true;
}

bool CHttpDownloader2::Resume()
{
	m_sParam.nOperation = IDownloader::RESUME;
	return true;
}

bool CHttpDownloader2::Cancel()
{
	m_sParam.nOperation = IDownloader::CANCEL;
	return true;
}

DOWNLOAD_PARAM* CHttpDownloader2::GetAttribute()
{
	return &m_sParam;
}

long CHttpDownloader2::run()
{
	while(!m_bExit)
	{
		switch(m_sParam.nOperation)
		{
		case IDownloader::START:
			OnCmdStart();
			break;
		case IDownloader::PAUSE:
			OnCmdPause();
			break;
		case IDownloader::RESUME:
			OnCmdResume();
			break;
		case IDownloader::CANCEL:
			OnCmdCancel();
			break;
		default:
			break;
		}

		UpdateStatus();

		if(m_sParam.nStatus == IDownloader::CANCELLED)
		{
			if (m_pIHttpTask)
			{
				theHTTP2DownloaderHelper.DestroyTask(m_pIHttpTask);
				m_pIHttpTask = NULL;
			}
			return 0;
		}
		else if(m_sParam.nStatus == IDownloader::COMPLETE)
		{
			if (m_pIHttpTask)
			{
				m_pIHttpTask->Clean(); //delete idx file
				theHTTP2DownloaderHelper.DestroyTask(m_pIHttpTask);
				m_pIHttpTask = NULL;
			}
			return 0;
		}

		Sleep(1000);
	}

	if((m_sParam.nStatus == IDownloader::DOWNLOADING || m_sParam.nStatus == IDownloader::PAUSED) && m_pIHttpTask)
	{
		m_pIHttpTask->Stop();
	}

	return 0;
}

bool CHttpDownloader2::OnCmdStart()
{
	m_sParam.nOperation = IDownloader::NOOPERATION;
	
	if(m_pIHttpTask)
	{
		m_pIHttpTask->Start();
	}

	return true;
}

bool CHttpDownloader2::OnCmdPause()
{
	m_pListener->OnPauseBegin(m_sParam.sId);

	if(m_pIHttpTask)
	{
		if(m_pIHttpTask->CanSuspend())
		{
			m_pIHttpTask->Suspend();
		}
		else
		{
			m_pListener->OnError(m_sParam.sId, IDownloader::ERRPAUSE);
		}
	}

	m_sParam.nOperation = IDownloader::NOOPERATION;

	return true;
}

bool CHttpDownloader2::OnCmdResume()
{
	m_pListener->OnResumeBegin(m_sParam.sId);

	if(m_pIHttpTask)
	{
		if(m_pIHttpTask->CanResume())
		{
			m_pIHttpTask->Resume();
		}
		else
		{
			m_pListener->OnError(m_sParam.sId, IDownloader::ERRRESUME);
		}
	}

	m_sParam.nOperation = IDownloader::NOOPERATION;

	return true;
}

bool CHttpDownloader2::OnCmdCancel()
{
	m_pListener->OnCancelBegin(m_sParam.sId);

	if(m_pIHttpTask)
	{
		// track debug info
		_tstring s = m_pIHttpTask->GetErrorMsg(HD_ERROR_DEBUG);
		m_pListener->OnError(m_sParam.sId, 1200+HD_ERROR_DEBUG, s);

		m_pIHttpTask->Stop();
		m_pIHttpTask->Clean(TRUE);
	}

	m_sParam.nStatus = IDownloader::CANCELLED;
	m_pListener->OnCancelEnd(m_sParam.sId);

	m_sParam.nOperation = IDownloader::NOOPERATION;

	return true;
}

bool CHttpDownloader2::UpdateStatus()
{
	int nStatus = m_pIHttpTask->GetStatus();
	if(nStatus == HD_TASK_STATUS_COMPLETED && m_sParam.nStatus != IDownloader::COMPLETE)
	{
		m_sParam.nStatus = IDownloader::COMPLETE;
		m_pListener->OnComplete(m_sParam.sId);
	}

	if(m_pIHttpTask && m_sParam.nStatus == IDownloader::DOWNLOADING)
	{
		DWORD nRate = m_pIHttpTask->GetDownloadSpeed();
		m_sParam.nRate = (nRate == MAXDWORD ? 0 : nRate);
		m_sParam.downloadSize = m_pIHttpTask->GetDownloadedSize();
		m_sParam.nDownloadPercent = m_pIHttpTask->GetProgress()/100;
		m_pListener->UpdateStatus(m_sParam.sId, &m_sParam);
	}

	return true;
}

BOOL CHttpDownloader2::_HttpCallback(__in HDCallbackType type, __in DWORD dwValue)
{
	if (m_pIHttpTask == NULL)
	{
		return FALSE;
	}

	switch (type)
	{
	case HD_CBTYPE_STATUS_CHANGED: 
		{
			switch(dwValue)
			{
			case HD_TASK_STATUS_INIT:
			case HD_TASK_STATUS_CONNECTING:
				break;
			case HD_TASK_STATUS_DOWNLOADING:
				if(m_sParam.nStatus < IDownloader::DOWNLOADING)
				{
					m_sParam.nStatus = IDownloader::DOWNLOADING;
					m_pListener->OnStartEnd(m_sParam.sId);
				}
				else if(m_sParam.nStatus == IDownloader::PAUSED)
				{
					m_sParam.nStatus = IDownloader::DOWNLOADING;
					m_pListener->OnResumeEnd(m_sParam.sId);
				}
				m_sParam.totleSize = m_pIHttpTask->GetTotalSize();
				break;
			case HD_TASK_STATUS_SUSPENDED:
				if(m_sParam.nStatus == IDownloader::DOWNLOADING)
				{
					m_pListener->OnPauseEnd(m_sParam.sId);
					m_sParam.nStatus = IDownloader::PAUSED;
				}
				m_sParam.nRate = 0;
				m_sParam.nDownloadPercent = m_pIHttpTask->GetProgress()/100;
				m_pListener->UpdateStatus(m_sParam.sId, &m_sParam);
				break;
			case HD_TASK_STATUS_STOPPING:
			case HD_TASK_STATUS_STOPPED:
				break;
			case HD_TASK_STATUS_COMPLETED:
				{
					// track debug info
					_tstring s = m_pIHttpTask->GetErrorMsg(HD_ERROR_DEBUG);
					m_pListener->OnError(m_sParam.sId, 1200+HD_ERROR_DEBUG, s);
				}
				m_sParam.nStatus = IDownloader::COMPLETE;
				m_pListener->OnComplete(m_sParam.sId);
				break;
			case HD_TASK_STATUS_ERROR:
				break;
			}
		}
		break;

	case HD_CBTYPE_PROGRESS_CHANGED:
		break;

	case HD_CBTYPE_COMPLETE:
		break;

	case HD_CBTYPE_ERROR:
		{
			// track error
			_tstring s = m_pIHttpTask->GetErrorMsg(HD_ERROR_DEBUG);
			m_pListener->OnError(m_sParam.sId, 1200+dwValue, s);

			switch (dwValue)
			{
			case HD_ERROR_SUCCESS: break;
			case HD_ERROR_INVALID_URL: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_CANNOT_CONNECT: 
				m_pListener->OnError(m_sParam.sId, IDownloader::START);
				break;
			case HD_ERROR_AUTH_CERT_NEEDED: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_LOGIN_FAILURE: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_SERVER_FILENOTFOUND: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_SERVER_FILEMODIFIED: 
				break;
			case HD_ERROR_SERVER_FORBIDDEN: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_SERVER_TIMEOUT: 
				OutputDebugString(_T("timeout\n"));
				if(m_sParam.nStatus < IDownloader::DOWNLOADING)
				{
					m_pListener->OnError(m_sParam.sId, IDownloader::ERRSTART);
				}
				else
				{
					if(m_bAlwaysConnect)
					{
						m_sParam.nOperation = IDownloader::START;
					}
					else
					{
						m_pListener->OnError(m_sParam.sId, IDownloader::ERRCONNECT);
					}
				}
				break;
			case HD_ERROR_FILE_INSUFFICIENTDISKSPACE: 
				m_pListener->OnError(m_sParam.sId, IDownloader::NOTENOUGHSPACE);
				break;
			case HD_ERROR_FILE_ACCESS_DENIED: 
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_FILE_DISK_ERROR:
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::PATHNOTFOUND);
				break;
			case HD_ERROR_FILE_MD5FILENOTFOUND:
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_FILE_MD5FILEERROR:
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_MEMORY_INSUFFICIENT: 
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_THREAD_CREATEFAILED:
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				break;
			case HD_ERROR_NAME_NOT_RESOLVED:
				m_sParam.nStatus = IDownloader::CANCELLED;
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRSTART);
				break;

			default: ATLASSERT(FALSE); break;
			}
		} break;
	default: 
		ATLASSERT(FALSE); 
		break;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
CHttpDownloader2Helper theHTTP2DownloaderHelper;

bool CHttpDownloader2Helper::InitializeHTTPDownlader2(DWORD& dwErrorCode)
{
	MutexGuard autoLock(this);
	
	if (m_hHttp2Module != NULL)
	{
		UninitlizeHTTPDownlader2();
	}

	m_hHttp2Module = ::LoadLibrary(_T("HttpDownloader.dll"));
	if(m_hHttp2Module == NULL)
	{
		return false;
	}

	FNHTTPDOWNINITIALIZE fnHttpInitialize = (FNHTTPDOWNINITIALIZE)::GetProcAddress(m_hHttp2Module, "HttpDownInitialize");
	if(fnHttpInitialize == NULL)
	{
		return false;
	}

	if(!fnHttpInitialize(HTTPDOWN_VERSION_CURRENT, _HttpCallBack, this))
	{
		return false;
	}

	return true;
}

bool CHttpDownloader2Helper::UninitlizeHTTPDownlader2()
{
	MutexGuard autoLock(this);
	
	__InerUninitlizeHTTPDownlader2();

	return true;
}

void CHttpDownloader2Helper::__InerUninitlizeHTTPDownlader2()
{
	__try
	{
		if(m_hHttp2Module)
		{
			FNHTTPDOWNUNINITIALIZE fnHttpUninitialize = (FNHTTPDOWNUNINITIALIZE)::GetProcAddress(m_hHttp2Module, "HttpDownUninitialize");
			if(fnHttpUninitialize)
			{
				fnHttpUninitialize();
			}

			::FreeLibrary(m_hHttp2Module);
			m_hHttp2Module = NULL;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Log(_T("HttpDownUninitialize occur exception!!"));
	}
	 
}

BOOL CHttpDownloader2Helper::_HttpCallBack(IHttpDownTask *pIHttpDownTask, LPVOID lpCaller, HDCallbackType dwType, DWORD dwValue)
{
	CHttpDownloader2Helper* lpHelper = (CHttpDownloader2Helper*)lpCaller;
	if(lpHelper == NULL)
		return FALSE;

	CHttpDownloader2* pDownloader = (CHttpDownloader2*)pIHttpDownTask->GetCustomData();
	if (pDownloader != NULL)
	{
		return pDownloader->_HttpCallback(dwType, dwValue);
	}

	return FALSE;
}

IHttpDownTask* CHttpDownloader2Helper::CreateTask(CHttpDownloader2* pDownloader,LPCTSTR lpszURL, LPCTSTR lpszSavePath,BOOL bMD5Check)
{
	MutexGuard autoLock(this);
	
	IHttpDownTask* pTask = NULL;
	FNHTTPDOWNCREATETASK fnHttpCreateTask = (FNHTTPDOWNCREATETASK)::GetProcAddress(m_hHttp2Module, "HttpDownCreateTask");
	if(fnHttpCreateTask)
	{
		if(HD_CREATETASK_ERROR_SUCCESS != fnHttpCreateTask(lpszURL, lpszSavePath, &pTask, bMD5Check))
		{
			return NULL;
		}

		pTask->SetCustomData((DWORD)pDownloader);
	}
	
	return pTask;
}

bool CHttpDownloader2Helper::DestroyTask(IHttpDownTask* pTask)
{
	MutexGuard autoLock(this);

	if (pTask == NULL)
	{
		return false;
	}

	FNHTTPDOWNDESTORYTASK fnHttpDestroyTask = (FNHTTPDOWNDESTORYTASK)::GetProcAddress(m_hHttp2Module, "HttpDownDestoryTask");
	if (fnHttpDestroyTask == NULL)
	{
		return false;
	}

	pTask->SetCustomData(NULL);
	fnHttpDestroyTask(&pTask);

	return true;
}