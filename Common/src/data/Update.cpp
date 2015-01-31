#include <shlobj.h>
#include <shlwapi.h>
#include "data/Update.h"
#include "data/DataPool.h"
#include "data/WebData.h"
#include "download/unzip.h"
#include "tinyxml/tinyxml.h"
#include "data/Utility.h"

CUpdate g_theUpdate ;

CUpdate::CUpdate(IDownloaderListener* pListener, IUpdateListener* pUptListener, int nDownloadType) 
	: m_nStartIndex(0), 
	  m_nCurIndex(0), 
	  m_nTotalSize(0), 
	  m_nUpdatedSize(0),
	  m_nUpdateMode(0),
	  m_pDownloadListener(pListener),
	  m_pUpdateListener(pUptListener),
	  m_nDownloadType(nDownloadType)
{
	m_sVerPath = theDataPool.GetBaseDir() + FILE_VER_CLIENT ;
	m_sDownPath = theDataPool.GetBaseDir() + _T("tmp\\") ;
	m_sSeedPath = m_sDownPath + FILE_VER_SEED ;	

	TCHAR szVersion[128] = {0};
	::GetPrivateProfileString(INI_CFG_CLIENT_UPDATE,INI_CFG_CLIENT_UPDATE_PARTNERVER,_T("2.6.0.1"),\
			szVersion,128,theDataPool.GetConfigPath().c_str());
	m_sPandoVer.assign(szVersion) ;

	//memset(&m_sParam, 0, sizeof(DOWNLOAD_PARAM)) ; will invoke default constructor automatically.
}

CUpdate::~CUpdate() {}

bool CUpdate::Init()
{
	m_bStop = false ;

	return Thread::start();
}

bool CUpdate::Uninit()
{
	m_bStop = true;
	Thread::stop(0) ;

	return true ;
}

bool CUpdate::Start()
{
	m_sDownloadMgr.Init();

	UPDATE_TASK sTask = {0} ;
	if (GetPrivateProfileInt(INI_CFG_UPDATE,INI_CFG_UPDATE_SILENTUPDATE,0,theDataPool.GetUserProfilePath().c_str()) == 1 
		&& INVALID_FILE_ATTRIBUTES != GetFileAttributes(m_sSeedPath.c_str()))
	{
		sTask.nType = SILENT_UPDATE ;
	}
	else
	{
		sTask.nType = CHECK_UPDATE ;
	}
	m_wTaskQueue.enter(sTask) ;

	return true ;
}

bool CUpdate::Stop()
{
	return m_sDownloadMgr.Uninit() ;
}

void CUpdate::HttpDownload()
{
	UPDATE_TASK sTask = {0} ;
	sTask.nType = HTTP_UPDATE ;
	m_wTaskQueue.enter(sTask) ;
}

bool CUpdate::DoUpdate()
{
	UPDATE_TASK sTask = {0} ;
	sTask.nType = EXTRACT_PACKAGES ;
	m_wTaskQueue.enter(sTask) ;

	return true ;
}

bool CUpdate::CancelUpdate()
{
	if (m_vecVerInfo.size() <= 0)
	{
		return true ;
	}

	_tstring sFilePath = m_sDownPath + m_vecVerInfo[m_nCurIndex]._szFile ;
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(sFilePath.c_str()))
	{
		if (!m_sDownloadMgr.Cancel(m_vecVerInfo[m_nCurIndex]._szVersion))
		{
			m_sDownloadMgr.Cancel(m_vecVerInfo[m_nCurIndex]._szVersion) ;
		}
	}

	DeleteDownloadedPacks() ;

	return true ;
}

bool CUpdate::DeleteDownloadedPacks()
{
	if (m_vecVerInfo.size() <= 0)
	{
		return false ;
	}

	for (int nIndex = m_nStartIndex; nIndex <= m_nCurIndex; ++ nIndex)
	{//delete all downloaded packages
		_tstring sFilePath = m_sDownPath + m_vecVerInfo[nIndex]._szFile ;
		DeleteFile(sFilePath.c_str()) ;
	}

	return true ;
}

bool CUpdate::OnStartBegin(_tstring sId)
{
	return m_pDownloadListener->OnStartBegin(sId) ;
}

