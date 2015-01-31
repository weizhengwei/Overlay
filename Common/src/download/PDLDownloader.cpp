#include "stdafx.h"
#include "download/PDLDownloader.h"
#include "data/DataPool.h"
#include "log/local.h"
#include "data/Tracking.h"

#define IF_BREAK(exp)	{if(exp)break;}
#define PDL_DLL_NAME	_T("pdl.dll")

//////////////////////////////////////////////////////////////////////////

CPDLDownloader::CPDLDownloader(void)
{
	MutexGuard autoLock(&m_lock);
	m_pGameTask = NULL;
	m_bAlwaysConnect = false;
	m_preOperator = IDownloader::NOOPERATION;

	m_hOpEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_dwLastUpdateTime = 0;

	m_speedCount = 0;
	m_speedTotal = 0;
/*	m_retryDownloadJson = 10;*/
	m_retryDownloadInitPackage = 50;
	m_retryDownloadMiniPackage = 50;
	m_retryActiveDownloadPackage = 50;
	m_retryPassiveDownloadPackage = 50;
	m_retryPassiveDownloadDiskError = 50;

	m_hasSendLaunchPoint = false;

	m_bPlayStartGame = false;
	m_bSendError = false;
}

CPDLDownloader::~CPDLDownloader(void)
{
	MutexGuard autoLock(&m_lock);
	Uninit();

	CloseHandle(m_hOpEvent);

}

bool CPDLDownloader::Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds)
{
	MutexGuard autoLock(&m_lock);
	if(ds.sUrl.empty() || ds.sId.empty() || ds.sDst.empty() || !pListener || !pDownloaderMgr || !pMutex)
	{
		return false;
	}


	m_pListener = pListener;
	m_pDownloaderMgr = pDownloaderMgr;
	m_csAPIMutex = pMutex;
	m_bAlwaysConnect = ds.bAlwaysConnect;

	m_sParam.sDst = ds.sDst;
	if (ds.sDst[ds.sDst.length() - 1] != _T('\\'))
	{
		m_sParam.sDst = m_sParam.sDst + _T("\\");
	}
	m_sParam.sUrl = ds.sUrl;
	m_sParam.sId = ds.sId;
	
	m_sParam.sFileName = ds.sGameName;
	m_sParam.totleSize = 0;
	m_sParam.downloadSize = 0;
	m_sParam.nDownloadPercent = 0;
	m_sParam.nStatus = IDownloader::INIT;
	m_sParam.nErrCode = IDownloader::NOERR;
	m_sParam.nOperation = IDownloader::NOOPERATION;
	
	m_pGameTask = thePDLHelper.CreateTask(ds.sGameName.c_str(), m_sParam.sUrl.c_str(), m_sParam.sDst.c_str(), this);
	if (m_pGameTask == NULL)
	{
		return FALSE;
	}

	return StartDownload();
}

bool CPDLDownloader::Uninit()
{
	m_lock.lock();
	m_bExit = true;
	m_lock.unlock();

	SetEvent(m_hOpEvent);
	if (_threadHandle != NULL)
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
	}

	if (m_pGameTask != NULL)
	{
		m_pGameTask->SetCustomData(NULL);
		thePDLHelper.StopTask(m_pGameTask);
		thePDLHelper.DestroyTask(m_pGameTask);
		m_pGameTask = NULL;
	}

	
	return true;
}

bool CPDLDownloader::PlayPDL()
{
	MutexGuard autoLock(&m_lock);
	
	if (m_pGameTask == NULL || _threadHandle == NULL)
	{
		return false;
	}

	if (!m_hasSendLaunchPoint)
	{
		return false;
	}
	
	if (m_bPlayStartGame)
		return false;

	m_sParam.nOperation = IDownloader::START;
	m_bPlayStartGame = true;
	SetEvent(m_hOpEvent);
	
	return true;
}

