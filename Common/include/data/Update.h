#pragma once
#include <tchar.h>
#include <vector>
#include "tstring/tstring.h"
#include "thread/thread.h"
#include "thread/WQueue.h"
#include "data/WebData.h"
#include "download/IDownloader.h"

using namespace String ;
using namespace std ;

class IUpdateListener
{
public:
	virtual BOOL OnCheckUpdate(BOOL bUpdate) {return true;} ;
	virtual void OnStartHttp() {return ;} ;
	virtual void OnEndHttp(BOOL bRes) {return ;} ;
	virtual BOOL OnStartUnzip() {return true;} ;
	virtual void OnEndUnzip(int nRes) {return ;} ;
	virtual void OnSilentUpdate() {return;};
};

class CUpdate : public IDownloaderListener, public Thread
{
public:

	enum{
		CHECK_UPDATE = 0,
		P2SP_UPDATE,
		HTTP_UPDATE,
		EXTRACT_PACKAGES,
		SILENT_UPDATE,
		//USER_CANCEL,
		MAX
	};

	enum UPDATE_MODE
	{
		NOUPDATE = 0,
		MANDATORY,
		SILENT,
		OPT
	};

	enum UNZIP_RETURN_VALUE
	{
		UNZIP_OK = 0,
		UNZIP_FILENOTEXIST,
		UNZIP_NOTENOUGHSPACE,
		UNZIP_INTERNAL
	};

	typedef struct _VERSION_INFO
	{
		_tstring  _szVersion ;  //version id of the package, unique identify the package
		_tstring  _szFile ;     //name of the package
		_tstring  _szPackageURL;//pando url address of the package
		_tstring  _szPackageHttp ;//http url address of the package
		_tstringA _szMD5  ;    //hash of the package, reserve for future use
		_tstring  _szBuildVer; //build version of the package, reserve for future use
		UINT      _uFileSize ; //size of the package, in KB
		UINT      _uFileCount; //file count in the package, reserve for future use

	}VERSION_INFO,*PVERSION_INFO;

	typedef struct _UPDATE_TASK{

		int nType ;
		VERSION_INFO sInfo ;

	}UPDATE_TASK, *PUPDATE_TASK;

	CUpdate(IDownloaderListener* pListener = NULL, \
		IUpdateListener* pUptListener = NULL, \
		int nDownloadType = CDownloaderMgr::HTTP2) ;
    ~CUpdate() ;

	bool Init() ;
	bool Uninit() ;

	bool Start() ;
	bool Stop() ;

 __int64 GetTotalSize() ;
	bool CancelUpdate() ;
	void HttpDownload() ;
	bool DoUpdate() ;
	bool CheckForUpdate() ;
	bool DeleteDownloadedPacks() ;
	int GetUpdateService(_tstring& sSeedUrl);
	int GetUpdateMode();

	// IDownloaderListener interface
	bool OnStartBegin(_tstring);
	bool OnStartEnd(_tstring);
	bool OnPauseBegin(_tstring);
	bool OnPauseEnd(_tstring);
	bool OnResumeBegin(_tstring);
	bool OnResumeEnd(_tstring);
	bool OnCancelBegin(_tstring);
	bool OnCancelEnd(_tstring);
	bool OnComplete(_tstring);
	bool UpdateStatus(_tstring, DOWNLOAD_PARAM*);
	bool OnError(_tstring, int, _tstring);

protected:
	virtual long run() ;

	bool ResolveVersionInfo() ;
	bool ParseVersionInfo();
	_tstring GetLocalVersion() ;
	int DoUnzip();

private:

	vector<VERSION_INFO> m_vecVerInfo ; //vector to save all of records in coreversion.xml
	int m_nStartIndex ; //start index of download package in m_vecVerInfo
	int m_nCurIndex ;   //index of current download pacakge in m_vecVerInfo
__int64 m_nTotalSize ;  //total size of the update pacakges in bytes
__int64 m_nUpdatedSize ;//downloaded size of the update pacakges in bytes

	//_tstring m_sSeedUrl; //remote address of coreversion.xml
	_tstring m_sSeedPath;//local path of downloaded coreversion.xml
	_tstring m_sVerPath ; //local path of version.ini
	_tstring m_sLocalVer ; //current version of coreclient
	_tstring m_sDownPath ; //download path of the packages
	_tstring m_sPandoVer ; //version info of Pando service
	int m_nUpdateMode;		// 1- Mandatory; 2 - Silent; 3 - Optional

	CDownloaderMgr m_sDownloadMgr ; //download manager pointer
	int m_nDownloadType ; //download service type
	IDownloaderListener* m_pDownloadListener ; //downloader listener
	IUpdateListener* m_pUpdateListener ;
	CWebData m_sWebData ;
	DOWNLOAD_PARAM m_sParam ;
	WQueue<UPDATE_TASK> m_wTaskQueue ;

	bool m_bStop ;
} ;

extern CUpdate g_theUpdate ;