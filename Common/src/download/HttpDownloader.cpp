#include "StdAfx.h"
#include "download/HttpDownloader.h"
#include "log/local.h"
#include <sys/stat.h>

//---------------------------------------------------------

CHttpDownloader::CHttpDownloader()
: m_curl(NULL)
, m_dwTick(0)
, m_StartSize(0)
, m_bAdd(false)
, m_fDownSize(0.0)
, m_nNetworkRetryCount(0)
, m_nPartFileRetryCount(0)
{

}

CHttpDownloader::~CHttpDownloader()
{
	if(m_curl)
	{
		curl_easy_cleanup(m_curl);
		m_curl = NULL;
	}
}

bool CHttpDownloader::Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds)
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

	m_sParam.sUrl = ds.sUrl;
	m_sParam.sId = ds.sId;
	m_sParam.sDst = ds.sDst;
	m_sParam.sFileName = ds.sDst.Right(ds.sDst.length() - nPos - 1);
	m_sParam.totleSize = 0;
	m_sParam.downloadSize = 0;
	m_sParam.nDownloadPercent = 0;
	m_sParam.nStatus = IDownloader::INIT;
	m_sParam.nErrCode = IDownloader::NOERR;
	m_sParam.nOperation = IDownloader::NOOPERATION;

	return StartDownload();
}

bool CHttpDownloader::Uninit()
{
	m_bExit = true;

	while(MsgWaitForMultipleObjects(1, &_threadHandle, 
		FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
	{
		MSG msg;
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	}

	CloseHandle(_threadHandle);
	_threadHandle = NULL;
	_threadId = 0;

	return true;
}

bool CHttpDownloader::StartDownload()
{
	m_sParam.nOperation = IDownloader::START;

	if(_threadHandle != NULL)
	{
		CloseHandle(_threadHandle);
		_threadId = 0;
	}

	return start();
}

bool CHttpDownloader::Pause()
{
	m_pListener->OnPauseBegin(m_sParam.sId);

	if(m_curl)
	{
		if(curl_easy_pause(m_curl, CURLPAUSE_ALL) == CURLE_OK)
		{
			m_sParam.nStatus = IDownloader::PAUSED;
			m_pListener->OnPauseEnd(m_sParam.sId);
			return true;
		}
	}

	m_pListener->OnError(m_sParam.sId, IDownloader::ERRPAUSE);

	return false;
}

bool CHttpDownloader::Resume()
{
	m_pListener->OnResumeBegin(m_sParam.sId);

	if(m_curl)
	{
		if(curl_easy_pause(m_curl, CURLPAUSE_CONT) == CURLE_OK)
		{
			m_sParam.nStatus = IDownloader::DOWNLOADING;
			m_pListener->OnResumeEnd(m_sParam.sId);
			return true;
		}
	}
	else
	{
		return StartDownload();
	}

	m_pListener->OnError(m_sParam.sId, IDownloader::ERRRESUME);

	return false;
}

bool CHttpDownloader::Cancel()
{
	m_sParam.nOperation = IDownloader::CANCEL;
	if (!m_curl)
	{
		DeleteFile(m_sParam.sDst.c_str());
		m_pListener->OnCancelEnd(m_sParam.sId);
	}
	return true;
}

DOWNLOAD_PARAM* CHttpDownloader::GetAttribute()
{
	return &m_sParam;
}

long CHttpDownloader::run()
{
	CURLcode res;
	long err = -1;
	if(m_sParam.nOperation == IDownloader::START)
	{
		m_pListener->OnStartBegin(m_sParam.sId);

start:
		if(m_curl)
		{
			curl_easy_cleanup(m_curl);
			m_curl = NULL;
		}

		bool bStart = false;
		FILE *pf = NULL;
		do 
		{
			m_curl = curl_easy_init();
			if(!m_curl)
			{
				break;
			}

			curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
			if(curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 20) != CURLE_OK)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_URL, m_sParam.sUrl.toNarrowString().c_str()) != CURLE_OK)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &CHttpDownloader::Callback_WriteFunc)  != CURLE_OK)
				break;

			if(m_bAdd)
			{
				pf = fopen(m_sParam.sDst.toNarrowString().c_str(), "ab");

				struct _stat64 buf = {0};
				_stat64(m_sParam.sDst.toUTF8().c_str(), &buf);
				m_StartSize = buf.st_size;
				if (curl_easy_setopt(m_curl, CURLOPT_RESUME_FROM_LARGE, m_StartSize))
				{
					break;
				}
			}
			else
			{
				pf = fopen(m_sParam.sDst.toNarrowString().c_str(), "wb");
			}
			if(!pf)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, pf) != CURLE_OK)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &CHttpDownloader::Callback_Progress) != CURLE_OK)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this) != CURLE_OK)
				break;

			if(curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0) != CURLE_OK)
			{
				break;
			}

			bStart = true;
		} while (false);

		if (!bStart)
		{
			if(pf != NULL) 
			{
				fclose(pf);
				pf = NULL;
			}
			m_pListener->OnError(m_sParam.sId, IDownloader::ERRSTART);
			return err;
		}

		m_pListener->OnStartEnd(m_sParam.sId);
		m_sParam.nStatus = IDownloader::DOWNLOADING;

		res = curl_easy_perform(m_curl);
		fclose(pf);
		pf = NULL;
		curl_easy_cleanup(m_curl);
		m_curl = NULL;

		// track error
		if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)
		{
			m_pListener->OnError(m_sParam.sId, 80+res);
		}

		if(res == CURLE_OK)
		{
			//reset retry count
			if(m_nNetworkRetryCount)
			{
				m_pListener->OnError(m_sParam.sId, 75);
				m_nNetworkRetryCount = 0;
			}
			if(m_nPartFileRetryCount)
			{
				m_pListener->OnError(m_sParam.sId, 75);
				m_nPartFileRetryCount = 0;
			}

			//complete
			HANDLE hFile = CreateFile(m_sParam.sDst.c_str(), 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
			if (hFile != NULL)
			{
				LARGE_INTEGER nLocalSize = {0};
				DWORD dwLocalSize  = GetFileSizeEx(hFile, &nLocalSize) ;
				CloseHandle(hFile) ;
				m_sParam.nStatus = IDownloader::COMPLETE;
				m_pListener->OnComplete(m_sParam.sId);

				if(nLocalSize.QuadPart != m_sParam.totleSize)
				{
					Log(_T("Local size: %du, Downloaded size: %I64u"), dwLocalSize, m_sParam.totleSize);
					//return err;
				}
			}
		}
		else if(res == CURLE_ABORTED_BY_CALLBACK)
		{
			//exit or canceled
			if(m_sParam.nOperation == IDownloader::CANCEL)
			{
				DeleteFile(m_sParam.sDst.c_str());
				m_pListener->OnCancelEnd(m_sParam.sId);
			}
			else
			{
				////error
				//Log(_T("http download failed, code: %d, file: %s"), res, m_sParam.sDst.c_str());
				//m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				//return err;
			}
		}
		else if(res == CURLE_RECV_ERROR || res == CURLE_COULDNT_RESOLVE_HOST)
		{
			m_nNetworkRetryCount++;
			if(m_nNetworkRetryCount < 3)
			{
				Sleep(5000);
				goto start;
			}
			else
			{
				m_pListener->OnError(m_sParam.sId, 76);	//track, failed even after retry 3 times

				//error
				Log(_T("http download failed, code: %d, file: %s"), res, m_sParam.sDst.c_str());
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				return err;
			}
		}
		else if (res == CURLE_PARTIAL_FILE)
		{
			m_nPartFileRetryCount++;
			if (m_nPartFileRetryCount < 3)		//Retry 3 times
			{
				Sleep(5000);
				goto start;
			}
			else
			{
				m_pListener->OnError(m_sParam.sId, 76);	//track, failed even after retry 3 times

				//error
				Log(_T("http download failed, code: %d, file: %s"), res, m_sParam.sDst.c_str());
				m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
				return err;
			}
		}
		else
		{
			//error
			Log(_T("http download failed, code: %d, file: %s"), res, m_sParam.sDst.c_str());
			m_pListener->OnError(m_sParam.sId, IDownloader::ERRINTERNAL);
			return err;
		}
	}

	return 0;
}