bool CPDLDownloader::StartDownload()
{
	MutexGuard autoLock(&m_lock);
	m_sParam.nOperation = IDownloader::START;

	if(_threadHandle != NULL)
	{
		CloseHandle(_threadHandle);
		_threadId = 0;
	}

	SetEvent(m_hOpEvent);
	return start();
}

bool CPDLDownloader::Pause()
{
	MutexGuard autoLock(&m_lock);
	m_sParam.nOperation = IDownloader::PAUSE;

	SetEvent(m_hOpEvent);
	return true;
}

bool CPDLDownloader::Resume()
{
	MutexGuard autoLock(&m_lock);
	m_sParam.nOperation = IDownloader::RESUME;
	
	SetEvent(m_hOpEvent);
	return true;
}

bool CPDLDownloader::Cancel()
{
	MutexGuard autoLock(&m_lock);
	m_sParam.nOperation = IDownloader::CANCEL;
	
	SetEvent(m_hOpEvent);
	return true;
}

DOWNLOAD_PARAM* CPDLDownloader::GetAttribute()
{
	MutexGuard autoLock(&m_lock);
	return &m_sParam;
}

long CPDLDownloader::run()
{
	while(!m_bExit)
	{
		DWORD bRet = WaitForSingleObject(m_hOpEvent, 60);
		if(bRet == WAIT_OBJECT_0)
		{
			ResetEvent(m_hOpEvent);
			//MutexGuard autoLock(&m_lock);
			switch(m_sParam.nOperation)
			{
			case IDownloader::START:
				OnOpStart();
				break;
			case IDownloader::PAUSE:
				OnOpPause();
				break;
			case IDownloader::RESUME:
				OnOpResume();
				break;
			case IDownloader::CANCEL:
				OnOpCancle();
				break;
			}

			if (m_sParam.nStatus == IDownloader::CANCELLED || m_sParam.nStatus == IDownloader::COMPLETE)
			{
				if (m_sParam.nStatus == COMPLETE)
				{
					thePDLHelper.StopTask(m_pGameTask);
					// Clear the download data only, uninstall will use standard method which will delete GameDir
					thePDLHelper.CleanTask(m_pGameTask, false);

					m_pListener->OnComplete(m_sParam.sId); 
				}
				return 0;
			}
		}
		else
		{
			continue;
		}
	}

	return 0;
}

void CPDLDownloader::OnOpStart()
{
	//MutexGuard autoLock(&m_lock);
// 	m_lock.lock();
// 	m_preOperator = IDownloader::START;
// 	m_sParam.nOperation = IDownloader::NOOPERATION;
// 	m_lock.unlock();

	m_lock.lock();
	m_preOperator = IDownloader::START;
	m_sParam.nOperation = IDownloader::NOOPERATION;
	m_sParam.nStatus = IDownloader::DOWNLOADING;
	m_lock.unlock();

	if(m_pGameTask)
	{	
		//m_lock.lock();
		if (m_pGameTask->IsRunning())
		{
			thePDLHelper.StopTask(m_pGameTask);
		}
		//m_lock.unlock();
		
		if(!m_bPlayStartGame)
		{
			m_pListener->OnStartBegin(m_sParam.sId);
			//OutputDebugString(_T("Start begin\n"));
		}
		
		thePDLHelper.StartTask(m_pGameTask);
	
		if(!m_bPlayStartGame && !m_bSendError && m_sParam.nStatus != COMPLETE)
		{
			m_pListener->OnStartEnd(m_sParam.sId);
			//OutputDebugString(_T("Start end\n"));
		}
	}
	else
	{
// 		m_lock.lock();
// 		m_preOperator = IDownloader::NOOPERATION;
// 		m_lock.unlock();
	}
}