bool CUpdate::OnStartEnd(_tstring sId)
{
	return m_pDownloadListener->OnStartEnd(sId) ;
}

bool CUpdate::OnPauseBegin(_tstring sId)
{
	return m_pDownloadListener->OnPauseBegin(sId) ;
}

bool CUpdate::OnPauseEnd(_tstring sId)
{
	return m_pDownloadListener->OnPauseEnd(sId) ;
}

bool CUpdate::OnResumeBegin(_tstring sId)
{
	return m_pDownloadListener->OnResumeBegin(sId) ;
}

bool CUpdate::OnResumeEnd(_tstring sId)
{
	return m_pDownloadListener->OnResumeEnd(sId) ;
}

bool CUpdate::OnCancelBegin(_tstring sId)
{
	return m_pDownloadListener->OnCancelBegin(sId) ;
}

bool CUpdate::OnCancelEnd(_tstring sId)
{
	return m_pDownloadListener->OnCancelEnd(sId) ;
}

bool CUpdate::OnComplete(_tstring sId)
{
	m_nUpdatedSize += m_vecVerInfo[m_nCurIndex]._uFileSize ;

	if ((m_vecVerInfo.size() - m_nCurIndex) <= 2)
	{
		m_pDownloadListener->OnComplete(sId) ;
	}
	else
	{
		++ m_nCurIndex ;
		UPDATE_TASK sTask = {0} ;
		sTask.nType = P2SP_UPDATE ;
		sTask.sInfo = m_vecVerInfo[m_nCurIndex] ;
		m_wTaskQueue.enter(sTask) ;
	}
	return true ;
}

bool CUpdate::UpdateStatus(_tstring sId, DOWNLOAD_PARAM *pDownloadParam)
{
	if(pDownloadParam->nStatus == IDownloader::COMPLETE)
	{
		return true ;
	}
	else if(pDownloadParam->downloadSize < 0)
	{
		return false ;
	}

	m_sParam.downloadSize = m_nUpdatedSize + pDownloadParam->downloadSize ;
	m_sParam.totleSize = m_nTotalSize ;
	m_sParam.nRate = pDownloadParam->nRate ;

	return m_pDownloadListener->UpdateStatus(sId, &m_sParam) ;
}

bool CUpdate::OnError(_tstring sId, int nErrType, _tstring sComment)
{
	return m_pDownloadListener->OnError(sId, nErrType, sComment) ;
}