size_t CHttpDownloader::Callback_WriteFunc(void *pBuffer, size_t nSize, size_t nMemb, VOID* pParam)
{
	FILE *pf = (FILE *)pParam;
	return fwrite(pBuffer, nSize, nMemb, pf);
}

int CHttpDownloader::Callback_Progress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CHttpDownloader *pdownloader = (CHttpDownloader*)clientp;
	if(pdownloader->m_bExit || pdownloader->m_sParam.nOperation == IDownloader::CANCEL)
	{
		return CURLE_ABORTED_BY_CALLBACK;
	}

	//update about once a second
	DWORD tick = GetTickCount();
	DWORD span = tick - pdownloader->m_dwTick;
	if(span < 1000)
	{
		return 0;
	}
	pdownloader->m_dwTick = tick;
	double fSize = dlnow - pdownloader->m_fDownSize;
	pdownloader->m_fDownSize = dlnow;
	pdownloader->m_sParam.nRate = (int)(fSize / span * 1000);

	pdownloader->m_sParam.downloadSize = (long long)dlnow + pdownloader->m_StartSize;
	pdownloader->m_sParam.totleSize = (long long)dltotal + pdownloader->m_StartSize;
	if(dltotal > 0.1)
	{
		pdownloader->m_sParam.nDownloadPercent = int(pdownloader->m_sParam.downloadSize * 100 / pdownloader->m_sParam.totleSize);

		//check disk space
		ULARGE_INTEGER ulFreeBytes = {0} ; 
		TCHAR szDrive[4] = _T("C:\\") ;
		szDrive[0] = pdownloader->m_sParam.sDst[0] ;
		if (GetDiskFreeSpaceEx(szDrive, &ulFreeBytes , NULL, NULL))
		{
			if ( ulFreeBytes.QuadPart <= pdownloader->m_sParam.totleSize - pdownloader->m_sParam.downloadSize)
			{//no enough disk space
				if(pdownloader->m_sParam.nStatus == IDownloader::DOWNLOADING)
				{
					pdownloader->m_pListener->OnError(pdownloader->m_sParam.sId, IDownloader::NOTENOUGHSPACE);
				}
				if(curl_easy_pause(pdownloader->m_curl, CURLPAUSE_ALL) == CURLE_OK)
				{
					pdownloader->m_sParam.nStatus = IDownloader::PAUSED;
				}
			}
		}
	}
	else
	{
		pdownloader->m_sParam.nDownloadPercent = 0;
	}

	if(pdownloader->m_sParam.nRate > 0)
	{
		//reset retry count
		if(pdownloader->m_nNetworkRetryCount)
		{
			pdownloader->m_nNetworkRetryCount = 0;
			pdownloader->m_pListener->OnError(pdownloader->m_sParam.sId, 75);
		}
		if(pdownloader->m_nPartFileRetryCount)
		{
			pdownloader->m_nPartFileRetryCount = 0;
			pdownloader->m_pListener->OnError(pdownloader->m_sParam.sId, 75);
		}
	}

	pdownloader->m_pListener->UpdateStatus(pdownloader->m_sParam.sId, &pdownloader->m_sParam);
	return 0;
}