void CPDLDownloader::OnOpPause()
{
	//MutexGuard autoLock(&m_lock);
// 	m_lock.lock();
// 	m_preOperator = IDownloader::PAUSE;
// 	m_sParam.nOperation = IDownloader::NOOPERATION;
// 	m_lock.unlock();
	
	m_lock.lock();
	m_preOperator = IDownloader::PAUSE;
	m_sParam.nOperation = IDownloader::NOOPERATION;
	m_sParam.nStatus = IDownloader::PAUSED;
	m_lock.unlock();

	if(m_pGameTask && (m_pGameTask->GetStatus() & PDL_STATUS_STOPPED) == 0)
	{
		m_pListener->OnPauseBegin(m_sParam.sId);
		//OutputDebugString(_T("Pause begin\n"));
		thePDLHelper.StopTask(m_pGameTask);

		if(!m_bSendError && m_sParam.nStatus != COMPLETE)
		{
			m_pListener->OnPauseEnd(m_sParam.sId);
			//OutputDebugString(_T("Pause end\n"));
		}
	}
	else
	{
		//OutputDebugString(_T("Pause begin simulate\n"));
		m_pListener->OnPauseBegin(m_sParam.sId);
// 		m_lock.lock();
// 		m_preOperator = IDownloader::NOOPERATION;
// 		m_lock.unlock();
		if(!m_bSendError && m_sParam.nStatus != COMPLETE)
		{
			//OutputDebugString(_T("Pause end simulate\n"));
			m_pListener->OnPauseEnd(m_sParam.sId);
		}
	}

	m_bPlayStartGame = false;

}

void CPDLDownloader::OnOpResume()
{
	//MutexGuard autoLock(&m_lock);
// 	m_lock.lock();
// 	m_preOperator = IDownloader::RESUME;
// 	m_sParam.nOperation = IDownloader::NOOPERATION;
// 	m_lock.unlock();

	m_lock.lock();
	m_preOperator = IDownloader::RESUME;
	m_sParam.nOperation = IDownloader::NOOPERATION;
	m_sParam.nStatus = IDownloader::DOWNLOADING;
	m_lock.unlock();

	if (m_pGameTask && (m_pGameTask->GetStatus() & PDL_STATUS_STOPPED) != 0)
	{
		m_pListener->OnResumeBegin(m_sParam.sId);
		//OutputDebugString(_T("Resum begin\n"));
		thePDLHelper.StartTask(m_pGameTask);

		if(!m_bSendError && m_sParam.nStatus != COMPLETE)
		{
			m_pListener->OnResumeEnd(m_sParam.sId);
			//OutputDebugString(_T("Resum end\n"));
		}
	}
	else
	{
		//OutputDebugString(_T("Resum begin simulate\n"));
		m_pListener->OnResumeBegin(m_sParam.sId);
// 		m_lock.lock();
// 		m_preOperator = IDownloader::NOOPERATION;
// 		m_lock.unlock();
		if(!m_bSendError && m_sParam.nStatus != COMPLETE)
		{
			m_pListener->OnResumeEnd(m_sParam.sId);
			//OutputDebugString(_T("Resum end simulate\n"));
		}
	}

	
}

void CPDLDownloader::OnOpCancle()
{
	//MutexGuard autoLock(&m_lock);
// 	m_lock.lock();
// 	m_preOperator = IDownloader::CANCEL;
// 	m_sParam.nOperation = IDownloader::NOOPERATION;
// 	m_lock.unlock();

	m_lock.lock();
	m_preOperator = IDownloader::CANCEL;
	m_sParam.nOperation = IDownloader::NOOPERATION;
	m_sParam.nStatus = IDownloader::CANCELLED;
	m_lock.unlock();
	
	if (m_pGameTask && (m_pGameTask->GetStatus() & PDL_STATUS_STOPPED) == 0)
	{
		//OutputDebugString(_T("Cancle begin stop clean\n"));
		m_pListener->OnCancelBegin(m_sParam.sId);

		thePDLHelper.StopTask(m_pGameTask);
		thePDLHelper.CleanTask(m_pGameTask,true);

		//if(!m_bSendError)
		{
			m_pListener->OnCancelEnd(m_sParam.sId);
			//OutputDebugString(_T("Cancle end stop clean\n"));
		}
	}
	else if(m_pGameTask != NULL)
	{
		//OutputDebugString(_T("Cancle begin clean\n"));
		m_pListener->OnCancelBegin(m_sParam.sId);

		thePDLHelper.CleanTask(m_pGameTask,true);

// 		m_lock.lock();
// 		m_preOperator = IDownloader::NOOPERATION;
// 		m_lock.unlock();
		
		//if(!m_bSendError)
		{
			m_pListener->OnCancelEnd(m_sParam.sId);
			//OutputDebugString(_T("Cancle end clean\n"));
		}
	}

	m_bPlayStartGame = false;
}