long CUpdate::run()
{
	while (!m_bStop)
	{
		UPDATE_TASK sTask = m_wTaskQueue.leave();

		switch(sTask.nType)
		{
		case SILENT_UPDATE:
			{
				ParseVersionInfo();
				DoUnzip();

				m_nStartIndex = 0;
				m_nCurIndex = 0;
				m_nTotalSize = 0;
				::WritePrivateProfileString(INI_CFG_UPDATE,INI_CFG_UPDATE_SILENTUPDATE, _T("0"), theDataPool.GetUserProfilePath().c_str());

				UPDATE_TASK sTaskTmp = {0} ;
				sTaskTmp.nType = CHECK_UPDATE ;
				sTaskTmp.sInfo = m_vecVerInfo[m_nCurIndex] ;
				m_wTaskQueue.enter(sTaskTmp) ;
			}
			break;
		case CHECK_UPDATE:
			{
				BOOL bRet = CheckForUpdate() ;
				BOOL bUpdate = TRUE ;
				if (m_pUpdateListener)
				{
					bUpdate = m_pUpdateListener->OnCheckUpdate(bRet) ;
				}
				if (bRet && bUpdate)
				{
					if (m_nUpdateMode == CUpdate::SILENT)
					{
						::WritePrivateProfileString(INI_CFG_UPDATE,INI_CFG_UPDATE_SILENTUPDATE, _T("1"), theDataPool.GetUserProfilePath().c_str());
					}
					UPDATE_TASK sTaskTmp = {0} ;
					sTaskTmp.nType = P2SP_UPDATE ;
					sTaskTmp.sInfo = m_vecVerInfo[m_nCurIndex] ;
					m_wTaskQueue.enter(sTaskTmp) ;
				}
			}
			break ;
		case P2SP_UPDATE:
			{
				_tstring sDst = m_sDownPath + sTask.sInfo._szFile.c_str() ;

				DOWNLOAD_SETTING ds;
				ds.sId = sTask.sInfo._szVersion;
				ds.sUrl = sTask.sInfo._szPackageHttp;
				ds.sDst = sDst;
				ds.bCheckMD5 = false;
				ds.bAlwaysConnect = false;
				if(!m_sDownloadMgr.Start(m_nDownloadType, static_cast<IDownloaderListener*>(this), ds))
				{
					m_sDownloadMgr.Start(CDownloaderMgr::HTTP, static_cast<IDownloaderListener*>(this), ds);
				}
			}
			break ;
		case HTTP_UPDATE:
			{
				if (m_pUpdateListener)
				{
					m_pUpdateListener->OnStartHttp() ;
				}

				m_nCurIndex = m_nStartIndex = m_vecVerInfo.size() - 1;// download the latest package directly.
				
				_tstring szUrl = m_vecVerInfo[m_nCurIndex]._szPackageHttp ;
				_tstring szLocalPath = m_sDownPath + m_vecVerInfo[m_nCurIndex]._szFile ;
				bool bRes = m_sWebData.DownloadFile(szUrl.c_str(), szLocalPath.c_str(), true, 4);
			
				if (m_pUpdateListener)
				{
					m_pUpdateListener->OnEndHttp(bRes) ;
				}	
                			
			}
			break ;
		case EXTRACT_PACKAGES:
			{
				if (m_pUpdateListener)
				{
					if (!m_pUpdateListener->OnStartUnzip())
					{//error occurs outside, do not unzip
						break ;
					}
				}

				int nRes= DoUnzip();

				//launch CORE Client main executable.
				if (m_pUpdateListener)
				{
					m_pUpdateListener->OnEndUnzip(nRes) ;
				}
			}
			break ;
		}
	}

	return 0 ;
}

_tstring CUpdate::GetLocalVersion()
{
	TCHAR szVersion[32] = {0} ; 
	::GetPrivateProfileString(INI_CFG_VERSION_INFO, INI_CFG_VERSION_INFO_VERSION, _T("0000"), \
		szVersion, 32, m_sVerPath.c_str());
	m_sLocalVer.assign(szVersion);	

	return m_sLocalVer ;
}

__int64 CUpdate::GetTotalSize() 
{
	return m_nTotalSize ;
}

bool CUpdate::ResolveVersionInfo()
{
	TiXmlDocument xmlDoc;
	TiXmlElement  *pRoot   = NULL;	
	TiXmlElement  *pElement= NULL;	
	FILE          *fp      = NULL;
	bool          bResult  = false;

	//open version file
	fp=_tfopen(m_sSeedPath.c_str(),_T("rb"));
	if(!fp)
	{
		return 0;
	}

	do 
	{
		//parse version file into xml format
		bResult= xmlDoc.LoadFile(fp);
		if (!bResult)
		{
			break;
		}
		pRoot  = xmlDoc.FirstChildElement("root") ;
		if(!pRoot)
		{
			bResult = false;
			break;
		}

		//retrieve latest package info
		pElement=pRoot->FirstChildElement("latestpackage");
		if(!pElement)
		{
			bResult = false;
			break;
		}

		VERSION_INFO sLatestVerInfo = {0} ;
		sLatestVerInfo._szVersion = pElement->Attribute("id");
		sLatestVerInfo._szPackageURL = pElement->Attribute("url");
		sLatestVerInfo._szPackageHttp= pElement->Attribute("httpurl") ;
		sLatestVerInfo._szFile    = pElement->Attribute("name");
		sLatestVerInfo._szBuildVer= pElement->Attribute("build_version") ;
		sLatestVerInfo._szMD5     = pElement->Attribute("md5") ;
		sLatestVerInfo._uFileCount= atoi(pElement->Attribute("filecount"));
		sLatestVerInfo._uFileSize = atoi(pElement->Attribute("size")) ;

		//locate at the first update pacakge record in version file.
		pElement=pRoot->FirstChildElement("updatepackage");
		if(!pElement)
		{
			bResult = false;
			break;
		}

		//retrieve all of update pacakges info, and add info into list
		while(pElement)
		{
			VERSION_INFO sVerInfo = {0} ;

			sVerInfo._szVersion = pElement->Attribute("id");
			sVerInfo._szPackageURL = pElement->Attribute("url");
			sVerInfo._szPackageHttp= pElement->Attribute("httpurl") ;
			sVerInfo._szFile    = pElement->Attribute("name");
			sVerInfo._szBuildVer= pElement->Attribute("build_version") ;
			sVerInfo._szMD5     = pElement->Attribute("md5") ;
			sVerInfo._uFileCount= atoi(pElement->Attribute("filecount"));
			sVerInfo._uFileSize = atoi(pElement->Attribute("size")) ;

			m_vecVerInfo.push_back(sVerInfo) ;

			pElement=pElement->NextSiblingElement("updatepackage");
		}

		//add latest package info into end of list.
		m_vecVerInfo.push_back(sLatestVerInfo) ;

	} while (false);

	xmlDoc.Clear();
	fclose(fp);
	return bResult;
}

