#pragma once

#include "pdldownloader/PDL.h"
#include "IDownLoader.h"
#include <map>


class CPDLDownloader : public IDownloader
{
public:
	CPDLDownloader(void);
	~CPDLDownloader(void);
	
	virtual bool Init(IDownloaderListener *pListener, CDownloaderMgr *pDownloaderMgr, CRITICAL_SECTION *pMutex, DOWNLOAD_SETTING ds);
	virtual bool Uninit();
	virtual bool StartDownload();
	virtual bool Pause();
	virtual bool Resume();
	virtual bool Cancel();
	virtual DOWNLOAD_PARAM* GetAttribute();
	virtual long run();

	virtual bool PlayPDL();

	BOOL OnPDLMessage(DWORD dwType, DWORD dwValue);
private:
	void OnOpStart();
	void OnOpPause();
	void OnOpResume();
	void OnOpCancle();

	BOOL OtherHandler(DWORD dwType, DWORD dwValue);
	BOOL OnError(IDownloader::DOWNLOAD_ERROR nErr, LPCTSTR szMessage);
	Mutex		m_lock;
	IGameTask*	m_pGameTask;
	bool		m_bAlwaysConnect;
	bool		m_bPlayStartGame;
	DOWNLOAD_OPERATION m_preOperator;
	HANDLE		m_hOpEvent;
	bool		m_bSendError;
	DWORD       m_dwLastUpdateTime;
	bool		m_hasSendLaunchPoint;
	volatile LONG m_speedTotal;
	volatile LONG m_speedCount;
	/*volatile LONG m_retryDownloadJson;*/
	volatile LONG m_retryDownloadInitPackage;
	volatile LONG m_retryDownloadMiniPackage;
	volatile LONG m_retryActiveDownloadPackage;
	volatile LONG m_retryPassiveDownloadPackage;
	volatile LONG m_retryPassiveDownloadDiskError;
};


class CPDLHelper : public Mutex
{
public:
	CPDLHelper();
	~CPDLHelper();

	bool InitializePDL(DWORD& pdwErrorCode);
	bool UninitlizePDL();

	IGameTask*	CreateTask(LPCTSTR lpszJsonFilename, LPCTSTR lpszJsonURL, LPCTSTR lpszGameDir, CPDLDownloader* pDownloader);
	BOOL		DestroyTask(IGameTask* pTask);

	void		StartTask(IGameTask* pTask);
	void		StopTask(IGameTask* pTask);
	void		CleanTask(IGameTask* pTask, BOOL bIncludeGameFiles=FALSE);

	int			GetConfigValue(DWORD dwConfigID, LPTSTR lpReturnValue, DWORD dwSize);
	BOOL		SetConfigValue(DWORD dwConfigID, int nValue, LPCTSTR lpValue);

private:
	static BOOL _PDLCallBack(IGameTask *pIGameTask, LPVOID lpCaller, DWORD dwType, DWORD dwValue);
private:
	HMODULE			m_hPDLModule;
};

extern CPDLHelper thePDLHelper;