BOOL CPDLDownloader::OnPDLMessage(DWORD dwType, DWORD dwValue)
{
	//MutexGuard autoLock(&m_lock);
	switch (dwType)
	{
 	case PDL_CBTYPE_STATUS_CHANGED: 
 		{
// 			m_lock.lock();
// 			DOWNLOAD_OPERATION preOp = m_preOperator;
// 			m_lock.unlock();
// 
// 			if ((dwValue & PDL_STATUS_STOPPED) != 0 && preOp == IDownloader::PAUSE)
// 			{
// 				m_lock.lock();
// 				m_preOperator = IDownloader::NOOPERATION;
// 				m_sParam.nStatus = IDownloader::PAUSED;
// 				m_lock.unlock();
// 
// 				m_pListener->OnPauseEnd(m_sParam.sId);
// 			}
// 			else if ((dwValue & PDL_STATUS_STOPPED) != 0 && preOp == IDownloader::CANCEL)
// 			{
// 				m_lock.lock();
// 				m_sParam.nStatus = IDownloader::CANCELLED;
// 				m_preOperator = IDownloader::NOOPERATION;
// 				m_lock.unlock();
// 
// 				SetEvent(m_hOpEvent);
// 
// 				m_pListener->OnCancelEnd(m_sParam.sId);
// 			}
// 			else if ((dwValue & PDL_STATUS_PAUSED) == 0 && preOp == IDownloader::RESUME)
// 			{
// 				m_lock.lock();
// 				m_sParam.nStatus = IDownloader::DOWNLOADING;
// 				m_preOperator = IDownloader::NOOPERATION;
// 				m_lock.unlock();
// 
// 				m_pListener->OnResumeEnd(m_sParam.sId);
// 			}
// 			else 
// 			{
// 				if(preOp == IDownloader::START)
// 				{
// 					m_lock.lock();
// 					m_sParam.nStatus = IDownloader::DOWNLOADING;
// 					m_preOperator = IDownloader::NOOPERATION;
// 					m_lock.unlock();
// 					
// 					if(!m_bPlayStartGame)
// 						m_pListener->OnStartEnd(m_sParam.sId);
// 				}
// 			}
 		}
 		break;
	case PDL_CBTYPE_DOWNLOADING_INITPACKAGE:
	case PDL_CBTYPE_UNCOMPRESSING_INITPACKAGE:
	case PDL_CBTYPE_DOWNLOADING_MINPACKAGE:
	case PDL_CBTYPE_RUNNING:
		{
			m_lock.lock();
			DWORD lastUpdateTime = m_dwLastUpdateTime;
			DWORD dwCurTime = GetTickCount();
			DWORD dwCurrentSpeed =  m_pGameTask->GetDownloadSpeed();
			m_lock.unlock();

			
			if (dwCurTime - 500 >= lastUpdateTime)
			{
				m_lock.lock();
				DWORD dwStatus = m_pGameTask->GetStatus();
				dwStatus &= 0x00FFFFFF;

				DWORD progress  = 0;
				if (dwStatus < PDL_STATUS_RUN)
				{ 
					progress = m_pGameTask->GetProgress(TRUE)/100*0.5;
				}
				else if(dwStatus == PDL_STATUS_RUN)
				{
					progress = m_pGameTask->GetProgress(FALSE)/100*0.5 + 50;
				}

				if (progress < 0)
					progress = 0;
				if (progress > 100)
					progress = 100;

				m_sParam.nDownloadPercent = progress;
				m_sParam.nRate = dwCurrentSpeed  * 1024;
				m_dwLastUpdateTime = ::GetTickCount();
				m_lock.unlock();

				m_pListener->UpdateStatus(m_sParam.sId, &m_sParam);
			}

			if(dwCurTime - 30 * 1000>= lastUpdateTime)
			{
				InterlockedExchangeAdd(&m_speedTotal, dwCurrentSpeed);
				InterlockedIncrement(&m_speedCount);
			}
		}
		break;
	case PDL_CBTYPE_NEEDLAUNCHGAME: 
		{
			if(m_pGameTask->IsCompleted())
				break;
			if (!m_hasSendLaunchPoint && m_speedCount != 0 && m_speedTotal != 0)
			{
				LONG avgSpeed = m_speedTotal / m_speedCount;
				_tstring sSpeed;
				sSpeed.Format(1, _T("%ld"), avgSpeed);
				m_pListener->OnTraceMessage(GAMETRACE_PDL_SPEED_TRACE, sSpeed);
			}

			if (!m_hasSendLaunchPoint)
			{
				m_pListener->OnLauchPoint(m_sParam.sId);
				m_hasSendLaunchPoint = true;
			}

			if(m_bPlayStartGame)
			{
				m_pListener->OnPlayPDLGame(m_sParam.sId);
				m_bPlayStartGame = false;
			}
		}
		break;
	case PDL_CBTYPE_COMPLETED: 
		{
 			m_lock.lock();
			m_sParam.nStatus = IDownloader::COMPLETE;
			m_lock.unlock();

			SetEvent(m_hOpEvent);// 不能在这里处理， 需要在线程里面处理
		}
		break;
	case PDL_CBTYPE_PASSIVE_DOWNLOAD_REPORT:
		m_pListener->OnTraceMessage(GAMETRACE_PDL_INTERNAL_TRACE, _tstring((LPCTSTR)dwValue));
		break;
	case PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_CHECKFAILED:
		return TRUE;
		break;
	case PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_CHECKFAILED:
		OnError(UNKONWNSTATUS, _T("check pdl data failed!"));
		break;
	default: 
		OtherHandler(dwType, dwValue);
		break;
	}
	return TRUE;
}