bool CUpdate::CheckForUpdate()
{
	//delete previous downloaded coreversion.xml
	if(INVALID_FILE_ATTRIBUTES!=GetFileAttributes(m_sSeedPath.c_str()))
	{
		DeleteFile(m_sSeedPath.c_str());
	}
	m_vecVerInfo.clear() ;

	_tstring sSeedUrl;
	int nRet = GetUpdateService(sSeedUrl);
	if (CUpdate::NOUPDATE == nRet)
	{
		return false;
	} 
	else if (CUpdate::SILENT == nRet)
	{
		if (m_pUpdateListener)
		{
			m_pUpdateListener->OnSilentUpdate();
		}
	}

	//download latest coreversion.xml from cdn
	if(m_sWebData.DownloadFile(sSeedUrl.c_str(), m_sSeedPath.c_str(), true, 4))
	{
		//parse coreversion.xml
		ParseVersionInfo();
		if (m_nUpdateMode != CUpdate::SILENT)
		{
			DeleteFile(m_sSeedPath.c_str());
		}

	}

	return m_nTotalSize > 0 ? true : false ;
}

bool CUpdate::ParseVersionInfo()
{
	bool bRet = ResolveVersionInfo();
	if (bRet)
	{
		if ((m_vecVerInfo.size() <= 0) || (0 <= GetLocalVersion().CompareNoCase(m_vecVerInfo[m_vecVerInfo.size()-1]._szVersion)))
		{
			return false ;
		}

		//get total size of packages needed to be updated
		if ((0 == GetLocalVersion().CompareNoCase(_T("0000"))) || GetLocalVersion().empty())
		{
			m_nStartIndex  = m_vecVerInfo.size() - 1 ;
			m_nTotalSize = m_vecVerInfo[m_vecVerInfo.size()-1]._uFileSize ;
		}
		else
		{
			for(int i = 0; i < m_vecVerInfo.size()-1; i++)
			{
				if(GetLocalVersion().CompareNoCase(m_vecVerInfo[i]._szVersion.c_str()) < 0)
				{
					m_nTotalSize += m_vecVerInfo[i]._uFileSize;
				}
				else
				{
					m_nStartIndex ++ ;
				}
			}

			if(m_nTotalSize >= m_vecVerInfo.back()._uFileSize)
			{
				m_nStartIndex = m_vecVerInfo.size() - 1;
				m_nTotalSize = m_vecVerInfo.back()._uFileSize;
			}
		}

		m_nCurIndex = m_nStartIndex ;
	}
	return true;
}