BOOL CPDLDownloader::OtherHandler(DWORD dwType, DWORD dwValue)
{
	//MutexGuard autoLock(&m_lock);
	switch(dwType)
	{
	case PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_FAILED:
		{	
			DWORD ret = InterlockedDecrement(&m_retryPassiveDownloadPackage);
			if (ret <= 0)
			{
				OnError(ERRCONNECT, _T("PDL download error, passtive download occur network error."));
				return FALSE;
			}
		} 
		break;
	case PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_DISKERROR:
		{
			DWORD ret = InterlockedDecrement(&m_retryPassiveDownloadDiskError);
			if (ret <= 0)
			{
				OnError(UNKONWNSTATUS, _T("PDL download error, passtive download occur disk error."));
				return FALSE;
			}
		}
		break;

	case PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_FAILED:
		{
			DWORD ret = InterlockedDecrement(&m_retryActiveDownloadPackage);
			if (ret <= 0)
			{
				OnError(UNKONWNSTATUS, _T("PDL download error, active download occur download error."));
				return FALSE;
			}
		}
		break;
	case PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_DISKERROR:
		{
			OnError(UNKONWNSTATUS, _T("PDL download error, active download occur disk error."));
			return FALSE;
		} 
		break;

	case PDL_CBTYPE_ERROR:
		{
			switch (dwValue)
			{
			case PDL_ERROR_NEEDUPDATE: OnError(ERRINTERNAL,_T("pdl version mismatch, need update pdl.dll!")); break;
			case PDL_ERROR_DOWNLOADJSON_DISKFULL: OnError(NOTENOUGHSPACE,_T("PDL download error, download json file occur no enough space")); break;
			case PDL_ERROR_DOWNLOADJSON_ACCESS_DENIED: OnError(ERRPERMISSION,_T("PDL download error, download json file occur no permission")); break;
			case PDL_ERROR_DOWNLOADJSON_DATAERROR: OnError(ERRINTERNAL,_T("PDL download error，download json file occur json file format error")); break;
			case PDL_ERROR_CREATE_GAMEDIR_ACCESS_DENIED: OnError(ERRPERMISSION,_T("Create game directory fail because of no permision")); break;
			case PDL_ERROR_CREATE_GAMEDIR_INVALID_NAME: OnError(ERRINTERNAL,_T("Create game directory fail because of path is invalid")); break;
			case PDL_ERROR_CREATE_GAMEDIR_TOOLONG: OnError(ERRINTERNAL,_T("Create game directory fail because of path is too long")); break;
			case PDL_ERROR_CREATE_GAMEDIR_PATH_NOT_FOUND: OnError(PATHNOTFOUND,_T("Create game directory fail because of path not find")); break;
			case PDL_ERROR_CREATE_GAMEDIR_INSUFFICIENT_DISK_SPACE: OnError(NOTENOUGHSPACE,_T("Create game directory fail because of no enough space")); break;
			case PDL_ERROR_REGISTERDRIVER_QUERYDOSDEVICE: OnError(ERRINTERNAL,_T("regist driver failed, qury dos device failed")); break;
			case PDL_ERROR_REGISTERDRIVER_COMMUNICATION: OnError(ERRINTERNAL,_T("regist driver failed, cannot connect comm port")); break;
			case PDL_ERROR_REGISTERDRIVER_SENDMESSAGE: OnError(ERRINTERNAL,_T("regist driver failed, cannot send message to comm port")); break;
			case PDL_ERROR_REGISTERDRIVER_INSTANCE_ALTITUDE_COLLISION: OnError(ERRINTERNAL,_T("regist driver failed, alitiude collison")); break;
			case PDL_ERROR_INIT_FILEMANAGE_JSONDATA: OnError(ERRINTERNAL,_T("init file manager failed, json file format error")); break;
			case PDL_ERROR_INIT_FILEMANAGE_DISKFULL: OnError(NOTENOUGHSPACE,_T("init file manager failed, no enough space")); break;
			case PDL_ERROR_INIT_FILEMANAGE_ACCESS_DENIED: OnError(ERRPERMISSION,_T("init file manager failed, no permission")); break;
			case PDL_ERROR_DOWNLOAD_INITPACKAGE_DISKFULL: OnError(NOTENOUGHSPACE,_T("download init package error, no enough space")); break;
			case PDL_ERROR_DOWNLOAD_INITPACKAGE_ACCESS_DENIED: OnError(ERRPERMISSION,_T("download init package error, no permision")); break;
			case PDL_ERROR_UNCOMPRESS_INITPACKAGE_DISKFULL: OnError(NOTENOUGHSPACE,_T("uncompress error，no enough space")); break;
			case PDL_ERROR_UNCOMPRESS_INITPACKAGE_ACCESS_DENIED: OnError(ERRPERMISSION,_T("uncompress error, no permission")); break;
			case PDL_ERROR_DOWNLOAD_MINPACKAGE_DISKERROR: OnError(ERRINTERNAL,_T("download mini package fail, disk error")); break;
			case PDL_ERROR_INIT_FILEMANAGE_PDLDATA_CHECKFILE: OnError(UNKONWNSTATUS, _T("init file manager failed, error to pdl check data failed")); break;
			case PDL_ERROR_DOWNLOAD_MINPACKAGE_CHECKFAILED: /*do not need to handle*/ break;
			case PDL_ERROR_DOWNLOADJSON_NETWORK: 
				{
					OnError(ERRCONNECT, _T("PDL download error, download json file occur network error."));
// 					DWORD ret = InterlockedDecrement(&m_retryDownloadJson);
// 					if (ret <= 0)
// 					{
// 						OnError(ERRCONNECT, _T("PDL download error, download json file occur network error."));
// 						return FALSE;
// 					}
					break;
				}
			case PDL_ERROR_DOWNLOAD_INITPACKAGE_NETWORK: 
				{
					DWORD ret = InterlockedDecrement(&m_retryDownloadInitPackage);
					if (ret <= 0)
					{
						OnError(ERRCONNECT, _T("PDL download error, download init package occur network error."));
						return FALSE;
					}
					break;
				}
			case PDL_ERROR_DOWNLOAD_MINPACKAGE_NETWORK: 
				{

					DWORD ret = InterlockedDecrement(&m_retryDownloadMiniPackage);
					if (ret <= 0)
					{
						OnError(ERRCONNECT, _T("PDL download error, download mini package occur network error."));
						return FALSE;
					}
					break;
				}
			default: 
				break;
			}
	
		} 
		break;
	default: 
		break;
	}

	return TRUE;
}

BOOL CPDLDownloader::OnError(IDownloader::DOWNLOAD_ERROR nErr, LPCTSTR szMessage)
{
	if (m_pListener != NULL)
	{
		//m_sParam.nStatus = IDownloader::ERR;
		Log(_T("CPDLDownloader::OnError -> AppID = %s, szMessage = %s"), m_sParam.sId.c_str(), szMessage);
		m_bSendError = true;
		return m_pListener->OnError(m_sParam.sId, nErr, _tstring(szMessage));
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
CPDLHelper thePDLHelper;

CPDLHelper::CPDLHelper()
{	
	MutexGuard autoLock(this);

	m_hPDLModule = NULL;
}

CPDLHelper::~CPDLHelper()
{
}

bool CPDLHelper::InitializePDL(DWORD& dwErrorCode)
{
	MutexGuard autoLock(this);
	
	dwErrorCode = PDL_INIT_ERR_LOADDRIVER_UNKOWN_ERROR;
	bool bRet = false;
	do 
	{
		m_hPDLModule = ::LoadLibrary(PDL_DLL_NAME);
		IF_BREAK(m_hPDLModule == NULL);

		FNDLLPDLINITIALIZE fnPDLInitialize = (FNDLLPDLINITIALIZE)::GetProcAddress(m_hPDLModule, "DllPDLInitialize");
		IF_BREAK(fnPDLInitialize == NULL);

		_tstring szPDLRootDir = theDataPool.GetBaseDir() + PDL_CONFIG_DIR;
		dwErrorCode = fnPDLInitialize(szPDLRootDir.c_str(), _PDLCallBack, this, NULL);
		if (dwErrorCode != PDL_INIT_ERR_SUCCESS)
		{
			break;
		}

		bRet = true;
	} while (0);

	return bRet;
}

bool CPDLHelper::UninitlizePDL()
{
	MutexGuard autoLock(this);

	bool bRet = false;
	do 
	{
		IF_BREAK(m_hPDLModule == NULL);

		FNDLLPDLUNINITIALIZE fnPDLUnitialize = (FNDLLPDLUNINITIALIZE)::GetProcAddress(m_hPDLModule, "DllPDLUninitialize");
		IF_BREAK(fnPDLUnitialize == NULL);

		fnPDLUnitialize();

		FreeLibrary(m_hPDLModule);
		m_hPDLModule = NULL;
		bRet = true;
	} while (0);

	return bRet;
}

IGameTask*	CPDLHelper::CreateTask(LPCTSTR lpszJsonFilename, LPCTSTR lpszJsonURL, LPCTSTR lpszGameDir,CPDLDownloader* pDownloader)
{
	//MutexGuard autoLock(this);

	if (lpszJsonFilename == NULL || lpszJsonURL == NULL || lpszGameDir == NULL || pDownloader == NULL)
	{
		return NULL;
	}

	FNDLLPDLCREATEGAMETASK fnCreateTask = (FNDLLPDLCREATEGAMETASK)GetProcAddress(m_hPDLModule, "DllPDLCreateGameTask");
	if (fnCreateTask == NULL)
	{
		return NULL;
	}

	IGameTask* pGameTask = NULL;
	fnCreateTask(lpszJsonFilename, lpszJsonURL, lpszGameDir, &pGameTask);

	if (pGameTask != NULL)
	{
		pGameTask->SetCustomData((DWORD)(pDownloader));
	}
	return pGameTask;
}

BOOL CPDLHelper::DestroyTask(IGameTask* pTask)
{
	MutexGuard autoLock(this);

	if (pTask == NULL)
	{
		return FALSE;
	}

	FNDLLPDLDESTROYGAMETASK fnDestroy = (FNDLLPDLDESTROYGAMETASK)GetProcAddress(m_hPDLModule, "DllPDLDestroyGameTask");
	if (fnDestroy == NULL)
	{
		return FALSE;
	}
	
	pTask->SetCustomData(NULL);
	fnDestroy(&pTask);

	return TRUE;
}

void CPDLHelper::StartTask(IGameTask* pTask)
{	
	MutexGuard autoLock(this);

	FNDLLPDLSTARTGAMETASK fnStartTask = (FNDLLPDLSTARTGAMETASK)GetProcAddress(m_hPDLModule, "DllPDLStartGameTask");
	if (fnStartTask != NULL)
	{
		fnStartTask(pTask);
	}
}

void CPDLHelper::StopTask(IGameTask* pTask)
{
	MutexGuard autoLock(this);

	FNDLLPDLSTOPGAMETASK fnStopTask = (FNDLLPDLSTOPGAMETASK)GetProcAddress(m_hPDLModule, "DllPDLStopGameTask");
	if (fnStopTask != NULL)
	{
		fnStopTask(pTask);
	}
}

void CPDLHelper::CleanTask(IGameTask* pTask, BOOL bIncludeGameFiles)
{
	MutexGuard autoLock(this);

	FNDLLPDLCLEANGAMETASK fnCleanTask = (FNDLLPDLCLEANGAMETASK)GetProcAddress(m_hPDLModule, "DllPDLCleanGameTask");
	if (fnCleanTask != NULL)
	{
		fnCleanTask(pTask, bIncludeGameFiles);
	}
}

int	 CPDLHelper::GetConfigValue(DWORD dwConfigID, LPTSTR lpReturnValue, DWORD dwSize)
{
	MutexGuard autoLock(this);

	FNDLLPDLGETCONFIGVALUE fnGetConfigValue = (FNDLLPDLGETCONFIGVALUE)GetProcAddress(m_hPDLModule, "DllPDLGetConfigValue");
	if (fnGetConfigValue == NULL)
	{
		return FALSE;
	}

	return fnGetConfigValue(dwConfigID, lpReturnValue, dwSize);
}

BOOL CPDLHelper::SetConfigValue(DWORD dwConfigID, int nValue, LPCTSTR lpValue)
{
	MutexGuard autoLock(this);

	FNDLLPDLSETCONFIGVALUE fnSetConfigValue = (FNDLLPDLSETCONFIGVALUE)GetProcAddress(m_hPDLModule, "DllPDLSetConfigValue");
	if (fnSetConfigValue == NULL)
	{
		return FALSE;
	}

	return fnSetConfigValue(dwConfigID, nValue, lpValue);
}

BOOL CPDLHelper::_PDLCallBack(IGameTask *pIGameTask, LPVOID lpCaller, DWORD dwType, DWORD dwValue)
{
	CPDLHelper* lpHelper = (CPDLHelper*)lpCaller;
	if(lpHelper == NULL)
		return FALSE;

	//MutexGuard autoLock(lpHelper);

	CPDLDownloader* pDownloader = (CPDLDownloader*)pIGameTask->GetCustomData();
	if (pDownloader != NULL)
	{
		return pDownloader->OnPDLMessage(dwType, dwValue);
	}

	return FALSE;
}