int CUpdate::DoUnzip()
{

	//Check file
	if (m_nStartIndex == m_vecVerInfo.size() - 1)
	{//Last item, full size update
		_tstring szSrcPath = m_sDownPath + m_vecVerInfo[m_nStartIndex]._szFile ;
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szSrcPath.c_str()))
		{
			return UNZIP_FILENOTEXIST;
		}
	}
	else
	{
		for (int i=m_nStartIndex; i<m_vecVerInfo.size()-1; i++)
		{
			_tstring szSrcPath = m_sDownPath + m_vecVerInfo[i]._szFile ;
			if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szSrcPath.c_str()))
			{
				return UNZIP_FILENOTEXIST;
			}
		}
	}

	//unpackage all of downloaded .zips and overwrite unpackage files to root directory of CORE Client.
	ZRESULT hZres = ZR_FAILED ;
	int nIndex = m_nStartIndex;
	do{
		_tstring szSrcPath = m_sDownPath + m_vecVerInfo[nIndex]._szFile ;
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szSrcPath.c_str()))
		{
			return UNZIP_FILENOTEXIST;
		}

		//open .zip file
		HZIP hZip = OpenZip(szSrcPath.c_str(),0);
		SetUnzipBaseDir(hZip,theDataPool.GetBaseDir().c_str());

		//unpackage zip
		ZIPENTRY  zipEntry; 
		UINT      numOfIndex = 0 ;
		GetZipItem(hZip,-1,&zipEntry); 
		numOfIndex=zipEntry.index;
		for (int j=0; j<numOfIndex; ++j)
		{ 
			GetZipItem(hZip,j,&zipEntry);
			_tstring sSrcFile = theDataPool.GetBaseDir() + zipEntry.name ;
			if (!CUtility::DeleteFileEx(sSrcFile.c_str(), NULL))
			{
				hZres = ZR_FAILED ;
				break ;
			}
			hZres = UnzipItem(hZip,j,zipEntry.name);		
			if ((hZres != ZR_OK) && (hZres != ZR_FLATE))
			{
				break ;
			}
		}

		//close .zip file and delete it.
		CloseZip(hZip);
		DeleteFile(szSrcPath.c_str());

		if ((hZres != ZR_OK) && (hZres != ZR_FLATE))
		{//unzipping a file with 0 bytes will get ZR_FALTE
			break ;
		}

		++ nIndex ;
	}while (nIndex < (m_vecVerInfo.size() - 1)) ;

	if ((nIndex >= (m_vecVerInfo.size() - 1)) && ((ZR_OK == hZres) || (ZR_FLATE == hZres)))
	{//unzip successful
		::WritePrivateProfileString(INI_CFG_VERSION_INFO,INI_CFG_VERSION_INFO_VERSION, \
			m_vecVerInfo[m_vecVerInfo.size()-1]._szVersion.c_str( ), m_sVerPath.c_str());

		return UNZIP_OK;
	}
	else if (ZR_WRITE == hZres)
	{//failed to write file, probably not enough space to extract
		return UNZIP_NOTENOUGHSPACE;
	}
	else
	{
		return UNZIP_INTERNAL;
	}
}

int CUpdate::GetUpdateService(_tstring& sSeedUrl)
{
	TCHAR szVersionUrl[128] = {0};
	::GetPrivateProfileString(INI_CFG_VERSION_INFO,INI_CFG_VERSION_INFO_SEEDURL,_T(""),szVersionUrl,128, m_sVerPath.c_str());
	sSeedUrl  = szVersionUrl ;

	TCHAR szVersion[128] = {0};
	::GetPrivateProfileString(INI_CFG_VERSION_INFO,INI_CFG_VERSION_INFO_VERSION,_T("0000"), szVersion, 128, m_sVerPath.c_str());

	_tstring sLocalVer = szVersion;
	_tstring sRemoteVer, sUrl;
	m_sWebData.GetVersionInfoService(theDataPool.GetLangAbbrString(), sLocalVer, sRemoteVer, sUrl, m_nUpdateMode);
	if (sSeedUrl.empty())
	{
		if (sUrl.empty())
		{
			sSeedUrl = URL_DEFAULT_SEEDURL;
			m_nUpdateMode = CUpdate::MANDATORY;
		}
		else
		{
			sSeedUrl = sUrl;
		}
	}

	if (sLocalVer == _T("0000"))
	{
		m_nUpdateMode = CUpdate::MANDATORY;
	}

	return m_nUpdateMode;
}

int CUpdate::GetUpdateMode()
{
	return m_nUpdateMode;
}